/*******************************************************************************
  SPI Driver Implementation File

  Company:
    Microchip Technology Inc.

  File Name:
    drv_spi.c

  Summary:
    Bare-metal SPI master driver for the SAMD20 bootloader, on SERCOM1.

  Description:
    Direct register-level driver: no Harmony. This driver owns four things:

      1. The OSC8M configuration. At reset OSC8M ships with PRESC=3 (1 MHz);
         we lift it to the full 8 MHz so SPI and CPU run fast enough to
         keep image copies under a couple of seconds.

      2. The PM clock gate for SERCOM1 and the GCLK channel that feeds
         SERCOM1_CORE from GCLK_GEN0. On SAMD20 the SERCOM1_CORE GCLK ID
         is 14 (not 0x15 as on SAMD21) - verified from the device
         datasheet and the modem project's plib_clock.c.

      3. The PORT configuration for PA16/PA18/PA19 (SERCOM1 peripheral
         function C) and PA17 (GPIO CS). PA20 (HOLD#) and PA22 (WP#) are
         driven high so the SST26 is not held indefinitely even on boards
         without external pull-ups.

      4. The SERCOM1 SPI master configuration: mode 0, MSB first, 8-bit,
         BAUD = 1 -> fsck = 2 MHz. This is well below the SST26 maximum
         of 104 MHz and gives ~1.1 s for a 256 KB block copy.
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

#include "drv_spi.h"
#include "device.h"

// *****************************************************************************
// *****************************************************************************
// Section: Local Constants
// *****************************************************************************
// *****************************************************************************

/* PORT layout on the FLASH2_Click header. */
#define DRV_SPI_PORT_GROUP_A    0U
#define DRV_SPI_PIN_MISO        16U
#define DRV_SPI_PIN_CS          17U
#define DRV_SPI_PIN_MOSI        18U
#define DRV_SPI_PIN_SCK         19U
#define DRV_SPI_PIN_HOLD        20U
#define DRV_SPI_PIN_WP          22U

/* Peripheral function C on SAMD20 PA16..PA19 routes the pin to SERCOM1. */
#define DRV_SPI_PMUX_FUNC_C     2U

/* GCLK peripheral channel ID for SERCOM1_CORE on SAMD20. */
#define DRV_SPI_GCLK_ID_SERCOM1 14U

/* Baud register value. fref = 8 MHz, fsck = fref / (2 * (BAUD + 1)).
 * BAUD = 1 gives fsck = 2 MHz. */
#define DRV_SPI_BAUD_VALUE      1U

// *****************************************************************************
// *****************************************************************************
// Section: Local Functions - Forward Declarations
// *****************************************************************************
// *****************************************************************************

static void lDRV_SPI_ClockInitialize(void);
static void lDRV_SPI_PortInitialize(void);
static void lDRV_SPI_Sercom1Initialize(void);

// *****************************************************************************
// *****************************************************************************
// Section: Public Functions
// *****************************************************************************
// *****************************************************************************

void DRV_SPI_Initialize(void)
{
    lDRV_SPI_ClockInitialize();
    lDRV_SPI_PortInitialize();
    lDRV_SPI_Sercom1Initialize();
}

void DRV_SPI_Deinitialize(void)
{
    SERCOM1_REGS->SPIM.SERCOM_CTRLA = SERCOM_SPIM_CTRLA_SWRST_Msk;
    while ((SERCOM1_REGS->SPIM.SERCOM_CTRLA & SERCOM_SPIM_CTRLA_SWRST_Msk) != 0U)
    {
        /* spin */
    }
    while ((SERCOM1_REGS->SPIM.SERCOM_STATUS & (uint16_t) SERCOM_SPIM_STATUS_SYNCBUSY_Msk)
            == (uint16_t) SERCOM_SPIM_STATUS_SYNCBUSY_Msk)
    {
        /* spin */
    }
}

uint8_t DRV_SPI_TransferByte(uint8_t txData)
{
    uint8_t rxData;

    /* Wait for the shift register to be ready for a new byte. */
    while ((SERCOM1_REGS->SPIM.SERCOM_INTFLAG & SERCOM_SPIM_INTFLAG_DRE_Msk) == 0U)
    {
        /* spin */
    }

    SERCOM1_REGS->SPIM.SERCOM_DATA = (uint16_t) txData;

    /* Wait for the full-duplex byte to arrive before returning so the
     * caller always sees valid MISO data. */
    while ((SERCOM1_REGS->SPIM.SERCOM_INTFLAG & SERCOM_SPIM_INTFLAG_RXC_Msk) == 0U)
    {
        /* spin */
    }

    rxData = (uint8_t) SERCOM1_REGS->SPIM.SERCOM_DATA;

    return rxData;
}

