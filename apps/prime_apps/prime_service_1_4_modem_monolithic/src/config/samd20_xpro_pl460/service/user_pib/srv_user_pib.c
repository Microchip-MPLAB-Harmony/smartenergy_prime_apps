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

    getResult = 0; /* false */
    pibValue = 0;

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

}
