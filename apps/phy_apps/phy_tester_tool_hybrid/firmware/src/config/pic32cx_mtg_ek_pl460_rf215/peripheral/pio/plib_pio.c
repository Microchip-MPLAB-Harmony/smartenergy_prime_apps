/*******************************************************************************
  SYS PORTS Static Functions for PORTS System Service

  Company:
    Microchip Technology Inc.

  File Name:
    plib_pio.c

  Summary:
    PIO function implementations for the PIO PLIB.

  Description:
    The PIO PLIB provides a simple interface to manage peripheral
    input-output controller.

*******************************************************************************/

//DOM-IGNORE-BEGIN
/*******************************************************************************
* Copyright (C) 2018 Microchip Technology Inc. and its subsidiaries.
*
* Subject to your compliance with these terms, you may use Microchip software
* and any derivatives exclusively with Microchip products. It is your
* responsibility to comply with third party license terms applicable to your
* use of third party software (including open source software) that may
* accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
* EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
* WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
* PARTICULAR PURPOSE.
*
* IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
* INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
* WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
* BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
* FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
* ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
*******************************************************************************/
//DOM-IGNORE-END

#include "plib_pio.h"
#include "interrupts.h"

/* Array to store callback objects of each configured interrupt */
volatile static PIO_PIN_CALLBACK_OBJ portPinCbObj[2];

/* Array to store number of interrupts in each PORT Channel + previous interrupt count */
volatile static uint8_t portNumCb[7 + 1] = { 0, 1, 1, 2, 2, 2, 2, 2, };

/* PIO base address for each port group */
static pio_registers_t* const PIO_REGS[PIO_PORT_MAX] = { PIO0_REGS, PIO0_REGS, PIO0_REGS, PIO1_REGS };
/* Index of each port group */
static const uint32_t PIO_INDEX[PIO_PORT_MAX] = { 0U, 1U, 2U, 0U };

