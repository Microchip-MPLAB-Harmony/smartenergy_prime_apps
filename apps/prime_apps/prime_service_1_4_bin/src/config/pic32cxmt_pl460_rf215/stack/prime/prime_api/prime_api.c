/*******************************************************************************
  PRIME API Source

  Company:
    Microchip Technology Inc.

  File Name:
    prime_api.c

  Summary:
    PRIME API Source File

  Description:
    This module manages the the PRIME stack from the PRIME application.
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
// Section: Include Files
// *****************************************************************************
// *****************************************************************************

#include <stdio.h>
#include "cmsis_compiler.h"
#include "prime_api.h"
#include "prime_api_types.h"
#include <libpic32c.h>
#include "stack/prime/prime_api/prime_hal_wrapper.h"
#include "stack/prime/mac/mac.h"
#include "stack/prime/mngp/mngp.h"
#include "stack/prime/conv/sscs/null/cl_null.h"
#include "stack/prime/conv/sscs/null/cl_null_api.h"
#include "stack/prime/conv/sscs/iec_4_32/cl_432.h"
#include "stack/prime/conv/sscs/iec_4_32/cl_432_api.h"

// *****************************************************************************
// *****************************************************************************
// Section: Macro Definitions
// *****************************************************************************
// *****************************************************************************

/* MISRA C-2012 deviation block start */
/* MISRA C-2012 Rule 21.1 deviated once. Deviation record ID - H3_MISRAC_2012_R_21_1_DR_1 */
/* Configuration of the CORTEX-M4 Processor and Core Peripherals */
/* PIC32CXMT */
#define __NVIC_PRIO_BITS         4
/* MISRA C-2012 deviation block end */

/* Security profile for PRIME 1.4 */
#define MAC_SECURITY_PROFILE     0

// *****************************************************************************
// *****************************************************************************
// Section: Global Data
// *****************************************************************************
// *****************************************************************************

/* MISRA C-2012 deviation block start */
/* MISRA C-2012 Rule 8.4 deviated once. Deviation record ID - H3_MISRAC_2012_R_8_4_DR_1 */
void __attribute__((optimize("-O1"), section(".text.Reset_Handler"), long_call)) Reset_Handler(void)
{
    // Avoid warning in compilation. Reset Handler is not needed.
}
/* MISRA C-2012 deviation block end */

__attribute__ ((section(".vectors"), used))
static const PRIME_API PRIME_API_Interface =
{
    .vendor = PRIME_PIB_VENDOR,
    .model = PRIME_PIB_MODEL,
    .version = PRIME_FW_VERSION,
    .Initialize = PRIME_API_Initialize,
    .Tasks = PRIME_API_Tasks,
    .Status = PRIME_API_Status,
    .MacSetCallbacks = CL_NULL_SetCallbacks,
    .MacEstablishRequest = CL_NULL_EstablishRequest,
    .MacEstablishResponse = CL_NULL_EstablishResponse,
    .MacReleaseRequest = CL_NULL_ReleaseRequest,
    .MacReleaseResponse = CL_NULL_ReleaseResponse,
    .MacJoinRequest = CL_NULL_JoinRequest,
    .MacJoinResponse = CL_NULL_JoinResponse,
    .MacLeaveRequest = CL_NULL_LeaveRequest,
    .MacDataRequest = CL_NULL_DataRequest,
    .PlmeResetRequest = CL_NULL_PlmeResetRequest,
    .PlmeSleepRequest = CL_NULL_PlmeSleepRequest,
    .PlmeResumeRequest = CL_NULL_PlmeResumeRequest,
    .PlmeTestModeRequest = CL_NULL_PlmeTestModeRequest,
    .PlmeGetRequest = CL_NULL_PlmeGetRequest,
    .PlmeSetRequest = CL_NULL_PlmeSetRequest,
    .MlmeRegisterRequest = CL_NULL_MlmeRegisterRequest,
    .MlmeUnregisterRequest = CL_NULL_MlmeUnregisterRequest,
    .MlmePromoteRequest = CL_NULL_MlmePromoteRequest,
    .MlmeMpPromoteRequest = CL_NULL_MlmeMpPromoteRequest,
    .MlmeDemoteRequest = CL_NULL_MlmeDemoteRequest,
    .MlmeMpDemoteRequest = CL_NULL_MlmeMpDemoteRequest,
    .MlmeResetRequest = CL_NULL_MlmeResetRequest,
    .MlmeGetRequest = CL_NULL_MlmeGetRequest,
    .MlmeListGetRequest = CL_NULL_MlmeListGetRequest,
    .MlmeSetRequest = CL_NULL_MlmeSetRequest,
    .Cl432SetCallbacks = CL_432_SetCallbacks,
    .Cl432EstablishRequest = CL_432_EstablishRequest,
    .Cl432ReleaseRequest = CL_432_ReleaseRequest,
    .Cl432DlDataRequest = CL_432_DlDataRequest,
};

/* Object obtained from PAL Initialize */
static SYS_MODULE_OBJ palSysObj;

/* State of the PRIME API */
static PRIME_API_STATE primeApiState;

/* MAC version information */
static MAC_VERSION_INFO primeApiMacInfo;

/* Port for the Management Plane */
static uint8_t primeApiMngPlanePort;