void DRV_SPI_Write(const uint8_t *txData, uint32_t len)
{
    uint32_t i;

    for (i = 0U; i < len; i++)
    {
        (void) DRV_SPI_TransferByte(txData[i]);
    }
}

void DRV_SPI_Read(uint8_t *rxData, uint32_t len)
{
    uint32_t i;

    for (i = 0U; i < len; i++)
    {
        rxData[i] = DRV_SPI_TransferByte(0xFFU);
    }
}

void DRV_SPI_CsAssert(void)
{
    PORT_REGS->GROUP[DRV_SPI_PORT_GROUP_A].PORT_OUTCLR =
        (1UL << DRV_SPI_PIN_CS);
}

void DRV_SPI_CsDeassert(void)
{
    PORT_REGS->GROUP[DRV_SPI_PORT_GROUP_A].PORT_OUTSET =
        (1UL << DRV_SPI_PIN_CS);
}

// *****************************************************************************
// *****************************************************************************
// Section: Local Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    static void lDRV_SPI_ClockInitialize ( void )

  Summary:
    Configures the minimum clock tree needed to drive SERCOM1 at 8 MHz.

  Description:
    OSC8M PRESC is cleared so the RC oscillator produces 8 MHz. The APBC
    gate for SERCOM1 is opened, and the SERCOM1_CORE GCLK channel is
    routed to GCLK_GEN0 (which still feeds from OSC8M by default after
    reset). No DFLL, no XOSC32K; the bootloader stays as simple as
    possible.
*/

static void lDRV_SPI_ClockInitialize(void)
{
    uint32_t osc8m;

    /* Preserve the factory CALIB and FRANGE bits, clear PRESC to 0 so
     * OSC8M outputs 8 MHz, and make sure the oscillator is enabled. */
    osc8m = SYSCTRL_REGS->SYSCTRL_OSC8M
          & (SYSCTRL_OSC8M_CALIB_Msk | SYSCTRL_OSC8M_FRANGE_Msk);
    SYSCTRL_REGS->SYSCTRL_OSC8M = osc8m
                                | SYSCTRL_OSC8M_ENABLE_Msk
                                | SYSCTRL_OSC8M_PRESC(0U);

    while ((SYSCTRL_REGS->SYSCTRL_PCLKSR & SYSCTRL_PCLKSR_OSC8MRDY_Msk)
            != SYSCTRL_PCLKSR_OSC8MRDY_Msk)
    {
        /* spin */
    }

    /* Enable the APBC clock for SERCOM1. */
    PM_REGS->PM_APBCMASK |= PM_APBCMASK_SERCOM1_Msk;

    /* Route GCLK_GEN0 to the SERCOM1_CORE channel. The write is 16-bit. */
    GCLK_REGS->GCLK_CLKCTRL = (uint16_t) (
          GCLK_CLKCTRL_ID((uint16_t) DRV_SPI_GCLK_ID_SERCOM1)
        | GCLK_CLKCTRL_GEN(0U)
        | GCLK_CLKCTRL_CLKEN_Msk);

    while ((GCLK_REGS->GCLK_STATUS & GCLK_STATUS_SYNCBUSY_Msk)
            == GCLK_STATUS_SYNCBUSY_Msk)
    {
        /* spin */
    }
}

/*******************************************************************************
  Function:
    static void lDRV_SPI_PortInitialize ( void )

  Summary:
    Configures the four SPI pins plus HOLD# and WP# GPIOs.

  Description:
    PA16 (MISO), PA18 (MOSI) and PA19 (SCK) are switched to peripheral
    function C (SERCOM1). PA17 stays as a GPIO output for the chip-select.
    PA20 (HOLD#) and PA22 (WP#) are driven high to keep the SST26 active
    and allow writes. Output values are programmed before the output
    enable so none of the lines glitches low during the transition.
*/

