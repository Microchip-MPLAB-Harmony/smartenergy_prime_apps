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
/* SST26 layout v3: DOWNLOAD occupies the first 512 KB of the chip
 * (offset 0..0x7FFFF). The PRIME firmware-upgrade service stages the
 * incoming bundle in this region as the BS sends it; the bootloader
 * then parses BUNDLE_HEADER from offset 0 and dispatches the install.
 * The legacy 256 KB TELECARGA + 256 KB REVERT split is gone in v3 --
 * REVERT now lives in dedicated per-image zones (APP_REVERT,
 * PL360_REVERT) further into the chip, owned by the bootloader. */
#define PRIME_FU_MEM_SIZE       (uint32_t)(0x80000)

/* Destination address in internal flash where the bootloader installs
 * the image. Matches APP_BOOTLOADER_APP_START. */
#define PRIME_FU_APP_START_ADDR (uint32_t)(0x2000)

#define MEMORY_WRITE_SIZE       (uint32_t)(256)
#define MAX_BUFFER_READ_SIZE    (uint32_t)(256)

/* Crypto session identifier for the wolfcrypt-based hash + ECDSA verify
 * used by SRV_FU_VerifyImage. Single-context so any non-zero value works. */
#define SESSION_ID                    1

/* Maximum size reserved in RAM for the incoming image signature. ECDSA
 * P-256 raw is 64 B (r||s); the buffer is sized at 128 B to also fit
 * DER-encoded signatures (up to ~72 B + slack) before lSRV_FU_ConvertDer
 * collapses them to raw r||s. */
#define PRIME_SIGNATURE_SIZE          128

/* SHA-256 hash size in bytes. */
#define HASH_SIZE_SHA_256             32

/* ECDSA P-256 raw signature size (r||s, 32 B + 32 B). */
#define SIGNATURE_SIZE_ECDSA_256      64

/* External-memory BOOT_MODE_INFO handshake — must match the bootloader.
 * See app_bootloader.h: APP_BOOTLOADER_SST26_BOOT_FLAG_OFFSET and
 * APP_BOOTLOADER_BOOT_MODE_MAGIC. The 12-byte structure lives at the
 * start of the BOOT_FLAG sector; we erase 4 KB and write the first
 * 256-byte page (struct + 0xFF padding). */
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

/* Latest decoded BOOT_MODE_INFO read from BOOT_FLAG. Updated when
 * SRV_FU_ExtMemBootModeGet completes, or mirrored from the most recent
 * Set so a Status==OK after Set still gives a valid Result. */
static SRV_FU_EXT_MEM_BOOT_MODE_INFO bootModeResult;

static SRV_FU_CRC_STATE crcState;

static uint32_t crcReadAddress;

static uint32_t crcSize;

static uint32_t crcRemainingSize;

static uint32_t calculatedCrc;

/* Image signature captured from the tail of the FU bundle as the BS
 * streams it in (see lSRV_FU_StoreImageInfo). The buffer is verified
 * after the full image lands in DOWNLOAD via lSRV_FU_VerifySignature. */
static uint8_t imageSignature[PRIME_SIGNATURE_SIZE];

/* SHA-256 digest of the image (everything except the signature itself),
 * computed asynchronously chunk by chunk in SRV_FU_VERIFY_SIGNATURE_BLOCK. */
static uint8_t hashDigest[HASH_SIZE_SHA_256];

/* wolfcrypt hash context held across the chunked SHA-256 update calls. */
static st_Crypto_Hash_Sha_Ctx hashCtx;

/* DSA / signature-verification state machine. NO_PUBLIC_KEY at boot so
 * SRV_FU_VerifyImage rejects until SRV_FU_SetECDSAPublicKey is called. */
static SRV_FU_DSA_STATE dsaState;

/* Public key + length passed by the application via SRV_FU_SetECDSAPublicKey.
 * Stored as pointer (not copied) so the caller's static buffer must
 * outlive the FU service. */
static uint8_t *ECDSAPublicKey;
static uint32_t ECDSAPublicKeyLen;

/* Read-cursor / chunking bookkeeping for the SHA-256 streaming pass. */
static uint32_t dsaReadAddress;
static uint32_t dsaSize;
static uint32_t dsaRemainingSize;


// *****************************************************************************
// *****************************************************************************
// Section: File scope functions
// *****************************************************************************
// *****************************************************************************

/* Capture the trailing signature bytes from the FU bundle as the BS
 * streams it in. Mirrors the signature-only portion of the dual_modem
 * lSRV_FU_StoreImageInfo: the SAMD20 build has no per-image vendor /
 * model / metadata block, so only the last `signLength` bytes of the
 * image are extracted into imageSignature[]. Called from
 * SRV_FU_DataWrite for every incoming segment. */
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

