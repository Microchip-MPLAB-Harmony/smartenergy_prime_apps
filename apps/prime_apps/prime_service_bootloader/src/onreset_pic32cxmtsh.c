/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    onreset_pic32cxmtsh.c

  Summary:
    This file contains the source code for reset actons needed for a PL460.

  Description:
    This file contains the source code with the special actions that need 
    to be carried out after the reset to ensure the proper functioning 
    of the PL460.
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
#include "configuration.h"
#include "definitions.h"
#include "device.h"

// *****************************************************************************
// *****************************************************************************
// Section: Macro Definitions
// *****************************************************************************
// *****************************************************************************

#define PL460_RESET_PIN                     SYS_PORT_PIN_PA18

// *****************************************************************************
// *****************************************************************************
// Section: File scope functions
// *****************************************************************************
// *****************************************************************************

// </editor-fold>
// <editor-fold defaultstate="collapsed" desc="_on_reset() critical function">
/* MISRA C-2012 deviation block start */
/* MISRA C-2012 Rule 8.4 deviated once. Deviation record ID - H3_MISRAC_2012_R_8_4_DR_1 */
/* MISRA C-2012 Rule 21.2 deviated once. Deviation record ID - H3_MISRAC_2012_R_21_2_DR_1 */

/* This routine must initialize the PL460 control pins as soon as possible */
/* after a power up reset to avoid risks on starting up PL460 device when */
/* pull up resistors are configured by default */
void _on_reset(void)
{
   /* Enable PIOA clock */
    PMC_REGS->PMC_PCR = PMC_PCR_CMD_Msk | PMC_PCR_EN_Msk | PMC_PCR_PID(ID_PIOA);
    while((PMC_REGS->PMC_CSR0 & PMC_CSR0_PID17_Msk) == 0U)
    {
        /* Wait for clock to be initialized */
    }
    /* Enable and Clear Reset Pin */
    SYS_PORT_PinOutputEnable(PL460_RESET_PIN);
    SYS_PORT_PinClear(PL460_RESET_PIN);
}

/* MISRA C-2012 deviation block end */

/*******************************************************************************
 End of File
 */
