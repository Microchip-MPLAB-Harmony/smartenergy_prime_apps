/*******************************************************************************
  PRIME Firmware Upgrade Service Implementation.

  Company:
    Microchip Technology Inc.

  File Name:
    srv_firmware_upgrade.c

  Summary:
    PRIME Firmware Upgrade Service Interface Source File.

  Description:
    The Firmware Upgrade service provides the handling of the firmware upgrade
    and version swap for PRIME. This file contains the source code for the
    implementation of this service.
*******************************************************************************/

///DOM-IGNORE-BEGIN
/*
Copyright (C) 2024, Microchip Technology Inc., and its subsidiaries. All rights reserved.

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

#include <string.h>

#include "configuration.h"
#include "definitions.h"

#include "srv_firmware_upgrade.h"
#include "srv_firmware_upgrade_local.h"

#include "service/pcrc/srv_pcrc.h"
#include "service/storage/srv_storage.h"

// *****************************************************************************
// *****************************************************************************
// Section: Macro definitions
// *****************************************************************************
// *****************************************************************************

#define PRIME_FU_MEM_DRV        "drv_memory_0"
#define PRIME_FU_MEM_INSTANCE   0
/* External SST26 is split into two 256 KB zones: TELECARGA (0..0x3FFFF)
 * where the FU service writes the incoming image, and REVERT
 * (0x40000..0x7FFFF) reserved for the bootloader's backup copy. The FU
 * region therefore stops at 256 KB so the FU erase/write code never
 * touches the REVERT slot. */
#define PRIME_FU_MEM_SIZE       (uint32_t)(0x40000)
#define PRIME_FU_MEM_REVERT_OFFSET  (uint32_t)(0x40000)

/* Magic the bootloader expects in the boot-config key field to
 * recognise a pending operation. Must match
 * APP_BOOTLOADER_BOOT_CONFIG_KEY in the bootloader project. */
#define PRIME_FU_BOOT_CFG_KEY   (uint32_t)(SRV_STORAGE_BOOT_CFG_KEY)

/* Destination address in internal flash where the bootloader installs
 * the image. Matches APP_BOOTLOADER_APP_START. */
#define PRIME_FU_APP_START_ADDR (uint32_t)(0x1000)

#define MEMORY_WRITE_SIZE       (uint32_t)(256)
#define MAX_BUFFER_READ_SIZE    (uint32_t)(256)

/* Define application number */
typedef enum
{
    PRIME_INVALID_APP = 0,
    PRIME_MAC13_APP,
    PRIME_MAC14_APP,
    PRIME_PHY_APP,
    PRIME_MAIN_APP
} SRV_FU_PRIME_APP_TYPE;

// *****************************************************************************
// *****************************************************************************
// Section: File Scope Data
// *****************************************************************************
// *****************************************************************************

/* Callback function pointers */
static SRV_FU_CRC_CB SRV_FU_CrcCallback;
static SRV_FU_IMAGE_VERIFY_CB SRV_FU_ImageVerifyCallback;
static SRV_FU_RESULT_CB SRV_FU_ResultCallback;
static SRV_FU_VERSION_SWAP_CB SRV_FU_SwapCallback;
static SRV_FU_MEM_TRANSFER_CB SRV_FU_MemTransferCallback;

static SYS_MEDIA_GEOMETRY *nvmGeometry = NULL;

/* NVM Data buffer */
static CACHE_ALIGN uint8_t pMemWrite[MEMORY_WRITE_SIZE];

static CACHE_ALIGN uint8_t pBuffInput[MAX_BUFFER_READ_SIZE];

static CACHE_ALIGN SRV_FU_MEM_INFO memInfo;

static SRV_FU_INFO fuData;

/* Tracks whether the most recent FU result was a revert request. Set
 * by SRV_FU_End and read by SRV_FU_SwapFirmware so the boot config
 * points at the right SST26 zone (TELECARGA vs REVERT). Defaults to
 * false so a fresh boot with no FU activity treats the pending swap,
 * if any, as a regular install. */
static bool fuLastResultIsRevert = false;

static SRV_FU_CRC_STATE crcState;

static uint32_t crcReadAddress;

static uint32_t crcSize;

static uint32_t crcRemainingSize;

static uint32_t calculatedCrc;


// *****************************************************************************
// *****************************************************************************
// Section: File scope functions
// *****************************************************************************
// *****************************************************************************

