/*******************************************************************************
  PRIME Service Node Bootloader Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app_bootloader.c

  Summary:
    Bare-metal bootloader for the SAMD20J18. Dispatches firmware-upgrade
    operations on every reset based on the BOOT_MODE_INFO handshake
    persisted in the SST26 BOOT_FLAG sector.

  Description:
    The bootloader runs first after every reset. It reads the 12-byte
    BOOT_MODE_INFO structure stored at the start of the SST26 BOOT_FLAG
    sector (offset 0x140000) and dispatches one of four actions:

      - NORMAL          jump straight to the application.
      - INSTALL_PENDING install the bundle currently in DOWNLOAD.
      - REVERT_PENDING  restore from the per-image REVERT zones.
      - UART_PENDING    enter the UART recovery loop.

    The bootloader is intentionally dumb: it does not validate CRC or
    signature. The PRIME firmware-upgrade service has already done that
    by the time the application sets INSTALL_PENDING; the bootloader
    only does structural sanity checks on the bundle header.

    Power-loss safety relies on imageStep persisted inside BOOT_MODE_INFO
    (0=pristine, 1=backup_done, 2=install_done) so each per-image
    install or revert is idempotent across power cuts.

    See BOOTLOADER_PLAN.md (v3) for the full design rationale.
*******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stddef.h>

#include "app_bootloader.h"
#include "device.h"
#include "drv_spi.h"
#include "drv_sst26.h"
#include "drv_nvmctrl.h"
#include "drv_boot_mode.h"
#include "drv_uart.h"

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

/* SW0 (board user button) on PA15, active-low, with external pull-up.
 * The bootloader polls it briefly at startup to give the operator a
 * way to force UART recovery even when the application is broken and
 * cannot run its own SW0 handler. */
#define APP_BOOTLOADER_SW0_PORT_GRP         0U
#define APP_BOOTLOADER_SW0_PIN              15U
#define APP_BOOTLOADER_SW0_MASK             (1UL << APP_BOOTLOADER_SW0_PIN)
#define APP_BOOTLOADER_SW0_DEBOUNCE_MS      100U
#define APP_BOOTLOADER_SW0_POLL_INTERVAL_MS 10U

/* Toggle divisors tuned so the install-loop blink lands roughly at
 * 5-7 Hz on the scope (one toggle every N pages of 256 B). */
#define APP_BOOTLOADER_LED_INSTALL_DIV      4U
#define APP_BOOTLOADER_LED_BACKUP_DIV       32U

/* Panic blink cadence: faster than normal bootloader activity so a
 * stuck node is visually distinct from one that is merely slow. */
#define APP_BOOTLOADER_PANIC_DELAY_MS       50U     /* 10 Hz */

// *****************************************************************************
// *****************************************************************************
// Section: USI Sub-Protocol (HDLC framing + CRC8)
//
// The bootloader UART loop speaks the same wire format as the rest of the
// PRIME stack USI: 0x7E start delimiter, escape byte 0x7D with mask 0x20,
// CRC8 (polynomial 0x07, init 0) over the unescaped payload, 0x7E end
// delimiter. Sub-protocol ID is 0x32 (first byte of every payload).
// *****************************************************************************
// *****************************************************************************

#define APP_BOOTLOADER_USI_DELIM            0x7EU
#define APP_BOOTLOADER_USI_ESC              0x7DU
#define APP_BOOTLOADER_USI_ESC_MASK         0x20U
#define APP_BOOTLOADER_USI_PROTO_ID         0x32U
#define APP_BOOTLOADER_USI_CRC_POLY         0x07U

/* Bootloader version reported to the host. Layout: byte 3 = major,
 * byte 2 = minor, byte 1+0 = patch (little-endian uint32). */
#define APP_BOOTLOADER_VERSION              (0x00010000UL)

// *****************************************************************************
// *****************************************************************************
// Section: UART Command Codes (sub-protocol 0x32)
// *****************************************************************************
// *****************************************************************************

#define APP_BOOTLOADER_CMD_REQ_INFO         (0x01U)
#define APP_BOOTLOADER_CMD_REQ_WRITE        (0x02U)
#define APP_BOOTLOADER_CMD_REQ_INSTALL      (0x03U)
#define APP_BOOTLOADER_CMD_REQ_EXIT         (0x04U)
#define APP_BOOTLOADER_CMD_REQ_ERASE_ALL    (0x05U)
#define APP_BOOTLOADER_CMD_REQ_READ         (0x06U)
#define APP_BOOTLOADER_CMD_REQ_READ_FLASH   (0x07U)

#define APP_BOOTLOADER_CMD_BOOT_HELLO       (0x80U)
#define APP_BOOTLOADER_CMD_RSP_INFO         (0x81U)
#define APP_BOOTLOADER_CMD_RSP_WRITE        (0x82U)
#define APP_BOOTLOADER_CMD_RSP_INSTALL      (0x83U)
#define APP_BOOTLOADER_CMD_RSP_EXIT         (0x84U)
#define APP_BOOTLOADER_CMD_RSP_ERASE_ALL    (0x85U)
#define APP_BOOTLOADER_CMD_RSP_READ         (0x86U)
#define APP_BOOTLOADER_CMD_RSP_READ_FLASH   (0x87U)

/* RSP_WRITE status codes. */
#define APP_BOOTLOADER_WRITE_OK             (0x00U)
#define APP_BOOTLOADER_WRITE_ERR_RANGE      (0x01U)
#define APP_BOOTLOADER_WRITE_ERR_ALIGN      (0x02U)

/* RSP_INSTALL status codes. */
#define APP_BOOTLOADER_INSTALL_OK           (0x00U)
#define APP_BOOTLOADER_INSTALL_ERR_BUNDLE   (0x01U)

/* RSP_READ / RSP_READ_FLASH status codes (shared layout). */
#define APP_BOOTLOADER_READ_OK              (0x00U)
#define APP_BOOTLOADER_READ_ERR_RANGE       (0x01U)
#define APP_BOOTLOADER_READ_ERR_LEN         (0x02U)

/* Maximum bytes a single REQ_READ response can carry. The USI frame
 * accumulator is 320 B unescaped; the response payload is
 * proto_id(1) + cmd(1) + status(1) + len(2) + data(N) + crc(1), so
 * the worst case unescaped frame is 6 + N. Capping data to 256 B
 * keeps the response well within the 320 B accumulator and matches
 * the SST26 page granularity the operator usually wants to dump. */
#define APP_BOOTLOADER_READ_MAX_LEN         (256U)

/* Total SST26 capacity (VF064B = 8 MB). Bound check for REQ_READ. */
#define APP_BOOTLOADER_SST26_CAPACITY       (0x800000UL)

/* RSP_ERASE_ALL status codes. */
#define APP_BOOTLOADER_ERASE_ALL_OK             (0x00U)
#define APP_BOOTLOADER_ERASE_ALL_ERR_BAD_MAGIC  (0x01U)

/* Magic word the host must send in REQ_ERASE_ALL to confirm intent.
 * Sent as the 4-byte payload (LE). Anything else is rejected so a
 * stray frame never wipes the flash by accident. */
#define APP_BOOTLOADER_ERASE_ALL_MAGIC      (0xDEADBEEFUL)

// *****************************************************************************
// *****************************************************************************
// Section: UART Loop Timing
//
// The recovery loop polls the UART without a timer interrupt — every byte
// is consumed inside a tight while(true) at 8 MHz. The two divisors below
// approximate cadences relative to that loop. Numbers were calibrated by
// counting cycles in the disassembly of a release build (-O1) and walked
// back to whole 100s for readability. They are coarse: ±20 % is acceptable
// for both the LED blink and the 2 s BOOT_HELLO emission.
// *****************************************************************************
// *****************************************************************************

/* SysTick configuration. The Cortex-M0+ system timer is loaded with
 * one tick = 10 ms at 8 MHz CPU clock (80000 cycles). The UART loop
 * polls SysTick->CTRL.COUNTFLAG; that bit latches when the counter
 * wraps and self-clears on read, so each "tick" event corresponds to
 * 10 ms of real time regardless of how fast or slow the rest of the
 * loop runs. This frees the LED / HELLO timing from any dependency
 * on -O0 vs -O1 iteration rates. */
#define APP_BOOTLOADER_SYSTICK_RELOAD       (80000UL - 1UL)
#define APP_BOOTLOADER_TICK_MS              (10UL)

/* LED phase durations expressed in 10 ms ticks.
 *
 * IDLE: symmetric 50 ms / 50 ms (10 Hz fast blink, "no host yet").
 *
 * ATTACHED: asymmetric 200 ms ON / 1800 ms OFF (one short flash every
 * 2 s, ~0.5 Hz heartbeat). Visually distinct from the application's
 * 1 Hz steady blink so the operator can tell at a glance whether the
 * device is in the bootloader or has booted into the app. */
/* The bootloader LED runs at a steady 10 Hz whenever it is idle, so an
 * operator can tell at a glance "we are in the bootloader" -- distinct
 * from the application's 1 Hz steady blink. Command handlers (Install,
 * Backup, EraseSst26Zone, the per-row NVMCTRL writes, ...) take over
 * the LED while they run with their own per-iteration toggle cadence,
 * which makes them visually different from the idle 10 Hz. */
#define APP_BOOTLOADER_LED_IDLE_ON_TICKS    (5U)    /* 50 ms */
#define APP_BOOTLOADER_LED_IDLE_OFF_TICKS   (5U)    /* 50 ms */

/* BOOT_HELLO period while waiting for a host = 2 s
 * (decision §3 #17 of BOOTLOADER_PLAN.md). */
#define APP_BOOTLOADER_HELLO_TICKS          (200U)  /* 2000 ms */

/* RX accumulator size. The largest unescaped frame is REQ_WRITE
 * (proto_id + cmd + 4 B offset + 2 B len + 256 B data + CRC = 265 B);
 * 320 leaves margin for protocol growth without dwarfing the bss. */
#define APP_BOOTLOADER_USI_MAX_FRAME        320U

// *****************************************************************************
// *****************************************************************************
// Section: Local Functions — Forward Declarations
// *****************************************************************************
// *****************************************************************************

static void        lAPP_BOOTLOADER_DisableWdt(void);
static void        lAPP_BOOTLOADER_LedInit(void);
static void        lAPP_BOOTLOADER_LedOff(void);
static inline void lAPP_BOOTLOADER_LedToggle(void);
static void        lAPP_BOOTLOADER_DelayMs(uint32_t ms);
static bool        lAPP_BOOTLOADER_Sw0Held(void);
static void        lAPP_BOOTLOADER_SelfMirrorAppCurrentIfNeeded(void);
static void        lAPP_BOOTLOADER_ClearBootMode(void);
static void        lAPP_BOOTLOADER_PersistInstallStep(uint8_t imageIdx,
                                                     uint8_t imageStep);
static void        lAPP_BOOTLOADER_PersistRevertStep(uint8_t imageIdx);
static void        lAPP_BOOTLOADER_RebootIntoUart(void) __attribute__((noreturn));
static uint8_t     lAPP_BOOTLOADER_Crc8(const uint8_t *data, uint16_t len);
static void        lAPP_BOOTLOADER_UsiRxReset(void);
static bool        lAPP_BOOTLOADER_UsiFeedByte(uint8_t b, uint16_t *outLen);
static void        lAPP_BOOTLOADER_UsiSendByteEscaped(uint8_t b);
static void        lAPP_BOOTLOADER_UsiSendFrame(uint8_t cmd,
                                                const uint8_t *payload,
                                                uint16_t       payloadLen);