/******************************************************************************
  Function:
    PIO_Initialize ( void )

  Summary:
    Initialize the PIO library.

  Remarks:
    See plib_pio.h for more details.
*/
void PIO_Initialize ( void )
{
 /* Port A Peripheral function A configuration */
   PIOA_REGS->PIO_MSKR = 0xf0030U;
   PIOA_REGS->PIO_CFGR = 0x1U;

 /* Port A Peripheral function B configuration */
   PIOA_REGS->PIO_MSKR = 0x1e00000U;
   PIOA_REGS->PIO_CFGR = 0x2U;

 /* Port A Peripheral function GPIO configuration */
   PIOA_REGS->PIO_MSKR = 0x2000387U;
   PIOA_REGS->PIO_CFGR = 0x0U;

 /* Port A Pin 0 configuration */
   PIOA_REGS->PIO_MSKR = 0x1U;
   PIOA_REGS->PIO_CFGR = (PIOA_REGS->PIO_CFGR & (PIO_CFGR_FUNC_Msk)) | 0x100U;

 /* Port A Pin 1 configuration */
   PIOA_REGS->PIO_MSKR = 0x2U;
   PIOA_REGS->PIO_CFGR = (PIOA_REGS->PIO_CFGR & (PIO_CFGR_FUNC_Msk)) | 0x100U;

 /* Port A Pin 2 configuration */
   PIOA_REGS->PIO_MSKR = 0x4U;
   PIOA_REGS->PIO_CFGR = (PIOA_REGS->PIO_CFGR & (PIO_CFGR_FUNC_Msk)) | 0x3001400U;

 /* Port A Pin 7 configuration */
   PIOA_REGS->PIO_MSKR = 0x80U;
   PIOA_REGS->PIO_CFGR = (PIOA_REGS->PIO_CFGR & (PIO_CFGR_FUNC_Msk)) | 0x200U;

 /* Port A Pin 8 configuration */
   PIOA_REGS->PIO_MSKR = 0x100U;
   PIOA_REGS->PIO_CFGR = (PIOA_REGS->PIO_CFGR & (PIO_CFGR_FUNC_Msk)) | 0x100U;

 /* Port A Pin 9 configuration */
   PIOA_REGS->PIO_MSKR = 0x200U;
   PIOA_REGS->PIO_CFGR = (PIOA_REGS->PIO_CFGR & (PIO_CFGR_FUNC_Msk)) | 0x100U;

 /* Port A Pin 25 configuration */
   PIOA_REGS->PIO_MSKR = 0x2000000U;
   PIOA_REGS->PIO_CFGR = (PIOA_REGS->PIO_CFGR & (PIO_CFGR_FUNC_Msk)) | 0x100U;

 /* Port A Latch configuration */
   PIOA_REGS->PIO_CODR = 0x2000387U;

    /* Clear the ISR register */
   (uint32_t)PIOA_REGS->PIO_ISR;
 /* Port B Peripheral function GPIO configuration */
   PIOB_REGS->PIO_MSKR = 0x8002U;
   PIOB_REGS->PIO_CFGR = 0x0U;

 /* Port B Pin 15 configuration */
   PIOB_REGS->PIO_MSKR = 0x8000U;
   PIOB_REGS->PIO_CFGR = (PIOB_REGS->PIO_CFGR & (PIO_CFGR_FUNC_Msk)) | 0x200U;

 /* Port B Latch configuration */
   PIOB_REGS->PIO_CODR = 0x8002U;

 /* Port C Peripheral function GPIO configuration */
   PIOC_REGS->PIO_MSKR = 0x80U;
   PIOC_REGS->PIO_CFGR = 0x0U;

 /* Port C Pin 7 configuration */
   PIOC_REGS->PIO_MSKR = 0x80U;
   PIOC_REGS->PIO_CFGR = (PIOC_REGS->PIO_CFGR & (PIO_CFGR_FUNC_Msk)) | 0x4001000U;

 /* Port C Latch configuration */
   PIOC_REGS->PIO_CODR = 0x80U;

    /* Clear the ISR register */
   (uint32_t)PIOC_REGS->PIO_ISR;
 /* Port D Peripheral function GPIO configuration */
   PIOD_REGS->PIO_MSKR = 0xd8008U;
   PIOD_REGS->PIO_CFGR = 0x0U;

 /* Port D Pin 3 configuration */
   PIOD_REGS->PIO_MSKR = 0x8U;
   PIOD_REGS->PIO_CFGR = (PIOD_REGS->PIO_CFGR & (PIO_CFGR_FUNC_Msk)) | 0x100U;

 /* Port D Pin 15 configuration */
   PIOD_REGS->PIO_MSKR = 0x8000U;
   PIOD_REGS->PIO_CFGR = (PIOD_REGS->PIO_CFGR & (PIO_CFGR_FUNC_Msk)) | 0x100U;

 /* Port D Pin 16 configuration */
   PIOD_REGS->PIO_MSKR = 0x10000U;
   PIOD_REGS->PIO_CFGR = (PIOD_REGS->PIO_CFGR & (PIO_CFGR_FUNC_Msk)) | 0x100U;

 /* Port D Pin 18 configuration */
   PIOD_REGS->PIO_MSKR = 0x40000U;
   PIOD_REGS->PIO_CFGR = (PIOD_REGS->PIO_CFGR & (PIO_CFGR_FUNC_Msk)) | 0x100U;

 /* Port D Pin 19 configuration */
   PIOD_REGS->PIO_MSKR = 0x80000U;
   PIOD_REGS->PIO_CFGR = (PIOD_REGS->PIO_CFGR & (PIO_CFGR_FUNC_Msk)) | 0x100U;

 /* Port D Latch configuration */
   PIOD_REGS->PIO_SODR = 0x50008U;
   PIOD_REGS->PIO_CODR = 0xd8008U & ~0x50008U;




    uint32_t i;
    /* Initialize Interrupt Pin data structures */
    portPinCbObj[0 + 0].pin = PIO_PIN_PA2;
    
    portPinCbObj[1 + 0].pin = PIO_PIN_PC7;
    
    for(i = 0U; i < 2U; i++)
    {
        portPinCbObj[i].callback = NULL;
    }

}