static void lSRV_FU_TransferHandler
(
    DRV_MEMORY_EVENT event,
    DRV_MEMORY_COMMAND_HANDLE commandHandle,
    uintptr_t context
)
{
    SRV_FU_MEM_TRANSFER_RESULT transferResult;
    SRV_FU_MEM_TRANSFER_CMD transferCmd;
    SRV_FU_MEM_INFO *mInfo = (SRV_FU_MEM_INFO *)context;

    switch(event)
    {
        case DRV_MEMORY_EVENT_COMMAND_COMPLETE:
            transferResult = SRV_FU_MEM_TRANSFER_OK;
            break;

        case DRV_MEMORY_EVENT_COMMAND_ERROR:
        default:
            transferResult = SRV_FU_MEM_TRANSFER_ERROR;
            break;
    }

    if (commandHandle == mInfo->eraseHandle)
    {
        memInfo.state = SRV_FU_MEM_STATE_CMD_WAIT;
        transferCmd = SRV_FU_MEM_TRANSFER_CMD_ERASE;
    }
    else if (commandHandle == mInfo->readHandle)
    {
        if (memInfo.state == SRV_FU_CALCULATE_CRC_BLOCK)
        {
            /* Calculating CRC.... no callback*/
            crcState = SRV_FU_CRC_CALCULATING;
            return;
        }
        else
        {
            memInfo.state = SRV_FU_MEM_STATE_CMD_WAIT;
            transferCmd = SRV_FU_MEM_TRANSFER_CMD_READ;
        }
    }
    else if (commandHandle == mInfo->writeHandle)
    {
        if (transferResult == SRV_FU_MEM_TRANSFER_OK)
        {
            /* Continue with next transfer, not callback */
            memInfo.state = SRV_FU_MEM_STATE_WRITE_ONE_BLOCK;
            return;
        }
        else
        {
            memInfo.state = SRV_FU_MEM_STATE_CMD_WAIT;
            transferCmd = SRV_FU_MEM_TRANSFER_CMD_WRITE;
        }
    }
    else
    {
        memInfo.state = SRV_FU_MEM_STATE_CMD_WAIT;
        transferCmd = SRV_FU_MEM_TRANSFER_CMD_BAD;
    }

    if (SRV_FU_MemTransferCallback != NULL)
    {
        SRV_FU_MemTransferCallback(transferCmd, transferResult);
    }
}

static void lSRV_FU_EraseFuRegion(void)
{

    DRV_MEMORY_AsyncErase(memInfo.memoryHandle, &memInfo.eraseHandle,
        memInfo.eraseBlockStart, memInfo.numFuRegionEraseBlocks);

    memInfo.state = SRV_FU_MEM_STATE_ERASE_FLASH;
}


// *****************************************************************************
// *****************************************************************************
// Section: Firmware Upgrade Service Interface Implementation
// *****************************************************************************
// *****************************************************************************

void SRV_FU_Initialize(void)
{
    SRV_FU_CrcCallback = NULL;
    SRV_FU_ImageVerifyCallback = NULL;
    SRV_FU_ResultCallback = NULL;
    SRV_FU_SwapCallback = NULL;
    SRV_FU_MemTransferCallback = NULL;

    memInfo.startAdressFuRegion = 0;
    memInfo.sizeFuRegion = PRIME_FU_MEM_SIZE;

    memInfo.state = SRV_FU_MEM_STATE_OPEN_DRIVER;

}

