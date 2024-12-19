/*******************************************************************************
  PRIME Firmware Upgrade Service Implementation.

  Company:
    Microchip Technology Inc.

  File Name:
    srv_firmware_upgrade.c

  Summary:
    PRIME Firmware Upgrade Service Interface Source File.

  Description:
    The Firmware Upgrade service provides the handling of the firmare upgrade
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
#define PRIME_FU_MEM_SIZE       (uint32_t)(0x60000)

#define MEMORY_WRITE_SIZE       (uint32_t)(512)
#define MAX_BUFFER_READ_SIZE    (uint32_t)(1024)


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
            crcState = SRC_FU_CRC_CALCULATING;
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
            memInfo.memoryHandle = DRV_MEMORY_Open(DRV_MEMORY_INDEX_0, DRV_IO_INTENT_READWRITE);

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
            (void)memcpy( &pMemWrite[offset], &pBuffInput[memInfo.bytesWritten] , memInfo.writeSize);

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
                    /* Aling CRC size with the readPageSize */
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


    memInfo.state = SRV_FU_MEM_STATE_WRITE_ONE_BLOCK;
}

void SRV_FU_CfgRead(void *dst, uint16_t size)
{
	uint32_t bufferValue[4];
	uint32_t *pointerBuffer;

	pointerBuffer = (uint32_t *)bufferValue;
	*pointerBuffer++ = SUPC_GPBRRead(GPBR_REGS_0);
	*pointerBuffer++ = SUPC_GPBRRead(GPBR_REGS_1);
	*pointerBuffer++ = SUPC_GPBRRead(GPBR_REGS_2);
	*pointerBuffer = SUPC_GPBRRead(GPBR_REGS_3);
	(void)memcpy(dst, (void *)bufferValue, size);

}

void SRV_FU_CfgWrite(void *src, uint16_t size)
{
	uint32_t bufferValue[4];
	uint32_t *pointerBuffer;

	(void)memcpy(bufferValue, (uint32_t *)src, size);

	pointerBuffer = (uint32_t *)bufferValue;
	SUPC_GPBRWrite(GPBR_REGS_0, *pointerBuffer++);
	SUPC_GPBRWrite(GPBR_REGS_1, *pointerBuffer++);
	SUPC_GPBRWrite(GPBR_REGS_2, *pointerBuffer++);
	SUPC_GPBRWrite(GPBR_REGS_3, *pointerBuffer);
}

void SRV_FU_Start(SRV_FU_INFO *fuInfo)
{
	fuData.imageSize = fuInfo->imageSize;
	fuData.pageSize = fuInfo->pageSize;
	fuData.signAlgorithm = SRV_FU_SIGNATURE_ALGO_NO_SIGNATURE;
	fuData.signLength = 0;

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
    /* Aling CRC size with the readPageSize */
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


void SRV_FU_VerifyImage(void)
{
	/* Check pointer function */
	if (SRV_FU_ImageVerifyCallback == NULL)
    {
		return;
	}

}