// *****************************************************************************
// *****************************************************************************
// Section: PIO APIs which operates on multiple pins of a port
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Function:
    uint32_t PIO_PortRead ( PIO_PORT port )

  Summary:
    Read all the I/O lines of the selected port.

  Description:
    This function reads the live data values on all the I/O lines of the
    selected port.  Bit values returned in each position indicate corresponding
    pin levels.
    1 = Pin is high.
    0 = Pin is low.

    This function reads the value regardless of pin configuration, whether it is
    set as as an input, driven by the PIO Controller, or driven by a peripheral.

  Remarks:
    If the port has less than 32-bits, unimplemented pins will read as
    low (0).
    Implemented pins are Right aligned in the 32-bit return value.
*/
uint32_t PIO_PortRead(PIO_PORT port)
{
    return PIO_REGS[port]->PIO_GROUP[PIO_INDEX[port]].PIO_PDSR;
}


// *****************************************************************************
/* Function:
    void PIO_PortWrite (PIO_PORT port, uint32_t mask, uint32_t value);

  Summary:
    Write the value on the masked I/O lines of the selected port.

  Remarks:
    See plib_pio.h for more details.
*/
void PIO_PortWrite(PIO_PORT port, uint32_t mask, uint32_t value)
{
    PIO_REGS[port]->PIO_GROUP[PIO_INDEX[port]].PIO_MSKR = mask;
    PIO_REGS[port]->PIO_GROUP[PIO_INDEX[port]].PIO_ODSR = value;
}


// *****************************************************************************
/* Function:
    uint32_t PIO_PortLatchRead ( PIO_PORT port )

  Summary:
    Read the latched value on all the I/O lines of the selected port.

  Remarks:
    See plib_pio.h for more details.
*/
uint32_t PIO_PortLatchRead(PIO_PORT port)
{
    return PIO_REGS[port]->PIO_GROUP[PIO_INDEX[port]].PIO_ODSR;
}


// *****************************************************************************
/* Function:
    void PIO_PortSet ( PIO_PORT port, uint32_t mask )

  Summary:
    Set the selected IO pins of a port.

  Remarks:
    See plib_pio.h for more details.
*/
void PIO_PortSet(PIO_PORT port, uint32_t mask)
{
    PIO_REGS[port]->PIO_GROUP[PIO_INDEX[port]].PIO_SODR = mask;
}

// *****************************************************************************
/* Function:
    void PIO_PortClear ( PIO_PORT port, uint32_t mask )

  Summary:
    Clear the selected IO pins of a port.

  Remarks:
    See plib_pio.h for more details.
*/
void PIO_PortClear(PIO_PORT port, uint32_t mask)
{
    PIO_REGS[port]->PIO_GROUP[PIO_INDEX[port]].PIO_CODR = mask;
}

// *****************************************************************************
/* Function:
    void PIO_PortToggle ( PIO_PORT port, uint32_t mask )

  Summary:
    Toggles the selected IO pins of a port.

  Remarks:
    See plib_pio.h for more details.
*/
void PIO_PortToggle(PIO_PORT port, uint32_t mask)
{
    /* Write into Clr and Set registers */
    PIO_REGS[port]->PIO_GROUP[PIO_INDEX[port]].PIO_MSKR = mask;
    PIO_REGS[port]->PIO_GROUP[PIO_INDEX[port]].PIO_ODSR ^= mask;
}

// *****************************************************************************
/* Function:
    void PIO_PortInputEnable ( PIO_PORT port, uint32_t mask )

  Summary:
    Enables selected IO pins of a port as input.

  Remarks:
    See plib_pio.h for more details.
*/
void PIO_PortInputEnable(PIO_PORT port, uint32_t mask)
{
    PIO_REGS[port]->PIO_GROUP[PIO_INDEX[port]].PIO_MSKR = mask;
    PIO_REGS[port]->PIO_GROUP[PIO_INDEX[port]].PIO_CFGR &= ~PIO_CFGR_DIR_Msk;
}

