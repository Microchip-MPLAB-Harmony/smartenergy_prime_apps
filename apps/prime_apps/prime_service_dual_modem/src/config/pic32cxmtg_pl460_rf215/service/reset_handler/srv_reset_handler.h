/*******************************************************************************
  PRIME Reset Handler Service Interface Header File

  Company:
    Microchip Technology Inc.

  File Name:
    srv_reset_handler.h

  Summary:
    PRIME Reset Handler Service Interface Header File.

  Description:
    The Reset Handler service provides a simple interface to trigger system
    resets and to manage and store reset causes. This file provides the
    interface definition for this service.
*******************************************************************************/

//DOM-IGNORE-BEGIN
/*
Copyright (C) 2024, Microchip Technology Inc., and its subsidiaries. All rights reserved.

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

#ifndef SRV_RESET_HANDLER_H    // Guards against multiple inclusion
#define SRV_RESET_HANDLER_H

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

extern volatile uint32_t saved_r0;
extern volatile uint32_t saved_r1;
extern volatile uint32_t saved_r2;
extern volatile uint32_t saved_r3;
extern volatile uint32_t saved_r12;
extern volatile uint32_t saved_lr;
extern volatile uint32_t saved_pc;
extern volatile uint32_t saved_psr;
extern volatile uint32_t saved_hfsr;
extern volatile uint32_t saved_cfsr;

// *****************************************************************************
// *****************************************************************************
// Section: Data Types
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* PRIME Reset Causes

 Summary:
    Reset causes defined for a PRIME system.

 Description:
    The PRIME reset causes include the device reset causes and reset causes
    specific to the PRIME operation.

 Remarks:
    None.
*/
typedef enum {
    RESET_HANDLER_GENERAL_RESET       = 0,      /* First power reset */
	RESET_HANDLER_BACKUP_RESET        = 1,      /* Wake up from backup mode */
	RESET_HANDLER_WATCHDOG_RESET      = 2,      /* Watchdog fault */
	RESET_HANDLER_SOFTWARE_RESET      = 3,      /* Reset requested by the software */
	RESET_HANDLER_USER_RESET          = 4,      /* Reset requested by the user (NRST pin low) */
	RESET_HANDLER_FU_RESET            = 5,      /* Reset during firmware upgrade */
	RESET_HANDLER_USAGE_FAULT_RESET   = 6,      /* Usage fault */
	RESET_HANDLER_BUS_FAULT_RESET     = 7,      /* Bus fault */
	RESET_HANDLER_MEM_MANAGE_RESET    = 8,      /* Memory management fault */
	RESET_HANDLER_HARD_FAULT_RESET    = 9,      /* Hard fault */
	RESET_HANDLER_VECTOR_FAULT_RESET  = 10,     /* Vector fault */
} SRV_RESET_HANDLER_RESET_CAUSE;

// *****************************************************************************
// *****************************************************************************
// Section: Reset Handler Service Interface Definition
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Function:
    void SRV_RESET_HANDLER_Initialize(void)

  Summary:
    Initializes the Reset Handler service.

  Description:
    This routine initializes the PRIME Reset Handler service.

  Precondition:
    None.

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    int main()
    {
        SRV_RESET_HANDLER_Initialize();
    }
    </code>

  Remarks:
    This routine must be called before any other PRIME Reset Handler service
    routine. This function is normally not called directly by an application.
    It is called by the system's initialize routine (SYS_Initialize).
*/

void SRV_RESET_HANDLER_Initialize(void);

// *****************************************************************************
/* Function:
    void SRV_RESET_HANDLER_RestartSystem(SRV_RESET_HANDLER_RESET_CAUSE resetType)

  Summary:
    Trigger a system restart.

  Description:
    This routine triggers a system restart. It can be invoked by the PRIME stack
    as well as by the application.

  Precondition:
    The SRV_RESET_HANDLER_Initialize routine must have been called before.

  Parameters:
    resetType      - Type of reset

  Returns:
    None.

  Example:
    <code>
    int main(void)
    {
        SRV_RESET_HANDLER_RestartSystem(RESET_HANDLER_SOFTWARE_RESET);
    }
    </code>

  Remarks:
    None.
*/

void SRV_RESET_HANDLER_RestartSystem(SRV_RESET_HANDLER_RESET_CAUSE resetType);

void DWDT0_Handler(void);

#endif //SRV_RESET_HANDLER_H
