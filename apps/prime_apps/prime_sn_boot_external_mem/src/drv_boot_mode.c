/*******************************************************************************
  Boot Mode Driver Implementation File

  Company:
    Microchip Technology Inc.

  File Name:
    drv_boot_mode.c

  Summary:
    Bare-metal R/W of the BOOT_MODE_INFO handshake structure on the SST26.

  Description:
    Hand-written driver: no Harmony. See drv_boot_mode.h for the
    contract. This implementation:

      - Read:  one SST26 read of 12 bytes at the start of the BOOT_FLAG
               sector, then validates magic and modeXor.

      - Write: builds the on-flash struct (computing magic and modeXor),
               erases the 4 KB sector, and writes the first page (256 B,
               with the 12-byte struct followed by 0xFF padding).
*******************************************************************************/

//DOM-IGNORE-BEGIN
/*
Copyright (C) 2026 Microchip Technology Inc., and its subsidiaries. All rights reserved.

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
#include "drv_boot_mode.h"
#include "drv_sst26.h"

// *****************************************************************************
// *****************************************************************************
// Section: Public Functions
// *****************************************************************************
// *****************************************************************************

APP_BOOTLOADER_BOOT_MODE_INFO DRV_BOOT_MODE_Read(void)
{
    APP_BOOTLOADER_BOOT_MODE_INFO info;
    uint8_t  raw[sizeof(APP_BOOTLOADER_BOOT_MODE_INFO)];
    uint32_t expectedXor;

    /* Read the persisted bytes. */
    DRV_SST26_Read(APP_BOOTLOADER_SST26_BOOT_FLAG_OFFSET,
                   raw, sizeof(raw));
    (void) memcpy(&info, raw, sizeof(info));

    expectedXor = (uint32_t) info.mode ^ (APP_BOOTLOADER_BOOT_MODE_MAGIC & 0xFFU);

    if ((info.magic != APP_BOOTLOADER_BOOT_MODE_MAGIC) ||
        (info.modeXor != expectedXor))
    {
        info.magic     = 0U;
        info.mode      = (uint8_t) APP_BOOTLOADER_BOOT_MODE_NORMAL;
        info.imageIdx  = 0U;
        info.imageStep = 0U;
        info.reserved  = 0U;
        info.modeXor   = 0U;
    }

    return info;
}

void DRV_BOOT_MODE_Write(const APP_BOOTLOADER_BOOT_MODE_INFO *info)
{
    /* Page-sized buffer for the page program. */
    uint8_t page[DRV_SST26_PAGE_SIZE];
    APP_BOOTLOADER_BOOT_MODE_INFO toWrite;

    if (info == NULL)
    {
        return;
    }

    /* Build the on-flash structure. */
    toWrite.magic     = APP_BOOTLOADER_BOOT_MODE_MAGIC;
    toWrite.mode      = info->mode;
    toWrite.imageIdx  = info->imageIdx;
    toWrite.imageStep = info->imageStep;
    toWrite.reserved  = 0U;
    toWrite.modeXor   = (uint32_t) info->mode
                      ^ (APP_BOOTLOADER_BOOT_MODE_MAGIC & 0xFFU);

    (void) memset(page, 0xFFU, sizeof(page));
    (void) memcpy(page, &toWrite, sizeof(toWrite));

    /* Erase the 4 KB sector reserved for BOOT_FLAG */
    DRV_SST26_SectorErase4K(APP_BOOTLOADER_SST26_BOOT_FLAG_OFFSET);
    DRV_SST26_WritePage(APP_BOOTLOADER_SST26_BOOT_FLAG_OFFSET,
                        page, sizeof(page));
}

/*******************************************************************************
 End of File
*/
