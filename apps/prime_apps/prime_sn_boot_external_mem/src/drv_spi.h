/*******************************************************************************
  SPI Driver Header File

  Company:
    Microchip Technology Inc.

  File Name:
    drv_spi.h

  Summary:
    Minimal bare-metal SPI master driver for the SAMD20 bootloader.

  Description:
    This driver is dedicated to talking to the on-board SST26VF032 serial
    flash over SERCOM1. Board wiring:
      - PA16  SERCOM1 PAD0  MISO
      - PA17  GPIO          CS  (active low, driven manually)
      - PA18  SERCOM1 PAD2  MOSI
      - PA19  SERCOM1 PAD3  SCK
      - PA20  GPIO          HOLD# (driven high, SST26 inactive hold)
      - PA22  GPIO          WP#   (driven high, SST26 write-protect off)

    Chip-select is kept as a regular GPIO so the driver can hold CS low
    across multi-byte command sequences (opcode + address + payload) that
    SST26 requires.
*******************************************************************************/

#ifndef DRV_SPI_H
#define DRV_SPI_H

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
// Section: Public Function Prototypes
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void DRV_SPI_Initialize ( void )

  Summary:
    Brings up the clock tree, PORT pins, and SERCOM1 as an SPI master.

  Description:
    After this call SERCOM1 is enabled in SPI master mode, 8-bit, MSB-first,
    mode 0, at 2 MHz. CS, HOLD# and WP# are all driven high so the SST26
    is ready to accept commands.
*/

void DRV_SPI_Initialize(void);

/*******************************************************************************
  Function:
    void DRV_SPI_Deinitialize ( void )

  Summary:
    Returns SERCOM1 to its reset state.

  Description:
    Issues a software reset on SERCOM1. Must be called before handing
    control over to the application: the application's Harmony plib
    re-initializes SERCOM1 by writing CTRLB without doing a SWRST first,
    and SAMD20 silently rejects CTRLB writes while ENABLE=1, which
    leaves the peripheral wedged on the next SYNCBUSY poll.

    PORT pins (PA16/PA17/PA18/PA19/PA20/PA22) and clock gates are left
    untouched — the application reconfigures them as part of its own
    initialization.
*/

void DRV_SPI_Deinitialize(void);

/*******************************************************************************
  Function:
    uint8_t DRV_SPI_TransferByte ( uint8_t txData )

  Summary:
    Blocking full-duplex byte transfer.

  Description:
    Sends txData and returns the byte shifted in on MISO during the same
    frame. Spins on DRE and RXC, so the function does not return until
    the hardware has completed the transfer.
*/

uint8_t DRV_SPI_TransferByte(uint8_t txData);

/*******************************************************************************
  Function:
    void DRV_SPI_Write ( const uint8_t *txData, uint32_t len )

  Summary:
    Writes len bytes to the bus, discarding incoming data.
*/

void DRV_SPI_Write(const uint8_t *txData, uint32_t len);

/*******************************************************************************
  Function:
    void DRV_SPI_Read ( uint8_t *rxData, uint32_t len )

  Summary:
    Reads len bytes from the bus by clocking 0xFF and capturing MISO.
*/

void DRV_SPI_Read(uint8_t *rxData, uint32_t len);

/*******************************************************************************
  Function:
    void DRV_SPI_CsAssert ( void )
    void DRV_SPI_CsDeassert ( void )

  Summary:
    Drive the SST26 CS line low (assert) or high (deassert).
*/

void DRV_SPI_CsAssert(void);
void DRV_SPI_CsDeassert(void);

#ifdef __cplusplus
}
#endif

#endif /* DRV_SPI_H */

/*******************************************************************************
 End of File
*/
