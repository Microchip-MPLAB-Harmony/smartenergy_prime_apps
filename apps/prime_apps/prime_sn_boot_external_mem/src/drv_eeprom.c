/*******************************************************************************
  Emulated EEPROM Driver Implementation File

  Company:
    Microchip Technology Inc.

  File Name:
    drv_eeprom.c

  Summary:
    Implementation of the boot-config write helper for the emulated
    EEPROM row.

  Description:
    The whole 256-byte row is staged in a stack-local buffer, which is
    fine: the bootloader runs on the reset stack, which is sized at the
    full 32 KB minus a few hundred bytes. The buffer is aligned so
    DRV_NVMCTRL_PageWrite can consume it word-by-word without an
    extra staging copy.
*******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "drv_eeprom.h"
#include "drv_nvmctrl.h"

// *****************************************************************************
// *****************************************************************************
// Section: Public Functions
// *****************************************************************************
// *****************************************************************************

void DRV_EEPROM_WriteBootConfig(const APP_BOOTLOADER_BOOT_CONFIG *cfg)
{
    /* 256-byte row buffer, word-aligned so it can be passed directly to
     * DRV_NVMCTRL_PageWrite as a uint32_t *. */
    uint32_t        rowBuf[APP_BOOTLOADER_EEPROM_ROW_SIZE / 4U];
    uint8_t        *pRowBytes;
    const uint8_t  *pFlashBytes;
    const uint8_t  *pCfgBytes;
    uint32_t        i;
    uint32_t        pageAddr;

    pRowBytes   = (uint8_t *) rowBuf;
    pFlashBytes = (const uint8_t *) APP_BOOTLOADER_EEPROM_ROW_ADDR;

    /* 1. Read the current row contents into RAM. */
    for (i = 0U; i < APP_BOOTLOADER_EEPROM_ROW_SIZE; i++)
    {
        pRowBytes[i] = pFlashBytes[i];
    }

    /* 2. Overlay the new boot configuration at the shared offset. */
    pCfgBytes = (const uint8_t *) cfg;
    for (i = 0U; i < sizeof(APP_BOOTLOADER_BOOT_CONFIG); i++)
    {
        pRowBytes[APP_BOOTLOADER_BOOT_CONFIG_OFFSET + i] = pCfgBytes[i];
    }

    /* 3. Unlock the region, erase the 256-byte row, rewrite all four
     *    64-byte pages. */
    DRV_NVMCTRL_RegionUnlock(APP_BOOTLOADER_EEPROM_ROW_ADDR);
    DRV_NVMCTRL_RowErase(APP_BOOTLOADER_EEPROM_ROW_ADDR);

    pageAddr = APP_BOOTLOADER_EEPROM_ROW_ADDR;
    for (i = 0U; i < (APP_BOOTLOADER_EEPROM_ROW_SIZE / DRV_NVMCTRL_PAGE_SIZE); i++)
    {
        DRV_NVMCTRL_PageWrite(&rowBuf[i * (DRV_NVMCTRL_PAGE_SIZE / 4U)],
                              pageAddr);
        pageAddr += DRV_NVMCTRL_PAGE_SIZE;
    }
}

/*******************************************************************************
 End of File
*/