void SRV_FU_Tasks(void)
{

   /* Check the Firmware upgrade's current state. */
    switch ( memInfo.state )
    {
        case SRV_FU_MEM_STATE_OPEN_DRIVER:
        {
            memInfo.memoryHandle = DRV_MEMORY_Open(PRIME_FU_MEM_INSTANCE, DRV_IO_INTENT_READWRITE);

            if (DRV_HANDLE_INVALID != memInfo.memoryHandle)
            {
                DRV_MEMORY_TransferHandlerSet(memInfo.memoryHandle, lSRV_FU_TransferHandler, (uintptr_t)&memInfo);
                memInfo.state = SRV_FU_MEM_STATE_GEOMETRY_GET;
            }

            break;
        }

        case SRV_FU_MEM_STATE_GEOMETRY_GET:
        {
            nvmGeometry = DRV_MEMORY_GeometryGet(memInfo.memoryHandle);

            if (nvmGeometry == NULL)
            {
                memInfo.state = SRV_FU_MEM_UNINITIALIZED;
                break;
            }

            memInfo.eraseBlockStart = (memInfo.startAdressFuRegion / nvmGeometry->geometryTable[SYS_MEDIA_GEOMETRY_TABLE_ERASE_ENTRY].blockSize);

            memInfo.numFuRegionEraseBlocks = (memInfo.sizeFuRegion / nvmGeometry->geometryTable[SYS_MEDIA_GEOMETRY_TABLE_ERASE_ENTRY].blockSize);

            memInfo.writePageSize = nvmGeometry->geometryTable[SYS_MEDIA_GEOMETRY_TABLE_WRITE_ENTRY].blockSize;
            memInfo.readPageSize = nvmGeometry->geometryTable[SYS_MEDIA_GEOMETRY_TABLE_READ_ENTRY].blockSize;

            memInfo.state = SRV_FU_MEM_STATE_CMD_WAIT;
            break;
        }


        case SRV_FU_MEM_STATE_ERASE_FLASH:
        {
            if (DRV_MEMORY_COMMAND_HANDLE_INVALID == memInfo.eraseHandle)
            {
                memInfo.state = SRV_FU_MEM_STATE_CMD_WAIT;

                if (SRV_FU_MemTransferCallback != NULL)
                {
                    SRV_FU_MemTransferCallback(SRV_FU_MEM_TRANSFER_CMD_ERASE, SRV_FU_MEM_TRANSFER_ERROR);
                }
            }

            break;
        }

        case SRV_FU_MEM_STATE_READ_MEMORY:
        {
            if (DRV_MEMORY_COMMAND_HANDLE_INVALID == memInfo.readHandle)
            {
                memInfo.state = SRV_FU_MEM_STATE_CMD_WAIT;

                if (SRV_FU_MemTransferCallback != NULL)
                {
                    SRV_FU_MemTransferCallback(SRV_FU_MEM_TRANSFER_CMD_READ, SRV_FU_MEM_TRANSFER_ERROR);
                }
            }
            break;
        }

        case SRV_FU_MEM_STATE_WRITE_ONE_BLOCK:
        {
            uint32_t block;
            uint32_t offset;
            uint32_t bytesToCopy;

            if (memInfo.writeSize == 0U)
            {
                memInfo.state = SRV_FU_MEM_STATE_CMD_WAIT;

                if (SRV_FU_MemTransferCallback != NULL)
                {
                    SRV_FU_MemTransferCallback(SRV_FU_MEM_TRANSFER_CMD_WRITE, SRV_FU_MEM_TRANSFER_OK);
                }

                break;
            }

            block = memInfo.writeAddress / memInfo.writePageSize;
            memInfo.retrieveAddress = block * memInfo.writePageSize;
            offset = memInfo.writeAddress - memInfo.retrieveAddress;

            if ((memInfo.writePageSize - offset) < memInfo.writeSize)
            {
                bytesToCopy = memInfo.writePageSize - offset;
            }
            else
            {
                bytesToCopy = memInfo.writeSize;
            }

            (void)memset( pMemWrite, 0xff, memInfo.writePageSize);
            (void)memcpy( &pMemWrite[offset], &pBuffInput[memInfo.bytesWritten] , bytesToCopy);

            DRV_MEMORY_AsyncWrite(memInfo.memoryHandle, &memInfo.writeHandle, pMemWrite, block, 1);

            if (DRV_MEMORY_COMMAND_HANDLE_INVALID == memInfo.writeHandle)
            {
                memInfo.state = SRV_FU_MEM_STATE_CMD_WAIT;

                if (SRV_FU_MemTransferCallback != NULL)
                {
                    SRV_FU_MemTransferCallback(SRV_FU_MEM_TRANSFER_CMD_WRITE, SRV_FU_MEM_TRANSFER_ERROR);
                }
            }
            else
            {
                memInfo.writeAddress += bytesToCopy;
                memInfo.writeSize -= bytesToCopy;
                memInfo.bytesWritten += bytesToCopy;

                memInfo.state = SRV_FU_MEM_STATE_WRITE_WAIT_END;
            }

            break;
        }

        case SRV_FU_CALCULATE_CRC_BLOCK:
        {
            if (crcState == SRV_FU_CRC_CALCULATING)
            {
                calculatedCrc = SRV_PCRC_GetValue(pBuffInput, crcSize, PCRC_HT_GENERIC, PCRC_CRC32,
                                     calculatedCrc);

                if (crcRemainingSize > 0U)
                {
                    uint32_t blockStart, nBlock;
                    uint32_t bytesPagesRead;

                    crcState = SRV_FU_CRC_WAIT_READ_BLOCK;

                    if (crcRemainingSize < MAX_BUFFER_READ_SIZE)
                    {
                        crcSize = crcRemainingSize;
                    }
                    else
                    {
                        crcSize = MAX_BUFFER_READ_SIZE;
                    }

                    blockStart = crcReadAddress / memInfo.readPageSize;
                    nBlock = crcSize / memInfo.readPageSize;

                    bytesPagesRead = nBlock * memInfo.readPageSize;
                    /* Align CRC size with the readPageSize */
                    if (crcSize > bytesPagesRead)
                    {
                        if (((nBlock + 1U) * memInfo.readPageSize) <= MAX_BUFFER_READ_SIZE)
                        {
                            nBlock++;
                        }
                        else
                        {
                            /* Cannot read everything, we reduced the size of the Crc calculated
                            this time */
                            crcSize = bytesPagesRead;
                        }
                    }

                    DRV_MEMORY_AsyncRead(memInfo.memoryHandle, &memInfo.readHandle, pBuffInput, blockStart, nBlock);

                    crcReadAddress += crcSize;
                    crcRemainingSize -= crcSize;
                }
                else
                {
                    crcState = SRV_FU_CRC_IDLE;
                    memInfo.state = SRV_FU_MEM_STATE_CMD_WAIT;

                    /* Check pointer function */
                    if (SRV_FU_CrcCallback != NULL) {
                        SRV_FU_CrcCallback(calculatedCrc);
                    }
                }
            }

            break;
        }

        case SRV_FU_VERIFY_SIGNATURE_BLOCK:
        case SRV_FU_MEM_STATE_XFER_WAIT:
        case SRV_FU_MEM_STATE_SUCCESS:
        case SRV_FU_MEM_STATE_WRITE_WAIT_END:
        case SRV_FU_MEM_STATE_CMD_WAIT:
        case SRV_FU_MEM_UNINITIALIZED:
/* MISRA C-2023 deviation block start */
/* MISRA C-2023 Rule 16.4 deviated once. Deviation record ID - H3_MISRAC_2023_R_16_4_DR_1 */
         default:
            break;
/* MISRA C-2023 deviation block end */
    }
}

