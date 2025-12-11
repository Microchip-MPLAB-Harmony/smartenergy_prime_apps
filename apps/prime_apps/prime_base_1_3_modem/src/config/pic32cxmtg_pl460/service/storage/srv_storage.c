/*******************************************************************************
  PRIME Non-Volatile Storage Service Implementation.

  Company:
    Microchip Technology Inc.

  File Name:
    srv_storage.c

  Summary:
    Source code for the PRIME non-volatile Storage service implementation.

  Description:
    The non-volatile Storage service provides a simple interface to read and
    write non-volatile data used by the PRIME stack. This file contains the
    source code for the implementation of the Storage service.
*******************************************************************************/

//DOM-IGNORE-BEGIN
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
#include "srv_storage.h"
#include "device.h"
#include "peripheral/sefc/plib_sefc0.h"

// *****************************************************************************
// *****************************************************************************
// Section: Macro definitions
// *****************************************************************************
// *****************************************************************************

/* Offsets in bytes of each storage information type */
#define SRV_STORAGE_MAC_INFO_OFFSET   0
#define SRV_STORAGE_PHY_INFO_OFFSET   16
#define SRV_STORAGE_BN_INFO_OFFSET    32
#define SRV_STORAGE_MODE_PRIME_OFFSET 48
#define SRV_STORAGE_SECURITY_OFFSET   64
#define SRV_STORAGE_BOOT_INFO_OFFSET  112

/* Total size of non-volatile data */
#define SRV_STORAGE_TOTAL_SIZE 136U

// *****************************************************************************
// *****************************************************************************
// Section: File Scope Data
// *****************************************************************************
// *****************************************************************************

/* Offset in bytes for each storage information type */
static const uint8_t srvStorageOffsetList[SRV_STORAGE_TYPE_END_LIST] = {
    SRV_STORAGE_MAC_INFO_OFFSET,
    SRV_STORAGE_PHY_INFO_OFFSET,
    SRV_STORAGE_BN_INFO_OFFSET,
    SRV_STORAGE_MODE_PRIME_OFFSET,
    SRV_STORAGE_SECURITY_OFFSET,
    SRV_STORAGE_BOOT_INFO_OFFSET
};

/* Array to read/write non-volatile data */
static uint8_t srvStorageData[SRV_STORAGE_TOTAL_SIZE];

// *****************************************************************************
// *****************************************************************************
// Section: Storage Service Interface Implementation
// *****************************************************************************
// *****************************************************************************

void SRV_STORAGE_Initialize(void)
{
    /* Disable User Signature write protection */
    SEFC0_WriteProtectionSet(0);

    /* Enable write and read User Signature (block 0 / area 1) rights */
    SEFC0_UserSignatureRightsSet(SEFC_EEFC_USR_RDENUSB1_Msk | SEFC_EEFC_USR_WRENUSB1_Msk);
}

bool SRV_STORAGE_GetConfigInfo(SRV_STORAGE_TYPE infoType, uint8_t size, void* pData)
{
    uint16_t totalSize;
    uint8_t offset;

    if (infoType >= SRV_STORAGE_TYPE_END_LIST)
    {
        /* Invalid type */
        return false;
    }

    /* Get offset depending on info type */
    offset = srvStorageOffsetList[infoType];
    totalSize = (uint16_t) offset + (uint16_t) size;

    if (totalSize > SRV_STORAGE_TOTAL_SIZE)
    {
        /* Invalid size */
        return false;
    }

    /* Read data from User Signature */
    if (SEFC0_UserSignatureRead((void*) srvStorageData, ((uint32_t) totalSize + 3U) >> 2, BLOCK_0, PAGE_0) == false)
    {
        /* Error reading User Signature */
        return false;
    }

    /* Copy data to pointer given as parameter */
    (void) memcpy(pData, (void*) &srvStorageData[offset], size);

    return true;
}

bool SRV_STORAGE_SetConfigInfo(SRV_STORAGE_TYPE infoType, uint8_t size, void* pData)
{
    uint16_t totalSize;
    uint8_t offset;

    if (infoType >= SRV_STORAGE_TYPE_END_LIST)
    {
        /* Invalid type */
        return false;
    }

    /* Get offset depending on info type */
    offset = srvStorageOffsetList[infoType];
    totalSize = (uint16_t) offset + (uint16_t) size;

    if (totalSize > SRV_STORAGE_TOTAL_SIZE)
    {
        /* Invalid size */
        return false;
    }

    /* Read data from User Signature */
    if (SEFC0_UserSignatureRead((void*) srvStorageData, SRV_STORAGE_TOTAL_SIZE >> 2, BLOCK_0, PAGE_0) == false)
    {
        /* Error reading User Signature */
        return false;
    }

    /* Copy new data */
    (void) memcpy((void*) &srvStorageData[offset], pData, size);

    /* Erase User Signature */
    SEFC0_UserSignatureErase(BLOCK_0);

    /* Wait until User Signature is erased */
    while(SEFC0_IsBusy()){}

    /* Write User Signature */
    if (SEFC0_UserSignatureWrite((void*) srvStorageData, SRV_STORAGE_TOTAL_SIZE >> 2, BLOCK_0, PAGE_0) == false)
    {
        /* Error writing User Signature */
        return false;
    }

    /* Wait until User Signature is written */
    while(SEFC0_IsBusy()){}

    return true;
}
