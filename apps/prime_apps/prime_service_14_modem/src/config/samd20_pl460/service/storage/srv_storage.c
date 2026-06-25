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
#include "srv_storage.h"
#include "device.h"
#include "definitions.h"

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

/* Non-volatile data block (16 slots * 4 B = 64 B) */
#define SRV_STORAGE_NON_VOLATILE_DATA_OFFSET       136

/* Total size of non-volatile data: 136 existing + 64 non-volatile = 200 */
#define SRV_STORAGE_TOTAL_SIZE 200U

/* Flash row reserved by NVMCTRL_EEPROM_SIZE = SIZE_256BYTES fuse (0x0003FF00-0x0003FFFF) */
#define SRV_STORAGE_FLASH_ADDR    NVMCTRL_EMULATED_EEPROM_START_ADDRESS

/* 200 bytes span 4 pages of 64 bytes; last page has 8 bytes data + 56 bytes 0xFF */
#define SRV_STORAGE_NUM_PAGES     4U
#define SRV_STORAGE_LAST_PAGE_OFFSET  ((SRV_STORAGE_NUM_PAGES - 1U) * NVMCTRL_EMULATED_EEPROM_PAGESIZE)  /* 192 */
#define SRV_STORAGE_LAST_PAGE_DATA    (SRV_STORAGE_TOTAL_SIZE - SRV_STORAGE_LAST_PAGE_OFFSET)             /* 8   */
#define SRV_STORAGE_LAST_PAGE_PAD     (NVMCTRL_EMULATED_EEPROM_PAGESIZE - SRV_STORAGE_LAST_PAGE_DATA)     /* 56  */

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

/* RAM cache. aligned(4) allows direct use as uint32_t*  */
static uint8_t srvStorageData[SRV_STORAGE_TOTAL_SIZE] __attribute__((aligned(4)));

// *****************************************************************************
// *****************************************************************************
// Section: Local Functions
// *****************************************************************************
// *****************************************************************************

static void lSRV_STORAGE_WriteToFlash(void)
{
    uint32_t lastPage[NVMCTRL_EMULATED_EEPROM_PAGESIZE / 4U];

    NVMCTRL_RegionUnlock(SRV_STORAGE_FLASH_ADDR);
    while (NVMCTRL_IsBusy()) {}

    /* Erase the 256-byte row */
    (void) NVMCTRL_RowErase(SRV_STORAGE_FLASH_ADDR);
    while (NVMCTRL_IsBusy()) {}

    /* Page 0: srvStorageData[0..63] -- base is aligned(4), offset 0 */
    (void) NVMCTRL_PageWrite((uint32_t *)(void *)&srvStorageData[0],
                             SRV_STORAGE_FLASH_ADDR);
    while (NVMCTRL_IsBusy()) {}

    /* Page 1: srvStorageData[64..127] -- offset 64 is divisible by 4 */
    (void) NVMCTRL_PageWrite((uint32_t *)(void *)&srvStorageData[NVMCTRL_EMULATED_EEPROM_PAGESIZE],
                             SRV_STORAGE_FLASH_ADDR + NVMCTRL_EMULATED_EEPROM_PAGESIZE);
    while (NVMCTRL_IsBusy()) {}

    /* Page 2: srvStorageData[128..191] -- contains tail of BOOT_INFO + full non-volatile data block */
    (void) NVMCTRL_PageWrite((uint32_t *)(void *)&srvStorageData[2U * NVMCTRL_EMULATED_EEPROM_PAGESIZE],
                             SRV_STORAGE_FLASH_ADDR + (2U * NVMCTRL_EMULATED_EEPROM_PAGESIZE));
    while (NVMCTRL_IsBusy()) {}

    /* Page 3 (last): 8 bytes of actual data + 56 bytes 0xFF padding */
    (void) memcpy(lastPage, &srvStorageData[SRV_STORAGE_LAST_PAGE_OFFSET],
                  SRV_STORAGE_LAST_PAGE_DATA);
    (void) memset((uint8_t *)lastPage + SRV_STORAGE_LAST_PAGE_DATA, 0xFFU,
                  SRV_STORAGE_LAST_PAGE_PAD);
    (void) NVMCTRL_PageWrite(lastPage,
                             SRV_STORAGE_FLASH_ADDR + SRV_STORAGE_LAST_PAGE_OFFSET);
    while (NVMCTRL_IsBusy()) {}
}