// *****************************************************************************
/* Function:
    void PIO_PortOutputEnable ( PIO_PORT port, uint32_t mask )

  Summary:
    Enables selected IO pins of a port as output(s).

  Remarks:
    See plib_pio.h for more details.
*/
void PIO_PortOutputEnable(PIO_PORT port, uint32_t mask)
{
    PIO_REGS[port]->PIO_GROUP[PIO_INDEX[port]].PIO_MSKR = mask;
    PIO_REGS[port]->PIO_GROUP[PIO_INDEX[port]].PIO_CFGR |= PIO_CFGR_DIR_Msk;
}
// *****************************************************************************
/* Function:
    void PIO_PortInterruptEnable(PIO_PORT port, uint32_t mask)

  Summary:
    Enables IO interrupt on selected IO pins of a port.

  Remarks:
    See plib_pio.h for more details.
*/
void PIO_PortInterruptEnable(PIO_PORT port, uint32_t mask)
{
    PIO_REGS[port]->PIO_GROUP[PIO_INDEX[port]].PIO_IER = mask;
}

// *****************************************************************************
/* Function:
    void PIO_PortInterruptDisable(PIO_PORT port, uint32_t mask)

  Summary:
    Disables IO interrupt on selected IO pins of a port.

  Remarks:
    See plib_pio.h for more details.
*/
void PIO_PortInterruptDisable(PIO_PORT port, uint32_t mask)
{
    PIO_REGS[port]->PIO_GROUP[PIO_INDEX[port]].PIO_IDR = mask;
}

// *****************************************************************************
// *****************************************************************************
// Section: PIO APIs which operates on one pin at a time
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Function:
    bool PIO_PinInterruptCallbackRegister(
        PIO_PIN pin,
        const PIO_PIN_CALLBACK callback,
        uintptr_t context
    );

  Summary:
    Allows application to register callback for every pin.

  Remarks:
    See plib_pio.h for more details.
*/
bool PIO_PinInterruptCallbackRegister(
    PIO_PIN pin,
    const PIO_PIN_CALLBACK callback,
    uintptr_t context
)
{
    uint8_t i;
    uint8_t portIndex;

    portIndex = pin >> 5U;

    for(i = portNumCb[portIndex]; i < portNumCb[portIndex +1U]; i++)
    {
        if (portPinCbObj[i].pin == pin)
        {
            portPinCbObj[i].callback = callback;
            portPinCbObj[i].context  = context;
            return true;
        }
    }
    return false;
}

// *****************************************************************************
// *****************************************************************************
// Section: Interrupt Service Routine (ISR) Implementation(s)
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Function:
    void PIOA_InterruptHandler (void)

  Summary:
    Interrupt handler for PORTA.

  Description:
    This function defines the Interrupt service routine for PORTA.
    This is the function which by default gets into Interrupt Vector Table.

  Remarks:
    User should not call this function.
*/
void __attribute__((used)) PIOA_InterruptHandler(void)
{
    uint32_t status;
    uint8_t j;
    /* Additional local variable to prevent MISRA C violations (Rule 13.x) */
    PIO_PIN pin;
    uintptr_t context;

    status = PIOA_REGS->PIO_ISR;
    status &= PIOA_REGS->PIO_IMR;

    for( j = 0U; j < 1U; j++ )
    {
        pin = portPinCbObj[j].pin;
        context = portPinCbObj[j].context;
        if((portPinCbObj[j].callback != NULL) && ((status & (1UL << (pin & 0x1FU))) != 0U))
        {
            portPinCbObj[j].callback ( portPinCbObj[j].pin, context);
        }
    }
}
// *****************************************************************************
/* Function:
    void PIOC_InterruptHandler (void)

  Summary:
    Interrupt handler for PORTC.

  Description:
    This function defines the Interrupt service routine for PORTC.
    This is the function which by default gets into Interrupt Vector Table.

  Remarks:
    User should not call this function.
*/
void __attribute__((used)) PIOC_InterruptHandler(void)
{
    uint32_t status;
    uint8_t j;
    /* Additional local variable to prevent MISRA C violations (Rule 13.x) */
    PIO_PIN pin;
    uintptr_t context;

    status = PIOC_REGS->PIO_ISR;
    status &= PIOC_REGS->PIO_IMR;

    for( j = 1U; j < 2U; j++ )
    {
        pin = portPinCbObj[j].pin;
        context = portPinCbObj[j].context;
        if((portPinCbObj[j].callback != NULL) && ((status & (1UL << (pin & 0x1FU))) != 0U))
        {
            portPinCbObj[j].callback ( portPinCbObj[j].pin, context);
        }
    }
}

/*******************************************************************************
 End of File
*/
