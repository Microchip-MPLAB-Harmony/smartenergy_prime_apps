/*******************************************************************************
  PRIME PAL PLC PL360 Boot Streamer Source

  Company:
    Microchip Technology Inc.

  File Name:
    pal_plc_boot.c

  Summary:
    Synchronous SST26 reader exposed as a DRV_PLC_BOOT_DATA_CALLBACK.

  Description:
    See pal_plc_boot.h for the contract. This translation unit owns a
    dedicated DRV_MEMORY client and the streaming cursor used by the
    callback. It uses a static fragment buffer sized to the PLC boot
    driver's MAX_FRAG_SIZE (512 B).
*******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "pal_plc_boot.h"
#include "definitions.h"
#include "driver/memory/drv_memory.h"
#include "system/system_media.h"

// *****************************************************************************
// *****************************************************************************
// Section: Layout Constants — must mirror app_bootloader.h
// *****************************************************************************
// *****************************************************************************

/* PL360_CURRENT zone — bootloader v3 layout. */
#define PAL_PLC_BOOT_ZONE_OFFSET            (0x00100000UL)
#define PAL_PLC_BOOT_ZONE_SIZE              (0x00020000UL)   /* 128 KB */
#define PAL_PLC_BOOT_ZONE_HEADER_SIZE       (256U)
#define PAL_PLC_BOOT_ZONE_MAGIC             (0x43434C50UL)   /* 'PLCC' LE */

#define PAL_PLC_BOOT_PAGE_SIZE              (256U)

/* Maximum fragment the PLC boot driver requests per callback (matches
 * MAX_FRAG_SIZE inside drv_plc_boot.c). 2 SST26 pages. */
#define PAL_PLC_BOOT_FRAG_SIZE              (512U)

#define PAL_PLC_BOOT_MAX_PAYLOAD \
    (PAL_PLC_BOOT_ZONE_SIZE - PAL_PLC_BOOT_ZONE_HEADER_SIZE)

/* BOOT_FLAG sector (handshake with the bootloader). When the PL360_CURRENT
 * zone is unusable (factory-fresh device, corrupted image) we persist
 * UART_PENDING here and trigger a reset so the bootloader's UART
 * recovery picks up where the application cannot. Layout mirrors the
 * bootloader's APP_BOOTLOADER_BOOT_MODE_INFO struct (12 B). */
#define PAL_PLC_BOOT_FLAG_OFFSET            (0x00140000UL)
#define PAL_PLC_BOOT_FLAG_ERASE_SIZE        (4096U)
#define PAL_PLC_BOOT_MODE_MAGIC             (0x444F4D42UL)
#define PAL_PLC_BOOT_MODE_UART_PENDING      (0x03U)

// *****************************************************************************
// *****************************************************************************
// Section: File Scope Data
// *****************************************************************************
// *****************************************************************************

typedef enum
{
    PAL_PLC_BOOT_STATE_UNINITIALIZED = 0,
    PAL_PLC_BOOT_STATE_STREAMING,
    PAL_PLC_BOOT_STATE_DONE,
    PAL_PLC_BOOT_STATE_FAILED,
} PAL_PLC_BOOT_STATE;

/* Aligned fragment buffer — DRV_PLC_BOOT_CMD_WRITE_BUF reads from this
 * buffer over SPI when the callback returns. CACHE_ALIGN keeps it
 * 4-byte aligned without forcing a section attribute the bootloader
 * doesn't have. */
static CACHE_ALIGN uint8_t palPlcBootFragBuf[PAL_PLC_BOOT_FRAG_SIZE];

static DRV_HANDLE                  palPlcBootMemHandle = DRV_HANDLE_INVALID;
static DRV_MEMORY_COMMAND_HANDLE   palPlcBootCmdHandle = DRV_MEMORY_COMMAND_HANDLE_INVALID;
static volatile bool               palPlcBootXferDone;
static volatile bool               palPlcBootXferError;

static PAL_PLC_BOOT_STATE          palPlcBootState     = PAL_PLC_BOOT_STATE_UNINITIALIZED;
static uint32_t                    palPlcBootSrcOffset;     /* SST26 byte offset */
static uint32_t                    palPlcBootPendingBytes;  /* payload bytes left */

/* Read block size as reported by DRV_MEMORY at runtime. Harmony's
 * DRV_SST26 reports 1 (the device is byte-addressable for reads), but
 * the value is queried instead of hard-coded so the same code keeps
 * working if the driver ever switches to a page-based read API. */
static uint32_t                    palPlcBootReadBlockSize;

// *****************************************************************************
// *****************************************************************************
// Section: File Scope Functions
// *****************************************************************************
// *****************************************************************************