// *****************************************************************************
// *****************************************************************************
// Section: Storage Service Interface Implementation
// *****************************************************************************
// *****************************************************************************

void SRV_STORAGE_Initialize(void)
{
    /* Read the emulated-EEPROM row directly into the aligned cache */
    (void) NVMCTRL_Read((uint32_t *)(void *)srvStorageData, SRV_STORAGE_TOTAL_SIZE,
                        SRV_STORAGE_FLASH_ADDR);

    if (((uint16_t)srvStorageData[0] | ((uint16_t)srvStorageData[1] << 8U)) == 0xFFFFU)
    {
        /* First cfgKey is 0xFFFF -> flash is erased; default to all-zeros */
        (void) memset(srvStorageData, 0, SRV_STORAGE_TOTAL_SIZE);
    }
}

bool SRV_STORAGE_GetConfigInfo(SRV_STORAGE_TYPE infoType, uint8_t size, void* pData)
{
    uint16_t totalSize;
    uint8_t offset;
    bool interruptStatus;

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

    interruptStatus = SYS_INT_Disable();
    /* Serve from the RAM cache loaded at initialization */
    (void) memcpy(pData, (const void *) &srvStorageData[offset], size);
    SYS_INT_Restore(interruptStatus);

    return true;
}

bool SRV_STORAGE_SetConfigInfo(SRV_STORAGE_TYPE infoType, uint8_t size, void* pData)
{
    uint16_t totalSize;
    uint8_t offset;
    bool interruptStatus;

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

    interruptStatus = SYS_INT_Disable();
    /* Update the RAM cache and persist the whole row to the emulated EEPROM */
    (void) memcpy((void *) &srvStorageData[offset], pData, size);
    lSRV_STORAGE_WriteToFlash();

    SYS_INT_Restore(interruptStatus);
    return true;
}

uint32_t SRV_STORAGE_ReadNonVolatileData(uint8_t slot)
{
    uint32_t value;
    bool interruptStatus;

    if (slot >= SRV_STORAGE_NON_VOLATILE_DATA_NUM_SLOTS)
    {
        return 0U;
    }

    interruptStatus = SYS_INT_Disable();
    (void) memcpy(&value,
                  &srvStorageData[SRV_STORAGE_NON_VOLATILE_DATA_OFFSET + ((uint16_t)slot * 4U)],
                  sizeof(uint32_t));
    SYS_INT_Restore(interruptStatus);

    return value;
}

void SRV_STORAGE_WriteBlockNonVolatileData(uint8_t startSlot, uint8_t count,
                                const uint32_t *values)
{
    bool interruptStatus;

    if ((values == NULL) ||
        (startSlot >= SRV_STORAGE_NON_VOLATILE_DATA_NUM_SLOTS) ||
        ((uint16_t)startSlot + (uint16_t)count > SRV_STORAGE_NON_VOLATILE_DATA_NUM_SLOTS))
    {
        return;
    }

    interruptStatus = SYS_INT_Disable();
    (void) memcpy(&srvStorageData[SRV_STORAGE_NON_VOLATILE_DATA_OFFSET + ((uint16_t)startSlot * 4U)],
                  values, (size_t)count * 4U);
    lSRV_STORAGE_WriteToFlash();
    SYS_INT_Restore(interruptStatus);
}

void SRV_STORAGE_WriteNonVolatileData(uint8_t slot, uint32_t value)
{
    SRV_STORAGE_WriteBlockNonVolatileData(slot, 1U, &value);
}
