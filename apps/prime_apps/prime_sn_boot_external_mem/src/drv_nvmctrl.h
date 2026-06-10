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
    project's srv_storage does the same - the ER/WP commands are
    silently rejected on a locked region and INTFLAG.READY can even get
    stuck on some SAMD20 silicon.
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

#ifndef DRV_NVMCTRL_H
#define DRV_NVMCTRL_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include <stdbool.h>

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

extern "C" {

#endif
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Constants
// *****************************************************************************
// *****************************************************************************

#define DRV_NVMCTRL_PAGE_SIZE       64U
#define DRV_NVMCTRL_ROW_SIZE        256U
#define DRV_NVMCTRL_PAGES_PER_ROW   4U

/* DRV_NVMCTRL_GetError() return values. */
typedef enum
{
    DRV_NVMCTRL_ERROR_NONE   = 0,
    DRV_NVMCTRL_ERROR_PROG   = 1,    /* PROGE: programming error    */
    DRV_NVMCTRL_ERROR_LOCK   = 2,    /* LOCKE: row in locked region */
    DRV_NVMCTRL_ERROR_NVM    = 3,    /* NVME:  generic NVM error    */
} DRV_NVMCTRL_ERROR;

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

/*******************************************************************************
  Function:
    void DRV_NVMCTRL_CacheInvalidate ( void )

  Summary:
    Invalidates every NVMCTRL cache line.

  Description:
    The SAMD20 NVMCTRL has an internal flash cache that is enabled by
    default. Memory-mapped reads after a write or row erase can return
    stale data unless the cache is invalidated. Issues the CMD INVALL
    (0x46) and waits for completion.
*/

void DRV_NVMCTRL_CacheInvalidate(void);

/*******************************************************************************
  Function:
    DRV_NVMCTRL_ERROR DRV_NVMCTRL_GetError ( void )

  Summary:
    Returns the first error reported in NVMCTRL.STATUS and clears the
    status bits (W1C) so the next call only sees fresh errors.

  Description:
    Should be called after every RowErase / PageWrite / RegionUnlock that
    targets a row whose lock state could have prevented the operation.
    Without this check the bootloader silently proceeds even when the
    underlying flash cycle was rejected, which produces the "write over
    non-erased row -> result = prev AND new" bit-AND corruption pattern
    we observed before this fix landed.
*/

DRV_NVMCTRL_ERROR DRV_NVMCTRL_GetError(void);

//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif /* DRV_NVMCTRL_H */

/*******************************************************************************
 End of File
*/