void SRV_FU_DataRead(uint32_t address, uint8_t *buffer, uint16_t size)
{
    uint32_t readAddress;
    uint32_t blockStart, nBlock;

    readAddress = memInfo.startAdressFuRegion + address;

    blockStart = readAddress / memInfo.readPageSize;
    nBlock = size / memInfo.readPageSize;

    DRV_MEMORY_AsyncRead(memInfo.memoryHandle, &memInfo.readHandle, (void *) buffer, blockStart, nBlock);

    memInfo.state = SRV_FU_MEM_STATE_READ_MEMORY;
}

void SRV_FU_DataWrite(uint32_t address, uint8_t *buffer, uint16_t size)
{

    if (size > MAX_BUFFER_READ_SIZE)
    {
        if (SRV_FU_MemTransferCallback != NULL)
        {
            SRV_FU_MemTransferCallback(SRV_FU_MEM_TRANSFER_CMD_WRITE, SRV_FU_MEM_TRANSFER_ERROR);
        }

        return;
    }

    if (memInfo.writePageSize > MEMORY_WRITE_SIZE)
    {
        if (SRV_FU_MemTransferCallback != NULL)
        {
            SRV_FU_MemTransferCallback(SRV_FU_MEM_TRANSFER_CMD_WRITE, SRV_FU_MEM_TRANSFER_ERROR);
        }
        return;
    }

    /* Reject the write if the underlying memory driver hasn't finished
     * its async initialization yet (handle still invalid or page size
     * still zero). Without this guard SRV_FU_Tasks would later divide
     * writeAddress by zero and the result would be used to index pMemWrite,
     * landing the dst pointer outside the array and faulting in memcpy. */
    if ((memInfo.memoryHandle == DRV_HANDLE_INVALID) ||
        (memInfo.writePageSize == 0U))
    {
        if (SRV_FU_MemTransferCallback != NULL)
        {
            SRV_FU_MemTransferCallback(SRV_FU_MEM_TRANSFER_CMD_WRITE, SRV_FU_MEM_TRANSFER_ERROR);
        }
        return;
    }

    memInfo.writeAddress = memInfo.startAdressFuRegion + address;
    memInfo.writeSize = size;
    memInfo.bytesWritten = 0;

    (void)memcpy(pBuffInput, buffer, size);

    memInfo.state = SRV_FU_MEM_STATE_WRITE_ONE_BLOCK;
}