static void lDRV_SPI_PortInitialize(void)
{
    uint32_t gpioMask;

    gpioMask = (1UL << DRV_SPI_PIN_CS)
             | (1UL << DRV_SPI_PIN_HOLD)
             | (1UL << DRV_SPI_PIN_WP);

    /* Drive all GPIO-controlled lines high before enabling their output
     * drivers. This keeps CS and HOLD#/WP# deasserted through the
     * direction change. */
    PORT_REGS->GROUP[DRV_SPI_PORT_GROUP_A].PORT_OUTSET = gpioMask;
    PORT_REGS->GROUP[DRV_SPI_PORT_GROUP_A].PORT_DIRSET = gpioMask;

    /* PINCFG: enable peripheral multiplexing on the three SPI pins.
     * For MISO the input buffer is enabled automatically once PMUXEN=1
     * and the peripheral takes ownership of the pin. */
    PORT_REGS->GROUP[DRV_SPI_PORT_GROUP_A].PORT_PINCFG[DRV_SPI_PIN_MISO] =
        PORT_PINCFG_PMUXEN_Msk;
    PORT_REGS->GROUP[DRV_SPI_PORT_GROUP_A].PORT_PINCFG[DRV_SPI_PIN_MOSI] =
        PORT_PINCFG_PMUXEN_Msk;
    PORT_REGS->GROUP[DRV_SPI_PORT_GROUP_A].PORT_PINCFG[DRV_SPI_PIN_SCK] =
        PORT_PINCFG_PMUXEN_Msk;

    /* PMUX registers group pins in even/odd pairs.
     *   PMUX[8]: PA16 (even) / PA17 (odd)  -> MISO on function C, CS is GPIO
     *   PMUX[9]: PA18 (even) / PA19 (odd)  -> MOSI and SCK on function C   */
    PORT_REGS->GROUP[DRV_SPI_PORT_GROUP_A].PORT_PMUX[DRV_SPI_PIN_MISO / 2U] =
        (uint8_t) PORT_PMUX_PMUXE(DRV_SPI_PMUX_FUNC_C);
    PORT_REGS->GROUP[DRV_SPI_PORT_GROUP_A].PORT_PMUX[DRV_SPI_PIN_MOSI / 2U] =
        (uint8_t) (PORT_PMUX_PMUXE(DRV_SPI_PMUX_FUNC_C)
                 | PORT_PMUX_PMUXO(DRV_SPI_PMUX_FUNC_C));
}

/*******************************************************************************
  Function:
    static void lDRV_SPI_Sercom1Initialize ( void )

  Summary:
    Configures SERCOM1 as an SPI master: mode 0, MSB-first, 8-bit, 2 MHz.

  Description:
    A software reset runs first so the peripheral comes up in a known
    state irrespective of what the caller left behind. DOPO = 1 matches
    the board wiring (MOSI on PAD2, SCK on PAD3) and DIPO = 0 puts MISO
    on PAD0.
*/

static void lDRV_SPI_Sercom1Initialize(void)
{
    /* Software reset. SWRST self-clears when the reset finishes. */
    SERCOM1_REGS->SPIM.SERCOM_CTRLA = SERCOM_SPIM_CTRLA_SWRST_Msk;
    while ((SERCOM1_REGS->SPIM.SERCOM_CTRLA & SERCOM_SPIM_CTRLA_SWRST_Msk) != 0U)
    {
        /* spin */
    }
    while ((SERCOM1_REGS->SPIM.SERCOM_STATUS & (uint16_t) SERCOM_SPIM_STATUS_SYNCBUSY_Msk)
            == (uint16_t) SERCOM_SPIM_STATUS_SYNCBUSY_Msk)
    {
        /* spin */
    }

    /* CTRLB: 8-bit characters and receiver enabled. Must be written
     * while the module is disabled. */
    SERCOM1_REGS->SPIM.SERCOM_CTRLB = SERCOM_SPIM_CTRLB_CHSIZE_8_BIT
                                    | SERCOM_SPIM_CTRLB_RXEN_Msk;
    while ((SERCOM1_REGS->SPIM.SERCOM_STATUS & (uint16_t) SERCOM_SPIM_STATUS_SYNCBUSY_Msk)
            == (uint16_t) SERCOM_SPIM_STATUS_SYNCBUSY_Msk)
    {
        /* spin */
    }

    SERCOM1_REGS->SPIM.SERCOM_BAUD = (uint8_t) DRV_SPI_BAUD_VALUE;

    /* CTRLA: master mode, DOPO=1, DIPO=0, SPI mode 0, MSB first, enable. */
    SERCOM1_REGS->SPIM.SERCOM_CTRLA = SERCOM_SPIM_CTRLA_MODE_SPI_MASTER
                                   | SERCOM_SPIM_CTRLA_DOPO_PAD1
                                   | SERCOM_SPIM_CTRLA_DIPO_PAD0
                                   | SERCOM_SPIM_CTRLA_CPOL_IDLE_LOW
                                   | SERCOM_SPIM_CTRLA_CPHA_LEADING_EDGE
                                   | SERCOM_SPIM_CTRLA_DORD_MSB
                                   | SERCOM_SPIM_CTRLA_ENABLE_Msk;
    while ((SERCOM1_REGS->SPIM.SERCOM_STATUS & (uint16_t) SERCOM_SPIM_STATUS_SYNCBUSY_Msk)
            == (uint16_t) SERCOM_SPIM_STATUS_SYNCBUSY_Msk)
    {
        /* spin */
    }
}

/*******************************************************************************
 End of File
*/
