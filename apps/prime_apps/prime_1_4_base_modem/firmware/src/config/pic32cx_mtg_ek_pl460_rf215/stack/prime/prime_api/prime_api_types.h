/*******************************************************************************
  PRIME API Types Header

  Company:
    Microchip Technology Inc.

  File Name:
    prime_api_types.h

  Summary:
    PRIME API Types Header File

  Description:
    This file contains types of the PRIME API functions to be used by the 
    PRIME application.
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

#ifndef PRIME_API_TYPES_H_INCLUDE
#define PRIME_API_TYPES_H_INCLUDE

// *****************************************************************************
// *****************************************************************************
// Section: File includes
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include "prime_api_defs.h"
#include "stack/prime/mac/mac_defs.h"
#include "stack/prime/conv/sscs/iec_4_32/cl_432_defs.h"
#include "stack/prime/mngp/bmng_defs.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    extern "C" {

#endif
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Data Types
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* PRIME API definition

  Summary:
    Definition of the PRIME API.

  Description:
    This structure defines the functions that are included in the PRIME API 
    pointer.

  Remarks:
    None.
*/
typedef struct
{
    uint16_t                     vendor;
    uint16_t                     model;
    char                         version[20];
    PRIME_API_INITIALIZE         Initialize;
    PRIME_API_TASKS              Tasks;
    PRIME_API_STATUS             Status;
    MAC_SET_CALLBACKS            MacSetCallbacks;
    MAC_ESTABLISH_REQUEST        MacEstablishRequest; 
    MAC_ESTABLISH_RESPONSE       MacEstablishResponse;
    MAC_RELEASE_REQUEST          MacReleaseRequest;
    MAC_RELEASE_RESPONSE         MacReleaseResponse;
    MAC_REDIRECT_RESPONSE        MacRedirectResponse;
    MAC_JOIN_REQUEST             MacJoinRequest;
    MAC_JOIN_RESPONSE            MacJoinResponse;
    MAC_LEAVE_REQUEST            MacLeaveRequest;
    MAC_DATA_REQUEST             MacDataRequest;
    PLME_RESET_REQUEST           PlmeResetRequest;
    PLME_SLEEP_REQUEST           PlmeSleepRequest;
    PLME_RESUME_REQUEST          PlmeResumeRequest;
    PLME_TESTMODE_REQUEST        PlmeTestModeRequest;
    PLME_GET_REQUEST             PlmeGetRequest;
    PLME_SET_REQUEST             PlmeSetRequest;
    MLME_PROMOTE_REQUEST         MlmePromoteRequest;
    MLME_MP_PROMOTE_REQUEST      MlmeMpPromoteRequest;
    MLME_RESET_REQUEST           MlmeResetRequest;
    MLME_GET_REQUEST             MlmeGetRequest;
    MLME_LIST_GET_REQUEST        MlmeListGetRequest;
    MLME_SET_REQUEST             MlmeSetRequest;
    CL_432_SET_CALLBACKS         Cl432SetCallbacks;
    CL_432_RELEASE_REQUEST       Cl432ReleaseRequest;
    CL_432_DL_DATA_REQUEST       Cl432DlDataRequest;
    BMNG_SET_CALLBACKS                     BmngSetCallbacks;
    BMNG_FUP_CLEAR_TARGET_LIST_REQUEST     BmngFupClearTargetListRequest;
    BMNG_FUP_ADD_TARGET_REQUEST            BmngFupAddTargetRequest;
    BMNG_FUP_SET_FW_DATA_REQUEST           BmngFupSetFwDataRequest;
    BMNG_FUP_SET_UPGRADE_OPTIONS_REQUEST   BmngFupSetUpgradeOptionsRequest;
    BMNG_FUP_INIT_FILE_TX_REQUEST          BmngFupInitFileTxRequest;
    BMNG_FUP_DATA_FRAME_REQUEST            BmngFupDataFrameRequest;
    BMNG_FUP_CHECK_CRC_REQUEST             BmngFupCheckCrcRequest;
    BMNG_FUP_ABORT_FU_REQUEST              BmngFupAbortFuRequest;
    BMNG_FUP_START_FU_REQUEST              BmngFupStartFuRequest;
    BMNG_FUP_SET_MATCH_RULE_REQUEST        BmngFupSetMatchRuleRequest;
    BMNG_FUP_GET_VERSION_REQUEST           BmngFupGetVersionRequest;
    BMNG_FUP_GET_STATE_REQUEST             BmngFupGetStateRequest;
    BMNG_FUP_SET_SIGNATURE_DATA_REQUEST    BmngFupSetSignatureDataRequest;
    BMNG_PPROF_GET_REQUEST                 BmngPprofGetRequest;
    BMNG_PPROF_SET_REQUEST                 BmngPprofSetRequest;
    BMNG_PPROF_RESET_REQUEST               BmngPprofResetRequest;
    BMNG_PPROF_REBOOT_REQUEST              BmngPprofRebootRequest;
    BMNG_PPROF_GET_ENHANCED_REQUEST        BmngPprofGetEnhancedRequest;
    BMNG_PPROF_GET_ZC_DIFF_REQUEST         BmngPprofGetZcDiffRequest;
    BMNG_WHITELIST_ADD_REQUEST             BmngWhitelistAddRequest;
    BMNG_WHITELIST_REMOVE_REQUEST          BmngWhitelistRemoveRequest;
} PRIME_API;

//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif /* PRIME_API_TYPES_H_INCLUDE */

/*******************************************************************************
 End of File
*/
