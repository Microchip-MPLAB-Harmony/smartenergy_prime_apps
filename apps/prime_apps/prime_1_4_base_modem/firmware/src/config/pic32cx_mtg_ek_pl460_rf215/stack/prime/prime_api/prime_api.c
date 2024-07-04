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
#include "stack/prime/prime_api/prime_hal_wrapper.h"
#include "stack/prime/mac/mac.h"
#include "stack/prime/mngp/mngp.h"
#include "stack/prime/conv/sscs/null/cl_null.h"
#include "stack/prime/conv/sscs/null/cl_null_api.h"
#include "stack/prime/conv/sscs/iec_4_32/cl_432.h"
#include "stack/prime/conv/sscs/iec_4_32/cl_432_api.h"
#include "stack/prime/mngp/bmng_api.h"

// *****************************************************************************
// *****************************************************************************
// Section: Macro Definitions
// *****************************************************************************
// *****************************************************************************
/* Configuration of the CORTEX-M4 Processor and Core Peripherals */
/* PIC32CXMT */
#define __NVIC_PRIO_BITS         4

/* Security profile for PRIME 1.4 */
#define MAC_SECURITY_PROFILE     0

// *****************************************************************************
// *****************************************************************************
// Section: Global Data
// *****************************************************************************
// *****************************************************************************
const PRIME_API PRIME_API_Interface =
{
    .vendor = PRIME_PIB_VENDOR,
    .model = PRIME_PIB_MODEL,
    .version = PRIME_FW_VERSION,
    .Initialize = PRIME_API_Initialize,
    .Tasks = PRIME_API_Tasks,
    .MacSetCallbacks = CL_NULL_SetCallbacks,     
    .MacEstablishRequest = CL_NULL_EstablishRequest, 
    .MacEstablishResponse = CL_NULL_EstablishResponse,
    .MacReleaseRequest = CL_NULL_ReleaseRequest,
    .MacReleaseResponse = CL_NULL_ReleaseResponse,
    .MacRedirectResponse = CL_NULL_RedirectResponse,
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
    .MlmePromoteRequest = CL_NULL_MlmePromoteRequest,    
    .MlmeMpPromoteRequest = CL_NULL_MlmeMpPromoteRequest,
    .MlmeResetRequest = CL_NULL_MlmeResetRequest,
    .MlmeGetRequest = CL_NULL_MlmeGetRequest,
    .MlmeListGetRequest = CL_NULL_MlmeListGetRequest,
    .MlmeSetRequest = CL_NULL_MlmeSetRequest,
    .Cl432SetCallbacks = CL_432_SetCallbacks,
    .Cl432ReleaseRequest = CL_432_ReleaseRequest,
    .Cl432DlDataRequest = CL_432_DlDataRequest,
    .BmngSetCallbacks = BMNG_SetCallbacks,
    .BmngFupClearTargetListRequest = BMNG_FUP_ClearTargetListRequest,
    .BmngFupAddTargetRequest = BMNG_FUP_AddTargetRequest,
    .BmngFupSetFwDataRequest = BMNG_FUP_SetFwDataRequest,
    .BmngFupSetUpgradeOptionsRequest = BMNG_FUP_SetUpgradeOptionsRequest,
    .BmngFupInitFileTxRequest = BMNG_FUP_InitFileTxRequest,
    .BmngFupDataFrameRequest = BMNG_FUP_DataFrameRequest,
    .BmngFupCheckCrcRequest = BMNG_FUP_CheckCrcRequest,
    .BmngFupAbortFuRequest = BMNG_FUP_AbortFuRequest,
    .BmngFupStartFuRequest = BMNG_FUP_StartFuRequest,
    .BmngFupSetMatchRuleRequest = BMNG_FUP_SetMatchRuleRequest,
    .BmngFupGetVersionRequest =  BMNG_FUP_GetVersionRequest,
    .BmngFupGetStateRequest = BMNG_FUP_GetStateRequest,
    .BmngFupSetSignatureDataRequest = BMNG_FUP_SetSignatureDataRequest,
    .BmngPprofGetRequest = BMNG_PPROF_GetRequest,
    .BmngPprofSetRequest = BMNG_PPROF_SetRequest,
    .BmngPprofResetRequest = BMNG_PPROF_ResetRequest,
    .BmngPprofRebootRequest = BMNG_PPROF_RebootRequest,
    .BmngPprofGetEnhancedRequest = BMNG_PPROF_GetEnhancedRequest,
    .BmngPprofGetZcDiffRequest = BMNG_PPROF_GetZcDiffRequest,
    .BmngWhitelistAddRequest = BMNG_WHITELIST_AddRequest,
    .BmngWhitelistRemoveRequest = BMNG_WHITELIST_RemoveRequest,
};

