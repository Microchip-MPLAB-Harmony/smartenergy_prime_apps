/*******************************************************************************
  SST26 Serial Flash Driver Implementation File

  Company:
    Microchip Technology Inc.

  File Name:
    drv_sst26.c

  Summary:
    Bare-metal driver for the SST26VF032B serial flash.

  Description:
    All operations are synchronous (polling-based). The driver owns the
    CS line via DRV_SPI_CsAssert / DRV_SPI_CsDeassert; it never returns
    with CS asserted. WREN is issued internally before every program or
    erase so higher-level code does not have to remember to do it.

    The only state this driver has is the one implicit in the SST26
    itself: the WEL bit between WREN and the next command, and the
    BUSY bit while an erase or program is in flight. Both are managed
    by the helper sequences below.
*******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "drv_sst26.h"
#include "drv_spi.h"

// *****************************************************************************
// *****************************************************************************
// Section: Local Constants
// *****************************************************************************
// *****************************************************************************

/* SST26VF032B command opcodes. */
#define DRV_SST26_CMD_WREN      0x06U
#define DRV_SST26_CMD_RDSR      0x05U
#define DRV_SST26_CMD_READ      0x03U
#define DRV_SST26_CMD_PP        0x02U
#define DRV_SST26_CMD_BE_64K    0xD8U
#define DRV_SST26_CMD_ULBPR     0x98U
#define DRV_SST26_CMD_JEDEC     0x9FU

/* Status register bits. */
#define DRV_SST26_SR_BUSY_Msk   0x01U

// *****************************************************************************
// *****************************************************************************
// Section: Local Functions - Forward Declarations
// *****************************************************************************
// *****************************************************************************

static void lDRV_SST26_Command1(uint8_t cmd);
static void lDRV_SST26_WriteEnable(void);
static void lDRV_SST26_SendAddress(uint32_t address);

// *****************************************************************************
// *****************************************************************************
// Section: Public Functions
// *****************************************************************************
// *****************************************************************************

void DRV_SST26_Initialize(void)
{
    /* Make sure any pending reset-recovery state has finished. */
    DRV_SST26_WaitReady();

    /* Clear the global Block Protection Register so subsequent program
     * and erase commands are accepted. ULBPR itself requires WREN. */
    lDRV_SST26_WriteEnable();
    lDRV_SST26_Command1(DRV_SST26_CMD_ULBPR);

    /* Wait for the unlock to take effect before leaving the function. */
    DRV_SST26_WaitReady();
}

uint32_t DRV_SST26_ReadJedecId(void)
{
    uint32_t id;
    uint8_t  b0;
    uint8_t  b1;
    uint8_t  b2;

    DRV_SPI_CsAssert();
    (void) DRV_SPI_TransferByte(DRV_SST26_CMD_JEDEC);
    b0 = DRV_SPI_TransferByte(0xFFU);
    b1 = DRV_SPI_TransferByte(0xFFU);
    b2 = DRV_SPI_TransferByte(0xFFU);
    DRV_SPI_CsDeassert();

    id = ((uint32_t) b0 << 16) | ((uint32_t) b1 << 8) | (uint32_t) b2;

    return id;
}

void DRV_SST26_Read(uint32_t address, uint8_t *buf, uint32_t len)
{
    DRV_SPI_CsAssert();

    (void) DRV_SPI_TransferByte(DRV_SST26_CMD_READ);
    lDRV_SST26_SendAddress(address);

    DRV_SPI_Read(buf, len);

    DRV_SPI_CsDeassert();
}

void DRV_SST26_WritePage(uint32_t address, const uint8_t *buf, uint32_t len)
{
    uint32_t writeLen;

    writeLen = len;
    if (writeLen > DRV_SST26_PAGE_SIZE)
    {
        writeLen = DRV_SST26_PAGE_SIZE;
    }

    lDRV_SST26_WriteEnable();

    DRV_SPI_CsAssert();
    (void) DRV_SPI_TransferByte(DRV_SST26_CMD_PP);
    lDRV_SST26_SendAddress(address);
    DRV_SPI_Write(buf, writeLen);
    DRV_SPI_CsDeassert();

    /* Typical tPP is 1.5 ms; RDSR polling is fast enough that the cost
     * of the wait is dominated by the program time itself. */
    DRV_SST26_WaitReady();
}

void DRV_SST26_BlockErase64K(uint32_t address)
{
    lDRV_SST26_WriteEnable();

    DRV_SPI_CsAssert();
    (void) DRV_SPI_TransferByte(DRV_SST26_CMD_BE_64K);
    lDRV_SST26_SendAddress(address);
    DRV_SPI_CsDeassert();

    /* tBE is typically 25 ms, worst-case 50 ms. */
    DRV_SST26_WaitReady();
}

void DRV_SST26_WaitReady(void)
{
    uint8_t status;

    DRV_SPI_CsAssert();
    (void) DRV_SPI_TransferByte(DRV_SST26_CMD_RDSR);

    do
    {
        status = DRV_SPI_TransferByte(0xFFU);
    }
    while ((status & DRV_SST26_SR_BUSY_Msk) != 0U);

    DRV_SPI_CsDeassert();
}

// *****************************************************************************
// *****************************************************************************
// Section: Local Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    static void lDRV_SST26_Command1 ( uint8_t cmd )

  Summary:
    Shifts a single-byte command (no address, no data) out to the SST26.

  Description:
    Used for WREN and ULBPR where the command is the complete SPI
    transaction.
*/

static void lDRV_SST26_Command1(uint8_t cmd)
{
    DRV_SPI_CsAssert();
    (void) DRV_SPI_TransferByte(cmd);
    DRV_SPI_CsDeassert();
}

/*******************************************************************************
  Function:
    static void lDRV_SST26_WriteEnable ( void )

  Summary:
    Sets the Write Enable Latch so the next program / erase / unlock
    command is accepted by the SST26.
*/

static void lDRV_SST26_WriteEnable(void)
{
    lDRV_SST26_Command1(DRV_SST26_CMD_WREN);
}

/*******************************************************************************
  Function:
    static void lDRV_SST26_SendAddress ( uint32_t address )

  Summary:
    Shifts out a 24-bit address MSB first.

  Description:
    Used by READ (03h), PAGE PROGRAM (02h) and BLOCK ERASE 64K (D8h).
    Must be called between CsAssert and CsDeassert, right after the
    opcode byte.
*/

static void lDRV_SST26_SendAddress(uint32_t address)
{
    (void) DRV_SPI_TransferByte((uint8_t) ((address >> 16) & 0xFFU));
    (void) DRV_SPI_TransferByte((uint8_t) ((address >>  8) & 0xFFU));
    (void) DRV_SPI_TransferByte((uint8_t) ( address        & 0xFFU));
}

/*******************************************************************************
 End of File
*/