/* Collapse a DER-encoded P-256 signature (70 B: 0x30 len 0x02 32 r... 0x02
 * 32 s...) into the 64-byte r||s raw form expected by Crypto_DigiSign_Ecdsa_Verify.
 * Called by lSRV_FU_VerifySignature when fuData.signLength == 70. */
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

    /* Boot-mode handshake completions are handled here without ever
     * touching the FU state machine or invoking SRV_FU_MemTransferCallback.
     * Tasks() picks up the new state on the next iteration. */
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
                /* Erase ok → Tasks() will kick the page write. */
                mInfo->state = SRV_FU_MEM_STATE_EXT_MEM_BOOT_MODE_WRITE_KICK;
                break;

            case SRV_FU_MEM_STATE_EXT_MEM_BOOT_MODE_WRITE:
                /* Page written → Set sequence done. */
                mInfo->bootModeStatus = (uint8_t) SRV_FU_EXT_MEM_BOOT_MODE_STATUS_OK;
                mInfo->state = SRV_FU_MEM_STATE_CMD_WAIT;
                break;

            case SRV_FU_MEM_STATE_EXT_MEM_BOOT_MODE_READ:
                /* Page read → Tasks() will validate magic + modeXor. */
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
            /* Calculating SHA.... no callback. Tasks() will pick up the
             * new dsaState and feed the chunk into the hash context. */
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

/* Kick off the chunked SHA-256 + ECDSA-P256 verification of the image
 * sitting in DOWNLOAD. Returns true if a verification has been started
 * (or no verification is required), false on hard error. The actual
 * hashing runs asynchronously in SRV_FU_VERIFY_SIGNATURE_BLOCK. */
static bool lSRV_FU_VerifySignature(void)
{
    crypto_Hash_Status_E stateCryptoHash;

    if (fuData.signAlgorithm == SRV_FU_SIGNATURE_ALGO_NO_SIGNATURE)
    {
        /* Unsigned image: nothing to verify, finish synchronously with
         * SUCCESS so the PRIME stack moves on to the swap step. */
        SRV_FU_ImageVerifyCallback(SRV_FU_VERIFY_RESULT_SUCCESS);
        dsaState = SRV_FU_DSA_IDLE;
        memInfo.state = SRV_FU_MEM_STATE_CMD_WAIT;

        return true;
    }

    if (fuData.signAlgorithm != SRV_FU_SIGNATURE_ALGO_ECDSA256_SHA256)
    {
        /* Only ECDSA P-256 + SHA-256 is supported on this build. */
        return false;
    }

    if (dsaState != SRV_FU_DSA_IDLE)
    {
        /* DSA state machine not idle; either still verifying, or no
         * public key was registered yet (NO_PUBLIC_KEY). */
        return false;
    }

    /* Some BS / signing tools wrap the raw r||s into a 70-byte ASN.1
     * DER SEQUENCE. Detect by length and collapse to raw 64-byte form
     * so the verify call below can take it as-is. */
    if (fuData.signLength == 70UL)
    {
        lSRV_FU_ConvertDerFormatSignature();
    }

    /* Start to verify the signature: init the SHA-256 context and queue
     * the first chunk read from DOWNLOAD. */
    stateCryptoHash = Crypto_Hash_Sha_Init(&hashCtx, CRYPTO_HASH_SHA2_256,
                                            CRYPTO_HANDLER_SW_WOLFCRYPT,
                                            SESSION_ID);

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
        /* Round the read up to whole pages so DRV_MEMORY_AsyncRead has
         * a clean operand; the SHA size stays aligned with what we
         * actually intend to feed into the hash. */
        if (dsaSize > bytesPagesRead)
        {
            if (((nBlock + 1U) * memInfo.readPageSize) <= MAX_BUFFER_READ_SIZE)
            {
                nBlock++;
            }
            else
            {
                /* Cannot read everything; cap dsaSize at full pages. */
                dsaSize = bytesPagesRead;
            }
        }

        DRV_MEMORY_AsyncRead(memInfo.memoryHandle, &memInfo.readHandle,
                             pBuffInput, blockStart, nBlock);

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

    /* No public key registered yet: SRV_FU_VerifyImage must reject
     * signed images until the application calls SRV_FU_SetECDSAPublicKey. */
    dsaState = SRV_FU_DSA_NO_PUBLIC_KEY;
    ECDSAPublicKey = NULL;
    ECDSAPublicKeyLen = 0U;

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
            /* Erase finished, queue the page write that contains the
             * 12-byte struct (rest of the page is 0xFF padding). */
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
            /* Decode the struct from pBuffInput. Bad magic or modeXor is
             * not an error: it just means the sector is virgin or
             * corrupted, so report NORMAL with status = OK and let the
             * caller treat it as the safe default. */
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
                    /* Intermediate chunk: feed it to the SHA context. */
                    stateCryptoHash = Crypto_Hash_Sha_Update(&hashCtx,
                                                              pBuffInput, dsaSize);
                }
                else
                {
                    /* Last chunk: feed it AND finalize the digest so the
                     * ECDSA verify call below has a valid hash. */
                    stateCryptoHash = Crypto_Hash_Sha_Update(&hashCtx,
                                                              pBuffInput, dsaSize);
                    stateCryptoHash = Crypto_Hash_Sha_Final(&hashCtx, hashDigest);
                }

                if (stateCryptoHash != CRYPTO_HASH_SUCCESS)
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
                    if (dsaSize > bytesPagesRead)
                    {
                        if (((nBlock + 1U) * memInfo.readPageSize) <= MAX_BUFFER_READ_SIZE)
                        {
                            nBlock++;
                        }
                        else
                        {
                            dsaSize = bytesPagesRead;
                        }
                    }

                    DRV_MEMORY_AsyncRead(memInfo.memoryHandle, &memInfo.readHandle,
                                         pBuffInput, blockStart, nBlock);

                    dsaReadAddress += dsaSize;
                    dsaRemainingSize -= dsaSize;
                }
                else
                {
                    crypto_DigiSign_Status_E stateCryptoECDSA;
                    int8_t validDSA = 0;

                    /* Hash done; perform ECDSA P-256 / SHA-256 verify against
                     * the signature captured at the tail of the image. */
                    stateCryptoECDSA = Crypto_DigiSign_Ecdsa_Verify(
                            CRYPTO_HANDLER_SW_WOLFCRYPT,
                            hashDigest,
                            HASH_SIZE_SHA_256,
                            imageSignature,
                            SIGNATURE_SIZE_ECDSA_256,
                            ECDSAPublicKey,
                            ECDSAPublicKeyLen,
                            &validDSA,
                            CRYPTO_ECC_CURVE_P256,
                            SESSION_ID);

                    if ((validDSA != 1) ||
                        (stateCryptoECDSA != CRYPTO_DIGISIGN_SUCCESS))
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

    /* Capture the trailing signature bytes (if any) into imageSignature[]
     * before the segment is committed to DOWNLOAD. SRV_FU_VerifyImage
     * later runs the SHA-256 over the rest of the image and compares
     * against this captured signature. */
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
    uint32_t bufferValue[4] = {0U};

    (void)memcpy((void *)bufferValue, src, size);

    SRV_STORAGE_WriteBlockNonVolatileData(0U, 4U, bufferValue);
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
    uint8_t mode;

    /* v3 hand-off: kick the BOOT_MODE_INFO write at the start of the
     * SST26 BOOT_FLAG sector. The bootloader (v3) reads it on every
     * reset and dispatches install or revert based on `mode`. The
     * actual sector erase + page program is async -- it runs through
     * SRV_FU_Tasks; the caller polls SRV_FU_ExtMemBootModeStatus()
     * until OK before resetting the device. */
    mode = fuLastResultIsRevert
         ? SRV_FU_EXT_MEM_BOOT_MODE_REVERT_PENDING
         : SRV_FU_EXT_MEM_BOOT_MODE_INSTALL_PENDING;

    return SRV_FU_ExtMemBootModeSet(mode, 0U, 0U);
}