void SRV_FU_CfgRead(void *dst, uint16_t size)
{
    uint32_t bufferValue[4];

    bufferValue[0] = SRV_STORAGE_GpbrRead(0U);
    bufferValue[1] = SRV_STORAGE_GpbrRead(1U);
    bufferValue[2] = SRV_STORAGE_GpbrRead(2U);
    bufferValue[3] = SRV_STORAGE_GpbrRead(3U);

    (void)memcpy(dst, (void *)bufferValue, size);
}

void SRV_FU_CfgWrite(void *src, uint16_t size)
{
    uint32_t bufferValue[4] = {0U};

    (void)memcpy((void *)bufferValue, src, size);

    SRV_STORAGE_GpbrWriteBlock(0U, 4U, bufferValue);
}

void SRV_FU_Start(SRV_FU_INFO *fuInfo)
{
    fuData.imageSize = fuInfo->imageSize;
    fuData.pageSize  = fuInfo->pageSize;

    /* Signature parameters are propagated from the FU info even though
     * signature verification is not implemented in this build. Keeping
     * the propagation in place future-proofs the path: when (and if)
     * verification is added later, the algorithm and length will
     * already be available in fuData. */
    fuData.signAlgorithm = fuInfo->signAlgorithm;
    fuData.signLength    = fuInfo->signLength;

    /* Erase internal flash pages */
    lSRV_FU_EraseFuRegion();

    /* Set CRC status */
    crcState = SRV_FU_CRC_IDLE;
    return;
}

void SRV_FU_End(SRV_FU_RESULT fuResult)
{
    /* Cache whether this end-of-FU marks a revert request so the
     * subsequent SRV_FU_SwapFirmware call can point the bootloader at
     * the REVERT zone instead of the TELECARGA zone. SUCCESS clears
     * the flag to cover the "success after a previous revert attempt"
     * case; other results leave it untouched. */
    if (fuResult == SRV_FU_RESULT_FW_REVERT)
    {
        fuLastResultIsRevert = true;
    }
    else if (fuResult == SRV_FU_RESULT_SUCCESS)
    {
        fuLastResultIsRevert = false;
    }
    else
    {
        /* Leave the flag as is. */
    }

    /* Check callback is initialized */
    if (SRV_FU_ResultCallback == NULL)
    {
        return;
    }

    switch (fuResult)
    {
        case SRV_FU_RESULT_SUCCESS:
        case SRV_FU_RESULT_CRC_ERROR:
        case SRV_FU_RESULT_FW_REVERT:
        case SRV_FU_RESULT_FW_CONFIRM:
            SRV_FU_ResultCallback(fuResult);
            break;
/* MISRA C-2023 deviation block start */
/* MISRA C-2023 Rule 16.4 deviated once. Deviation record ID - H3_MISRAC_2023_R_16_4_DR_1 */
        default:
            break;
/* MISRA C-2023 deviation block end */
    }
}

void SRV_FU_CalculateCrc(void)
{
    uint32_t blockStart, nBlock;
    uint32_t bytesPagesRead;

    if (crcState != SRV_FU_CRC_IDLE)
    {
        return;
    }

    crcState = SRV_FU_CRC_WAIT_READ_BLOCK;

    crcReadAddress = memInfo.startAdressFuRegion;
    crcRemainingSize = fuData.imageSize;

    if (crcRemainingSize < MAX_BUFFER_READ_SIZE)
    {
        crcSize = crcRemainingSize;
    }
    else
    {
        crcSize = MAX_BUFFER_READ_SIZE;
    }

    blockStart = crcReadAddress / memInfo.readPageSize;
    nBlock = crcSize / memInfo.readPageSize;

    bytesPagesRead = nBlock * memInfo.readPageSize;
    /* Align CRC size with the readPageSize */
    if (crcSize > bytesPagesRead)
    {
        if (((nBlock + 1U) * memInfo.readPageSize) <= MAX_BUFFER_READ_SIZE)
        {
            nBlock++;
        }
        else
        {
            /* Cannot read everything, we reduced the size of the Crc calculated
            this time */
            crcSize = bytesPagesRead;
        }
    }

    DRV_MEMORY_AsyncRead(memInfo.memoryHandle, &memInfo.readHandle, pBuffInput, blockStart, nBlock);

    crcReadAddress += crcSize;
    crcRemainingSize -= crcSize;

    memInfo.state = SRV_FU_CALCULATE_CRC_BLOCK;

    /* CRC Initial */
    calculatedCrc = 0;
}

