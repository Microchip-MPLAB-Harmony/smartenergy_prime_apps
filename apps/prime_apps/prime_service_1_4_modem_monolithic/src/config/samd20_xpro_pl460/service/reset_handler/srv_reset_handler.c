/*******************************************************************************
  PRIME Reset Handler Service Implementation.

  Company:
    Microchip Technology Inc.

  File Name:
    srv_reset_handler.c

  Summary:
    Source code for the PRIME Reset Handle service implementation.

  Description:
    The Reset Handler service provides a simple interface to trigger system
    resets and to manage and store reset causes.This file contains the source
    code for the implementation of this service.
*******************************************************************************/

///DOM-IGNORE-BEGIN
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

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "definitions.h"
#include "srv_reset_handler.h"
#include "device.h"


// *****************************************************************************
// *****************************************************************************
// Section: Reset Handler Service Interface Implementation
// *****************************************************************************
// *****************************************************************************

void SRV_RESET_HANDLER_Initialize(void)
{
}

void SRV_RESET_HANDLER_RestartSystem(SRV_RESET_HANDLER_RESET_CAUSE resetType)
{
    (void)resetType;


    /* Trigger software reset */
    NVIC_SystemReset();
}