void SRV_FU_VerifyImage(void)
{
    if (SRV_FU_ImageVerifyCallback == NULL)
    {
        return;
    }

    /* Kick the chunked SHA-256 + ECDSA verify state machine. Returns
     * synchronously SUCCESS when the image is unsigned (signAlgorithm
     * == NO_SIGNATURE); for ECDSA images, completion happens later in
     * SRV_FU_VERIFY_SIGNATURE_BLOCK and the callback is invoked there.
     * Returns false for unsupported algorithms or when the FU is not
     * idle / no public key registered. */
    if (lSRV_FU_VerifySignature() != true)
    {
        SRV_FU_ImageVerifyCallback(SRV_FU_VERIFY_RESULT_SIGNATURE_FAIL);
    }
}

void SRV_FU_SetECDSAPublicKey(uint8_t *pubKey, uint32_t pubKeyLen)
{
    ECDSAPublicKey = pubKey;
    ECDSAPublicKeyLen = pubKeyLen;
    /* Move out of NO_PUBLIC_KEY now that the verifier has something to
     * check against. SRV_FU_VerifyImage will refuse to start while
     * dsaState != IDLE; this transition unblocks it for signed images. */
    dsaState = SRV_FU_DSA_IDLE;
}

bool SRV_FU_ExtMemBootModeSet(uint8_t mode, uint8_t imageIdx, uint8_t imageStep)
{
    uint32_t eraseBlockSize;
    uint32_t eraseBlockStartBootFlag;

    /* Only acceptable when the FU service is fully initialized and idle.
     * The boot-mode path shares the FU's DRV_MEMORY client; running it
     * concurrently with a real FU op would corrupt memInfo.state. */
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


