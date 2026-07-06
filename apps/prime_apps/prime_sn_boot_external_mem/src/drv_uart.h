/*******************************************************************************
  UART Driver Header File

  Company:
    Microchip Technology Inc.

  File Name:
    drv_uart.h

  Summary:
    Polled SERCOM3 UART driver for the bare-metal SAMD20J18 bootloader.

  Description:
    Direct register-level driver: no Harmony. Configures SERCOM3 in async
    USART mode at 115200 8N1 using the
    OSC8M 8 MHz clock that the SAMD20 boots up on. The bootloader's
    UART recovery loop drives this driver directly: there is no ring
    buffer, no interrupt, no DMA. SendByte spins until the data
    register is empty; RecvByteIfReady returns false immediately when
    no byte is pending.

    Pins:
      - PA24 -> SERCOM3 PAD2 = TX (function C)
      - PA25 -> SERCOM3 PAD3 = RX (function C)

    Layout matches what the modem application uses for its own UART so
    the same physical wiring works for both bootloader recovery and
    normal operation.
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

#ifndef DRV_UART_H
#define DRV_UART_H

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
// Section: Public Function Prototypes
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void DRV_UART_Initialize ( void )

  Summary:
    Configures SERCOM3 USART at 115200 8N1 using GCLK0 (OSC8M 8 MHz).

  Description:
    Routes GCLK0 to the SERCOM3 core, sets up PA24/PA25 as SERCOM3
    PAD2/PAD3 (TX/RX) and writes CTRLA + CTRLB + BAUD with the values
    needed for 115200 baud at fREF = 8 MHz. Blocks on every SYNCBUSY
    transition.
*/

void DRV_UART_Initialize(void);

/*******************************************************************************
  Function:
    void DRV_UART_SendByte ( uint8_t value )

  Summary:
    Transmits one byte. Spins on DRE until the SERCOM accepts it.
*/

void DRV_UART_SendByte(uint8_t value);

/*******************************************************************************
  Function:
    bool DRV_UART_RecvByteIfReady ( uint8_t *out )

  Summary:
    Returns true when a received byte is available (writes it to *out)
    or false otherwise. Non-blocking.
*/

bool DRV_UART_RecvByteIfReady(uint8_t *out);

//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif /* DRV_UART_H */

/*******************************************************************************
 End of File
*/