static void lPAL_PLC_BOOT_TransferHandler(DRV_MEMORY_EVENT event,
                                          DRV_MEMORY_COMMAND_HANDLE cmdHandle,
                                          uintptr_t context)
{
    (void) context;

    if (cmdHandle != palPlcBootCmdHandle)
    {
        return;
    }

    palPlcBootXferError = (event != DRV_MEMORY_EVENT_COMMAND_COMPLETE);
    palPlcBootXferDone  = true;
}

static bool lPAL_PLC_BOOT_SyncRead(void *dst, uint32_t blockStart, uint32_t nBlocks)
{
    palPlcBootXferDone  = false;
    palPlcBootXferError = false;

    DRV_MEMORY_AsyncRead(palPlcBootMemHandle, &palPlcBootCmdHandle,
                         dst, blockStart, nBlocks);

    if (palPlcBootCmdHandle == DRV_MEMORY_COMMAND_HANDLE_INVALID)
    {
        return false;
    }

    /* Pump DRV_MEMORY_Tasks until the transfer handler flips the flag.
     * The state machine drives DRV_SST26 underneath; SPI completion
     * arrives via SERCOM1 interrupt, so this loop never spins
     * forever as long as global interrupts are on. */
    while (palPlcBootXferDone == false)
    {
        DRV_MEMORY_Tasks(sysObj.drvMemory0);
    }

    return (palPlcBootXferError == false);
}

static void lPAL_PLC_BOOT_Close(void)
{
    if (palPlcBootMemHandle != DRV_HANDLE_INVALID)
    {
        DRV_MEMORY_Close(palPlcBootMemHandle);
        palPlcBootMemHandle = DRV_HANDLE_INVALID;
    }
}

static bool lPAL_PLC_BOOT_SyncErase(uint32_t blockStart, uint32_t nBlocks)
{
    palPlcBootXferDone  = false;
    palPlcBootXferError = false;

    DRV_MEMORY_AsyncErase(palPlcBootMemHandle, &palPlcBootCmdHandle,
                          blockStart, nBlocks);

    if (palPlcBootCmdHandle == DRV_MEMORY_COMMAND_HANDLE_INVALID)
    {
        return false;
    }

    while (palPlcBootXferDone == false)
    {
        DRV_MEMORY_Tasks(sysObj.drvMemory0);
    }

    return (palPlcBootXferError == false);
}

static bool lPAL_PLC_BOOT_SyncWrite(const void *src, uint32_t blockStart,
                                    uint32_t nBlocks)
{
    palPlcBootXferDone  = false;
    palPlcBootXferError = false;

    DRV_MEMORY_AsyncWrite(palPlcBootMemHandle, &palPlcBootCmdHandle,
                          (void *) src, blockStart, nBlocks);

    if (palPlcBootCmdHandle == DRV_MEMORY_COMMAND_HANDLE_INVALID)
    {
        return false;
    }

    while (palPlcBootXferDone == false)
    {
        DRV_MEMORY_Tasks(sysObj.drvMemory0);
    }

    return (palPlcBootXferError == false);
}

/* Persist BOOT_MODE_INFO with mode = UART_PENDING and trigger a reset.
 * Called when the PL360_CURRENT zone is virgin or otherwise unusable:
 * the application has nothing to boot the PL360 chip with and cannot
 * recover on its own, so it diverts to the bootloader's UART recovery
 * loop, where an operator can push a fresh PL360 binary. */