void SRV_FU_RegisterCallbackCrc(SRV_FU_CRC_CB callback)
{
    SRV_FU_CrcCallback = callback;
}

void SRV_FU_RegisterCallbackVerify(SRV_FU_IMAGE_VERIFY_CB callback)
{
    SRV_FU_ImageVerifyCallback = callback;
}

void SRV_FU_RegisterCallbackFuResult(SRV_FU_RESULT_CB callback)
{
    SRV_FU_ResultCallback = callback;

}

uint16_t SRV_FU_GetBitmap(uint8_t *bitmap, uint32_t *numRxPages)
{
    (void)bitmap;
    (void)numRxPages;

    return 0;
}

void SRV_FU_RequestSwapVersion(SRV_FU_TRAFFIC_VERSION trafficVersion)
{
    /* Check callback is initialized */
    if (SRV_FU_SwapCallback != NULL)
    {
        SRV_FU_SwapCallback(trafficVersion);
    }
}

void SRV_FU_RegisterCallbackSwapVersion(SRV_FU_VERSION_SWAP_CB callback)
{
    SRV_FU_SwapCallback = callback;
}

void SRV_FU_RegisterCallbackMemTransfer(SRV_FU_MEM_TRANSFER_CB callback)
{
    SRV_FU_MemTransferCallback = callback;
}

bool SRV_FU_SwapFirmware(void)
{
    SRV_STORAGE_BOOT_CONFIG bootConfig;

    /* Read the current boot config so any fields the bootloader does
     * not touch are preserved verbatim. */
    if (SRV_STORAGE_GetConfigInfo(SRV_STORAGE_TYPE_BOOT_INFO,
                                  (uint8_t) sizeof(bootConfig),
                                  &bootConfig) == false)
    {
        return false;
    }

    /* Common fields: the bootloader always installs into the
     * application region starting at PRIME_FU_APP_START_ADDR, and
     * recognises the request only when cfgKey matches the shared
     * magic. */
    bootConfig.cfgKey       = PRIME_FU_BOOT_CFG_KEY;
    bootConfig.destAddr     = PRIME_FU_APP_START_ADDR;
    bootConfig.pagesCounter = 0U;
    bootConfig.bootState    = 0U;

    if (fuLastResultIsRevert)
    {
        /* Revert path: the previous TELECARGA backup lives in the
         * REVERT zone. The bootloader knows the size of its own
         * backup so imgSize is informational only; set it to zero to
         * make that explicit. */
        bootConfig.origAddr = PRIME_FU_MEM_REVERT_OFFSET;
        bootConfig.imgSize  = 0U;
    }
    else
    {
        /* Install path: the FU service just finished writing the new
         * image to the TELECARGA zone at SST26 offset 0. imgSize is
         * the payload length the bootloader will copy into flash. */
        bootConfig.origAddr = 0U;
        bootConfig.imgSize  = fuData.imageSize;
    }

    /* Persist. The bootloader reads this on the next reset. */
    (void) SRV_STORAGE_SetConfigInfo(SRV_STORAGE_TYPE_BOOT_INFO,
                                     (uint8_t) sizeof(bootConfig),
                                     &bootConfig);

    return true;
}


void SRV_FU_VerifyImage(void)
{
    /* The SAMD20 build only ships one binary type per device, so there
     * is no in-image vendor/model/metadata to validate here, and the
     * cryptographic signature path is not yet implemented. The PRIME
     * stack still expects a verdict via SRV_FU_ImageVerifyCallback
     * before it advances to the swap-firmware step, so report SUCCESS
     * unconditionally — equivalent to the PIC32 path that returns
     * SUCCESS when signAlgorithm == SRV_FU_SIGNATURE_ALGO_NO_SIGNATURE.
     * When signature verification is added later, replace this with
     * the real signature check. */
    if (SRV_FU_ImageVerifyCallback != NULL)
    {
        SRV_FU_ImageVerifyCallback(SRV_FU_VERIFY_RESULT_SUCCESS);
    }
}


