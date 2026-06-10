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

#include <string.h>

#include "configuration.h"
#include "definitions.h"

#include "srv_firmware_upgrade.h"
#include "srv_firmware_upgrade_local.h"

#include "service/pcrc/srv_pcrc.h"
#include "service/storage/srv_storage.h"
#include "crypto/common_crypto/crypto_common.h"
#include "crypto/common_crypto/crypto_hash.h"
#include "crypto/common_crypto/crypto_digsign.h"

// *****************************************************************************
// *****************************************************************************
// Section: Macro definitions
// *****************************************************************************
// *****************************************************************************

#define PRIME_FU_MEM_DRV        "drv_memory_0"
#define PRIME_FU_MEM_INSTANCE   0
#define PRIME_FU_MEM_SIZE       (uint32_t)(0x80000)

#define MEMORY_WRITE_SIZE       (uint32_t)(256)
#define MAX_BUFFER_READ_SIZE    (uint32_t)(256)

/* Destination address in internal flash where the bootloader installs
 * the image. Matches the bootloader's application start address. */
#define PRIME_FU_APP_START_ADDR (uint32_t)(0x2000)

/* External-memory BOOT_MODE_INFO handshake -- must match the bootloader.
 * The 12-byte structure lives at the start of the BOOT_FLAG sector; */
#define PRIME_FU_BOOT_FLAG_OFFSET   (uint32_t)(0x140000)
#define PRIME_FU_BOOT_MODE_MAGIC    (uint32_t)(0x444F4D42UL)
#define PRIME_FU_BOOT_MODE_SIZE     (uint32_t)(12)

/* Define application number */
typedef enum
{
    PRIME_INVALID_APP = 0,
    PRIME_MAC13_APP,
    PRIME_MAC14_APP,
    PRIME_PHY_APP,
    PRIME_MAIN_APP
} SRV_FU_PRIME_APP_TYPE;

/* Define Session Id for crypto */
#define SESSION_ID                    1

/* Maximum size of the signature */
#define PRIME_SIGNATURE_SIZE          128

/* Size for a hash using SHA 256 in bytes */
#define HASH_SIZE_SHA_256             32

/* Size for a signature ECDSA 256 in bytes */
#define SIGNATURE_SIZE_ECDSA_256      64

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

static SRV_FU_CRC_STATE crcState;

static uint32_t crcReadAddress;

static uint32_t crcSize;

static uint32_t crcRemainingSize;

static uint32_t calculatedCrc;


/* Tracks whether the most recent FU result was a revert request. */
static bool fuLastResultIsRevert = false;

/* Latest decoded BOOT_MODE_INFO read from BOOT_FLAG. */
static SRV_FU_EXT_MEM_BOOT_MODE_INFO bootModeResult;

static uint8_t imageSignature[PRIME_SIGNATURE_SIZE];

static uint8_t hashDigest[HASH_SIZE_SHA_256];

static st_Crypto_Hash_Sha_Ctx hashCtx;

static SRV_FU_DSA_STATE dsaState;

static uint8_t *ECDSAPublicKey;

static uint32_t ECDSAPublicKeyLen;

static uint32_t dsaReadAddress;

static uint32_t dsaSize;

static uint32_t dsaRemainingSize;

