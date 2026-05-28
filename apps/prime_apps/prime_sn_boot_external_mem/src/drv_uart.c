/*******************************************************************************
  UART Driver Implementation File

  Company:
    Microchip Technology Inc.

  File Name:
    drv_uart.c

  Summary:
    Polled SERCOM3 USART driver for the bare-metal SAMD20J18 bootloader.

  Description:
    Direct register-level driver: no Harmony, no ring buffer, no IRQ. The
    bootloader runs from OSC8M (8 MHz) so the BAUD value is computed for
    fREF = 8 MHz at 16x async oversample.
*******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stddef.h>

#include "drv_uart.h"
#include "device.h"

// *****************************************************************************
// *****************************************************************************
// Section: Local Constants
// *****************************************************************************
// *****************************************************************************

/* SERCOM3 PADs on SAMD20J18, function C: PA24 = PAD2, PA25 = PAD3.
 * Matches what the modem application uses for its own USI UART. */
#define DRV_UART_PORT_GROUP                 0U          /* Port A */
#define DRV_UART_TX_PIN                     24U
#define DRV_UART_RX_PIN                     25U
#define DRV_UART_PIN_FN_C                   2U          /* PMUX function C */

/* GCLK CLKCTRL ID for SERCOM3_CORE on SAMD20. */
#define DRV_UART_GCLK_ID_SERCOM3_CORE       16U

/* BAUD value for 115200 baud at fREF = 8 MHz, 16x async oversample:
 *   BAUD = 65536 * (1 - 16 * fBAUD / fREF)
 *        = 65536 * (1 - 16 * 115200 / 8000000)
 *        ≈ 50439                                                       */
#define DRV_UART_BAUD_VALUE_8MHZ            (50439U)

// *****************************************************************************
// *****************************************************************************
// Section: Public Functions
// *****************************************************************************
// *****************************************************************************

