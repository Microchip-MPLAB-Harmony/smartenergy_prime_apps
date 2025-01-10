/*******************************************************************************
  PRIME User PIBs Service Implementation.

  Company:
    Microchip Technology Inc.

  File Name:
    srv_user_pib.c

  Summary:
    Source code for the PRIME User PIBs service implementation.

  Description:
    The User PIBs service provides a simple interface to handle a parameter
    interface base defined by the user from the PRIME stack. This file contains
    the source code for the implementation of this service.
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

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "definitions.h"
#include "srv_user_pib.h"
#include "device.h"

// *****************************************************************************
// *****************************************************************************
// Section: File Scope Data
// *****************************************************************************
// *****************************************************************************

/* PIB values */
static uint32_t srvUserPibValues[11];

/* Callback function pointers */
static SRV_USER_PIB_GET_REQUEST_CALLBACK SRV_USER_PIB_GetRequestCb;
static SRV_USER_PIB_SET_REQUEST_CALLBACK SRV_USER_PIB_SetRequestCb;

// *****************************************************************************
// *****************************************************************************
// Section: User PIBs Service Interface Implementation
// *****************************************************************************
// *****************************************************************************

void SRV_USER_PIB_GetRequest(uint16_t pibAttrib)
{
    uint32_t pibValue;
    uint8_t getResult;

    /* Check PIB value */
    if ((pibAttrib >= PIB_USER_RESET_INFO) && (pibAttrib <= PIB_USER_R12))
    {
        getResult = 1; /* true */
        pibValue = srvUserPibValues[pibAttrib & 0x000FU];
    }
    else
    {
        getResult = 0; /* false */
        pibValue = 0;
    }

    /* Return result */
    if (SRV_USER_PIB_GetRequestCb != NULL)
    {
        SRV_USER_PIB_GetRequestCb(getResult, pibAttrib, &pibValue, 4);
    }
}

void SRV_USER_PIB_SetRequest(uint16_t pibAttrib, void *pibValue, uint8_t pibSize)
{
    (void)pibAttrib;
    (void)pibValue;
    (void)pibSize;

    /* Return result */
    if (SRV_USER_PIB_SetRequestCb != NULL)
    {
        SRV_USER_PIB_SetRequestCb(false);
    }
}

void SRV_USER_PIB_GetRequestCbRegister(SRV_USER_PIB_GET_REQUEST_CALLBACK callback)
{
    SRV_USER_PIB_GetRequestCb = callback;
}

void SRV_USER_PIB_SetRequestCbRegister(SRV_USER_PIB_SET_REQUEST_CALLBACK callback)
{
    SRV_USER_PIB_SetRequestCb = callback;
}

void SRV_USER_PIB_Initialize(void)
{
    SRV_USER_PIB_GetRequestCb = NULL;
    SRV_USER_PIB_SetRequestCb = NULL;

    /* Store PIB values */
    srvUserPibValues[PIB_USER_RESET_INFO & 0x000FU] = SUPC_GPBRRead(GPBR_REGS_0);
    srvUserPibValues[PIB_USER_PC & 0x000FU] = SUPC_GPBRRead(GPBR_REGS_1);
    srvUserPibValues[PIB_USER_LR & 0x000FU] = SUPC_GPBRRead(GPBR_REGS_2);
    srvUserPibValues[PIB_USER_PSR & 0x000FU] = SUPC_GPBRRead(GPBR_REGS_3);
    srvUserPibValues[PIB_USER_HFSR & 0x000FU] = SUPC_GPBRRead(GPBR_REGS_4);
    srvUserPibValues[PIB_USER_CFSR & 0x000FU] = SUPC_GPBRRead(GPBR_REGS_5);
    srvUserPibValues[PIB_USER_R0 & 0x000FU] = SUPC_GPBRRead(GPBR_REGS_6);
    srvUserPibValues[PIB_USER_R1 & 0x000FU] = SUPC_GPBRRead(GPBR_REGS_7);
    srvUserPibValues[PIB_USER_R2 & 0x000FU] = SUPC_GPBRRead(GPBR_REGS_8);
    srvUserPibValues[PIB_USER_R3 & 0x000FU] = SUPC_GPBRRead(GPBR_REGS_9);
    srvUserPibValues[PIB_USER_R12 & 0x000FU] = SUPC_GPBRRead(GPBR_REGS_10);

    /* Clear registers (except reset information) */
    SUPC_GPBRWrite(GPBR_REGS_1, 0);
    SUPC_GPBRWrite(GPBR_REGS_2, 0);
    SUPC_GPBRWrite(GPBR_REGS_3, 0);
    SUPC_GPBRWrite(GPBR_REGS_4, 0);
    SUPC_GPBRWrite(GPBR_REGS_5, 0);
    SUPC_GPBRWrite(GPBR_REGS_6, 0);
    SUPC_GPBRWrite(GPBR_REGS_7, 0);
    SUPC_GPBRWrite(GPBR_REGS_8, 0);
    SUPC_GPBRWrite(GPBR_REGS_9, 0);
    SUPC_GPBRWrite(GPBR_REGS_10, 0);
}