static void        lAPP_BOOTLOADER_HandleReqInfo(void);
static void        lAPP_BOOTLOADER_HandleReqWrite(const uint8_t *args,
                                                  uint16_t        argLen);
static void        lAPP_BOOTLOADER_HandleReqInstall(void);
static void        lAPP_BOOTLOADER_HandleReqExit(void) __attribute__((noreturn));
static void        lAPP_BOOTLOADER_HandleReqRead(const uint8_t *args,
                                                 uint16_t        argLen);
static void        lAPP_BOOTLOADER_HandleReqReadFlash(const uint8_t *args,
                                                      uint16_t        argLen);
static void        lAPP_BOOTLOADER_HandleReqEraseAll(const uint8_t *args,
                                                     uint16_t        argLen);
static void        lAPP_BOOTLOADER_SendBootHello(void);
static void        lAPP_BOOTLOADER_PrepareDownloadZone(void);
static bool        lAPP_BOOTLOADER_ParseBundle(
                       APP_BOOTLOADER_BUNDLE_HEADER_FIXED *header,
                       APP_BOOTLOADER_BUNDLE_IMAGE *images);
static void        lAPP_BOOTLOADER_EraseSst26Zone(uint32_t offset,
                                                  uint32_t sizeBytes);
static void        lAPP_BOOTLOADER_BackupZone(uint32_t currentOffset,
                                              uint32_t revertOffset,
                                              uint32_t zoneSize,
                                              uint32_t revertMagic);
static void        lAPP_BOOTLOADER_InstallZone(uint32_t srcOffset,
                                               uint32_t payloadSize,
                                               uint32_t zoneOffset,
                                               uint32_t zoneSize,
                                               uint32_t zoneMagic,
                                               bool     alsoToInternalFlash);
static bool        lAPP_BOOTLOADER_InstallBundle(
                       const APP_BOOTLOADER_BOOT_MODE_INFO *info,
                       uint8_t *outNumInstalled);
static void        lAPP_BOOTLOADER_RevertBundle(
                       const APP_BOOTLOADER_BOOT_MODE_INFO *info);
static void        lAPP_BOOTLOADER_UartRecovery(void);
static void        lAPP_BOOTLOADER_Panic(void) __attribute__((noreturn));

/* 256-byte page buffer reused by every backup/install loop. Aligned to
 * 4 bytes so DRV_NVMCTRL_PageWrite can consume it as uint32_t words. */
static uint32_t gPageBuf[APP_BOOTLOADER_SST26_PAGE_SIZE / 4U];

/* Response buffer for REQ_READ. Layout: [0]=status, [1..2]=len (LE),
 * [3..3+len-1]=data. Sized to (3 + APP_BOOTLOADER_READ_MAX_LEN). */
static uint8_t  gReadRspBuf[3U + APP_BOOTLOADER_READ_MAX_LEN];

/* Latched on the first REQ_WRITE of a UART session so the DOWNLOAD
 * zone is erased exactly once per session (not on every write). The
 * UART recovery loop resets this to false on entry. Inspection paths
 * that never write (REQ_READ, plain SW0 entry) leave DOWNLOAD intact. */
static bool gDownloadEraseDone;

/* RX state machine for the USI sub-protocol. The accumulator collects
 * bytes between two 0x7E delimiters, with 0x7D unescape applied
 * inline. usiRxLen reaches the unescaped frame length when the closing
 * delimiter is seen. */
typedef enum
{
    APP_BOOTLOADER_USI_RX_IDLE,     /* waiting for the start delimiter   */
    APP_BOOTLOADER_USI_RX_COLLECT,  /* between two delimiters, no escape */
    APP_BOOTLOADER_USI_RX_ESCAPED,  /* last byte was 0x7D                */
} APP_BOOTLOADER_USI_RX_STATE;

static uint8_t                     usiRxBuf[APP_BOOTLOADER_USI_MAX_FRAME];
static uint16_t                    usiRxLen;
static APP_BOOTLOADER_USI_RX_STATE usiRxState;

// *****************************************************************************
// *****************************************************************************
// Section: USI Framing and CRC8 Helpers
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    static uint8_t lAPP_BOOTLOADER_Crc8 ( const uint8_t *data, uint16_t len )

  Summary:
    Computes the standard CRC-8 (poly 0x07, init 0, no reflection) over a
    byte buffer.

  Description:
    Bitwise implementation — the bootloader handles small payloads (max
    ~265 B per frame) so a 256-byte lookup table would cost more flash
    than it saves in cycles.
*/

static uint8_t lAPP_BOOTLOADER_Crc8(const uint8_t *data, uint16_t len)
{
    uint8_t  crc;
    uint16_t i;
    uint32_t bit;

    crc = 0U;

    for (i = 0U; i < len; i++)
    {
        crc ^= data[i];

        for (bit = 0U; bit < 8U; bit++)
        {
            if ((crc & 0x80U) != 0U)
            {
                crc = (uint8_t) ((crc << 1) ^ APP_BOOTLOADER_USI_CRC_POLY);
            }
            else
            {
                crc = (uint8_t) (crc << 1);
            }
        }
    }

    return crc;
}

/*******************************************************************************
  Function:
    static void lAPP_BOOTLOADER_UsiRxReset ( void )

  Summary:
    Resets the RX accumulator back to the IDLE state. Called whenever
    the receiver gives up on the in-progress frame (overflow, bad CRC,
    or after a frame has been consumed).
*/

static void lAPP_BOOTLOADER_UsiRxReset(void)
{
    usiRxLen   = 0U;
    usiRxState = APP_BOOTLOADER_USI_RX_IDLE;
}

/*******************************************************************************
  Function:
    static bool lAPP_BOOTLOADER_UsiFeedByte ( uint8_t b, uint16_t *outLen )

  Summary:
    Feeds one byte into the RX state machine. Returns true (with *outLen
    set) when a complete frame whose CRC matches has been collected in
    usiRxBuf. The frame layout in the buffer on success:

      usiRxBuf[0]                : protocol ID (must be 0x32)
      usiRxBuf[1]                : command byte
      usiRxBuf[2..outLen-1]      : command-specific arguments (if any)

    The CRC byte and the start/end delimiters are NOT in the buffer.

  Description:
    State machine:
      - IDLE     : drop bytes until a 0x7E start delimiter is seen.
      - COLLECT  : append bytes to usiRxBuf, unescaping 0x7D <next>;
                   on 0x7E, validate CRC over usiRxBuf[0..usiRxLen-2]
                   against usiRxBuf[usiRxLen-1].
      - ESCAPED  : transient state — next non-delimiter byte is XORed
                   with 0x20 and appended.

    Any anomaly (overflow, bad CRC, escape immediately followed by 0x7E)
    resets the accumulator to IDLE so subsequent valid frames are
    accepted.
*/

static bool lAPP_BOOTLOADER_UsiFeedByte(uint8_t b, uint16_t *outLen)
{
    uint8_t  expectedCrc;
    uint16_t payloadLen;
    bool     frameReady;

    frameReady = false;

    switch (usiRxState)
    {
        case APP_BOOTLOADER_USI_RX_IDLE:
            if (b == APP_BOOTLOADER_USI_DELIM)
            {
                usiRxLen   = 0U;
                usiRxState = APP_BOOTLOADER_USI_RX_COLLECT;
            }
            break;

        case APP_BOOTLOADER_USI_RX_COLLECT:
            if (b == APP_BOOTLOADER_USI_DELIM)
            {
                /* Closing delimiter: at least proto_id + cmd + CRC. */
                if (usiRxLen < 3U)
                {
                    /* Two delimiters back-to-back, or empty frame.
                     * Treat this 0x7E as a fresh start. */
                    usiRxLen = 0U;
                    break;
                }

                payloadLen  = (uint16_t) (usiRxLen - 1U);
                expectedCrc = lAPP_BOOTLOADER_Crc8(usiRxBuf, payloadLen);

                if (expectedCrc != usiRxBuf[payloadLen])
                {
                    /* CRC mismatch — discard and look for next start. */
                    lAPP_BOOTLOADER_UsiRxReset();
                    break;
                }

                if (usiRxBuf[0] != APP_BOOTLOADER_USI_PROTO_ID)
                {
                    /* Frame valid but for some other USI sub-protocol —
                     * the bootloader only owns 0x32. */
                    lAPP_BOOTLOADER_UsiRxReset();
                    break;
                }

                if (outLen != NULL)
                {
                    *outLen = payloadLen;
                }

                /* Caller now consumes the frame; reset state for the
                 * next one. */
                usiRxState = APP_BOOTLOADER_USI_RX_IDLE;
                frameReady = true;
            }
            else if (b == APP_BOOTLOADER_USI_ESC)
            {
                usiRxState = APP_BOOTLOADER_USI_RX_ESCAPED;
            }
            else
            {
                if (usiRxLen >= APP_BOOTLOADER_USI_MAX_FRAME)
                {
                    lAPP_BOOTLOADER_UsiRxReset();
                    break;
                }
                usiRxBuf[usiRxLen] = b;
                usiRxLen++;
            }
            break;

        case APP_BOOTLOADER_USI_RX_ESCAPED:
        default:
            if (b == APP_BOOTLOADER_USI_DELIM)
            {
                /* Escape immediately followed by a delimiter is a
                 * protocol error: bail. */
                lAPP_BOOTLOADER_UsiRxReset();
                break;
            }
            if (usiRxLen >= APP_BOOTLOADER_USI_MAX_FRAME)
            {
                lAPP_BOOTLOADER_UsiRxReset();
                break;
            }
            usiRxBuf[usiRxLen] = (uint8_t) (b ^ APP_BOOTLOADER_USI_ESC_MASK);
            usiRxLen++;
            usiRxState = APP_BOOTLOADER_USI_RX_COLLECT;
            break;
    }

    return frameReady;
}

/*******************************************************************************
  Function:
    static void lAPP_BOOTLOADER_UsiSendByteEscaped ( uint8_t b )

  Summary:
    Sends one payload byte over UART, prepending the escape byte if the
    value collides with the framing delimiter or escape character.
*/

static void lAPP_BOOTLOADER_UsiSendByteEscaped(uint8_t b)
{
    if ((b == APP_BOOTLOADER_USI_DELIM) || (b == APP_BOOTLOADER_USI_ESC))
    {
        DRV_UART_SendByte(APP_BOOTLOADER_USI_ESC);
        DRV_UART_SendByte((uint8_t) (b ^ APP_BOOTLOADER_USI_ESC_MASK));
    }
    else
    {
        DRV_UART_SendByte(b);
    }
}

/*******************************************************************************
  Function:
    static void lAPP_BOOTLOADER_UsiSendFrame (
        uint8_t cmd,
        const uint8_t *payload,
        uint16_t payloadLen )

  Summary:
    Wire-encodes and transmits a complete USI frame.

  Description:
    Layout sent on the wire:
      0x7E  proto_id(0x32)  cmd  payload[0..payloadLen-1]  crc8  0x7E

    proto_id, cmd, payload bytes and the CRC are all individually
    escape-encoded; the two 0x7E delimiters are not. CRC is computed
    over [proto_id, cmd, payload[]] in their unescaped form.
*/

