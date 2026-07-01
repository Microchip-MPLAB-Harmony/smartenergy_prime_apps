/*******************************************************************************
  PRIME PAL PLC PL360 Boot Streamer Source

  Company:
    Microchip Technology Inc.

  File Name:
    pal_plc_boot.c

  Summary:
    Streams the PL360 firmware image to the PLC PHY driver from external memory.

  Description:
    Implements the boot-data callback that lets the PLC PHY boot driver load the
    PL360 firmware image fragment by fragment, each fragment served from
    external memory. It owns a dedicated DRV_MEMORY client and the streaming
    state used by PAL_PLC_BOOT_DataCallback, with a static fragment buffer sized
    to the PLC boot driver's maximum fragment (512 bytes).
*******************************************************************************/
//DOM-IGNORE-BEGIN
/*
Copyright (C) 2026, Microchip Technology Inc., and its subsidiaries. All rights reserved.

The software and documentation is provided by microchip and its contributors
"as is" and any express, implied or statutory warranties, including, but not
limited to, the implied warranties of merchantability, fitness for a particular
purpose and non-infringement of third party intellectual property rights are
disclaimed to the fullest extent permitted by law. In no event shall microchip
or its contributors be liable for any direct, indirect, incidental, special,
exemplary, or consequential damages (including, but not limited to, procurement
of substitute goods or services; loss of use, data, or profits; or business
interruption) however caused and on any theory of liability, whether in contract,
strict liability, or tort (including negligence or otherwise) arising in any way
out of the use of the software and documentation, even if advised of the
possibility of such damage.

Except as expressly permitted hereunder and subject to the applicable license terms
for any third-party software incorporated in the software and any applicable open
source software license terms, no license or other rights, whether express or
implied, are granted under any patent or other intellectual property rights of
Microchip or any third party.
*/
//DOM-IGNORE-END


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
#include "system/system_media.h"

// *****************************************************************************
// *****************************************************************************
// Section: Macro Definitions
// *****************************************************************************
// *****************************************************************************

/* The layout constants below must mirror the bootloader. */

/* PL360_CURRENT zone - bootloader layout. */
#define PAL_PLC_BOOT_ZONE_OFFSET            (0x00100000UL)
#define PAL_PLC_BOOT_ZONE_SIZE              (0x00020000UL)   /* 128 KB */
#define PAL_PLC_BOOT_ZONE_HEADER_SIZE       (256U)
#define PAL_PLC_BOOT_ZONE_MAGIC             (0x43434C50UL)   /* zone magic = ASCII "PLCC", little-endian */

#define PAL_PLC_BOOT_PAGE_SIZE              (256U)

/* Maximum fragment the PLC boot driver requests per callback. */
#define PAL_PLC_BOOT_FRAG_SIZE              (512U)

#define PAL_PLC_BOOT_MAX_PAYLOAD \
    (PAL_PLC_BOOT_ZONE_SIZE - PAL_PLC_BOOT_ZONE_HEADER_SIZE)

/* BOOT_FLAG sector (handshake with the bootloader). */
#define PAL_PLC_BOOT_FLAG_OFFSET            (0x00140000UL)
#define PAL_PLC_BOOT_FLAG_ERASE_SIZE        (4096U)
#define PAL_PLC_BOOT_MODE_MAGIC             (0x444F4D42UL)
#define PAL_PLC_BOOT_MODE_UART_PENDING      (0x03U)

// *****************************************************************************
// *****************************************************************************
// Section: File Scope Data
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* PLC boot streamer state

  Summary:
    State of the external-memory firmware-streaming sequence.

  Description:
    Tracks the progress of PAL_PLC_BOOT_DataCallback across its successive
    invocations: from the initial open, through streaming, to completion
    or failure.

  Remarks:
    None.
*/
typedef enum
{
    PAL_PLC_BOOT_STATE_UNINITIALIZED = 0,
    PAL_PLC_BOOT_STATE_STREAMING,
    PAL_PLC_BOOT_STATE_DONE,
    PAL_PLC_BOOT_STATE_FAILED,
} PAL_PLC_BOOT_STATE;

static CACHE_ALIGN uint8_t palPlcBootFragBuf[PAL_PLC_BOOT_FRAG_SIZE];

static DRV_HANDLE                  palPlcBootMemHandle = DRV_HANDLE_INVALID;
static DRV_MEMORY_COMMAND_HANDLE   palPlcBootCmdHandle = DRV_MEMORY_COMMAND_HANDLE_INVALID;
static volatile bool               palPlcBootTransferDone;
static volatile bool               palPlcBootTransferError;

static PAL_PLC_BOOT_STATE          palPlcBootState     = PAL_PLC_BOOT_STATE_UNINITIALIZED;
static uint32_t                    palPlcBootSrcOffset;     /* external memory byte offset */
static uint32_t                    palPlcBootPendingBytes;  /* payload bytes left */