// *****************************************************************************
// *****************************************************************************
// Section: File scope functions
// *****************************************************************************
// *****************************************************************************
static void lSRV_FU_StoreImageInfo(uint32_t address, uint32_t size)
{
    uint32_t iniSignature;
    uint32_t offsetSegment;
    uint32_t offsetSignature;
    uint32_t sizeToCopy;

    /* Nothing to do if the image is unsigned. */
    if (fuData.signLength == 0U)
    {
        return;
    }

    iniSignature = fuData.imageSize - fuData.signLength;

    /* Skip segments that fall entirely before the signature tail. */
    if ((address + size) <= iniSignature)
    {
        return;
    }

    if (address < iniSignature)
    {
        offsetSegment = iniSignature - address;
    }
    else
    {
        offsetSegment = 0U;
    }

    if (address > iniSignature)
    {
        offsetSignature = address - iniSignature;
    }
    else
    {
        offsetSignature = 0U;
    }

    sizeToCopy = (uint32_t) fuData.signLength - offsetSignature;
    if ((size - offsetSegment) < sizeToCopy)
    {
        sizeToCopy = size - offsetSegment;
    }

    (void) memcpy(&imageSignature[offsetSignature],
                  &pBuffInput[offsetSegment], sizeToCopy);
}

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

    /* Boot-mode handshake completions */
    if (commandHandle == mInfo->bootModeHandle)
    {
        if (transferResult != SRV_FU_MEM_TRANSFER_OK)
        {
            mInfo->bootModeStatus = (uint8_t) SRV_FU_EXT_MEM_BOOT_MODE_STATUS_ERROR;
            mInfo->state = SRV_FU_MEM_STATE_CMD_WAIT;
            return;
        }

        switch (mInfo->state)
        {
            case SRV_FU_MEM_STATE_EXT_MEM_BOOT_MODE_ERASE:
                /* Erase ok -> Tasks() will kick the page write. */
                mInfo->state = SRV_FU_MEM_STATE_EXT_MEM_BOOT_MODE_WRITE_KICK;
                break;

            case SRV_FU_MEM_STATE_EXT_MEM_BOOT_MODE_WRITE:
                /* Page written -> Set sequence done. */
                mInfo->bootModeStatus = (uint8_t) SRV_FU_EXT_MEM_BOOT_MODE_STATUS_OK;
                mInfo->state = SRV_FU_MEM_STATE_CMD_WAIT;
                break;

            case SRV_FU_MEM_STATE_EXT_MEM_BOOT_MODE_READ:
                /* Page read -> Tasks() will validate magic + modeXor. */
                mInfo->state = SRV_FU_MEM_STATE_EXT_MEM_BOOT_MODE_READ_DONE;
                break;

            default:
                mInfo->bootModeStatus = (uint8_t) SRV_FU_EXT_MEM_BOOT_MODE_STATUS_ERROR;
                mInfo->state = SRV_FU_MEM_STATE_CMD_WAIT;
                break;
        }

        return;
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
       else if (memInfo.state == SRV_FU_VERIFY_SIGNATURE_BLOCK)
        {
            /* Calculating SHA.... no callback*/
            dsaState = SRV_FU_DSA_CALCULATING;
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

static void lSRV_FU_ConvertDerFormatSignature(void)
{
    uint8_t index;

    for (index = 0U; index < 32U; index++)
    {
        imageSignature[index] = imageSignature[4U + index];
    }

    for (index = 0U; index < 32U; index++)
    {
        imageSignature[32U + index] = imageSignature[38U + index];
    }

}

static bool lSRV_FU_VerifySignature(void)
{
    crypto_Hash_Status_E stateCryptoHash;

    if (fuData.signAlgorithm == SRV_FU_SIGNATURE_ALGO_NO_SIGNATURE)
    {
        /* No need to check any signature, finish checking */
        SRV_FU_ImageVerifyCallback(SRV_FU_VERIFY_RESULT_SUCCESS);
        dsaState = SRV_FU_DSA_IDLE;
        memInfo.state = SRV_FU_MEM_STATE_CMD_WAIT;

        return true;
    }

    if (fuData.signAlgorithm != SRV_FU_SIGNATURE_ALGO_ECDSA256_SHA256)
    {
        /* Only ECDSA256_SHA256 is supported */
        return false;
    }

    if (dsaState != SRV_FU_DSA_IDLE)
    {
        /* DSA state machine not idle */
        return false;
    }

    /* Check if signature comes in DER format */
    if (fuData.signLength == 70UL) {
        lSRV_FU_ConvertDerFormatSignature();
    }

    /* Start to verify the signature */
    stateCryptoHash = Crypto_Hash_Sha_Init(&hashCtx, CRYPTO_HASH_SHA2_256, CRYPTO_HANDLER_SW_WOLFCRYPT, SESSION_ID);

    if (stateCryptoHash != CRYPTO_HASH_SUCCESS)
    {
        return false;
    }
    else
    {
        uint32_t blockStart, nBlock;
        uint32_t bytesPagesRead;

        dsaState = SRV_FU_DSA_WAIT_READ_BLOCK;

        dsaReadAddress = memInfo.startAdressFuRegion;
        dsaRemainingSize = fuData.imageSize - fuData.signLength;

        if (dsaRemainingSize < MAX_BUFFER_READ_SIZE)
        {
            dsaSize = dsaRemainingSize;
        }
        else
        {
            dsaSize = MAX_BUFFER_READ_SIZE;
        }

        blockStart = dsaReadAddress / memInfo.readPageSize;
        nBlock = dsaSize / memInfo.readPageSize;

        bytesPagesRead = nBlock * memInfo.readPageSize;
        /* Align SHA size with the readPageSize */
        if (dsaSize > bytesPagesRead)
        {
            if (((nBlock + 1U) * memInfo.readPageSize) <= MAX_BUFFER_READ_SIZE)
            {
                nBlock++;
            }
            else
            {
                /* Cannot read everything, we reduced the size of the Crc calculated
                this time */
                dsaSize = bytesPagesRead;
            }
        }

        DRV_MEMORY_AsyncRead(memInfo.memoryHandle, &memInfo.readHandle, pBuffInput, blockStart, nBlock);

        dsaReadAddress += dsaSize;
        dsaRemainingSize -= dsaSize;

        memInfo.state = SRV_FU_VERIFY_SIGNATURE_BLOCK;

        return true;
    }
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

    memInfo.bootModeHandle = DRV_MEMORY_COMMAND_HANDLE_INVALID;
    memInfo.bootModeStatus = (uint8_t) SRV_FU_EXT_MEM_BOOT_MODE_STATUS_IDLE;
    memInfo.bootModeRequestMode = 0U;
    memInfo.bootModeRequestImageIdx = 0U;
    memInfo.bootModeRequestImageStep = 0U;

    (void) memset(&bootModeResult, 0, sizeof(bootModeResult));

    memInfo.state = SRV_FU_MEM_STATE_OPEN_DRIVER;

    dsaState = SRV_FU_DSA_NO_PUBLIC_KEY;
    ECDSAPublicKey = NULL;
    ECDSAPublicKeyLen = 0U;
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

        case SRV_FU_MEM_STATE_EXT_MEM_BOOT_MODE_ERASE:
        case SRV_FU_MEM_STATE_EXT_MEM_BOOT_MODE_WRITE:
        case SRV_FU_MEM_STATE_EXT_MEM_BOOT_MODE_READ:
        {
            /* Operation in flight; the transfer handler advances state. */
            if (DRV_MEMORY_COMMAND_HANDLE_INVALID == memInfo.bootModeHandle)
            {
                memInfo.bootModeStatus = (uint8_t) SRV_FU_EXT_MEM_BOOT_MODE_STATUS_ERROR;
                memInfo.state = SRV_FU_MEM_STATE_CMD_WAIT;
            }
            break;
        }

        case SRV_FU_MEM_STATE_EXT_MEM_BOOT_MODE_WRITE_KICK:
        {
            /* Erase finished, queue the page write. */
            uint32_t pageBlockStart;
            uint32_t modeXorByte;

            pageBlockStart = PRIME_FU_BOOT_FLAG_OFFSET / memInfo.writePageSize;
            modeXorByte = (uint32_t) memInfo.bootModeRequestMode
                        ^ (PRIME_FU_BOOT_MODE_MAGIC & 0xFFU);

            (void) memset(pMemWrite, 0xFF, sizeof(pMemWrite));

            /* Build the on-flash struct directly inside pMemWrite at offset 0. */
            {
                SRV_FU_EXT_MEM_BOOT_MODE_INFO toWrite;

                toWrite.magic     = PRIME_FU_BOOT_MODE_MAGIC;
                toWrite.mode      = memInfo.bootModeRequestMode;
                toWrite.imageIdx  = memInfo.bootModeRequestImageIdx;
                toWrite.imageStep = memInfo.bootModeRequestImageStep;
                toWrite.reserved  = 0U;
                toWrite.modeXor   = modeXorByte;

                (void) memcpy(pMemWrite, &toWrite, sizeof(toWrite));
                (void) memcpy(&bootModeResult, &toWrite, sizeof(toWrite));
            }

            DRV_MEMORY_AsyncWrite(memInfo.memoryHandle, &memInfo.bootModeHandle,
                                  pMemWrite, pageBlockStart, 1U);

            if (DRV_MEMORY_COMMAND_HANDLE_INVALID == memInfo.bootModeHandle)
            {
                memInfo.bootModeStatus = (uint8_t) SRV_FU_EXT_MEM_BOOT_MODE_STATUS_ERROR;
                memInfo.state = SRV_FU_MEM_STATE_CMD_WAIT;
            }
            else
            {
                memInfo.state = SRV_FU_MEM_STATE_EXT_MEM_BOOT_MODE_WRITE;
            }
            break;
        }

        case SRV_FU_MEM_STATE_EXT_MEM_BOOT_MODE_READ_DONE:
        {
            SRV_FU_EXT_MEM_BOOT_MODE_INFO decoded;
            uint32_t expectedXor;

            (void) memcpy(&decoded, pBuffInput, sizeof(decoded));

            expectedXor = (uint32_t) decoded.mode
                        ^ (PRIME_FU_BOOT_MODE_MAGIC & 0xFFU);

            if ((decoded.magic != PRIME_FU_BOOT_MODE_MAGIC) ||
                (decoded.modeXor != expectedXor))
            {
                (void) memset(&decoded, 0, sizeof(decoded));
            }

            (void) memcpy(&bootModeResult, &decoded, sizeof(decoded));

            memInfo.bootModeStatus = (uint8_t) SRV_FU_EXT_MEM_BOOT_MODE_STATUS_OK;
            memInfo.state = SRV_FU_MEM_STATE_CMD_WAIT;
            break;
        }
        case SRV_FU_VERIFY_SIGNATURE_BLOCK:
        {
            if (dsaState == SRV_FU_DSA_CALCULATING)
            {
                crypto_Hash_Status_E stateCryptoHash;

                if (dsaRemainingSize > 0U)
                {
                    stateCryptoHash = Crypto_Hash_Sha_Update(&hashCtx, pBuffInput, dsaSize);
                }
                else
                {
                    stateCryptoHash = Crypto_Hash_Sha_Update(&hashCtx, pBuffInput, dsaSize);
                    stateCryptoHash = Crypto_Hash_Sha_Final(&hashCtx, hashDigest);
                }

                if (stateCryptoHash!= CRYPTO_HASH_SUCCESS)
                {
                    dsaState = SRV_FU_DSA_IDLE;
                    memInfo.state = SRV_FU_MEM_STATE_CMD_WAIT;

                    SRV_FU_ImageVerifyCallback(SRV_FU_VERIFY_RESULT_SIGNATURE_FAIL);

                    break;
                }

                if (dsaRemainingSize > 0U)
                {
                    uint32_t blockStart, nBlock;
                    uint32_t bytesPagesRead;

                    dsaState = SRV_FU_DSA_WAIT_READ_BLOCK;

                    if (dsaRemainingSize < MAX_BUFFER_READ_SIZE)
                    {
                        dsaSize = dsaRemainingSize;
                    }
                    else
                    {
                        dsaSize = MAX_BUFFER_READ_SIZE;
                    }

                    blockStart = dsaReadAddress / memInfo.readPageSize;
                    nBlock = dsaSize / memInfo.readPageSize;

                    bytesPagesRead = nBlock * memInfo.readPageSize;
                    /* Align SHA size with the readPageSize */
                    if (dsaSize > bytesPagesRead)
                    {
                        if (((nBlock + 1U) * memInfo.readPageSize) <= MAX_BUFFER_READ_SIZE)
                        {
                            nBlock++;
                        }
                        else
                        {
                            /* Cannot read everything, we reduced the size of the Crc calculated
                            this time */
                            dsaSize = bytesPagesRead;
                        }
                    }

                    DRV_MEMORY_AsyncRead(memInfo.memoryHandle, &memInfo.readHandle, pBuffInput, blockStart, nBlock);

                    dsaReadAddress += dsaSize;
                    dsaRemainingSize -= dsaSize;
                }
                else
                {
                    crypto_DigiSign_Status_E stateCryptoECDSA;
                    int8_t validDSA = 0;

                    /* Hash already done, do ECDSA256_SHA256 verification */
                    stateCryptoECDSA = Crypto_DigiSign_Ecdsa_Verify(CRYPTO_HANDLER_SW_WOLFCRYPT,
                                                                    hashDigest,
                                                                    HASH_SIZE_SHA_256,
                                                                    imageSignature,
                                                                    SIGNATURE_SIZE_ECDSA_256,
                                                                    ECDSAPublicKey,
                                                                    ECDSAPublicKeyLen,
                                                                    &validDSA,
                                                                    CRYPTO_ECC_CURVE_P256,
                                                                    SESSION_ID);

                    /* Check verification result ECDSA256_SHA256 */
                    if ((validDSA != 1) || (stateCryptoECDSA != CRYPTO_DIGISIGN_SUCCESS))
                    {
                        SRV_FU_ImageVerifyCallback(SRV_FU_VERIFY_RESULT_SIGNATURE_FAIL);
                    }
                    else
                    {
                        SRV_FU_ImageVerifyCallback(SRV_FU_VERIFY_RESULT_SUCCESS);
                    }

                    dsaState = SRV_FU_DSA_IDLE;
                    memInfo.state = SRV_FU_MEM_STATE_CMD_WAIT;
                }
            }

            break;
        }
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
     * its async initialization yet. */
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

    lSRV_FU_StoreImageInfo(address, size);

    memInfo.state = SRV_FU_MEM_STATE_WRITE_ONE_BLOCK;
}

void SRV_FU_CfgRead(void *dst, uint16_t size)
{
    uint32_t bufferValue[4];
    bufferValue[0] = SRV_STORAGE_ReadNonVolatileData(0U);
    bufferValue[1] = SRV_STORAGE_ReadNonVolatileData(1U);
    bufferValue[2] = SRV_STORAGE_ReadNonVolatileData(2U);
    bufferValue[3] = SRV_STORAGE_ReadNonVolatileData(3U);

    (void)memcpy(dst, (void *)bufferValue, size);
}

void SRV_FU_CfgWrite(void *src, uint16_t size)
{
    uint32_t bufferValue[4];

    (void)memcpy(bufferValue, (uint32_t *)src, size);

    SRV_STORAGE_WriteBlockNonVolatileData(0U, 4U, bufferValue);
}

void SRV_FU_Start(SRV_FU_INFO *fuInfo)
{
    fuData.imageSize = fuInfo->imageSize;
    fuData.pageSize = fuInfo->pageSize;
    fuData.signAlgorithm = fuInfo->signAlgorithm;
    fuData.signLength = fuInfo->signLength;

    /* Erase internal flash pages */
    lSRV_FU_EraseFuRegion();

    /* Set CRC status */
    crcState = SRV_FU_CRC_IDLE;
    return;
}

void SRV_FU_End(SRV_FU_RESULT fuResult)
{
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
    uint8_t mode;

    mode = fuLastResultIsRevert
         ? SRV_FU_EXT_MEM_BOOT_MODE_REVERT_PENDING
         : SRV_FU_EXT_MEM_BOOT_MODE_INSTALL_PENDING;

    return SRV_FU_ExtMemBootModeSet(mode, 0U, 0U);
}

void SRV_FU_SetECDSAPublicKey(uint8_t *pubKey, uint32_t pubKeyLen)
{
    ECDSAPublicKey = pubKey;
    ECDSAPublicKeyLen = pubKeyLen;

    dsaState = SRV_FU_DSA_IDLE;
}

void SRV_FU_VerifyImage(void)
{
    /* Check pointer function */
    if (SRV_FU_ImageVerifyCallback == NULL)
    {
        return;
    }

    /* External-bootloader build: no per-image metadata check, go straight
     * to the chunked SHA-256 + ECDSA signature verification. */
    if (lSRV_FU_VerifySignature() != true)
    {
        /* Wrong signature */
        SRV_FU_ImageVerifyCallback(SRV_FU_VERIFY_RESULT_SIGNATURE_FAIL);
    }
}

bool SRV_FU_ExtMemBootModeSet(uint8_t mode, uint8_t imageIdx, uint8_t imageStep)
{
    uint32_t eraseBlockSize;
    uint32_t eraseBlockStartBootFlag;

    if (memInfo.state != SRV_FU_MEM_STATE_CMD_WAIT)
    {
        return false;
    }

    if (nvmGeometry == NULL)
    {
        return false;
    }

    eraseBlockSize = nvmGeometry->geometryTable[SYS_MEDIA_GEOMETRY_TABLE_ERASE_ENTRY].blockSize;

    if (eraseBlockSize == 0U)
    {
        return false;
    }

    eraseBlockStartBootFlag = PRIME_FU_BOOT_FLAG_OFFSET / eraseBlockSize;

    memInfo.bootModeRequestMode      = mode;
    memInfo.bootModeRequestImageIdx  = imageIdx;
    memInfo.bootModeRequestImageStep = imageStep;
    memInfo.bootModeStatus           = (uint8_t) SRV_FU_EXT_MEM_BOOT_MODE_STATUS_BUSY;

    DRV_MEMORY_AsyncErase(memInfo.memoryHandle, &memInfo.bootModeHandle,
                          eraseBlockStartBootFlag, 1U);

    if (DRV_MEMORY_COMMAND_HANDLE_INVALID == memInfo.bootModeHandle)
    {
        memInfo.bootModeStatus = (uint8_t) SRV_FU_EXT_MEM_BOOT_MODE_STATUS_ERROR;
        return false;
    }

    memInfo.state = SRV_FU_MEM_STATE_EXT_MEM_BOOT_MODE_ERASE;
    return true;
}

bool SRV_FU_ExtMemBootModeGet(void)
{
    uint32_t pageBlockStart;

    if (memInfo.state != SRV_FU_MEM_STATE_CMD_WAIT)
    {
        return false;
    }

    if ((nvmGeometry == NULL) || (memInfo.readPageSize == 0U))
    {
        return false;
    }

    pageBlockStart = PRIME_FU_BOOT_FLAG_OFFSET / memInfo.readPageSize;

    memInfo.bootModeStatus = (uint8_t) SRV_FU_EXT_MEM_BOOT_MODE_STATUS_BUSY;

    DRV_MEMORY_AsyncRead(memInfo.memoryHandle, &memInfo.bootModeHandle,
                         pBuffInput, pageBlockStart, 1U);

    if (DRV_MEMORY_COMMAND_HANDLE_INVALID == memInfo.bootModeHandle)
    {
        memInfo.bootModeStatus = (uint8_t) SRV_FU_EXT_MEM_BOOT_MODE_STATUS_ERROR;
        return false;
    }

    memInfo.state = SRV_FU_MEM_STATE_EXT_MEM_BOOT_MODE_READ;
    return true;
}

SRV_FU_EXT_MEM_BOOT_MODE_STATUS SRV_FU_ExtMemBootModeStatus(void)
{
    return (SRV_FU_EXT_MEM_BOOT_MODE_STATUS) memInfo.bootModeStatus;
}

void SRV_FU_ExtMemBootModeResult(SRV_FU_EXT_MEM_BOOT_MODE_INFO *info)
{
    if (info == NULL)
    {
        return;
    }

    (void) memcpy(info, &bootModeResult, sizeof(*info));
}