static void lAPP_BOOTLOADER_UsiSendFrame(uint8_t cmd,
                                         const uint8_t *payload,
                                         uint16_t       payloadLen)
{
    uint8_t  prefix[2];
    uint8_t  crc;
    uint16_t i;

    prefix[0] = APP_BOOTLOADER_USI_PROTO_ID;
    prefix[1] = cmd;

    /* CRC over unescaped bytes: [proto_id, cmd, payload[]]. The bitwise
     * helper accepts a single contiguous buffer at a time, so chain
     * the two segments. */
    crc = lAPP_BOOTLOADER_Crc8(prefix, sizeof(prefix));
    if ((payload != NULL) && (payloadLen > 0U))
    {
        uint8_t crcAcc;
        uint8_t bit;
        uint8_t poly;

        crcAcc = crc;
        poly   = APP_BOOTLOADER_USI_CRC_POLY;

        for (i = 0U; i < payloadLen; i++)
        {
            crcAcc ^= payload[i];
            for (bit = 0U; bit < 8U; bit++)
            {
                if ((crcAcc & 0x80U) != 0U)
                {
                    crcAcc = (uint8_t) ((crcAcc << 1) ^ poly);
                }
                else
                {
                    crcAcc = (uint8_t) (crcAcc << 1);
                }
            }
        }

        crc = crcAcc;
    }

    /* Wire emission. */
    DRV_UART_SendByte(APP_BOOTLOADER_USI_DELIM);

    lAPP_BOOTLOADER_UsiSendByteEscaped(prefix[0]);
    lAPP_BOOTLOADER_UsiSendByteEscaped(prefix[1]);

    for (i = 0U; i < payloadLen; i++)
    {
        lAPP_BOOTLOADER_UsiSendByteEscaped(payload[i]);
    }

    lAPP_BOOTLOADER_UsiSendByteEscaped(crc);
    DRV_UART_SendByte(APP_BOOTLOADER_USI_DELIM);
}

// *****************************************************************************
// *****************************************************************************
// Section: Bootloader Main and Jump
// *****************************************************************************
// *****************************************************************************