/* Object obtained from PAL Initialize */
static SYS_MODULE_OBJ palSysObj;

// *****************************************************************************
// *****************************************************************************
// Section: File scope functions
// *****************************************************************************
// *****************************************************************************
static void lPRIME_API_SetPrimeVersion(MAC_VERSION_INFO *macInfo)
{
    uint8_t sizeConfig, sizeInfo;
    uint8_t copyLen;

    memset(macInfo, 0, sizeof(MAC_VERSION_INFO));

    /* Update MODEL */
    sizeConfig = sizeof(PRIME_FW_MODEL);
    sizeInfo = sizeof(macInfo->fwModel);
    if (sizeConfig < sizeInfo) 
    {
        copyLen = sizeConfig;
    } 
    else 
    {
        copyLen = sizeInfo;
    }

    memcpy(macInfo->fwModel, PRIME_FW_MODEL, copyLen);
    macInfo->pibModel = PRIME_PIB_MODEL;

    /* Update VENDOR */
    sizeConfig = sizeof(PRIME_FW_VENDOR);
    sizeInfo = sizeof(macInfo->fwVendor);
    if (sizeConfig < sizeInfo) {
        copyLen = sizeConfig;
    } else {
        copyLen = sizeInfo;
    }

    memcpy(macInfo->fwVendor, PRIME_FW_VENDOR, copyLen);
    macInfo->pibVendor = PRIME_PIB_VENDOR;

    /* Update VERSION */
    sizeConfig = sizeof(PRIME_FW_VERSION);
    sizeInfo = sizeof(macInfo->fwVersion);
    if (sizeConfig < sizeInfo) {
        copyLen = sizeConfig;
    } else {
        copyLen = sizeInfo;
    }

    memcpy(macInfo->fwVersion, PRIME_FW_VERSION, copyLen);
}

// *****************************************************************************
// *****************************************************************************
// Section: PRIME API Interface Implementation
// *****************************************************************************
// *****************************************************************************
void PRIME_API_Initialize(PRIME_API_INIT *init)
{
    MAC_VERSION_INFO macInfo;

    /* Set critical region */
    __set_BASEPRI( 2 << (8 - __NVIC_PRIO_BITS));


    /* Set PRIME HAL wrapper */
    PRIME_HAL_WRP_Configure(init->halApi);

    /* Set PRIME version from configuration */
    lPRIME_API_SetPrimeVersion(&macInfo);
    
    /* Initialize PAL layer */
    palSysObj = PRIME_HAL_WRP_PAL_Initialize(init->palIndex);

    /* Initialize MAC layer */
    MAC_Initialize(&macInfo, (uint8_t)MAC_SECURITY_PROFILE);

    /* Initialize Convergence layers */
    CL_NULL_Initialize();
    CL_432_Initialize();

    /* Initialize Management Plane */
    MNGP_Initialize(&macInfo, init->mngPlaneUsiPort);

    /* Set critical region */
    __set_BASEPRI(0);
}

void PRIME_API_Tasks(void)
{
    /* Set critical region */
    __set_BASEPRI( 3 << (8 - __NVIC_PRIO_BITS));
    
    /* Proccess PAL layer */
    PRIME_HAL_WRP_PAL_Tasks(palSysObj);

	/* Process MAC layer */
	MAC_Tasks();


    /* Set critical region */
    __set_BASEPRI(0);
}

void PRIME_API_GetPrimeAPI(PRIME_API **pPrimeApi)
{
    *pPrimeApi = (PRIME_API *)&PRIME_API_Interface;
}