// *****************************************************************************
// *****************************************************************************
// Section: File scope functions
// *****************************************************************************
// *****************************************************************************
static void lPRIME_API_PrimeDataStartup(void)
{
    __pic32c_data_initialization();
}

static void lPRIME_API_SetPrimeVersion(MAC_VERSION_INFO *macInfo)
{
    uint8_t sizeConfig, sizeInfo;
    uint8_t copyLen;

    (void)memset(macInfo, 0, sizeof(MAC_VERSION_INFO));

    /* Update MODEL */
    sizeConfig = (uint8_t)sizeof(PRIME_FW_MODEL);
    sizeInfo = (uint8_t)sizeof(macInfo->fwModel);
    if (sizeConfig < sizeInfo)
    {
        copyLen = sizeConfig;
    }
    else
    {
        copyLen = sizeInfo;
    }

/* MISRA C-2012 deviation block start */
/* MISRA C-2012 Rule 7.4 deviated once. Deviation record ID - H3_MISRAC_2012_R_7_4_DR_1 */
    (void)memcpy(macInfo->fwModel, PRIME_FW_MODEL, copyLen);
/* MISRA C-2012 deviation block end */

    macInfo->pibModel = PRIME_PIB_MODEL;

    /* Update VENDOR */
    sizeConfig = (uint8_t)sizeof(PRIME_FW_VENDOR);
    sizeInfo = (uint8_t)sizeof(macInfo->fwVendor);
    if (sizeConfig < sizeInfo) {
        copyLen = sizeConfig;
    } else {
        copyLen = sizeInfo;
    }

/* MISRA C-2012 deviation block start */
/* MISRA C-2012 Rule 7.4 deviated once. Deviation record ID - H3_MISRAC_2012_R_7_4_DR_1 */
    (void)memcpy(macInfo->fwVendor, PRIME_FW_VENDOR, copyLen);
/* MISRA C-2012 deviation block end */

    macInfo->pibVendor = PRIME_PIB_VENDOR;

    /* Update VERSION */
    sizeConfig = (uint8_t)sizeof(PRIME_FW_VERSION);
    sizeInfo = (uint8_t)sizeof(macInfo->fwVersion);
    if (sizeConfig < sizeInfo) {
        copyLen = sizeConfig;
    } else {
        copyLen = sizeInfo;
    }

/* MISRA C-2012 deviation block start */
/* MISRA C-2012 Rule 7.4 deviated once. Deviation record ID - H3_MISRAC_2012_R_7_4_DR_1 */
    (void)memcpy(macInfo->fwVersion, PRIME_FW_VERSION, copyLen);
/* MISRA C-2012 deviation block end */

}

// *****************************************************************************
// *****************************************************************************
// Section: PRIME API Interface Implementation
// *****************************************************************************
// *****************************************************************************
void PRIME_API_Initialize(PRIME_API_INIT *init)
{
    lPRIME_API_PrimeDataStartup();

    /* Set PRIME HAL wrapper */
    PRIME_HAL_WRP_Configure(init->halApi);

    /* Set PRIME version from configuration */
    lPRIME_API_SetPrimeVersion(&primeApiMacInfo);

    /* Store port for PRIME initialization */
    primeApiMngPlanePort = init->mngPlaneUsiPort;

    /* Initialize PAL layer */
    palSysObj = PRIME_HAL_WRP_PAL_Initialize(init->palIndex);

    primeApiState = PRIME_API_STATE_PAL_INITIALIZING;
}

void PRIME_API_Tasks(void)
{
    switch (primeApiState)
    {
        case PRIME_API_STATE_PAL_INITIALIZING:

            /* Process PAL layer */
            PRIME_HAL_WRP_PAL_Tasks(palSysObj);

            if (PRIME_HAL_WRP_PAL_Status(palSysObj) == SYS_STATUS_READY)
            {
                /* Initialize MAC layer */
                MAC_Initialize(&primeApiMacInfo, (uint8_t)MAC_SECURITY_PROFILE);

                /* Initialize Convergence layers */
                CL_NULL_Initialize();
                CL_432_Initialize();

                /* Initialize Management Plane */
                MNGP_Initialize(&primeApiMacInfo, primeApiMngPlanePort);

                primeApiState = PRIME_API_STATE_PRIME_RUNNING;
            }

            break;

        case PRIME_API_STATE_PRIME_RUNNING:

            /* Process PAL layer */
            PRIME_HAL_WRP_PAL_Tasks(palSysObj);

            /* Process MAC layer */
            MAC_Tasks();

            /* Process Management Plane */
            MNGP_Tasks();

            break;

/* MISRA C-2012 deviation block start */
/* MISRA C-2012 Rule 16.4 deviated once. Deviation record ID - H3_MISRAC_2012_R_16_4_DR_1 */
         default:
            break;
/* MISRA C-2012 deviation block end */
    }

}

SYS_STATUS PRIME_API_Status(void)
{
    SYS_STATUS status;

    /* Return the PRIME API status */
    switch (primeApiState)
    {
        case PRIME_API_STATE_PAL_INITIALIZING:
            status = SYS_STATUS_BUSY;
            break;

        case PRIME_API_STATE_PRIME_RUNNING:
            status = SYS_STATUS_READY;
            break;

        default:
            status = SYS_STATUS_UNINITIALIZED;
            break;
    }

    return status;
}

