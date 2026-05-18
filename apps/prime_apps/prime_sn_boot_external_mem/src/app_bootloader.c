/*******************************************************************************
  PRIME Service Node Bootloader Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app_bootloader.c

  Summary:
    Bare-metal bootloader for the SAMD20J18. Dispatches firmware-upgrade
    operations stored in the external SST26 serial flash and jumps to
    the application at APP_BOOTLOADER_APP_START.

  Description:
    The bootloader runs first after every reset. It reads the boot
    configuration persisted in the emulated-EEPROM row and, depending on
    its content, performs one of three things:

      - APPLY TELECARGA: back up the current application to the SST26
        REVERT zone, then install the new image sitting in the SST26
        TELECARGA zone into internal flash.

      - APPLY REVERT: reinstall the previous application from the SST26
        REVERT zone into internal flash.

      - JUMP: no operation pending (boot config key does not match);
        branch directly to the application.

    There is no image header and no in-bootloader CRC check. The
    PRIME firmware-upgrade service already validates the image CRC
    while it downloads; the boot-config handshake (cfgKey +
    origAddr + imgSize) is the sole source of truth for the bootloader.

    Power-loss safety relies on one invariant: between backup and
    install of a TELECARGA, the bootloader persists a BACKUP_DONE
    marker in the boot config. If power fails during the install
    phase, the next boot picks up where the previous one left off
    instead of re-backing-up the corrupted flash over the valid
    REVERT image.

    See BOOTLOADER_FROM_RAM_DESIGN.md §7 for the full flow diagram.
*******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "app_bootloader.h"
#include "device.h"
#include "drv_spi.h"
#include "drv_sst26.h"
#include "drv_nvmctrl.h"
#include "drv_eeprom.h"

// *****************************************************************************
// *****************************************************************************
// Section: Local Constants
// *****************************************************************************
// *****************************************************************************

/* Onboard LED0 on PA14, active-low (pin LOW = LED ON). Matches the
 * BSP convention used by the application on the same board. */
#define APP_BOOTLOADER_LED_PORT_GRP         0U
#define APP_BOOTLOADER_LED_PIN              14U
#define APP_BOOTLOADER_LED_MASK             (1UL << APP_BOOTLOADER_LED_PIN)

/* Toggle divisors tuned so the two memory-movement phases blink at
 * roughly 5-7 Hz on the scope, fast enough to be obviously different
 * from the application's slower blink pattern. */
#define APP_BOOTLOADER_LED_INSTALL_DIV      4U      /* ~7 Hz  (every 4 rows)       */
#define APP_BOOTLOADER_LED_BACKUP_DIV       32U     /* ~5 Hz  (every 32 SST26 pages) */

/* Panic blink cadence: faster than normal bootloader activity so a
 * stuck node is visually distinct from one that is merely slow. */
#define APP_BOOTLOADER_PANIC_DELAY_MS       50U     /* 10 Hz */

// *****************************************************************************
// *****************************************************************************
// Section: Local Functions — Forward Declarations
// *****************************************************************************
// *****************************************************************************

static APP_BOOTLOADER_BOOT_CONFIG lAPP_BOOTLOADER_ReadBootConfig(void);
static void        lAPP_BOOTLOADER_DisableWdt(void);
static void        lAPP_BOOTLOADER_LedInit(void);
static void        lAPP_BOOTLOADER_LedOff(void);
static inline void lAPP_BOOTLOADER_LedToggle(void);
static void        lAPP_BOOTLOADER_DelayMs(uint32_t ms);
static void        lAPP_BOOTLOADER_BackupToRevert(void);
static void        lAPP_BOOTLOADER_InstallFromZone(uint32_t zoneOffset,
                                                   uint32_t payloadSize);
static void        lAPP_BOOTLOADER_Panic(void) __attribute__((noreturn));

// *****************************************************************************
// *****************************************************************************
// Section: Bootloader Main and Jump
// *****************************************************************************
// *****************************************************************************

