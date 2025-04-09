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

#define PLC460_RESET_PIN                     SYS_PORT_PIN_PD3
#define PLC460_LDO_EN_PIN                    SYS_PORT_PIN_PD16

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
   /* Enable co-processor bus clock  */
   PMC_REGS->PMC_SCER = (PMC_SCER_CPKEY_PASSWD | PMC_SCER_CPBMCK_Msk);
   /* Coprocessor Peripheral Enable */
   RSTC_REGS->RSTC_MR |= (RSTC_MR_KEY_PASSWD | RSTC_MR_CPEREN_Msk);
   /* Program PMC_CPU_CKR.CPPRES and wait for PMC_SR.CPMCKRDY to be set   */
   uint32_t reg = (PMC_REGS->PMC_CPU_CKR & ~PMC_CPU_CKR_CPPRES_Msk);
   reg |= PMC_CPU_CKR_CPPRES_CLK_2;
   PMC_REGS->PMC_CPU_CKR = reg;
   PMC_REGS->PMC_PCR = PMC_PCR_CMD_Msk | PMC_PCR_EN_Msk | PMC_PCR_PID(ID_PIOA);
   while((PMC_REGS->PMC_CSR0 & PMC_CSR0_PID17_Msk) == 0U)
   {
       /* Wait for clock to be initialized */
   }
   /* Disable STBY Pin */
   SYS_PORT_PinOutputEnable(SYS_PORT_PIN_PA16);
   SYS_PORT_PinClear(SYS_PORT_PIN_PA16);
   while ((PMC_REGS->PMC_SR & PMC_SR_CPMCKRDY_Msk) != PMC_SR_CPMCKRDY_Msk)
   {
       /* Wait for status CPMCKRDY */
   }
   PMC_REGS->PMC_PCR = PMC_PCR_CMD_Msk | PMC_PCR_EN_Msk | PMC_PCR_PID(ID_PIOD);
   while((PMC_REGS->PMC_CSR2 & PMC_CSR2_PID85_Msk) == 0U)
   {
       /* Wait for clock to be initialized */
   }
   /* Enable Reset Pin */
   SYS_PORT_PinOutputEnable(PLC460_RESET_PIN);
   SYS_PORT_PinClear(PLC460_RESET_PIN);
   /* Enable LDO Pin */
   SYS_PORT_PinOutputEnable(PLC460_LDO_EN_PIN);
   SYS_PORT_PinSet(PLC460_LDO_EN_PIN);
}

/* MISRA C-2012 deviation block end */

/*******************************************************************************
 End of File
 */