/* Read block size as reported by DRV_MEMORY at runtime. */
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

    palPlcBootTransferError = (event != DRV_MEMORY_EVENT_COMMAND_COMPLETE);
    palPlcBootTransferDone  = true;
}

static bool lPAL_PLC_BOOT_SyncRead(void *dst, uint32_t blockStart, uint32_t nBlocks)
{
    palPlcBootTransferDone  = false;
    palPlcBootTransferError = false;

    DRV_MEMORY_AsyncRead(palPlcBootMemHandle, &palPlcBootCmdHandle,
                         dst, blockStart, nBlocks);

    if (palPlcBootCmdHandle == DRV_MEMORY_COMMAND_HANDLE_INVALID)
    {
        return false;
    }

    while (palPlcBootTransferDone == false)
    {
        DRV_MEMORY_Tasks(sysObj.drvMemory0);
    }

    return (palPlcBootTransferError == false);
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
    palPlcBootTransferDone  = false;
    palPlcBootTransferError = false;

    DRV_MEMORY_AsyncErase(palPlcBootMemHandle, &palPlcBootCmdHandle,
                          blockStart, nBlocks);

    if (palPlcBootCmdHandle == DRV_MEMORY_COMMAND_HANDLE_INVALID)
    {
        return false;
    }

    while (palPlcBootTransferDone == false)
    {
        DRV_MEMORY_Tasks(sysObj.drvMemory0);
    }

    return (palPlcBootTransferError == false);
}

static bool lPAL_PLC_BOOT_SyncWrite(const void *src, uint32_t blockStart,
                                    uint32_t nBlocks)
{
    palPlcBootTransferDone  = false;
    palPlcBootTransferError = false;

    DRV_MEMORY_AsyncWrite(palPlcBootMemHandle, &palPlcBootCmdHandle,
                          (void *) src, blockStart, nBlocks);

    if (palPlcBootCmdHandle == DRV_MEMORY_COMMAND_HANDLE_INVALID)
    {
        return false;
    }

    while (palPlcBootTransferDone == false)
    {
        DRV_MEMORY_Tasks(sysObj.drvMemory0);
    }

    return (palPlcBootTransferError == false);
}

/* Persist BOOT_MODE_INFO with mode = UART_PENDING and trigger a reset. */
static void lPAL_PLC_BOOT_RebootIntoUart(void)
{
    /* Page-sized buffer: the 12-byte boot-mode record (mirrors the bootloader
     * struct) followed by 0xFF padding. */
    static uint8_t  page[PAL_PLC_BOOT_PAGE_SIZE];
    uint32_t        eraseBlockStart;
    uint32_t        pageBlockStart;
    uint32_t        modeXor;

    if (palPlcBootMemHandle == DRV_HANDLE_INVALID)
    {
        return;
    }

    (void) memset(page, 0xFF, sizeof(page));

    /* Field layout matches the bootloader boot-mode record. */
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

    readBlockSize = geom->geometryTable[SYS_MEDIA_GEOMETRY_TABLE_READ_ENTRY].blockSize;
    if ((readBlockSize == 0U) ||
        (PAL_PLC_BOOT_ZONE_HEADER_SIZE % readBlockSize != 0U) ||
        (PAL_PLC_BOOT_ZONE_OFFSET % readBlockSize != 0U))
    {
        return false;
    }
    palPlcBootReadBlockSize = readBlockSize;

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

    if (palPlcBootState == PAL_PLC_BOOT_STATE_UNINITIALIZED)
    {
        if (lPAL_PLC_BOOT_Open() == true)
        {
            palPlcBootState = PAL_PLC_BOOT_STATE_STREAMING;
        }
        else
        {
            /* No usable PL360 image in external memory (factory-fresh device or
             * corrupted PL360_CURRENT zone). */
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

    /* Compute the next chunk size. */
    chunk = (palPlcBootPendingBytes > PAL_PLC_BOOT_FRAG_SIZE)
          ? PAL_PLC_BOOT_FRAG_SIZE
          : palPlcBootPendingBytes;

    /* Pad the fragment up to a 4-byte multiple. */
    alignedChunk = (chunk + 3U) & ~3U;

    blocksNeeded   = (alignedChunk + palPlcBootReadBlockSize - 1U)
                   / palPlcBootReadBlockSize;
    pageBlockStart = palPlcBootSrcOffset / palPlcBootReadBlockSize;

    /* Buffer-overrun guard. */
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

    palPlcBootSrcOffset    += alignedChunk;
    palPlcBootPendingBytes  = (palPlcBootPendingBytes > chunk)
                            ? (palPlcBootPendingBytes - chunk)
                            : 0U;
}

/*******************************************************************************
 End of File
*/