static void lPAL_PLC_BOOT_RebootIntoUart(void)
{
    /* Page-sized buffer: 12 B BOOT_MODE_INFO struct followed by 0xFF
     * padding so the surrounding page reads as erased flash if anything
     * later peeks at it. */
    static uint8_t  page[PAL_PLC_BOOT_PAGE_SIZE];
    uint32_t        eraseBlockStart;
    uint32_t        pageBlockStart;
    uint32_t        modeXor;

    if (palPlcBootMemHandle == DRV_HANDLE_INVALID)
    {
        return;
    }

    (void) memset(page, 0xFF, sizeof(page));

    /* Field layout matches APP_BOOTLOADER_BOOT_MODE_INFO. */
    page[0]  = (uint8_t) (PAL_PLC_BOOT_MODE_MAGIC        & 0xFFU);
    page[1]  = (uint8_t) ((PAL_PLC_BOOT_MODE_MAGIC >> 8) & 0xFFU);
    page[2]  = (uint8_t) ((PAL_PLC_BOOT_MODE_MAGIC >> 16) & 0xFFU);
    page[3]  = (uint8_t) ((PAL_PLC_BOOT_MODE_MAGIC >> 24) & 0xFFU);
    page[4]  = PAL_PLC_BOOT_MODE_UART_PENDING;
    page[5]  = 0U;     /* imageIdx  */
    page[6]  = 0U;     /* imageStep */
    page[7]  = 0U;     /* reserved  */

    modeXor = (uint32_t) PAL_PLC_BOOT_MODE_UART_PENDING
            ^ (PAL_PLC_BOOT_MODE_MAGIC & 0xFFU);
    page[8]  = (uint8_t) (modeXor       & 0xFFU);
    page[9]  = (uint8_t) ((modeXor >> 8) & 0xFFU);
    page[10] = (uint8_t) ((modeXor >> 16) & 0xFFU);
    page[11] = (uint8_t) ((modeXor >> 24) & 0xFFU);

    eraseBlockStart = PAL_PLC_BOOT_FLAG_OFFSET
                    / PAL_PLC_BOOT_FLAG_ERASE_SIZE;
    pageBlockStart  = PAL_PLC_BOOT_FLAG_OFFSET / PAL_PLC_BOOT_PAGE_SIZE;

    /* Best-effort. If either step fails the device just keeps trying
     * the same boot path on subsequent resets, which is no worse than
     * the current state. */
    if (lPAL_PLC_BOOT_SyncErase(eraseBlockStart, 1U) == true)
    {
        (void) lPAL_PLC_BOOT_SyncWrite(page, pageBlockStart, 1U);
    }

    NVIC_SystemReset();
}

static bool lPAL_PLC_BOOT_Open(void)
{
    SYS_MEDIA_GEOMETRY *geom;
    uint32_t            readBlockSize;
    uint32_t            headerBlockStart;
    uint32_t            headerNumBlocks;
    uint32_t            magic;
    uint32_t            size;

    palPlcBootMemHandle = DRV_MEMORY_Open(0U, DRV_IO_INTENT_READWRITE);
    if (palPlcBootMemHandle == DRV_HANDLE_INVALID)
    {
        return false;
    }

    DRV_MEMORY_TransferHandlerSet(palPlcBootMemHandle,
                                  lPAL_PLC_BOOT_TransferHandler, 0U);

    geom = DRV_MEMORY_GeometryGet(palPlcBootMemHandle);
    if (geom == NULL)
    {
        return false;
    }

    /* SST26 in Harmony reports read_blockSize = 1 (byte-addressable),
     * so blockStart and nBlocks for AsyncRead are both byte counts.
     * Use whatever the driver reports rather than hard-coding 256. */
    readBlockSize = geom->geometryTable[SYS_MEDIA_GEOMETRY_TABLE_READ_ENTRY].blockSize;
    if ((readBlockSize == 0U) ||
        (PAL_PLC_BOOT_ZONE_HEADER_SIZE % readBlockSize != 0U) ||
        (PAL_PLC_BOOT_ZONE_OFFSET % readBlockSize != 0U))
    {
        return false;
    }
    palPlcBootReadBlockSize = readBlockSize;

    /* Read the 256-byte ZONE_HEADER. We only consume the first 8 B
     * (magic + size) but pull the whole header into the fragment
     * buffer to keep things uniform. */
    headerBlockStart = PAL_PLC_BOOT_ZONE_OFFSET / readBlockSize;
    headerNumBlocks  = PAL_PLC_BOOT_ZONE_HEADER_SIZE / readBlockSize;

    if (lPAL_PLC_BOOT_SyncRead(palPlcBootFragBuf,
                               headerBlockStart, headerNumBlocks) == false)
    {
        return false;
    }

    (void) memcpy(&magic, &palPlcBootFragBuf[0], sizeof(magic));
    (void) memcpy(&size,  &palPlcBootFragBuf[4], sizeof(size));

    if (magic != PAL_PLC_BOOT_ZONE_MAGIC)
    {
        return false;
    }

    if ((size == 0U) || (size > PAL_PLC_BOOT_MAX_PAYLOAD))
    {
        return false;
    }

    palPlcBootSrcOffset    = PAL_PLC_BOOT_ZONE_OFFSET
                           + PAL_PLC_BOOT_ZONE_HEADER_SIZE;
    palPlcBootPendingBytes = size;
    return true;
}

// *****************************************************************************
// *****************************************************************************
// Section: Public Functions
// *****************************************************************************
// *****************************************************************************

