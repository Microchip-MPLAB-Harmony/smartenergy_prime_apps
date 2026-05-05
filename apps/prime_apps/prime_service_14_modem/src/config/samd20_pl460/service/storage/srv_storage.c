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

/* Total size of non-volatile data */
#define SRV_STORAGE_TOTAL_SIZE 136U

/* Flash row reserved by NVMCTRL_EEPROM_SIZE = SIZE_256BYTES fuse (0x0003FF00-0x0003FFFF) */
#define SRV_STORAGE_FLASH_ADDR    NVMCTRL_EMULATED_EEPROM_START_ADDRESS

/* 136 bytes span 3 pages of 64 bytes; last page has 8 bytes data + 56 bytes 0xFF */
#define SRV_STORAGE_NUM_PAGES     3U
#define SRV_STORAGE_LAST_PAGE_OFFSET  ((SRV_STORAGE_NUM_PAGES - 1U) * NVMCTRL_EMULATED_EEPROM_PAGESIZE)  /* 128 */
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

/* RAM cache. aligned(4) allows direct use as uint32_t* in NVMCTRL_Read/PageWrite
 * without an intermediate staging buffer (saves 192 bytes of static RAM). */
static uint8_t srvStorageData[SRV_STORAGE_TOTAL_SIZE] __attribute__((aligned(4)));

// *****************************************************************************
// *****************************************************************************
// Section: Local Functions
// *****************************************************************************
// *****************************************************************************

static void lSRV_STORAGE_WriteToFlash(void)
{
    /* 64-byte buffer for the last partial page (8 bytes data + 56 bytes 0xFF).
     * Stack use is temporary and far smaller than the 192-byte static buffer
     * that would otherwise be needed. */
    uint32_t lastPage[NVMCTRL_EMULATED_EEPROM_PAGESIZE / 4U];

    /* NVMCTRL_REGION_LOCKS fuse = 0xFFFF: all regions re-locked on every reset.
     * The ER/WP commands are silently rejected on a locked region and, on some
     * SAMD20 silicon, INTFLAG.READY is never set — causing an infinite busy-wait
     * and a subsequent WDT reset. Unlock before each write sequence. */
    NVMCTRL_RegionUnlock(SRV_STORAGE_FLASH_ADDR);
    while (NVMCTRL_IsBusy()) {}

    /* Erase the 256-byte row (~6 ms) */
    (void) NVMCTRL_RowErase(SRV_STORAGE_FLASH_ADDR);
    while (NVMCTRL_IsBusy()) {}

    /* Page 0: srvStorageData[0..63] — base is aligned(4), offset 0 */
    (void) NVMCTRL_PageWrite((uint32_t *)(void *)&srvStorageData[0],
                             SRV_STORAGE_FLASH_ADDR);
    while (NVMCTRL_IsBusy()) {}

    /* Page 1: srvStorageData[64..127] — offset 64 is divisible by 4 */
    (void) NVMCTRL_PageWrite((uint32_t *)(void *)&srvStorageData[NVMCTRL_EMULATED_EEPROM_PAGESIZE],
                             SRV_STORAGE_FLASH_ADDR + NVMCTRL_EMULATED_EEPROM_PAGESIZE);
    while (NVMCTRL_IsBusy()) {}

    /* Page 2: 8 bytes of actual data + 56 bytes 0xFF padding */
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
    /* Read 136 bytes from the emulated EEPROM row directly into the aligned cache.
     * On virgin flash (all 0xFF) zero the cache: maintains the same behaviour as
     * the previous BSS-zero stub so that SRV_FU and other callers that read boot
     * config without a cfgKey check do not see garbage 0xFF addresses. */
    (void) NVMCTRL_Read((uint32_t *)(void *)srvStorageData, SRV_STORAGE_TOTAL_SIZE,
                        SRV_STORAGE_FLASH_ADDR);

    if (((uint16_t)srvStorageData[0] | ((uint16_t)srvStorageData[1] << 8U)) == 0xFFFFU)
    {
        /* First cfgKey is 0xFFFF → flash is erased; default to all-zeros */
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
        return false;
    }

    offset = srvStorageOffsetList[infoType];
    totalSize = (uint16_t) offset + (uint16_t) size;

    if (totalSize > SRV_STORAGE_TOTAL_SIZE)
    {
        return false;
    }

    interruptStatus = SYS_INT_Disable();
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
        return false;
    }

    offset = srvStorageOffsetList[infoType];
    totalSize = (uint16_t) offset + (uint16_t) size;

    if (totalSize > SRV_STORAGE_TOTAL_SIZE)
    {
        return false;
    }

    /* Interrupts disabled for the full write: SAMD20 stalls the AHB bus during
     * NVM erase/write, so ISRs in flash cannot execute during this window.
     * Total duration: RegionUnlock (~1ms) + RowErase (~6ms) + 3×PageWrite (~7.5ms)
     * ≈ 14ms — well within the 500ms WDT timeout. */
    interruptStatus = SYS_INT_Disable();

    (void) memcpy((void *) &srvStorageData[offset], pData, size);
    lSRV_STORAGE_WriteToFlash();

    SYS_INT_Restore(interruptStatus);

    return true;
}