void APP_BOOTLOADER_Main(void)
{
    APP_BOOTLOADER_BOOT_CONFIG bootCfg;

    /* The application's fuse may leave the WDT armed after reset. Long
     * SST26/flash operations in this bootloader comfortably exceed the
     * default WDT period, so disable it up front; the application will
     * re-enable it during its own init. */
    lAPP_BOOTLOADER_DisableWdt();

    /* Bring PA14 up as a GPIO output so the LED can be toggled during
     * long memory operations. This gives a visible "bootloader active"
     * signal that blinks faster than the application's idle pattern. */
    lAPP_BOOTLOADER_LedInit();

    /* Bring up the peripherals the bootloader needs to talk to the SST26
     * and to the internal flash controller. */
    DRV_SPI_Initialize();
    DRV_SST26_Initialize();
    DRV_NVMCTRL_Initialize();

    bootCfg = lAPP_BOOTLOADER_ReadBootConfig();

    /* No pending operation: jump to the application unchanged. */
    if (bootCfg.cfgKey != APP_BOOTLOADER_BOOT_CONFIG_KEY)
    {
        lAPP_BOOTLOADER_LedOff();
        APP_BOOTLOADER_JumpToApp();
    }

    /* Sanity-check the request before acting on it. destAddr must point
     * at the application region start; imgSize must fit. A malformed
     * request is treated as a permanent error so the board doesn't
     * brick itself on garbage. */
    if (bootCfg.destAddr != APP_BOOTLOADER_APP_START)
    {
        lAPP_BOOTLOADER_Panic();
    }

    if (bootCfg.origAddr == APP_BOOTLOADER_SST26_TELECARGA_OFFSET)
    {
        /* APPLY TELECARGA. Two-phase: backup to REVERT, then install. */
        if ((bootCfg.imgSize == 0U) ||
            (bootCfg.imgSize > APP_BOOTLOADER_MAX_APP_SIZE))
        {
            lAPP_BOOTLOADER_Panic();
        }

        if (bootCfg.bootState != APP_BOOTLOADER_BOOT_BACKUP_DONE)
        {
            lAPP_BOOTLOADER_BackupToRevert();

            /* Persist the BACKUP_DONE marker so a power loss during the
             * upcoming install does not re-enter the backup phase and
             * overwrite the freshly-written REVERT with a corrupted
             * internal flash. */
            bootCfg.bootState = APP_BOOTLOADER_BOOT_BACKUP_DONE;
            DRV_EEPROM_WriteBootConfig(&bootCfg);
        }

        lAPP_BOOTLOADER_InstallFromZone(APP_BOOTLOADER_SST26_TELECARGA_OFFSET,
                                        bootCfg.imgSize);

        /* Install complete: clear the request so next reset jumps
         * straight to the application. */
        bootCfg.cfgKey    = 0U;
        bootCfg.bootState = (uint8_t) APP_BOOTLOADER_BOOT_IDLE;
        DRV_EEPROM_WriteBootConfig(&bootCfg);
    }
    else if (bootCfg.origAddr == APP_BOOTLOADER_SST26_REVERT_OFFSET)
    {
        /* APPLY REVERT: single-phase. The backup was written by a
         * previous TELECARGA, so no backup step is needed here. The
         * bootloader always backs up exactly MAX_APP_SIZE bytes, so
         * the size of the install is a compile-time constant and
         * bootCfg.imgSize is ignored. */
        lAPP_BOOTLOADER_InstallFromZone(APP_BOOTLOADER_SST26_REVERT_OFFSET,
                                        APP_BOOTLOADER_MAX_APP_SIZE);

        bootCfg.cfgKey    = 0U;
        bootCfg.bootState = (uint8_t) APP_BOOTLOADER_BOOT_IDLE;
        DRV_EEPROM_WriteBootConfig(&bootCfg);
    }
    else
    {
        /* Unknown operation requested. */
        lAPP_BOOTLOADER_Panic();
    }

    lAPP_BOOTLOADER_LedOff();
    APP_BOOTLOADER_JumpToApp();
}

void APP_BOOTLOADER_JumpToApp(void)
{
    const uint32_t *appVectors;
    uint32_t appSp;
    uint32_t appPc;

    /* Hand SERCOM1 back to the application in reset state. The Harmony
     * plib called by the modem writes CTRLB without an SWRST first,
     * and SAMD20 silently rejects CTRLB writes when ENABLE=1, which
     * causes its subsequent SYNCBUSY poll to spin forever. */
    DRV_SPI_Deinitialize();

    appVectors = (const uint32_t *) APP_BOOTLOADER_APP_START;
    appSp      = appVectors[0];
    appPc      = appVectors[1];

    /* Relocate the vector table so the application's interrupt vectors
     * resolve inside its own address range. */
    SCB->VTOR = APP_BOOTLOADER_APP_START;

    /* Load the application's stack pointer and branch to its reset
     * handler. Bit 0 of appPc already selects Thumb mode. */
    __asm volatile (
        "msr msp, %0 \n"
        "bx  %1      \n"
        : : "r" (appSp), "r" (appPc)
    );

    __builtin_unreachable();
}

// *****************************************************************************
// *****************************************************************************
// Section: Local Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    static APP_BOOTLOADER_BOOT_CONFIG lAPP_BOOTLOADER_ReadBootConfig ( void )

  Summary:
    Reads the 24-byte boot configuration from the emulated-EEPROM row.
*/