void PAL_PLC_BOOT_DataCallback(uint32_t *address, uint16_t *length,
                               uintptr_t context)
{
    uint32_t chunk;
    uint32_t alignedChunk;
    uint32_t pageBlockStart;
    uint32_t blocksNeeded;

    (void) context;

    if (address == NULL)
    {
        if (length != NULL)
        {
            *length = 0U;
        }
        return;
    }

    if (length == NULL)
    {
        return;
    }

    /* Lazy initialization: open the DRV_MEMORY client and read the
     * ZONE_HEADER on the first call. Any failure transitions to
     * FAILED, which causes every subsequent call to return *length=0
     * (the PLC boot driver then aborts cleanly). */
    if (palPlcBootState == PAL_PLC_BOOT_STATE_UNINITIALIZED)
    {
        if (lPAL_PLC_BOOT_Open() == true)
        {
            palPlcBootState = PAL_PLC_BOOT_STATE_STREAMING;
        }
        else
        {
            /* No usable PL360 image in SST26 (factory-fresh device or
             * corrupted CURRENT zone). Bail out of the modem app into
             * the bootloader's UART recovery so an operator can push a
             * fresh image without bricking the device. Returns only if
             * the persistence step failed; in that case fall through
             * to the inert FAILED state. */
            lPAL_PLC_BOOT_RebootIntoUart();
            palPlcBootState = PAL_PLC_BOOT_STATE_FAILED;
            lPAL_PLC_BOOT_Close();
        }
    }

    if ((palPlcBootState != PAL_PLC_BOOT_STATE_STREAMING) ||
        (palPlcBootPendingBytes == 0U))
    {
        if (palPlcBootState == PAL_PLC_BOOT_STATE_STREAMING)
        {
            palPlcBootState = PAL_PLC_BOOT_STATE_DONE;
        }
        lPAL_PLC_BOOT_Close();
        *length = 0U;
        return;
    }

    /* Compute the next chunk size. The DRV_MEMORY API works in
     * read_blockSize units; SST26 reports 1 (byte-addressable). For
     * the last (possibly partial) chunk we may need to round the
     * block count up — the trailing bytes that are not part of the
     * payload land in the buffer but are never reported to the PLC
     * driver, only `chunk` bytes are. The SST26 backing zone is left
     * erased past the payload by the bootloader, so those trailing
     * bytes read as 0xFF and are harmless. */
    chunk = (palPlcBootPendingBytes > PAL_PLC_BOOT_FRAG_SIZE)
          ? PAL_PLC_BOOT_FRAG_SIZE
          : palPlcBootPendingBytes;

    /* Pad the fragment up to a 4-byte multiple. The PLC chip's WRITE_BUF
     * command expects word-aligned sizes -- the driver's internal-flash
     * path does the same trick (drv_plc_boot.c "padding = 4 - frag%4").
     * Without this the last partial chunk (e.g. 366 B for a 69998 B
     * binary) lands on the chip with a stray 1-3 byte tail unwritten in
     * the final word, the firmware never executes cleanly and the chip
     * never asserts EXT_INT -> STARTINGUP loops forever. The extra
     * padding bytes come from the SST26 zone past the payload, which the
     * bootloader leaves erased (0xFF), so they are harmless filler. */
    alignedChunk = (chunk + 3U) & ~3U;

    blocksNeeded   = (alignedChunk + palPlcBootReadBlockSize - 1U)
                   / palPlcBootReadBlockSize;
    pageBlockStart = palPlcBootSrcOffset / palPlcBootReadBlockSize;

    /* Buffer-overrun guard: alignedChunk must fit in palPlcBootFragBuf.
     * FRAG_SIZE is itself a multiple of 4, so alignedChunk <= FRAG_SIZE
     * always holds when chunk <= FRAG_SIZE. The check is kept for
     * defence in depth. */
    if ((blocksNeeded * palPlcBootReadBlockSize) > PAL_PLC_BOOT_FRAG_SIZE)
    {
        palPlcBootState = PAL_PLC_BOOT_STATE_FAILED;
        lPAL_PLC_BOOT_Close();
        *length = 0U;
        return;
    }

    if (lPAL_PLC_BOOT_SyncRead(palPlcBootFragBuf,
                               pageBlockStart, blocksNeeded) == false)
    {
        palPlcBootState = PAL_PLC_BOOT_STATE_FAILED;
        lPAL_PLC_BOOT_Close();
        *length = 0U;
        return;
    }

    *address = (uint32_t) palPlcBootFragBuf;
    *length  = (uint16_t) alignedChunk;       /* report the padded size */

    /* Advance the source cursor by the padded amount so it stays aligned
     * with the chip's PROGRAM_ADDR (driver advances pDst by `fragSize`).
     * Pending bytes is the binary-byte counter and decrements by `chunk`
     * only; on the final partial chunk it reaches zero and the next
     * callback returns *length = 0. */
    palPlcBootSrcOffset    += alignedChunk;
    palPlcBootPendingBytes  = (palPlcBootPendingBytes > chunk)
                            ? (palPlcBootPendingBytes - chunk)
                            : 0U;
}

/*******************************************************************************
 End of File
*/
