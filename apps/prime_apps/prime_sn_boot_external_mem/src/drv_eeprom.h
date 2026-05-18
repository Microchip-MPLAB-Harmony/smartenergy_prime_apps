/*******************************************************************************
  Emulated EEPROM Driver Header File

  Company:
    Microchip Technology Inc.

  File Name:
    drv_eeprom.h

  Summary:
    Driver for writing the 24-byte boot configuration back to the
    emulated-EEPROM row without disturbing the other bytes the
    application keeps there.

  Description:
    The EEPROM row at APP_BOOTLOADER_EEPROM_ROW_ADDR is shared with the
    application's srv_storage service. It contains MAC, PHY, security,
    mode, boot info, GPBR emulation slots and spare bytes packed into
    256 bytes. This bootloader only needs to update one 24-byte region
    of that row (the boot configuration at offset 112) after it has
    successfully applied TELECARGA or REVERT.

    The only safe way to do this on SAMD20 flash is the classic
    read-modify-erase-write cycle on the whole row:

      1. Read all 256 bytes into a RAM buffer.
      2. Overwrite the 24-byte boot config region.
      3. Unlock the region, erase the row, reprogram all four pages.

    The function below wraps that sequence behind a single call.
*******************************************************************************/

#ifndef DRV_EEPROM_H
#define DRV_EEPROM_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include "app_bootloader.h"

#ifdef __cplusplus
extern "C" {
#endif

// *****************************************************************************
// *****************************************************************************
// Section: Public Function Prototypes
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void DRV_EEPROM_WriteBootConfig (
        const APP_BOOTLOADER_BOOT_CONFIG *cfg )

  Summary:
    Persists a new boot configuration into the EEPROM row.

  Description:
    The 24 bytes of cfg overwrite
    APP_BOOTLOADER_EEPROM_ROW_ADDR + APP_BOOTLOADER_BOOT_CONFIG_OFFSET;
    the remaining 232 bytes of the row are read beforehand and
    reprogrammed unchanged. Blocks for roughly 15 ms while the row is
    erased and the four pages are written.

    DRV_NVMCTRL_Initialize() must have been called before this function.
*/

void DRV_EEPROM_WriteBootConfig(const APP_BOOTLOADER_BOOT_CONFIG *cfg);

#ifdef __cplusplus
}
#endif

#endif /* DRV_EEPROM_H */

/*******************************************************************************
 End of File
*/
