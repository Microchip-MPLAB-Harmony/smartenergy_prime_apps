/*******************************************************************************
  SST26 Serial Flash Driver Header File

  Company:
    Microchip Technology Inc.

  File Name:
    drv_sst26.h

  Summary:
    Minimal bare-metal driver for the SST26VF032B serial flash used by the
    SAMD20 bootloader.

  Description:
    This driver exposes only the operations the bootloader needs to
    implement dual-zone firmware upgrade:

      - Read  from either zone (TELECARGA or REVERT)
      - Page program (256 bytes)   into the REVERT zone during backup
      - Block erase (64 KB)        to prepare the REVERT zone
      - Status / JEDEC ID queries  for readiness and sanity checks

    All writes and erases are synchronous: the driver polls the status
    register until the operation completes before returning. That keeps
    the bootloader logic linear, which is appropriate for a one-shot
    code path that the WDT will not reach.

    Every write/erase internally issues the mandatory WREN (06h) first,
    and Initialize() issues a ULBPR (98h) so the global Block Protection
    Register (which comes up fully locked after power-on) is cleared.

    DRV_SPI_Initialize() must have been called before any function in
    this module.
*******************************************************************************/

#ifndef DRV_SST26_H
#define DRV_SST26_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// *****************************************************************************
// *****************************************************************************
// Section: Constants
// *****************************************************************************
// *****************************************************************************

#define DRV_SST26_PAGE_SIZE         256U
#define DRV_SST26_SECTOR_SIZE_4K    (4UL * 1024UL)
#define DRV_SST26_BLOCK_SIZE_64K    (64UL * 1024UL)

/* JEDEC ID returned by the SST26VF064B mounted on the FLASH2 Click
 * carrier (Manufacturer 0xBF = Microchip/SST, Type 0x26 = SST26 serial
 * flash family, Capacity 0x43 = 64 Mbit / 8 MB). */
#define DRV_SST26_JEDEC_ID          0x00BF2643UL

// *****************************************************************************
// *****************************************************************************
// Section: Public Function Prototypes
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void DRV_SST26_Initialize ( void )

  Summary:
    Clears the global Block Protection Register so subsequent program and
    erase commands are accepted.

  Description:
    After power-on the SST26 has every block write-protected. This
    function waits for any pending operation to finish, issues WREN and
    then ULBPR (98h), and finally waits for the device to become ready
    again.
*/

void DRV_SST26_Initialize(void);

/*******************************************************************************
  Function:
    uint32_t DRV_SST26_ReadJedecId ( void )

  Summary:
    Returns the 24-bit JEDEC ID of the connected device.

  Description:
    Useful as a sanity check to make sure SPI wiring and SERCOM1 clocks
    are configured correctly. The expected value for the
    SST26VF032B is DRV_SST26_JEDEC_ID.
*/

uint32_t DRV_SST26_ReadJedecId(void);

/*******************************************************************************
  Function:
    void DRV_SST26_Read ( uint32_t address,
                          uint8_t *buf,
                          uint32_t len )

  Summary:
    Reads len bytes starting at address from the SST26 into buf.

  Description:
    Uses the legacy READ (03h) command, which the SST26 supports up to
    about 33 MHz. At the bootloader's 2 MHz SPI clock this leaves plenty
    of margin and avoids the dummy byte needed by the fast-read variants.
*/

void DRV_SST26_Read(uint32_t address, uint8_t *buf, uint32_t len);

/*******************************************************************************
  Function:
    void DRV_SST26_WritePage ( uint32_t address,
                               const uint8_t *buf,
                               uint32_t len )

  Summary:
    Programs up to DRV_SST26_PAGE_SIZE bytes into the SST26.

  Description:
    Issues WREN, then PAGE PROGRAM (02h), then waits for BUSY to clear.
    The write must not cross a 256-byte page boundary; the caller is
    responsible for honouring that. len is clamped to
    DRV_SST26_PAGE_SIZE.
*/

void DRV_SST26_WritePage(uint32_t address, const uint8_t *buf, uint32_t len);

/*******************************************************************************
  Function:
    void DRV_SST26_BlockErase64K ( uint32_t address )

  Summary:
    Erases the 64 KB block that contains address.

  Description:
    Issues WREN, then BLOCK ERASE 64 KB (D8h), then waits for BUSY to
    clear. The address does not need to be aligned: the SST26 ignores
    the least-significant address bits and erases the whole 64 KB block.
*/

void DRV_SST26_BlockErase64K(uint32_t address);

/*******************************************************************************
  Function:
    void DRV_SST26_SectorErase4K ( uint32_t address )

  Summary:
    Erases the 4 KB sector that contains address.

  Description:
    Issues WREN, then SECTOR ERASE 4 KB (20h), then waits for BUSY to
    clear. Used for small high-frequency-write zones (e.g. BOOT_FLAG).
    Like BlockErase64K, the address does not need to be aligned: the
    SST26 ignores the least-significant 12 bits and erases the whole
    4 KB sector.
*/

void DRV_SST26_SectorErase4K(uint32_t address);

/*******************************************************************************
  Function:
    void DRV_SST26_WaitReady ( void )

  Summary:
    Polls RDSR (05h) until the BUSY bit is clear.

  Description:
    Exposed publicly so higher-level code can bracket multi-page
    operations without re-polling inside every helper.
*/

void DRV_SST26_WaitReady(void);

#ifdef __cplusplus
}
#endif

#endif /* DRV_SST26_H */

/*******************************************************************************
 End of File
*/