static APP_BOOTLOADER_BOOT_CONFIG lAPP_BOOTLOADER_ReadBootConfig(void)
{
    APP_BOOTLOADER_BOOT_CONFIG cfg;
    const uint8_t *pSrc;
    uint8_t       *pDst;
    uint32_t       i;

    pSrc = (const uint8_t *) (APP_BOOTLOADER_EEPROM_ROW_ADDR
                              + APP_BOOTLOADER_BOOT_CONFIG_OFFSET);
    pDst = (uint8_t *) &cfg;

    for (i = 0U; i < sizeof(APP_BOOTLOADER_BOOT_CONFIG); i++)
    {
        pDst[i] = pSrc[i];
    }

    return cfg;
}

/*******************************************************************************
  Function:
    static void lAPP_BOOTLOADER_DisableWdt ( void )

  Summary:
    Turns the SAMD20 watchdog off for the duration of the bootloader.

  Description:
    With WDT_ALWAYSON = DISABLED the ENABLE bit in WDT_CTRL can be
    cleared at runtime. SYNCBUSY polling completes quickly if the WDT
    was enabled; if never enabled, the write is a no-op.
*/

static void lAPP_BOOTLOADER_DisableWdt(void)
{
    WDT_REGS->WDT_CTRL = 0U;

    while ((WDT_REGS->WDT_STATUS & WDT_STATUS_SYNCBUSY_Msk) != 0U)
    {
        /* spin */
    }
}

/*******************************************************************************
  Function:
    static void lAPP_BOOTLOADER_BackupToRevert ( void )

  Summary:
    Copies the current application image to the SST26 REVERT zone.

  Description:
    Erases the 256 KB REVERT zone (four 64 KB block erases) and then
    streams APP_BOOTLOADER_MAX_APP_SIZE bytes from internal flash into
    it in SST26 page (256 B) chunks. Payload lives at offset 0 of the
    zone — there is no header, no CRC. The bootloader is the only
    writer of this zone, and the SST26's page-program instruction
    silently keeps previously-erased bytes at 0xFF, so a partial write
    on power loss cleanly yields all-0xFF blocks at the end of the
    zone. A power loss during the install phase is handled via the
    BACKUP_DONE marker in the boot config.
*/

static void lAPP_BOOTLOADER_BackupToRevert(void)
{
    const uint8_t *pFlash;
    uint32_t       remaining;
    uint32_t       sst26Addr;
    uint32_t       chunk;
    uint32_t       blockAddr;
    uint32_t       i;

    /* 1. Erase the REVERT zone. 256 KB = four 64 KB blocks. */
    blockAddr = APP_BOOTLOADER_SST26_REVERT_OFFSET;
    for (i = 0U; i < (APP_BOOTLOADER_IMAGE_ZONE_SIZE
                      / APP_BOOTLOADER_SST26_BLOCK_64K_SIZE); i++)
    {
        DRV_SST26_BlockErase64K(blockAddr);
        blockAddr += APP_BOOTLOADER_SST26_BLOCK_64K_SIZE;
    }

    /* 2. Stream the current application from internal flash into the
     *    REVERT zone, 256 bytes at a time. */
    remaining = APP_BOOTLOADER_MAX_APP_SIZE;
    pFlash    = (const uint8_t *) APP_BOOTLOADER_APP_START;
    sst26Addr = APP_BOOTLOADER_SST26_REVERT_OFFSET;

    {
        uint32_t iter = 0U;
        while (remaining > 0U)
        {
            chunk = remaining;
            if (chunk > APP_BOOTLOADER_SST26_PAGE_SIZE)
            {
                chunk = APP_BOOTLOADER_SST26_PAGE_SIZE;
            }

            DRV_SST26_WritePage(sst26Addr, pFlash, chunk);

            pFlash    += chunk;
            sst26Addr += chunk;
            remaining -= chunk;

            iter++;
            if ((iter % APP_BOOTLOADER_LED_BACKUP_DIV) == 0U)
            {
                lAPP_BOOTLOADER_LedToggle();
            }
        }
    }
}

/*******************************************************************************
  Function:
    static void lAPP_BOOTLOADER_InstallFromZone (
        uint32_t zoneOffset,
        uint32_t payloadSize )

  Summary:
    Copies an SST26 image into the internal application flash region.

  Description:
    Row by row, 256 bytes at a time: read from SST26 (starting at the
    zone base — no header to skip), unlock the region, erase the row,
    program its four pages.
*/