void APP_BOOTLOADER_Main(void)
{
    APP_BOOTLOADER_BOOT_MODE_INFO info;

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
     * and to the internal flash controller. The boot-mode handshake
     * lives in SST26, so SPI must be ready before reading it. */
    DRV_SPI_Initialize();
    DRV_SST26_Initialize();
    DRV_NVMCTRL_Initialize();

    /* Sanity-check that the connected SST26 is the expected variant.
     * The bootloader's erase logic depends on the variable block-size
     * geometry of the SST26VFxxxB family (4 x 8 KB parameter blocks +
     * 1 x 32 KB sub-block in the bottom 64 KB). A different chip with
     * a uniform 64 KB geometry, or any other SPI flash, would silently
     * corrupt DOWNLOAD because EraseSst26Zone strides the bottom region
     * in 8 KB / 32 KB chunks. Fail loud rather than fail silently. */
    if (DRV_SST26_ReadJedecId() != DRV_SST26_JEDEC_ID)
    {
        lAPP_BOOTLOADER_Panic();
    }

    /* Emergency entry: holding SW0 (PA15) at power-up forces UART
     * recovery regardless of BOOT_MODE_INFO. Lets the operator escape
     * a brick where the application is corrupt and cannot run its
     * own SW0 handler. The recovery loop runs in place and never
     * returns -- it ends the session via REQ_EXIT (clears BOOT_FLAG to
     * NORMAL + reset) or REQ_INSTALL (writes INSTALL_PENDING + reset).
     * BOOT_FLAG is intentionally NOT modified before entering the
     * loop, so a release of SW0 followed by a power cycle returns to
     * whatever boot mode was previously set. */
    if (lAPP_BOOTLOADER_Sw0Held() == true)
    {
        lAPP_BOOTLOADER_UartRecovery();
        /* Unreachable: UartRecovery only exits via system reset. */
    }

    info = DRV_BOOT_MODE_Read();

    switch ((APP_BOOTLOADER_BOOT_MODE) info.mode)
    {
        case APP_BOOTLOADER_BOOT_MODE_NORMAL:
        default:
            /* No pending operation, or unknown / virgin sector: go
             * straight to the application. DRV_BOOT_MODE_Read collapses
             * any inconsistent read to NORMAL with magic = 0, so this
             * is also the safe default for bit-rot or first-ever boot.
             *
             * On the very first boot after factory programming the
             * SST26 APP_CURRENT zone is virgin even though the internal
             * flash holds a valid app. Mirror it now so the first FU
             * has a usable APP_REVERT to back up to. The function is a
             * no-op on every subsequent boot. */
            lAPP_BOOTLOADER_SelfMirrorAppCurrentIfNeeded();
            break;

        case APP_BOOTLOADER_BOOT_MODE_INSTALL_PENDING:
            if (lAPP_BOOTLOADER_InstallBundle(&info, NULL) == false)
            {
                /* Bundle in DOWNLOAD is malformed: an install request
                 * should never reach the bootloader without a valid
                 * bundle (the application validates CRC + signature
                 * first), so divert to UART recovery for operator
                 * intervention instead of bricking. */
                lAPP_BOOTLOADER_RebootIntoUart();
            }
            lAPP_BOOTLOADER_ClearBootMode();
            break;

        case APP_BOOTLOADER_BOOT_MODE_REVERT_PENDING:
            lAPP_BOOTLOADER_RevertBundle(&info);
            lAPP_BOOTLOADER_ClearBootMode();
            break;

        case APP_BOOTLOADER_BOOT_MODE_UART_PENDING:
            /* Recovery loop owns the rest of this boot. It only returns
             * when the operator has cleared the flag and asked for a
             * jump-to-app, otherwise it ends the session with a reset. */
            lAPP_BOOTLOADER_UartRecovery();
            lAPP_BOOTLOADER_ClearBootMode();
            break;
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
// Section: Local Functions — Boot Mode Dispatch
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    static void lAPP_BOOTLOADER_ClearBootMode ( void )

  Summary:
    Persists BOOT_MODE_NORMAL so the next reset jumps straight to the
    application.

  Description:
    Called after a successful INSTALL/REVERT/UART session. Erases the
    BOOT_FLAG sector and writes a fresh structure with mode = NORMAL,
    imageIdx = 0, imageStep = 0.
*/

static void lAPP_BOOTLOADER_ClearBootMode(void)
{
    APP_BOOTLOADER_BOOT_MODE_INFO clean;

    clean.magic     = APP_BOOTLOADER_BOOT_MODE_MAGIC;
    clean.mode      = (uint8_t) APP_BOOTLOADER_BOOT_MODE_NORMAL;
    clean.imageIdx  = 0U;
    clean.imageStep = 0U;
    clean.reserved  = 0U;
    clean.modeXor   = (uint32_t) APP_BOOTLOADER_BOOT_MODE_NORMAL
                    ^ (APP_BOOTLOADER_BOOT_MODE_MAGIC & 0xFFU);

    DRV_BOOT_MODE_Write(&clean);
}

/*******************************************************************************
  Function:
    static void lAPP_BOOTLOADER_PersistInstallStep (
        uint8_t imageIdx, uint8_t imageStep )

  Summary:
    Rewrites BOOT_MODE_INFO with INSTALL_PENDING and the new (idx, step).

  Description:
    Each persisted (idx, step) costs ~28 ms (4 KB sector erase + 256 B
    page program) and one SST26 sector erase cycle. The install loop
    calls this at every backup_done/install_done transition so a power
    cut resumes on the next boot without re-doing completed work.
*/

static void lAPP_BOOTLOADER_PersistInstallStep(uint8_t imageIdx, uint8_t imageStep)
{
    APP_BOOTLOADER_BOOT_MODE_INFO info;

    info.magic     = APP_BOOTLOADER_BOOT_MODE_MAGIC;
    info.mode      = (uint8_t) APP_BOOTLOADER_BOOT_MODE_INSTALL_PENDING;
    info.imageIdx  = imageIdx;
    info.imageStep = imageStep;
    info.reserved  = 0U;
    info.modeXor   = (uint32_t) APP_BOOTLOADER_BOOT_MODE_INSTALL_PENDING
                   ^ (APP_BOOTLOADER_BOOT_MODE_MAGIC & 0xFFU);

    DRV_BOOT_MODE_Write(&info);
}

/*******************************************************************************
  Function:
    static bool lAPP_BOOTLOADER_ParseBundle (
        APP_BOOTLOADER_BUNDLE_HEADER_FIXED *header,
        APP_BOOTLOADER_BUNDLE_IMAGE *images )

  Summary:
    Reads and validates the BUNDLE_HEADER at the start of the DOWNLOAD
    zone.

  Description:
    Reads the 16-byte fixed prefix, then numImages * 12 B descriptors,
    then the 4-byte magicEnd marker. Validates magicStart, formatVersion,
    numImages range, totalSize range, magicEnd, and per-image typeMagic
    + offset/size bounds. Returns true if everything is consistent.

    The caller passes pre-allocated storage (header + images[] of size
    APP_BOOTLOADER_BUNDLE_MAX_IMAGES) so this function does not own any
    memory.
*/

static bool lAPP_BOOTLOADER_ParseBundle(
    APP_BOOTLOADER_BUNDLE_HEADER_FIXED *header,
    APP_BOOTLOADER_BUNDLE_IMAGE *images)
{
    uint32_t descriptorsAddr;
    uint32_t magicEnd;
    uint32_t i;
    uint32_t maxSize;

    /* 1. Fixed 16 B prefix. */
    DRV_SST26_Read(APP_BOOTLOADER_SST26_DOWNLOAD_OFFSET,
                   (uint8_t *) header,
                   sizeof(APP_BOOTLOADER_BUNDLE_HEADER_FIXED));

    if (header->magicStart != APP_BOOTLOADER_BUNDLE_MAGIC_START)
    {
        return false;
    }

    if (header->formatVersion != APP_BOOTLOADER_BUNDLE_FORMAT_VERSION)
    {
        return false;
    }

    if ((header->numImages == 0U) ||
        (header->numImages > APP_BOOTLOADER_BUNDLE_MAX_IMAGES))
    {
        return false;
    }

    if ((header->totalSize < sizeof(APP_BOOTLOADER_BUNDLE_HEADER_FIXED)) ||
        (header->totalSize > (APP_BOOTLOADER_SST26_DOWNLOAD_SIZE - 4U)))
    {
        return false;
    }

    /* 2. Descriptors right after the fixed prefix. */
    descriptorsAddr = APP_BOOTLOADER_SST26_DOWNLOAD_OFFSET
                    + sizeof(APP_BOOTLOADER_BUNDLE_HEADER_FIXED);

    DRV_SST26_Read(descriptorsAddr,
                   (uint8_t *) images,
                   header->numImages
                   * sizeof(APP_BOOTLOADER_BUNDLE_IMAGE));

    /* 3. magicEnd lies exactly at offset totalSize from the bundle base. */
    DRV_SST26_Read(APP_BOOTLOADER_SST26_DOWNLOAD_OFFSET + header->totalSize,
                   (uint8_t *) &magicEnd,
                   sizeof(magicEnd));

    if (magicEnd != APP_BOOTLOADER_BUNDLE_MAGIC_END)
    {
        return false;
    }

    /* 4. Per-image sanity. */
    for (i = 0U; i < header->numImages; i++)
    {
        if ((images[i].typeMagic != APP_BOOTLOADER_TYPE_MAGIC_APP) &&
            (images[i].typeMagic != APP_BOOTLOADER_TYPE_MAGIC_PL360))
        {
            return false;
        }

        if (images[i].typeMagic == APP_BOOTLOADER_TYPE_MAGIC_APP)
        {
            maxSize = APP_BOOTLOADER_MAX_APP_SIZE;
        }
        else
        {
            maxSize = APP_BOOTLOADER_SST26_PL360_CURRENT_SIZE
                    - APP_BOOTLOADER_ZONE_HEADER_SIZE;
        }

        if ((images[i].size == 0U) || (images[i].size > maxSize))
        {
            return false;
        }

        if ((images[i].offset < sizeof(APP_BOOTLOADER_BUNDLE_HEADER_FIXED)) ||
            (images[i].offset + images[i].size > header->totalSize))
        {
            return false;
        }
    }

    return true;
}

/*******************************************************************************
  Function:
    static void lAPP_BOOTLOADER_EraseSst26Zone (
        uint32_t offset, uint32_t sizeBytes )

  Summary:
    Erases an SST26 zone covering its full extent, regardless of where
    in the memory map the zone sits.

  Description:
    The SST26VFxxxB family does not use a uniform 64 KB block size: the
    bottom 64 KB of the chip (and the top 64 KB) are split into smaller
    sub-blocks (4 x 8 KB parameter blocks + 1 x 32 KB block) instead of
    being a single 64 KB block. From the SST26VF016/032 datasheet:

      "Blocks are 64 Kbyte, 32 Kbyte or 8Kbyte, depending on location.
       Block Erase Address: AMS - A16 for 64 Kbyte; AMS - A15 for 32
       Kbyte; AMS - A13 for 8 Kbyte."

    The same opcode (D8h, BlockErase64K) is used for all sizes; the chip
    decides how much to erase based on which region the address falls
    in. Issuing BE(0x0000) therefore only erases 8 KB, NOT 64 KB. In v3
    the DOWNLOAD zone starts at offset 0 and thus crosses this region;
    a naive 64 KB stride would silently leave 0x2000..0xFFFF unerased,
    which then surfaces as a "previous content AND new bytes"
    bit-AND corruption pattern on the very next REQ_WRITE.

    This implementation walks the zone using whatever block size the
    current cursor maps to:

      cursor in [0x00000..0x07FFF]      -> 8 KB sub-blocks
      cursor in [0x08000..0x0FFFF]      -> 32 KB sub-block
      cursor >= 0x10000  (and below the
        symmetric top region we don't
        use in v3)                      -> 64 KB blocks

    Each iteration toggles the LED so a stuck erase is visible.
*/

static void lAPP_BOOTLOADER_EraseSst26Zone(uint32_t offset, uint32_t sizeBytes)
{
    uint32_t cursor;
    uint32_t end;
    uint32_t blockSize;
    uint8_t  sample[4];
    uint32_t i;

    cursor = offset;
    end    = offset + sizeBytes;

    while (cursor < end)
    {
        if (cursor < 0x8000UL)
        {
            blockSize = 0x2000UL;          /* 8 KB parameter sub-block  */
        }
        else if (cursor < 0x10000UL)
        {
            blockSize = 0x8000UL;          /* 32 KB sub-block           */
        }
        else
        {
            blockSize = APP_BOOTLOADER_SST26_BLOCK_64K_SIZE;
        }

        DRV_SST26_BlockErase64K(cursor);

        /* Post-erase verify: read the first 4 bytes of the block we
         * just erased and confirm they are 0xFF. If they are not, the
         * chip's actual geometry does not match the assumption made by
         * the size selection above (different chip variant, future
         * silicon revision, or simply a hardware fault), and continuing
         * would silently propagate "previous content AND new bytes"
         * corruption on the next program. Panic visibly. */
        DRV_SST26_Read(cursor, sample, (uint32_t) sizeof(sample));
        for (i = 0U; i < (uint32_t) sizeof(sample); i++)
        {
            if (sample[i] != 0xFFU)
            {
                lAPP_BOOTLOADER_Panic();
                /* unreachable */
            }
        }

        cursor += blockSize;
        lAPP_BOOTLOADER_LedToggle();
    }
}

/*******************************************************************************
  Function:
    static void lAPP_BOOTLOADER_SelfMirrorAppCurrentIfNeeded ( void )

  Summary:
    Mirrors the internal flash application region into SST26 APP_CURRENT
    on the very first boot after factory programming.

  Description:
    After PICkit/JTAG factory programming the internal flash holds the
    application image but APP_CURRENT in SST26 is virgin (0xFF). The
    first PRIME firmware-upgrade would back up this empty zone into
    APP_REVERT — leaving the device with no usable rollback target.
    This function fixes the asymmetry: at the first NORMAL boot, when
    APP_CURRENT does not yet have the 'APPC' magic, copy the entire
    APP_BOOTLOADER_MAX_APP_SIZE bytes (~248 KB) of internal flash into
    APP_CURRENT, prefixed by a fresh ZONE_HEADER. Subsequent boots see
    'APPC' and skip immediately.

    A vector-table sanity check guards against mirroring 0xFF garbage
    on a chip that has had its app erased: if the initial stack
    pointer or reset PC at 0x2000 do not look like a Cortex-M vector
    table, no mirror is performed (no app to back up).

    The operation is synchronous and one-shot, so the bootloader's
    ~2 s extra cost only affects the very first boot of a freshly
    programmed device.
*/

static void lAPP_BOOTLOADER_SelfMirrorAppCurrentIfNeeded(void)
{
    uint32_t        magic;
    uint32_t        sp;
    uint32_t        pc;
    const uint32_t *flashWords;
    uint32_t        numPages;
    uint32_t        srcAddr;
    uint32_t        dstAddr;
    uint32_t        i;
    uint32_t        k;

    /* Already mirrored on a previous boot? Read just the first word
     * (the magic) to keep the cold path cheap on every subsequent
     * boot — under one SPI byte transfer plus the 4-byte read. */
    DRV_SST26_Read(APP_BOOTLOADER_SST26_APP_CURRENT_OFFSET,
                   (uint8_t *) gPageBuf, 4U);
    magic = gPageBuf[0];

    if (magic == APP_BOOTLOADER_ZONE_MAGIC_APP_CURRENT)
    {
        return;
    }

    /* Vector-table plausibility check on the internal flash app slot.
     * Initial SP must point inside SAMD20J18 SRAM (0x20000000..
     * 0x20007FFF) and the reset handler PC must be inside the app
     * region with the Thumb bit set. If neither holds, the chip has
     * been erased or never programmed at all and there is nothing
     * useful to mirror. */
    flashWords = (const uint32_t *) APP_BOOTLOADER_APP_START;
    sp = flashWords[0];
    pc = flashWords[1];

    if ((sp < 0x20000000UL) || (sp > 0x20008000UL) ||
        ((pc & 1UL) == 0UL) ||
        (((pc & ~1UL) < APP_BOOTLOADER_APP_START) ||
         ((pc & ~1UL) >= APP_BOOTLOADER_APP_END)))
    {
        return;
    }

    /* Erase APP_CURRENT zone in SST26 (256 KB at 0x080000, all in
     * regular 64 KB blocks; EraseSst26Zone handles the strides). */
    lAPP_BOOTLOADER_EraseSst26Zone(APP_BOOTLOADER_SST26_APP_CURRENT_OFFSET,
                                   APP_BOOTLOADER_SST26_APP_CURRENT_SIZE);

    /* Write the ZONE_HEADER: 'APPC' magic + payload size in the first
     * 8 bytes of the first page, rest 0xFF padding. */
    for (k = 0U; k < (sizeof(gPageBuf) / sizeof(gPageBuf[0])); k++)
    {
        gPageBuf[k] = 0xFFFFFFFFUL;
    }
    gPageBuf[0] = APP_BOOTLOADER_ZONE_MAGIC_APP_CURRENT;
    gPageBuf[1] = APP_BOOTLOADER_MAX_APP_SIZE;

    DRV_SST26_WritePage(APP_BOOTLOADER_SST26_APP_CURRENT_OFFSET,
                        (const uint8_t *) gPageBuf,
                        APP_BOOTLOADER_SST26_PAGE_SIZE);

    /* Copy the entire app region byte-for-byte into the SST26 zone,
     * page by page (256 B). Internal flash is memory-mapped so a
     * straight word copy fills gPageBuf without an intermediate driver
     * call. The size used is APP_BOOTLOADER_MAX_APP_SIZE (= 248 KB);
     * that may include trailing 0xFF padding past the actual app
     * binary, which is fine — the install path uses the descriptor's
     * size when overwriting later. */
    numPages = APP_BOOTLOADER_MAX_APP_SIZE
             / APP_BOOTLOADER_SST26_PAGE_SIZE;
    srcAddr  = APP_BOOTLOADER_APP_START;
    dstAddr  = APP_BOOTLOADER_SST26_APP_CURRENT_OFFSET
             + APP_BOOTLOADER_ZONE_HEADER_SIZE;

    for (i = 0U; i < numPages; i++)
    {
        flashWords = (const uint32_t *) srcAddr;
        for (k = 0U;
             k < (APP_BOOTLOADER_SST26_PAGE_SIZE / 4U);
             k++)
        {
            gPageBuf[k] = flashWords[k];
        }

        DRV_SST26_WritePage(dstAddr, (const uint8_t *) gPageBuf,
                            APP_BOOTLOADER_SST26_PAGE_SIZE);

        srcAddr += APP_BOOTLOADER_SST26_PAGE_SIZE;
        dstAddr += APP_BOOTLOADER_SST26_PAGE_SIZE;

        if ((i % APP_BOOTLOADER_LED_INSTALL_DIV) == 0U)
        {
            lAPP_BOOTLOADER_LedToggle();
        }
    }
}

/*******************************************************************************
  Function:
    static void lAPP_BOOTLOADER_BackupZone (
        uint32_t currentOffset, uint32_t revertOffset,
        uint32_t zoneSize, uint32_t revertMagic )

  Summary:
    Copies CURRENT into REVERT, re-stamping the zone magic.

  Description:
    1. Read ZONE_HEADER from CURRENT to extract the payload size.
    2. Erase REVERT.
    3. Write a fresh ZONE_HEADER {revertMagic, currentSize} to REVERT[0].
    4. Loop page by page: read CURRENT[256 + i*256] → write
       REVERT[256 + i*256], for ceil(currentSize / 256) pages.

    If the CURRENT zone is virgin (magic == 0xFFFFFFFF) we skip the
    backup entirely — there is nothing to roll back to. The REVERT zone
    stays unchanged, which is fine: a future revert against a virgin
    REVERT is itself an anomaly handled in the REVERT_PENDING path.
*/

static void lAPP_BOOTLOADER_BackupZone(uint32_t currentOffset,
                                       uint32_t revertOffset,
                                       uint32_t zoneSize,
                                       uint32_t revertMagic)
{
    uint32_t currentMagic;
    uint32_t currentSize;
    uint32_t numPages;
    uint32_t i;
    uint32_t srcAddr;
    uint32_t dstAddr;

    /* 1. Read CURRENT header. The first 8 bytes give us magic and size;
     *    we only need those to plan the backup. */
    DRV_SST26_Read(currentOffset, (uint8_t *) gPageBuf,
                   APP_BOOTLOADER_ZONE_HEADER_SIZE);
    currentMagic = gPageBuf[0];
    currentSize  = gPageBuf[1];

    if ((currentMagic == 0xFFFFFFFFUL) || (currentSize == 0U) ||
        (currentSize > (zoneSize - APP_BOOTLOADER_ZONE_HEADER_SIZE)))
    {
        /* Virgin or malformed CURRENT: nothing valid to back up. */
        return;
    }

    /* 2. Erase REVERT. */
    lAPP_BOOTLOADER_EraseSst26Zone(revertOffset, zoneSize);

    /* 3. Build new ZONE_HEADER directly inside gPageBuf (magic + size at
     *    offsets 0 and 4, the rest of the 256 B page is 0xFF padding). */
    for (i = 0U; i < (sizeof(gPageBuf) / sizeof(gPageBuf[0])); i++)
    {
        gPageBuf[i] = 0xFFFFFFFFUL;
    }
    gPageBuf[0] = revertMagic;
    gPageBuf[1] = currentSize;

    DRV_SST26_WritePage(revertOffset, (const uint8_t *) gPageBuf,
                        APP_BOOTLOADER_SST26_PAGE_SIZE);

    /* 4. Copy payload page by page. */
    numPages = (currentSize + APP_BOOTLOADER_SST26_PAGE_SIZE - 1U)
             / APP_BOOTLOADER_SST26_PAGE_SIZE;
    srcAddr  = currentOffset + APP_BOOTLOADER_ZONE_HEADER_SIZE;
    dstAddr  = revertOffset  + APP_BOOTLOADER_ZONE_HEADER_SIZE;

    for (i = 0U; i < numPages; i++)
    {
        DRV_SST26_Read(srcAddr, (uint8_t *) gPageBuf,
                       APP_BOOTLOADER_SST26_PAGE_SIZE);
        DRV_SST26_WritePage(dstAddr, (const uint8_t *) gPageBuf,
                            APP_BOOTLOADER_SST26_PAGE_SIZE);

        srcAddr += APP_BOOTLOADER_SST26_PAGE_SIZE;
        dstAddr += APP_BOOTLOADER_SST26_PAGE_SIZE;

        if ((i % APP_BOOTLOADER_LED_BACKUP_DIV) == 0U)
        {
            lAPP_BOOTLOADER_LedToggle();
        }
    }
}

/*******************************************************************************
  Function:
    static void lAPP_BOOTLOADER_InstallZone (
        uint32_t srcOffset,
        uint32_t payloadSize,
        uint32_t zoneOffset,
        uint32_t zoneSize,
        uint32_t zoneMagic,
        bool     alsoToInternalFlash )

  Summary:
    Copies a payload from somewhere in SST26 into a CURRENT zone,
    optionally also into the internal application flash.

  Description:
    1. Erase the destination CURRENT zone.
    2. Write a fresh ZONE_HEADER {zoneMagic, payloadSize} to zone[0].
    3. Loop page by page (256 B): read source payload → write CURRENT
       payload area. If alsoToInternalFlash is true, the same page is
       also programmed into the internal flash app region (one row per
       page → unlock + RowErase + 4 × PageWrite).
    4. The final partial row is padded with 0xFF before being programmed.

    srcOffset is an absolute SST26 address pointing at the start of the
    raw payload (no header). Both INSTALL_PENDING (source = DOWNLOAD +
    image offset) and REVERT_PENDING (source = REVERT_offset +
    ZONE_HEADER_SIZE) reuse this routine unchanged.
*/

static void lAPP_BOOTLOADER_InstallZone(uint32_t srcOffset,
                                        uint32_t payloadSize,
                                        uint32_t zoneOffset,
                                        uint32_t zoneSize,
                                        uint32_t zoneMagic,
                                        bool     alsoToInternalFlash)
{
    uint32_t numPages;
    uint32_t i;
    uint32_t k;
    uint32_t bytesThisPage;
    uint32_t srcAddr;
    uint32_t dstSst26;
    uint32_t dstFlash;
    uint8_t *pageBytes;

    pageBytes = (uint8_t *) gPageBuf;

    /* 1. Erase the destination CURRENT zone. */
    lAPP_BOOTLOADER_EraseSst26Zone(zoneOffset, zoneSize);

    /* 2. Write ZONE_HEADER. The page buffer is reused everywhere; fill
     *    with 0xFF then drop magic and size at offsets 0 and 4. */
    for (k = 0U; k < (sizeof(gPageBuf) / sizeof(gPageBuf[0])); k++)
    {
        gPageBuf[k] = 0xFFFFFFFFUL;
    }
    gPageBuf[0] = zoneMagic;
    gPageBuf[1] = payloadSize;

    DRV_SST26_WritePage(zoneOffset, (const uint8_t *) gPageBuf,
                        APP_BOOTLOADER_SST26_PAGE_SIZE);

    /* 3. Streaming page-by-page copy. */
    numPages = (payloadSize + APP_BOOTLOADER_SST26_PAGE_SIZE - 1U)
             / APP_BOOTLOADER_SST26_PAGE_SIZE;
    srcAddr  = srcOffset;
    dstSst26 = zoneOffset + APP_BOOTLOADER_ZONE_HEADER_SIZE;
    dstFlash = APP_BOOTLOADER_APP_START;

    for (i = 0U; i < numPages; i++)
    {
        bytesThisPage = APP_BOOTLOADER_SST26_PAGE_SIZE;
        if ((i + 1U) == numPages)
        {
            bytesThisPage = payloadSize
                          - (i * APP_BOOTLOADER_SST26_PAGE_SIZE);
        }

        /* Load page from DOWNLOAD; pad short tail with 0xFF so both
         * SST26 and internal flash see a clean fully-formed page. */
        DRV_SST26_Read(srcAddr, pageBytes, bytesThisPage);
        if (bytesThisPage < APP_BOOTLOADER_SST26_PAGE_SIZE)
        {
            for (k = bytesThisPage;
                 k < APP_BOOTLOADER_SST26_PAGE_SIZE; k++)
            {
                pageBytes[k] = 0xFFU;
            }
        }

        /* SST26 destination. */
        DRV_SST26_WritePage(dstSst26, (const uint8_t *) gPageBuf,
                            APP_BOOTLOADER_SST26_PAGE_SIZE);

        /* Internal flash destination (APP only). One SST26 page = one
         * 256 B row = four 64 B flash pages.
         *
         * Each NVMCTRL command is followed by DRV_NVMCTRL_GetError so a
         * silently-rejected operation (LOCKE on a locked region, PROGE
         * on a row that was not erased, NVME on a hardware fault) is
         * surfaced as a panic LED pattern instead of producing the
         * "result = previous bits AND new bits" bit-AND corruption that
         * was masking real failures before this fix. CacheInvalidate
         * after the row write keeps subsequent flash reads (e.g. for
         * verify, JumpToApp, or future self-mirror) from returning
         * stale cached data. */
        if (alsoToInternalFlash)
        {
            DRV_NVMCTRL_RegionUnlock(dstFlash);
            if (DRV_NVMCTRL_GetError() != DRV_NVMCTRL_ERROR_NONE)
            {
                lAPP_BOOTLOADER_Panic();
            }

            DRV_NVMCTRL_RowErase(dstFlash);
            if (DRV_NVMCTRL_GetError() != DRV_NVMCTRL_ERROR_NONE)
            {
                lAPP_BOOTLOADER_Panic();
            }

            for (k = 0U; k < DRV_NVMCTRL_PAGES_PER_ROW; k++)
            {
                DRV_NVMCTRL_PageWrite(
                    &gPageBuf[k * (DRV_NVMCTRL_PAGE_SIZE / 4U)],
                    dstFlash + (k * DRV_NVMCTRL_PAGE_SIZE));
                if (DRV_NVMCTRL_GetError() != DRV_NVMCTRL_ERROR_NONE)
                {
                    lAPP_BOOTLOADER_Panic();
                }
            }

            DRV_NVMCTRL_CacheInvalidate();

            dstFlash += APP_BOOTLOADER_FLASH_ROW_SIZE;
        }

        srcAddr  += APP_BOOTLOADER_SST26_PAGE_SIZE;
        dstSst26 += APP_BOOTLOADER_SST26_PAGE_SIZE;

        if ((i % APP_BOOTLOADER_LED_INSTALL_DIV) == 0U)
        {
            lAPP_BOOTLOADER_LedToggle();
        }
    }
}

/*******************************************************************************
  Function:
    static void lAPP_BOOTLOADER_InstallBundle (
        const APP_BOOTLOADER_BOOT_MODE_INFO *info )

  Summary:
    INSTALL_PENDING handler — applies the bundle currently in the SST26
    DOWNLOAD zone.

  Description:
    Parses the BUNDLE_HEADER and iterates the descriptors. For each
    image, follows a 2-step sequence with a persistence point in between
    so a power loss restarts at the half-finished image without
    corrupting the existing REVERT:

      step 0 → 1 : back up the matching CURRENT zone into REVERT
      step 1 → 2 : install the DOWNLOAD payload into CURRENT (and into
                   internal flash for APP)

    After the last image, the caller writes mode = NORMAL and the next
    reset jumps to the application.
*/

static bool lAPP_BOOTLOADER_InstallBundle(const APP_BOOTLOADER_BOOT_MODE_INFO *info,
                                          uint8_t *outNumInstalled)
{
    APP_BOOTLOADER_BUNDLE_HEADER_FIXED header;
    APP_BOOTLOADER_BUNDLE_IMAGE images[APP_BOOTLOADER_BUNDLE_MAX_IMAGES];
    uint32_t i;
    uint8_t  step;
    uint32_t downloadPayloadOffset;
    uint32_t currentOffset;
    uint32_t revertOffset;
    uint32_t zoneSize;
    uint32_t zoneMagic;
    uint32_t revertMagic;
    bool     alsoToInternalFlash;

    if (outNumInstalled != NULL)
    {
        *outNumInstalled = 0U;
    }

    if (lAPP_BOOTLOADER_ParseBundle(&header, images) == false)
    {
        return false;
    }

    /* Resume from where the previous boot left off. info->imageIdx may
     * be ≥ numImages if a power cut hit just after the last (idx, 2)
     * persist but before the caller wrote mode = NORMAL — in that case
     * the loop simply runs zero iterations. */
    for (i = (uint32_t) info->imageIdx; i < header.numImages; i++)
    {
        step = (i == (uint32_t) info->imageIdx) ? info->imageStep : 0U;

        if (images[i].typeMagic == APP_BOOTLOADER_TYPE_MAGIC_APP)
        {
            currentOffset       = APP_BOOTLOADER_SST26_APP_CURRENT_OFFSET;
            revertOffset        = APP_BOOTLOADER_SST26_APP_REVERT_OFFSET;
            zoneSize            = APP_BOOTLOADER_SST26_APP_CURRENT_SIZE;
            zoneMagic           = APP_BOOTLOADER_ZONE_MAGIC_APP_CURRENT;
            revertMagic         = APP_BOOTLOADER_ZONE_MAGIC_APP_REVERT;
            alsoToInternalFlash = true;
        }
        else
        {
            currentOffset       = APP_BOOTLOADER_SST26_PL360_CURRENT_OFFSET;
            revertOffset        = APP_BOOTLOADER_SST26_PL360_REVERT_OFFSET;
            zoneSize            = APP_BOOTLOADER_SST26_PL360_CURRENT_SIZE;
            zoneMagic           = APP_BOOTLOADER_ZONE_MAGIC_PL360_CURRENT;
            revertMagic         = APP_BOOTLOADER_ZONE_MAGIC_PL360_REVERT;
            alsoToInternalFlash = false;
        }

        if (step < 1U)
        {
            lAPP_BOOTLOADER_BackupZone(currentOffset, revertOffset,
                                       zoneSize, revertMagic);
            lAPP_BOOTLOADER_PersistInstallStep((uint8_t) i, 1U);
        }

        if (step < 2U)
        {
            downloadPayloadOffset = APP_BOOTLOADER_SST26_DOWNLOAD_OFFSET
                                  + images[i].offset;

            lAPP_BOOTLOADER_InstallZone(downloadPayloadOffset,
                                        images[i].size,
                                        currentOffset, zoneSize,
                                        zoneMagic,
                                        alsoToInternalFlash);
            lAPP_BOOTLOADER_PersistInstallStep((uint8_t) i, 2U);
        }

        if (outNumInstalled != NULL)
        {
            *outNumInstalled = (uint8_t) (i + 1U);
        }
    }

    return true;
}

/*******************************************************************************
  Function:
    static void lAPP_BOOTLOADER_PersistRevertStep ( uint8_t imageIdx )

  Summary:
    Rewrites BOOT_MODE_INFO with REVERT_PENDING and the new imageIdx.

  Description:
    Revert is a single-phase operation per image (no backup), so only
    imageIdx is meaningful — imageStep is always written as 0. The
    install loop calls this after each image so a power cut resumes at
    the next image instead of reverting an already-reverted slot.
*/

static void lAPP_BOOTLOADER_PersistRevertStep(uint8_t imageIdx)
{
    APP_BOOTLOADER_BOOT_MODE_INFO info;

    info.magic     = APP_BOOTLOADER_BOOT_MODE_MAGIC;
    info.mode      = (uint8_t) APP_BOOTLOADER_BOOT_MODE_REVERT_PENDING;
    info.imageIdx  = imageIdx;
    info.imageStep = 0U;
    info.reserved  = 0U;
    info.modeXor   = (uint32_t) APP_BOOTLOADER_BOOT_MODE_REVERT_PENDING
                   ^ (APP_BOOTLOADER_BOOT_MODE_MAGIC & 0xFFU);

    DRV_BOOT_MODE_Write(&info);
}

/*******************************************************************************
  Function:
    static void lAPP_BOOTLOADER_RebootIntoUart ( void )

  Summary:
    Diverts a failed revert into the UART recovery path.

  Description:
    Called when REVERT_PENDING discovers that one of the REVERT zones is
    virgin or malformed — a state that should not occur in normal
    operation (every install backs CURRENT into REVERT before
    overwriting), and that the bootloader cannot self-resolve. Writes
    UART_PENDING into BOOT_MODE_INFO and triggers a system reset so the
    next boot enters the operator-driven recovery loop.
*/

static void lAPP_BOOTLOADER_RebootIntoUart(void)
{
    APP_BOOTLOADER_BOOT_MODE_INFO info;

    info.magic     = APP_BOOTLOADER_BOOT_MODE_MAGIC;
    info.mode      = (uint8_t) APP_BOOTLOADER_BOOT_MODE_UART_PENDING;
    info.imageIdx  = 0U;
    info.imageStep = 0U;
    info.reserved  = 0U;
    info.modeXor   = (uint32_t) APP_BOOTLOADER_BOOT_MODE_UART_PENDING
                   ^ (APP_BOOTLOADER_BOOT_MODE_MAGIC & 0xFFU);

    DRV_BOOT_MODE_Write(&info);

    NVIC_SystemReset();

    /* Reset is unconditional, but keep the compiler happy. */
    while (true) { /* unreachable */ }
}

/*******************************************************************************
  Function:
    static void lAPP_BOOTLOADER_RevertBundle (
        const APP_BOOTLOADER_BOOT_MODE_INFO *info )

  Summary:
    REVERT_PENDING handler — restores each image from its REVERT zone.

  Description:
    Re-parses the BUNDLE_HEADER in DOWNLOAD (which acts as the install
    log of the last operation). For each image, reads the matching
    REVERT zone header to learn how many bytes to roll back, then
    reuses lAPP_BOOTLOADER_InstallZone with source = REVERT payload
    area and destination = CURRENT.

    A virgin or malformed REVERT zone is the only anomaly that escapes
    this function: it diverts to UART_PENDING + reset and lets the
    operator recover.
*/

static void lAPP_BOOTLOADER_RevertBundle(const APP_BOOTLOADER_BOOT_MODE_INFO *info)
{
    APP_BOOTLOADER_BUNDLE_HEADER_FIXED header;
    APP_BOOTLOADER_BUNDLE_IMAGE images[APP_BOOTLOADER_BUNDLE_MAX_IMAGES];
    uint32_t i;
    uint32_t revertOffset;
    uint32_t currentOffset;
    uint32_t zoneSize;
    uint32_t expectedRevertMagic;
    uint32_t zoneMagic;
    bool     alsoToInternalFlash;
    uint32_t revertHeaderMagic;
    uint32_t revertHeaderSize;
    uint32_t maxPayload;

    if (lAPP_BOOTLOADER_ParseBundle(&header, images) == false)
    {
        /* A revert without a valid bundle log cannot be carried out
         * safely: bail to UART recovery so the operator can rebuild
         * the install state from scratch. */
        lAPP_BOOTLOADER_RebootIntoUart();
    }

    for (i = (uint32_t) info->imageIdx; i < header.numImages; i++)
    {
        if (images[i].typeMagic == APP_BOOTLOADER_TYPE_MAGIC_APP)
        {
            revertOffset        = APP_BOOTLOADER_SST26_APP_REVERT_OFFSET;
            currentOffset       = APP_BOOTLOADER_SST26_APP_CURRENT_OFFSET;
            zoneSize            = APP_BOOTLOADER_SST26_APP_CURRENT_SIZE;
            expectedRevertMagic = APP_BOOTLOADER_ZONE_MAGIC_APP_REVERT;
            zoneMagic           = APP_BOOTLOADER_ZONE_MAGIC_APP_CURRENT;
            alsoToInternalFlash = true;
        }
        else
        {
            revertOffset        = APP_BOOTLOADER_SST26_PL360_REVERT_OFFSET;
            currentOffset       = APP_BOOTLOADER_SST26_PL360_CURRENT_OFFSET;
            zoneSize            = APP_BOOTLOADER_SST26_PL360_CURRENT_SIZE;
            expectedRevertMagic = APP_BOOTLOADER_ZONE_MAGIC_PL360_REVERT;
            zoneMagic           = APP_BOOTLOADER_ZONE_MAGIC_PL360_CURRENT;
            alsoToInternalFlash = false;
        }

        /* Inspect REVERT header. The first 8 bytes are magic + size; the
         * rest of the page is irrelevant for the decision. */
        DRV_SST26_Read(revertOffset, (uint8_t *) gPageBuf,
                       APP_BOOTLOADER_ZONE_HEADER_SIZE);
        revertHeaderMagic = gPageBuf[0];
        revertHeaderSize  = gPageBuf[1];

        maxPayload = zoneSize - APP_BOOTLOADER_ZONE_HEADER_SIZE;

        if ((revertHeaderMagic != expectedRevertMagic) ||
            (revertHeaderSize == 0U) ||
            (revertHeaderSize > maxPayload))
        {
            /* Anomaly: nothing valid to roll back to. */
            lAPP_BOOTLOADER_RebootIntoUart();
        }

        lAPP_BOOTLOADER_InstallZone(revertOffset
                                    + APP_BOOTLOADER_ZONE_HEADER_SIZE,
                                    revertHeaderSize,
                                    currentOffset, zoneSize,
                                    zoneMagic,
                                    alsoToInternalFlash);

        /* Persist progress so a power cut does not re-run an already
         * completed image. The "+1" advances to the next slot; if i
         * was the last image, the loop simply exits next round. */
        lAPP_BOOTLOADER_PersistRevertStep((uint8_t) (i + 1U));
    }
}

// *****************************************************************************
// *****************************************************************************
// Section: UART Command Handlers
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    static void lAPP_BOOTLOADER_PrepareDownloadZone ( void )

  Summary:
    Erases the entire SST26 DOWNLOAD zone (512 KB).

  Description:
    Called once when the recovery loop starts. The convention shared
    with the modem application is that any new firmware-upload session
    begins with a fully erased DOWNLOAD region — REQ_WRITE then turns
    into pure page-program with no per-block tracking.
*/

static void lAPP_BOOTLOADER_PrepareDownloadZone(void)
{
    lAPP_BOOTLOADER_EraseSst26Zone(APP_BOOTLOADER_SST26_DOWNLOAD_OFFSET,
                                   APP_BOOTLOADER_SST26_DOWNLOAD_SIZE);
}

/*******************************************************************************
  Function:
    static void lAPP_BOOTLOADER_SendBootHello ( void )

  Summary:
    Emits a BOOT_HELLO frame (8 B payload: version + DOWNLOAD size).

  Description:
    Issued once on entry to the recovery loop, then every 2 s while no
    valid frame has been received yet. Lets the host discover that a
    bootloader is listening on the line at 115200 baud.
*/

static void lAPP_BOOTLOADER_SendBootHello(void)
{
    uint8_t  payload[8];
    uint32_t version;
    uint32_t dlSize;

    version = APP_BOOTLOADER_VERSION;
    dlSize  = APP_BOOTLOADER_SST26_DOWNLOAD_SIZE;

    payload[0] = (uint8_t) (version & 0xFFU);
    payload[1] = (uint8_t) ((version >> 8) & 0xFFU);
    payload[2] = (uint8_t) ((version >> 16) & 0xFFU);
    payload[3] = (uint8_t) ((version >> 24) & 0xFFU);
    payload[4] = (uint8_t) (dlSize & 0xFFU);
    payload[5] = (uint8_t) ((dlSize >> 8) & 0xFFU);
    payload[6] = (uint8_t) ((dlSize >> 16) & 0xFFU);
    payload[7] = (uint8_t) ((dlSize >> 24) & 0xFFU);

    lAPP_BOOTLOADER_UsiSendFrame(APP_BOOTLOADER_CMD_BOOT_HELLO,
                                 payload, sizeof(payload));
}

/*******************************************************************************
  Function:
    static void lAPP_BOOTLOADER_HandleReqInfo ( void )

  Summary:
    Replies to REQ_INFO with a 16-byte RSP_INFO payload.

  Description:
    Layout (all little-endian):
      [0..3]  version
      [4..7]  DOWNLOAD zone size (bytes)
      [8..11] SST26 page size (bytes — REQ_WRITE chunks must align)
      [12..15] SST26 erase granularity (bytes — informational)
*/

static void lAPP_BOOTLOADER_HandleReqInfo(void)
{
    uint8_t  payload[16];
    uint32_t fields[4];
    uint32_t i;
    uint32_t v;

    fields[0] = APP_BOOTLOADER_VERSION;
    fields[1] = APP_BOOTLOADER_SST26_DOWNLOAD_SIZE;
    fields[2] = APP_BOOTLOADER_SST26_PAGE_SIZE;
    fields[3] = APP_BOOTLOADER_SST26_SECTOR_SIZE;

    for (i = 0U; i < 4U; i++)
    {
        v = fields[i];
        payload[(i * 4U) + 0U] = (uint8_t) (v & 0xFFU);
        payload[(i * 4U) + 1U] = (uint8_t) ((v >> 8) & 0xFFU);
        payload[(i * 4U) + 2U] = (uint8_t) ((v >> 16) & 0xFFU);
        payload[(i * 4U) + 3U] = (uint8_t) ((v >> 24) & 0xFFU);
    }

    lAPP_BOOTLOADER_UsiSendFrame(APP_BOOTLOADER_CMD_RSP_INFO,
                                 payload, sizeof(payload));
}

/*******************************************************************************
  Function:
    static void lAPP_BOOTLOADER_HandleReqWrite (
        const uint8_t *args, uint16_t argLen )

  Summary:
    Validates and dispatches a REQ_WRITE.

  Description:
    Args layout: 4 B offset (LE) + 2 B len (LE) + len bytes of payload.

    The caller has already validated proto_id and CRC. argLen is the
    total length of the args field (excluding cmd byte). The handler
    cross-checks that argLen matches the announced len and that:
      - offset is page-aligned (SST26 page = 256 B)
      - len in [1..256]
      - offset + len fits inside the DOWNLOAD zone

    On success, programs one SST26 page at DOWNLOAD + offset (the
    DOWNLOAD zone has already been erased on entry to the recovery
    loop, so plain page-program is enough). Returns RSP_WRITE with the
    appropriate status code.
*/

static void lAPP_BOOTLOADER_HandleReqWrite(const uint8_t *args, uint16_t argLen)
{
    uint8_t  status;
    uint32_t offset;
    uint16_t len;

    if (argLen < 6U)
    {
        status = APP_BOOTLOADER_WRITE_ERR_RANGE;
    }
    else
    {
        offset = (uint32_t) args[0]
               | ((uint32_t) args[1] << 8)
               | ((uint32_t) args[2] << 16)
               | ((uint32_t) args[3] << 24);

        len = (uint16_t) args[4]
            | (uint16_t) ((uint16_t) args[5] << 8);

        if ((uint16_t) (argLen - 6U) != len)
        {
            status = APP_BOOTLOADER_WRITE_ERR_RANGE;
        }
        else if ((offset % APP_BOOTLOADER_SST26_PAGE_SIZE) != 0U)
        {
            status = APP_BOOTLOADER_WRITE_ERR_ALIGN;
        }
        else if ((len == 0U) ||
                 (len > APP_BOOTLOADER_SST26_PAGE_SIZE))
        {
            status = APP_BOOTLOADER_WRITE_ERR_RANGE;
        }
        else if ((offset + (uint32_t) len)
                 > APP_BOOTLOADER_SST26_DOWNLOAD_SIZE)
        {
            status = APP_BOOTLOADER_WRITE_ERR_RANGE;
        }
        else
        {
            /* Erase DOWNLOAD on the first write of the session. Keeps
             * the previous bundle intact for inspection up until the
             * operator actually starts a new flash. */
            if (gDownloadEraseDone == false)
            {
                lAPP_BOOTLOADER_PrepareDownloadZone();
                gDownloadEraseDone = true;
            }

            DRV_SST26_WritePage(APP_BOOTLOADER_SST26_DOWNLOAD_OFFSET + offset,
                                &args[6], len);
            status = APP_BOOTLOADER_WRITE_OK;
        }
    }

    lAPP_BOOTLOADER_UsiSendFrame(APP_BOOTLOADER_CMD_RSP_WRITE,
                                 &status, 1U);
}

/*******************************************************************************
  Function:
    static void lAPP_BOOTLOADER_HandleReqInstall ( void )

  Summary:
    Schedules the install on the next boot by writing INSTALL_PENDING
    into BOOT_MODE_INFO, ACKing the host, and triggering a reset.

  Description:
    The install itself runs through the standard INSTALL_PENDING
    dispatcher in APP_BOOTLOADER_Main -- the same code path that the
    application's PLC FU flow will trigger once block D5 lands. This
    keeps a single place where the install actually executes, which:

      - Unifies UART and PLC FU paths.
      - Lets a malformed bundle redirect to UART_PENDING via the
        existing RebootIntoUart anomaly handler (operator sees the
        device come back into UART recovery instead of into the app).
      - Preserves power-loss safety with no extra logic: the standard
        PersistInstallStep machinery owns it.

    Reply: RSP_INSTALL { status=OK, numInstalled=0 }. numInstalled is
    no longer meaningful here (the install has not run yet); kept at
    0 for wire compatibility with the existing host tool.
*/

static void lAPP_BOOTLOADER_HandleReqInstall(void)
{
    uint8_t reply[2];

    /* Persist INSTALL_PENDING(0,0) via the same primitive the install
     * loop uses for its progress points. After reset, the bootloader
     * dispatches the INSTALL_PENDING case from this fresh state. */
    lAPP_BOOTLOADER_PersistInstallStep(0U, 0U);

    reply[0] = APP_BOOTLOADER_INSTALL_OK;
    reply[1] = 0U;
    lAPP_BOOTLOADER_UsiSendFrame(APP_BOOTLOADER_CMD_RSP_INSTALL,
                                 reply, sizeof(reply));

    /* Give the SERCOM3 TX FIFO time to drain the RSP_INSTALL bytes
     * before the reset cuts the line. 5 ms covers ~57 bytes at
     * 115200 baud, well above the actual frame length. */
    lAPP_BOOTLOADER_DelayMs(5U);

    NVIC_SystemReset();

    while (true)
    {
        /* unreachable */
    }
}

/*******************************************************************************
  Function:
    static void lAPP_BOOTLOADER_HandleReqExit ( void )

  Summary:
    Clears BOOT_MODE_INFO to NORMAL and triggers a system reset.

  Description:
    Sends a best-effort RSP_EXIT before resetting; the byte may or may
    not propagate fully before the reset cuts the line, depending on
    the FIFO state. Either way the next boot reads NORMAL and jumps to
    the application.
*/

static void lAPP_BOOTLOADER_HandleReqExit(void)
{
    uint8_t status;

    status = 0x00U;
    lAPP_BOOTLOADER_UsiSendFrame(APP_BOOTLOADER_CMD_RSP_EXIT,
                                 &status, 1U);

    lAPP_BOOTLOADER_ClearBootMode();

    NVIC_SystemReset();

    while (true)
    {
        /* unreachable */
    }
}

/*******************************************************************************
  Function:
    static void lAPP_BOOTLOADER_HandleReqRead (
        const uint8_t *args, uint16_t argLen )

  Summary:
    Reads a range of the SST26 and returns it to the host.

  Description:
    Args layout: 4 B offset (LE) + 2 B len (LE).

    Validates len in [1..APP_BOOTLOADER_READ_MAX_LEN] and offset+len
    inside the SST26 capacity (8 MB). On success, fills gReadRspBuf
    with [status=OK, len_lo, len_hi, data...] and replies via
    RSP_READ. On failure replies with the appropriate status code
    and len = 0.

    Useful for inspecting BOOT_FLAG, ZONE_HEADERs, payload contents
    or the bundle in DOWNLOAD without a debugger -- the host tool
    can dump arbitrary regions and compare byte-by-byte against
    expected files.
*/

static void lAPP_BOOTLOADER_HandleReqRead(const uint8_t *args, uint16_t argLen)
{
    uint8_t  status;
    uint32_t offset;
    uint16_t len;

    len = 0U;

    if (argLen < 6U)
    {
        status = APP_BOOTLOADER_READ_ERR_LEN;
    }
    else
    {
        offset = (uint32_t) args[0]
               | ((uint32_t) args[1] << 8)
               | ((uint32_t) args[2] << 16)
               | ((uint32_t) args[3] << 24);

        len = (uint16_t) args[4]
            | (uint16_t) ((uint16_t) args[5] << 8);

        if ((len == 0U) || (len > APP_BOOTLOADER_READ_MAX_LEN))
        {
            status = APP_BOOTLOADER_READ_ERR_LEN;
            len = 0U;
        }
        else if ((offset + (uint32_t) len) > APP_BOOTLOADER_SST26_CAPACITY)
        {
            status = APP_BOOTLOADER_READ_ERR_RANGE;
            len = 0U;
        }
        else
        {
            DRV_SST26_Read(offset, &gReadRspBuf[3], (uint32_t) len);
            status = APP_BOOTLOADER_READ_OK;
        }
    }

    gReadRspBuf[0] = status;
    gReadRspBuf[1] = (uint8_t) (len & 0xFFU);
    gReadRspBuf[2] = (uint8_t) ((len >> 8) & 0xFFU);

    lAPP_BOOTLOADER_UsiSendFrame(APP_BOOTLOADER_CMD_RSP_READ,
                                 gReadRspBuf,
                                 (uint16_t) (3U + len));
}

/*******************************************************************************
  Function:
    static void lAPP_BOOTLOADER_HandleReqReadFlash (
        const uint8_t *args, uint16_t argLen )

  Summary:
    Reads a range of internal MCU flash and returns it to the host.

  Description:
    Args layout: 4 B offset (LE) + 2 B len (LE).

    Validates len in [1..APP_BOOTLOADER_READ_MAX_LEN] and
    [offset, offset+len) inside the application region
    [APP_BOOTLOADER_APP_START, APP_BOOTLOADER_APP_END). The bootloader
    region (0..0x1FFF) and the EEPROM emulation row (0x3FF00..0x3FFFF)
    are deliberately excluded from this command.

    On success, fills gReadRspBuf with [status=OK, len_lo, len_hi, data...]
    via a byte copy from the absolute address (internal flash is
    memory-mapped read-only on SAMD20) and replies via RSP_READ_FLASH.
    On failure replies with the appropriate status code and len = 0.

    Useful for verifying that an install correctly wrote the application
    image to internal flash by comparing the dump against the bundle
    payload byte by byte (or by computing CRC32 host-side).
*/

static void lAPP_BOOTLOADER_HandleReqReadFlash(const uint8_t *args,
                                               uint16_t argLen)
{
    uint8_t  status;
    uint32_t offset;
    uint16_t len;

    len = 0U;

    if (argLen < 6U)
    {
        status = APP_BOOTLOADER_READ_ERR_LEN;
    }
    else
    {
        offset = (uint32_t) args[0]
               | ((uint32_t) args[1] << 8)
               | ((uint32_t) args[2] << 16)
               | ((uint32_t) args[3] << 24);

        len = (uint16_t) args[4]
            | (uint16_t) ((uint16_t) args[5] << 8);

        if ((len == 0U) || (len > APP_BOOTLOADER_READ_MAX_LEN))
        {
            status = APP_BOOTLOADER_READ_ERR_LEN;
            len = 0U;
        }
        else if ((offset < APP_BOOTLOADER_APP_START) ||
                 ((offset + (uint32_t) len) > APP_BOOTLOADER_APP_END))
        {
            status = APP_BOOTLOADER_READ_ERR_RANGE;
            len = 0U;
        }
        else
        {
            const uint8_t *src;
            uint16_t       i;

            src = (const uint8_t *) offset;
            for (i = 0U; i < len; i++)
            {
                gReadRspBuf[3U + i] = src[i];
            }
            status = APP_BOOTLOADER_READ_OK;
        }
    }

    gReadRspBuf[0] = status;
    gReadRspBuf[1] = (uint8_t) (len & 0xFFU);
    gReadRspBuf[2] = (uint8_t) ((len >> 8) & 0xFFU);

    lAPP_BOOTLOADER_UsiSendFrame(APP_BOOTLOADER_CMD_RSP_READ_FLASH,
                                 gReadRspBuf,
                                 (uint16_t) (3U + len));
}

/*******************************************************************************
  Function:
    static void lAPP_BOOTLOADER_HandleReqEraseAll (
        const uint8_t *args, uint16_t argLen )

  Summary:
    Erases every SST26 zone defined by the v3 layout (DOWNLOAD,
    APP_CURRENT, APP_REVERT, PL360_CURRENT, PL360_REVERT, BOOT_FLAG)
    when the host sends the right magic word.

  Description:
    Args layout: 4 B magic (LE), must equal APP_BOOTLOADER_ERASE_ALL_MAGIC
    (0xDEADBEEF). This anti-accident gate prevents a stray frame from
    wiping the device.

    On a valid request, the handler erases all six zones in turn (total
    1.28 MB; ~1-2 s on a typical SST26 with 64 KB block erases),
    blinks the LED through the existing EraseSst26Zone routine, then
    replies with status = OK. On bad magic it replies immediately with
    ERR_BAD_MAGIC and does nothing.

    BOOT_FLAG goes last so a power loss mid-erase still leaves the
    handshake recoverable until the very last step. After OK, the host
    typically follows with REQ_EXIT or just resets the device.
*/

static void lAPP_BOOTLOADER_HandleReqEraseAll(const uint8_t *args,
                                              uint16_t argLen)
{
    uint8_t  status;
    uint32_t magic;

    if (argLen < 4U)
    {
        status = APP_BOOTLOADER_ERASE_ALL_ERR_BAD_MAGIC;
    }
    else
    {
        magic = (uint32_t) args[0]
              | ((uint32_t) args[1] << 8)
              | ((uint32_t) args[2] << 16)
              | ((uint32_t) args[3] << 24);

        if (magic != APP_BOOTLOADER_ERASE_ALL_MAGIC)
        {
            status = APP_BOOTLOADER_ERASE_ALL_ERR_BAD_MAGIC;
        }
        else
        {
            lAPP_BOOTLOADER_EraseSst26Zone(
                APP_BOOTLOADER_SST26_DOWNLOAD_OFFSET,
                APP_BOOTLOADER_SST26_DOWNLOAD_SIZE);
            lAPP_BOOTLOADER_EraseSst26Zone(
                APP_BOOTLOADER_SST26_APP_CURRENT_OFFSET,
                APP_BOOTLOADER_SST26_APP_CURRENT_SIZE);
            lAPP_BOOTLOADER_EraseSst26Zone(
                APP_BOOTLOADER_SST26_APP_REVERT_OFFSET,
                APP_BOOTLOADER_SST26_APP_REVERT_SIZE);
            lAPP_BOOTLOADER_EraseSst26Zone(
                APP_BOOTLOADER_SST26_PL360_CURRENT_OFFSET,
                APP_BOOTLOADER_SST26_PL360_CURRENT_SIZE);
            lAPP_BOOTLOADER_EraseSst26Zone(
                APP_BOOTLOADER_SST26_PL360_REVERT_OFFSET,
                APP_BOOTLOADER_SST26_PL360_REVERT_SIZE);

            /* BOOT_FLAG is one 4 KB sector, smaller than the 64 KB
             * granularity of EraseSst26Zone. Use the dedicated
             * sector-erase primitive. */
            DRV_SST26_SectorErase4K(APP_BOOTLOADER_SST26_BOOT_FLAG_OFFSET);

            status = APP_BOOTLOADER_ERASE_ALL_OK;
        }
    }

    lAPP_BOOTLOADER_UsiSendFrame(APP_BOOTLOADER_CMD_RSP_ERASE_ALL,
                                 &status, 1U);
}

/*******************************************************************************
  Function:
    static void lAPP_BOOTLOADER_UartRecovery ( void )

  Summary:
    UART_PENDING entry point — main recovery loop.

  Description:
    Initializes the UART, erases the DOWNLOAD zone, then enters a tight
    poll-driven loop:

      - Reads bytes from SERCOM3 RXC into the USI decoder. On every
        complete frame, dispatches the command (REQ_INFO/WRITE/INSTALL/
        EXIT) to its handler.
      - Emits BOOT_HELLO every ~2 s while no valid frame has been
        received yet. Stops once a host has attached.
      - Holds the LED at a steady 10 Hz blink whenever it is idle.
        Command handlers (Backup, Install, EraseSst26Zone, etc.) take
        over the LED while they run, with their own per-iteration
        toggle cadence -- that visual difference is the operator's cue
        that a command is in progress.

    Never returns: REQ_EXIT triggers a system reset; any other path
    keeps the loop running indefinitely so the operator can retry.
*/

static void lAPP_BOOTLOADER_UartRecovery(void)
{
    uint32_t ledCounter;
    uint32_t ledLimit;
    bool     ledIsOn;
    uint32_t helloCounter;
    bool     attached;
    uint8_t  rxByte;
    uint16_t frameLen;
    uint16_t argLen;
    uint8_t  cmd;

    DRV_UART_Initialize();

    /* Configure SysTick for a 10 ms tick. The loop below polls the
     * COUNTFLAG bit; each set→clear cycle is exactly one tick of real
     * time, independent of compiler optimization or how fast the loop
     * runs. */
    SysTick->LOAD = APP_BOOTLOADER_SYSTICK_RELOAD;
    SysTick->VAL  = 0U;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;

    /* Reset the "DOWNLOAD has been erased this session" latch. UART
     * entry alone does NOT erase DOWNLOAD any more -- only the first
     * REQ_WRITE in the session does (lAPP_BOOTLOADER_HandleReqWrite).
     * That preserves the previously-flashed bundle for inspection
     * (REQ_READ) while still guaranteeing a clean canvas at the start
     * of every new flash session. */
    gDownloadEraseDone = false;

    lAPP_BOOTLOADER_UsiRxReset();

    /* Send the first hello immediately so a host listening on the line
     * gets a prompt response without waiting the full 2 s period. */
    lAPP_BOOTLOADER_SendBootHello();

    attached     = false;
    ledCounter   = 0U;
    /* lAPP_BOOTLOADER_LedInit drove the pin HIGH (LED OFF, active-low),
     * so the next phase is the OFF phase -- we wait IDLE_OFF_TICKS
     * before the first toggle that turns the LED on. */
    ledIsOn      = false;
    ledLimit     = APP_BOOTLOADER_LED_IDLE_OFF_TICKS;
    helloCounter = 0U;

    while (true)
    {
        if (DRV_UART_RecvByteIfReady(&rxByte) == true)
        {
            if (lAPP_BOOTLOADER_UsiFeedByte(rxByte, &frameLen) == true)
            {
                /* Valid frame received: lock into the attached state.
                 * Stops emitting BOOT_HELLOs; LED stays at the same
                 * 10 Hz idle cadence so the operator's visual cue for
                 * "in bootloader" is identical with or without an
                 * attached host. Command handlers below take over the
                 * LED while they run. */
                attached     = true;
                helloCounter = 0U;

                if (frameLen >= 2U)
                {
                    cmd    = usiRxBuf[1];
                    argLen = (uint16_t) (frameLen - 2U);

                    switch (cmd)
                    {
                        case APP_BOOTLOADER_CMD_REQ_INFO:
                            lAPP_BOOTLOADER_HandleReqInfo();
                            break;

                        case APP_BOOTLOADER_CMD_REQ_WRITE:
                            lAPP_BOOTLOADER_HandleReqWrite(&usiRxBuf[2],
                                                           argLen);
                            break;

                        case APP_BOOTLOADER_CMD_REQ_INSTALL:
                            lAPP_BOOTLOADER_HandleReqInstall();
                            break;

                        case APP_BOOTLOADER_CMD_REQ_EXIT:
                            lAPP_BOOTLOADER_HandleReqExit();
                            /* never returns */
                            break;

                        case APP_BOOTLOADER_CMD_REQ_READ:
                            lAPP_BOOTLOADER_HandleReqRead(&usiRxBuf[2],
                                                          argLen);
                            break;

                        case APP_BOOTLOADER_CMD_REQ_READ_FLASH:
                            lAPP_BOOTLOADER_HandleReqReadFlash(&usiRxBuf[2],
                                                               argLen);
                            break;

                        case APP_BOOTLOADER_CMD_REQ_ERASE_ALL:
                            lAPP_BOOTLOADER_HandleReqEraseAll(&usiRxBuf[2],
                                                              argLen);
                            break;

                        default:
                            /* Unknown command — silently ignore. The
                             * host can detect timeouts and retry. */
                            break;
                    }
                }
            }
        }

        /* SysTick gate: counters tick once per 10 ms of real time.
         * Reading SysTick->CTRL self-clears COUNTFLAG, so this only
         * succeeds once per wrap. The rest of the loop body runs
         * many times per tick at whatever rate the build allows. */
        if ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == 0U)
        {
            continue;
        }

        /* BOOT_HELLO heartbeat — only while no host has shown up. */
        if (attached == false)
        {
            helloCounter++;
            if (helloCounter >= APP_BOOTLOADER_HELLO_TICKS)
            {
                lAPP_BOOTLOADER_SendBootHello();
                helloCounter = 0U;
            }
        }

        /* LED state machine. Symmetric 10 Hz blink whenever the
         * recovery loop is idle. While a command handler is running
         * (Install, Backup, EraseSst26Zone, ...) the inner loops own
         * the LED and toggle at their own per-iteration cadence; this
         * counter keeps incrementing meanwhile and just resyncs the
         * IDLE pattern once the handler returns. */
        ledCounter++;
        if (ledCounter >= ledLimit)
        {
            lAPP_BOOTLOADER_LedToggle();
            ledIsOn    = !ledIsOn;
            ledLimit   = ledIsOn ? APP_BOOTLOADER_LED_IDLE_ON_TICKS
                                 : APP_BOOTLOADER_LED_IDLE_OFF_TICKS;
            ledCounter = 0U;
        }
    }
}

// *****************************************************************************
// *****************************************************************************
// Section: Local Functions — Peripherals and Utility
// *****************************************************************************
// *****************************************************************************

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
    static bool lAPP_BOOTLOADER_Sw0Held ( void )

  Summary:
    Returns true if SW0 (PA15) is held LOW for the full debounce window.

  Description:
    Configures PA15 as a digital input with input synchronizer enabled
    and the internal pull-up engaged (the board has an external
    pull-up too, so PA15 reads HIGH when released and LOW when SW0 is
    pressed; the internal pull-up is harmless redundancy).

    Polls the pin every APP_BOOTLOADER_SW0_POLL_INTERVAL_MS for a total
    of APP_BOOTLOADER_SW0_DEBOUNCE_MS. As soon as a single sample reads
    HIGH the function returns false. Only an unbroken stretch of LOW
    samples is treated as a valid press.

    Used by APP_BOOTLOADER_Main as an emergency entry point into UART
    recovery when the application is broken and cannot run its own
    SW0-to-BOOT_MODE_INFO handler. The bootloader does NOT write
    UART_PENDING into BOOT_FLAG -- the recovery loop is entered
    in-place; any subsequent reset returns to the previous BOOT_MODE.
*/

static bool lAPP_BOOTLOADER_Sw0Held(void)
{
    uint32_t numSamples;
    uint32_t i;

    /* Configure PA15 as input with input enable and internal pull-up.
     * PINCFG.PULLEN + OUTSET selects pull-up; PULLEN + OUTCLR would
     * select pull-down. */
    PORT_REGS->GROUP[APP_BOOTLOADER_SW0_PORT_GRP].PORT_DIRCLR =
        APP_BOOTLOADER_SW0_MASK;
    PORT_REGS->GROUP[APP_BOOTLOADER_SW0_PORT_GRP].PORT_OUTSET =
        APP_BOOTLOADER_SW0_MASK;
    PORT_REGS->GROUP[APP_BOOTLOADER_SW0_PORT_GRP].
        PORT_PINCFG[APP_BOOTLOADER_SW0_PIN] =
            (uint8_t) (PORT_PINCFG_INEN_Msk | PORT_PINCFG_PULLEN_Msk);

    /* Allow the input synchronizer a couple of cycles before sampling. */
    lAPP_BOOTLOADER_DelayMs(1U);

    numSamples = APP_BOOTLOADER_SW0_DEBOUNCE_MS
               / APP_BOOTLOADER_SW0_POLL_INTERVAL_MS;
    for (i = 0U; i < numSamples; i++)
    {
        if ((PORT_REGS->GROUP[APP_BOOTLOADER_SW0_PORT_GRP].PORT_IN
             & APP_BOOTLOADER_SW0_MASK) != 0U)
        {
            return false;
        }
        lAPP_BOOTLOADER_DelayMs(APP_BOOTLOADER_SW0_POLL_INTERVAL_MS);
    }

    return true;
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
    Rough software delay; used only by the panic blink and the UART
    recovery placeholder.

  Description:
    At 8 MHz with -O1 the compiled inner loop takes roughly 4 cycles per
    iteration, so 2000 iterations approximate one millisecond. Accuracy
    is not important: the delay only paces the LED blink.
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
    The bootloader reaches this function on boot-mode requests it cannot
    yet honour (INSTALL_PENDING/REVERT_PENDING before B2/B3 land). A
    fast (~10 Hz) LED blink makes the state visually distinct from both
    a healthy bootloader run and a running application.
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