void DRV_UART_Initialize(void)
{
    /* 1. Make sure SERCOM3 has its APBC clock gated on. The reset
     *    default already enables it, but writing the bit explicitly
     *    keeps the bootloader independent of the previous boot's state
     *    (the application could have cleared it before reset). */
    PM_REGS->PM_APBCMASK |= PM_APBCMASK_SERCOM3_Msk;

    /* 2. Route GCLK0 (= OSC8M 8 MHz on a fresh reset) to SERCOM3 core.
     *    Disable first, then write the new selection with CLKEN. */
    GCLK_REGS->GCLK_CLKCTRL = GCLK_CLKCTRL_ID(DRV_UART_GCLK_ID_SERCOM3_CORE);

    while ((GCLK_REGS->GCLK_CLKCTRL & GCLK_CLKCTRL_CLKEN_Msk) != 0U)
    {
        /* wait for previous selection to clear */
    }

    GCLK_REGS->GCLK_CLKCTRL =
          GCLK_CLKCTRL_ID(DRV_UART_GCLK_ID_SERCOM3_CORE)
        | GCLK_CLKCTRL_GEN(0U)
        | GCLK_CLKCTRL_CLKEN_Msk;

    /* 3. Pinmux PA24 / PA25 to SERCOM3 PAD2 / PAD3 (function C).
     *    PORT_PMUX is shared by two consecutive pins; pins 24 and 25
     *    sit at PMUX[12]. PINCFG.PMUXEN must be set on each pin. */
    PORT_REGS->GROUP[DRV_UART_PORT_GROUP].PORT_PMUX[DRV_UART_TX_PIN / 2U] =
          (uint8_t) ((DRV_UART_PIN_FN_C << 0U) | (DRV_UART_PIN_FN_C << 4U));

    PORT_REGS->GROUP[DRV_UART_PORT_GROUP].PORT_PINCFG[DRV_UART_TX_PIN] =
          PORT_PINCFG_PMUXEN_Msk;
    PORT_REGS->GROUP[DRV_UART_PORT_GROUP].PORT_PINCFG[DRV_UART_RX_PIN] =
          PORT_PINCFG_PMUXEN_Msk;

    /* 4. Reset SERCOM3 so prior CTRLA writes (e.g. from the modem app)
     *    cannot block our reconfiguration. Wait for both reset SYNCBUSY
     *    and SWRST clear. */
    SERCOM3_REGS->USART_INT.SERCOM_CTRLA = SERCOM_USART_INT_CTRLA_SWRST_Msk;
    while ((SERCOM3_REGS->USART_INT.SERCOM_CTRLA
            & SERCOM_USART_INT_CTRLA_SWRST_Msk) != 0U)
    {
        /* wait for reset to complete */
    }
    while ((SERCOM3_REGS->USART_INT.SERCOM_STATUS
            & (uint16_t) SERCOM_USART_INT_STATUS_SYNCBUSY_Msk) != 0U)
    {
        /* wait for STATUS sync */
    }

    /* 5. Configure CTRLA: USART internal clock, async, RXPO=3 (PAD3),
     *    TXPO=1 (PAD2 with PAD3 free), data order LSB first, 16x
     *    sampling, no parity. */
    SERCOM3_REGS->USART_INT.SERCOM_CTRLA =
          SERCOM_USART_INT_CTRLA_MODE_USART_INT_CLK
        | SERCOM_USART_INT_CTRLA_RXPO(3UL)
        | SERCOM_USART_INT_CTRLA_TXPO(1UL)
        | SERCOM_USART_INT_CTRLA_DORD_Msk
        | SERCOM_USART_INT_CTRLA_FORM(0UL);

    /* 6. BAUD for 115200 at 8 MHz reference. */
    SERCOM3_REGS->USART_INT.SERCOM_BAUD =
          (uint16_t) SERCOM_USART_INT_BAUD_BAUD(DRV_UART_BAUD_VALUE_8MHZ);

    /* 7. Configure CTRLB: 8 bits, 1 stop bit, RX + TX enabled. CTRLB is
     *    sync-required even before enabling the SERCOM. */
    SERCOM3_REGS->USART_INT.SERCOM_CTRLB =
          SERCOM_USART_INT_CTRLB_CHSIZE_8_BIT
        | SERCOM_USART_INT_CTRLB_SBMODE_1_BIT
        | SERCOM_USART_INT_CTRLB_RXEN_Msk
        | SERCOM_USART_INT_CTRLB_TXEN_Msk;

    while ((SERCOM3_REGS->USART_INT.SERCOM_STATUS
            & (uint16_t) SERCOM_USART_INT_STATUS_SYNCBUSY_Msk) != 0U)
    {
        /* CTRLB sync */
    }

    /* 8. Enable. */
    SERCOM3_REGS->USART_INT.SERCOM_CTRLA |=
          SERCOM_USART_INT_CTRLA_ENABLE_Msk;

    while ((SERCOM3_REGS->USART_INT.SERCOM_STATUS
            & (uint16_t) SERCOM_USART_INT_STATUS_SYNCBUSY_Msk) != 0U)
    {
        /* ENABLE sync */
    }
}

void DRV_UART_SendByte(uint8_t value)
{
    /* DRE goes high once the SERCOM accepts a new byte into the shift
     * register. At 115200 each byte is ~87 us, well under any timeout
     * concern. */
    while ((SERCOM3_REGS->USART_INT.SERCOM_INTFLAG
            & (uint8_t) SERCOM_USART_INT_INTFLAG_DRE_Msk) == 0U)
    {
        /* spin */
    }

    SERCOM3_REGS->USART_INT.SERCOM_DATA = (uint16_t) value;
}

bool DRV_UART_RecvByteIfReady(uint8_t *out)
{
    if (out == NULL)
    {
        return false;
    }

    if ((SERCOM3_REGS->USART_INT.SERCOM_INTFLAG
         & (uint8_t) SERCOM_USART_INT_INTFLAG_RXC_Msk) == 0U)
    {
        return false;
    }

    *out = (uint8_t) SERCOM3_REGS->USART_INT.SERCOM_DATA;
    return true;
}

/*******************************************************************************
 End of File
*/
