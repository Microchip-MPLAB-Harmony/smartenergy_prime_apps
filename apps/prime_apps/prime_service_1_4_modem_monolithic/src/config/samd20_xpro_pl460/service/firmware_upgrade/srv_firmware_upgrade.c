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
#define PRIME_FU_MEM_SIZE       (uint32_t)(0x60000)

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

/* Define Session Id for crypto */
#define SESSION_ID                    1

/* Maximum size of the signature */
#define PRIME_SIGNATURE_SIZE          128

/* Size for a hash using SHA 256 in bytes */
#define HASH_SIZE_SHA_256             32

/* Size for a signature ECDSA 256 in bytes */
#define SIGNATURE_SIZE_ECDSA_256      64
#define PRIME_METADATA_SIZE          (uint32_t)(16)

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


static uint8_t imageMetadata[PRIME_METADATA_SIZE];

static SRV_FU_PRIME_APP_TYPE appToFu;

static uint16_t imageVendor;

static uint16_t imageModel;

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
    uint32_t iniMetadata, iniSignature;
    uint32_t offsetSegment, offsetMetadata, offsetSignature;
    uint32_t sizeToCopy;

    /* The first segment contains the Vendor and Model */
    if (address == 0U)
    {
        imageVendor = ((uint16_t) pBuffInput[0]) << 8;
        imageVendor |= pBuffInput[1];

        imageModel = ((uint16_t) pBuffInput[2]) << 8;
        imageModel |= pBuffInput[3];
    }
    
    iniMetadata = fuData.imageSize - fuData.signLength - PRIME_METADATA_SIZE;
    iniSignature = fuData.imageSize - fuData.signLength;

    /* Check if the segment to write is in metadata zone*/
    if ((address + size) < iniMetadata)
    {
        return;
    }

    if (address < iniMetadata)
    {
        offsetSegment = iniMetadata - address;
    }
    else
    {
        offsetSegment = 0;
    }

    if ((address > iniMetadata) && (address < iniSignature))
    {
        offsetMetadata = address - iniMetadata;
    }
    else
    {
        offsetMetadata = 0;
    }

    /* There is metadata inside the segment */
    if (address < iniSignature)
    {
        sizeToCopy = PRIME_METADATA_SIZE - offsetMetadata;

        if ((size - offsetSegment) < sizeToCopy)
        {
            sizeToCopy = size - offsetSegment;
        }

        (void)memcpy(&imageMetadata[offsetMetadata], &pBuffInput[offsetSegment], sizeToCopy);
    }
        
    /* Signature */
    
    /*  Check if the segment to write is in signature zone */
    if ((address + size) < iniSignature)
    {
        return;
    }

    if (address < iniSignature)
    {
        offsetSegment = iniSignature - address;
    }
    else
    {
        offsetSegment = 0;
    }

    if (address > iniSignature)
    {
        offsetSignature = address - iniSignature;
    }
    else
    {
        offsetSignature = 0;
    }

    sizeToCopy = fuData.imageSize - offsetSignature;

    if ((size - offsetSegment) < sizeToCopy)
    {
        sizeToCopy = size - offsetSegment;
    }

    (void)memcpy(&imageSignature[offsetSignature], &pBuffInput[offsetSegment], sizeToCopy);
}

static bool lSRV_FU_CheckImageData(void)
{
    appToFu = PRIME_INVALID_APP;


    return true;
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
            crcState = SRC_FU_CRC_CALCULATING;
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

    memInfo.state = SRV_FU_MEM_STATE_OPEN_DRIVER;

    dsaState = SRV_FU_DSA_NO_PUBLIC_KEY; 
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
            if (crcState == SRC_FU_CRC_CALCULATING)
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
/* MISRA C-2012 deviation block start */
/* MISRA C-2012 Rule 16.4 deviated once. Deviation record ID - H3_MISRAC_2012_R_16_4_DR_1 */
         default:
            break;
/* MISRA C-2012 deviation block end */
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

    (void)memcpy(dst, (void *)bufferValue, size);
}

void SRV_FU_CfgWrite(void *src, uint16_t size)
{
    uint32_t bufferValue[4];

    (void)memcpy(bufferValue, (uint32_t *)src, size);

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
/* MISRA C-2012 deviation block start */
/* MISRA C-2012 Rule 16.4 deviated once. Deviation record ID - H3_MISRAC_2012_R_16_4_DR_1 */
        default:
            break;
/* MISRA C-2012 deviation block end */
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
    uint32_t destAddress = 0;
    uint32_t destSize = 0;

    /* Check if the current stack is 1.3 */
    SRV_STORAGE_PRIME_MODE_INFO_CONFIG boardInfo;

    if(SRV_STORAGE_GetConfigInfo(SRV_STORAGE_TYPE_MODE_PRIME, (uint8_t)sizeof(boardInfo), (void *)&boardInfo) == false)
    {
        return false;
    }

    if (boardInfo.primeVersion == PRIME_VERSION_1_3)
    {
        /* Verify if this is a right image */
        if (lSRV_FU_CheckImageData() == false)
        {
            /* Trigger reset, needed in FU 1.3 */
            return true;
        }
    }


    if(destSize == 0U)
    {
        return false;
    }
    else
    {
        /* Update boot configuration */
        SRV_STORAGE_BOOT_CONFIG bootConfig;

        if(SRV_STORAGE_GetConfigInfo(SRV_STORAGE_TYPE_BOOT_INFO, (uint8_t)sizeof(bootConfig), &bootConfig))
        {
            bootConfig.origAddr = DRV_MEMORY_AddressGet(memInfo.memoryHandle);
            bootConfig.destAddr = destAddress;
            bootConfig.imgSize = destSize;
            bootConfig.cfgKey = 0;
            bootConfig.pagesCounter = 0;
            bootConfig.bootState = 0;

            /* Store the new boot configuration */
            (void) SRV_STORAGE_SetConfigInfo(SRV_STORAGE_TYPE_BOOT_INFO, (uint8_t)sizeof(bootConfig), &bootConfig);

            return true;
        }
        else
        {
            return false;
        }
    }
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

    if (lSRV_FU_CheckImageData() != true)
    {
        /* Wrong Metadata, vendor or model */
        SRV_FU_ImageVerifyCallback(SRV_FU_VERIFY_RESULT_IMAGE_FAIL);
        
        return;
    }
    
    if (lSRV_FU_VerifySignature() != true)
    {
        /* Wrong signature */
        SRV_FU_ImageVerifyCallback(SRV_FU_VERIFY_RESULT_SIGNATURE_FAIL);
    }
}


