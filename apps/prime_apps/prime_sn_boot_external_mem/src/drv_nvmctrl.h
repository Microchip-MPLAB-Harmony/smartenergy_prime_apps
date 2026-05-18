/*******************************************************************************
  NVMCTRL Driver Header File

  Company:
    Microchip Technology Inc.

  File Name:
    drv_nvmctrl.h

  Summary:
    Minimal bare-metal driver for the SAMD20 internal flash controller.

  Description:
    Used by the bootloader to program the application region and the
    emulated-EEPROM row. Operations are synchronous: every call waits for
    the previous one to finish before returning.

    Flash geometry on the ATSAMD20J18:
      - Row  = 256 bytes (smallest erase granularity)
      - Page = 64 bytes  (smallest program granularity)
      - 4 pages per row

    All addresses are byte addresses in the CPU memory map. The driver
    converts to the halfword-based ADDR register internally (address >> 1).

    The NVMCTRL_REGION_LOCKS fuse normally locks every region on reset,
    which means the caller must invoke DRV_NVMCTRL_RegionUnlock for the
    row being written before the RowErase/PageWrite sequence. The modem
    project's srv_storage does the same — the ER/WP commands are
    silently rejected on a locked region and INTFLAG.READY can even get
    stuck on some SAMD20 silicon.
*******************************************************************************/

#ifndef DRV_NVMCTRL_H
#define DRV_NVMCTRL_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// *****************************************************************************
// *****************************************************************************
// Section: Constants
// *****************************************************************************
// *****************************************************************************

#define DRV_NVMCTRL_PAGE_SIZE       64U
#define DRV_NVMCTRL_ROW_SIZE        256U
#define DRV_NVMCTRL_PAGES_PER_ROW   4U

// *****************************************************************************
// *****************************************************************************
// Section: Public Function Prototypes
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void DRV_NVMCTRL_Initialize ( void )

  Summary:
    Configures NVMCTRL CTRLB with manual-write mode and one read wait state.

  Description:
    Must be called once, before any other function in this module. MANW=1
    makes PageWrite the explicit trigger for the flash program cycle
    (instead of it happening automatically on the last page-buffer word),
    which gives the driver full control over when the stall starts.
*/

void DRV_NVMCTRL_Initialize(void);

/*******************************************************************************
  Function:
    void DRV_NVMCTRL_RegionUnlock ( uint32_t address )

  Summary:
    Unlocks the flash region that contains address.

  Description:
    Required before erasing/writing any location when NVMCTRL_REGION_LOCKS
    has locked all regions at reset (the default on these boards).
    Blocks until the command completes.
*/

void DRV_NVMCTRL_RegionUnlock(uint32_t address);

/*******************************************************************************
  Function:
    void DRV_NVMCTRL_RowErase ( uint32_t address )

  Summary:
    Erases the 256-byte row that contains address.

  Description:
    Blocks until the erase completes (~6 ms typical).
*/

void DRV_NVMCTRL_RowErase(uint32_t address);

/*******************************************************************************
  Function:
    void DRV_NVMCTRL_PageWrite ( const uint32_t *data, uint32_t address )

  Summary:
    Writes 64 bytes (one page) from data into flash at address.

  Description:
    address must be page-aligned and data must point to 16 uint32_t words.
    The row containing the page must have been erased before calling this
    function. Blocks until the program completes (~2.5 ms typical).
*/

void DRV_NVMCTRL_PageWrite(const uint32_t *data, uint32_t address);

/*******************************************************************************
  Function:
    bool DRV_NVMCTRL_IsBusy ( void )
    void DRV_NVMCTRL_WaitReady ( void )

  Summary:
    Status helpers. IsBusy is a single poll; WaitReady spins until READY.
*/

bool DRV_NVMCTRL_IsBusy(void);
void DRV_NVMCTRL_WaitReady(void);

#ifdef __cplusplus
}
#endif

#endif /* DRV_NVMCTRL_H */

/*******************************************************************************
 End of File
*/