static void lAPP_BOOTLOADER_InstallFromZone(uint32_t zoneOffset,
                                            uint32_t payloadSize)
{
    /* Word-aligned row-sized scratch buffer. */
    uint32_t buf[APP_BOOTLOADER_FLASH_ROW_SIZE / 4U];
    uint8_t *pBytes;
    uint32_t remaining;
    uint32_t sst26Addr;
    uint32_t flashAddr;
    uint32_t chunk;
    uint32_t i;

    pBytes    = (uint8_t *) buf;
    remaining = payloadSize;
    sst26Addr = zoneOffset;
    flashAddr = APP_BOOTLOADER_APP_START;

    {
        uint32_t rowIdx = 0U;
        while (remaining > 0U)
        {
            chunk = remaining;
            if (chunk > APP_BOOTLOADER_FLASH_ROW_SIZE)
            {
                chunk = APP_BOOTLOADER_FLASH_ROW_SIZE;
            }

            DRV_SST26_Read(sst26Addr, pBytes, chunk);

            /* Pad any tail short of a full row with 0xFF so the final row
             * matches what an erased-then-short-written flash would look
             * like. */
            if (chunk < APP_BOOTLOADER_FLASH_ROW_SIZE)
            {
                for (i = chunk; i < APP_BOOTLOADER_FLASH_ROW_SIZE; i++)
                {
                    pBytes[i] = 0xFFU;
                }
            }

            /* Unlock, erase, and reprogram this row. Unlocking every row
             * is cheap (a single controller command) and guarantees we
             * cross region boundaries cleanly. */
            DRV_NVMCTRL_RegionUnlock(flashAddr);
            DRV_NVMCTRL_RowErase(flashAddr);

            for (i = 0U; i < DRV_NVMCTRL_PAGES_PER_ROW; i++)
            {
                DRV_NVMCTRL_PageWrite(
                    &buf[i * (DRV_NVMCTRL_PAGE_SIZE / 4U)],
                    flashAddr + (i * DRV_NVMCTRL_PAGE_SIZE));
            }

            sst26Addr += chunk;
            flashAddr += APP_BOOTLOADER_FLASH_ROW_SIZE;
            remaining -= chunk;

            rowIdx++;
            if ((rowIdx % APP_BOOTLOADER_LED_INSTALL_DIV) == 0U)
            {
                lAPP_BOOTLOADER_LedToggle();
            }
        }
    }
}

/*******************************************************************************
  Function:
    static void lAPP_BOOTLOADER_LedInit ( void )

  Summary:
    Configures PA14 as a GPIO output driving the board LED off.
*/

static void lAPP_BOOTLOADER_LedInit(void)
{
    PORT_REGS->GROUP[APP_BOOTLOADER_LED_PORT_GRP].PORT_OUTSET =
        APP_BOOTLOADER_LED_MASK;
    PORT_REGS->GROUP[APP_BOOTLOADER_LED_PORT_GRP].PORT_DIRSET =
        APP_BOOTLOADER_LED_MASK;
}

/*******************************************************************************
  Function:
    static void lAPP_BOOTLOADER_LedOff ( void )

  Summary:
    Drives the LED to its inactive state before handing control to the
    application so the app starts from a predictable LED state.
*/

static void lAPP_BOOTLOADER_LedOff(void)
{
    PORT_REGS->GROUP[APP_BOOTLOADER_LED_PORT_GRP].PORT_OUTSET =
        APP_BOOTLOADER_LED_MASK;
}

/*******************************************************************************
  Function:
    static inline void lAPP_BOOTLOADER_LedToggle ( void )

  Summary:
    Toggles the LED through the PORT_OUTTGL register.
*/

static inline void lAPP_BOOTLOADER_LedToggle(void)
{
    PORT_REGS->GROUP[APP_BOOTLOADER_LED_PORT_GRP].PORT_OUTTGL =
        APP_BOOTLOADER_LED_MASK;
}

/*******************************************************************************
  Function:
    static void lAPP_BOOTLOADER_DelayMs ( uint32_t ms )

  Summary:
    Rough software delay; used only by the panic blink.

  Description:
    At 8 MHz with -O1 the compiled inner loop takes roughly 4 cycles per
    iteration, so 2000 iterations approximate one millisecond. Accuracy
    is not important: the delay is used to pace the panic LED blink,
    not to time any protocol.
*/

static void lAPP_BOOTLOADER_DelayMs(uint32_t ms)
{
    volatile uint32_t count;

    count = ms * 2000U;
    while (count > 0U)
    {
        count--;
    }
}

/*******************************************************************************
  Function:
    static void lAPP_BOOTLOADER_Panic ( void )

  Summary:
    Unrecoverable error: blink the LED fast forever.

  Description:
    The bootloader reaches this function on malformed boot configs
    (invalid origAddr, bad destAddr, out-of-range imgSize) —
    conditions that cannot be resolved without external action. A fast
    (~10 Hz) LED blink makes the state visually distinct from both a
    healthy bootloader run and a running application.
*/

static void lAPP_BOOTLOADER_Panic(void)
{
    while (true)
    {
        lAPP_BOOTLOADER_LedToggle();
        lAPP_BOOTLOADER_DelayMs(APP_BOOTLOADER_PANIC_DELAY_MS);
    }
}

/*******************************************************************************
 End of File
*/
