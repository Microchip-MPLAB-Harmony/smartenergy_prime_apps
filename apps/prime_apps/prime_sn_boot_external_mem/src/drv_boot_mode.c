/*******************************************************************************
  Boot Mode Driver Implementation File

  Company:
    Microchip Technology Inc.

  File Name:
    drv_boot_mode.c

  Summary:
    Bare-metal R/W of the BOOT_MODE_INFO handshake structure on the SST26.

  Description:
    See drv_boot_mode.h for the contract. This implementation:

      - Read:  one SST26 read of 12 bytes at the start of the BOOT_FLAG
               sector, then validates magic and modeXor.

      - Write: builds the on-flash struct (computing magic and modeXor),
               erases the 4 KB sector, and writes the first page (256 B,
               with the 12-byte struct followed by 0xFF padding).
*******************************************************************************/

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

    /* Sanity: magic and modeXor must match. Anything else (virgin sector
     * with magic 0xFFFFFFFF, partial write that broke the invariant,
     * bit rot) is treated as NORMAL — the bootloader jumps to the app. */
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
    /* Page-sized buffer for the page program. The struct lives at the
     * start of the first page; the rest is 0xFF padding. */
    uint8_t page[DRV_SST26_PAGE_SIZE];
    APP_BOOTLOADER_BOOT_MODE_INFO toWrite;

    if (info == NULL)
    {
        return;
    }

    /* Build the on-flash structure: copy the caller-provided fields and
     * compute the magic + modeXor sanity. */
    toWrite.magic     = APP_BOOTLOADER_BOOT_MODE_MAGIC;
    toWrite.mode      = info->mode;
    toWrite.imageIdx  = info->imageIdx;
    toWrite.imageStep = info->imageStep;
    toWrite.reserved  = 0U;
    toWrite.modeXor   = (uint32_t) info->mode
                      ^ (APP_BOOTLOADER_BOOT_MODE_MAGIC & 0xFFU);

    (void) memset(page, 0xFFU, sizeof(page));
    (void) memcpy(page, &toWrite, sizeof(toWrite));

    /* Erase the 4 KB sector reserved for BOOT_FLAG, then program the
     * 256-byte page that contains the struct. The remaining sector
     * stays at 0xFF and will be overwritten on the next call. */
    DRV_SST26_SectorErase4K(APP_BOOTLOADER_SST26_BOOT_FLAG_OFFSET);
    DRV_SST26_WritePage(APP_BOOTLOADER_SST26_BOOT_FLAG_OFFSET,
                        page, sizeof(page));
}

/*******************************************************************************
 End of File
*/
