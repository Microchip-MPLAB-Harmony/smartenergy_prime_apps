/*******************************************************************************
  Boot Mode Driver Header File

  Company:
    Microchip Technology Inc.

  File Name:
    drv_boot_mode.h

  Summary:
    Persists the BOOT_MODE_INFO handshake structure into the dedicated
    BOOT_FLAG sector of the external SST26 serial flash.

  Description:
    The bootloader and the modem application share a 12-byte structure
    that controls what the bootloader does on every reset:

      - NORMAL          jump to the application
      - INSTALL_PENDING install bundle currently in DOWNLOAD zone
      - REVERT_PENDING  restore from REVERT zones
      - UART_PENDING    enter UART recovery mode

    The application sets the structure to one of these modes and triggers
    a reset. The bootloader reads the structure on every boot and acts
    accordingly. Once the requested action completes, the bootloader
    writes mode = NORMAL.

    The structure is stored in the BOOT_FLAG zone of the SST26 starting at
    APP_BOOTLOADER_SST26_BOOT_FLAG_OFFSET. A 4 KB sector is reserved for
    it; only the first 12 bytes are used. Each write does a sector erase
    (~25 ms) plus a 256-byte page program (~3 ms).

    Endurance budget: SST26 sector ~100 000 erase cycles, ~6 writes per
    install → ~16 000 firmware upgrades over the device lifetime.

    Power-loss vulnerability: the ~28 ms window between the sector erase
    and the page write leaves the structure momentarily virgin. If a
    power-loss happens in that window the bootloader treats the read as
    NORMAL on next boot and falls back to running the application as it
    is (safe default). The pending operation must be retried by the
    higher level (BS PRIME / operator).

    DRV_SPI_Initialize() and DRV_SST26_Initialize() must have run before
    any function in this module.
*******************************************************************************/

#ifndef DRV_BOOT_MODE_H
#define DRV_BOOT_MODE_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

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
    APP_BOOTLOADER_BOOT_MODE_INFO DRV_BOOT_MODE_Read ( void )

  Summary:
    Reads the BOOT_MODE_INFO structure from the BOOT_FLAG sector.

  Description:
    Reads 12 bytes from the start of the BOOT_FLAG sector and validates
    them. The returned structure is "valid" only when both:
       - magic == APP_BOOTLOADER_BOOT_MODE_MAGIC
       - modeXor == (mode XOR (magic & 0xFF))

    On any inconsistency (virgin sector, partial write, bit rot) the
    function returns a structure with mode = NORMAL and magic = 0 — the
    safe default that makes the bootloader jump straight to the
    application.
*/

APP_BOOTLOADER_BOOT_MODE_INFO DRV_BOOT_MODE_Read(void);

/*******************************************************************************
  Function:
    void DRV_BOOT_MODE_Write (
        const APP_BOOTLOADER_BOOT_MODE_INFO *info )

  Summary:
    Persists a new BOOT_MODE_INFO structure into the BOOT_FLAG sector.

  Description:
    Computes magic and modeXor from the supplied mode/imageIdx/imageStep
    values, erases the BOOT_FLAG sector, and writes the 12-byte
    structure plus 0xFF padding into the first page of the sector.

    Blocks for ~28 ms (sector erase + page program). Returns once the
    SST26 reports BUSY = 0.
*/

void DRV_BOOT_MODE_Write(const APP_BOOTLOADER_BOOT_MODE_INFO *info);

#ifdef __cplusplus
}
#endif

#endif /* DRV_BOOT_MODE_H */

/*******************************************************************************
 End of File
*/
