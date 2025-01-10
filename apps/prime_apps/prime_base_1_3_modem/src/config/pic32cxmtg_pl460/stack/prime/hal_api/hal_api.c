/*******************************************************************************
  PRIME Hardware Abstraction Layer API Source

  Company:
    Microchip Technology Inc.

  File Name:
    hal_api.c

  Summary:
    PRIME Hardware Abstraction Layer API Source File

  Description:
    This module contains configuration and utils for the interface between the
    services connected to the hardware and the PRIME stack.
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

#include <stdio.h>
#include "hal_api.h"

// *****************************************************************************
// *****************************************************************************
// Section: Data Types
// *****************************************************************************
// *****************************************************************************

/* HAL API functions

  Summary:
    HAL API functions.

  Description:
    This structure contains the list of available functions in the HAL API.

  Remarks:
    The functions in this structure correspond to the required services by the
    PRIME stack.
 */

const HAL_API primeHalAPI = {
    SRV_RESET_HANDLER_RestartSystem,

    SRV_PCRC_GetValue,
    SRV_PCRC_ConfigureSNA,

    SRV_STORAGE_GetConfigInfo,
    SRV_STORAGE_SetConfigInfo,

    SRV_USI_Open,
    SRV_USI_CallbackRegister,
    SRV_USI_Send_Message,

    SRV_LOG_REPORT_Message_With_Code,

    SRV_USER_PIB_GetRequest,
    SRV_USER_PIB_GetRequestCbRegister,
    SRV_USER_PIB_SetRequest,
    SRV_USER_PIB_SetRequestCbRegister,

    SRV_RANDOM_Get32bits,

    CIPHER_Wrapper_AesCmacDirect,
    CIPHER_Wrapper_AesCcmSetkey,
    CIPHER_Wrapper_AesCcmEncryptAndTag,
    CIPHER_Wrapper_AesCcmAuthDecrypt,
    AES_Wrapper_WrapKey,
    AES_Wrapper_UnwrapKey,

    SRV_QUEUE_Init,
    SRV_QUEUE_Append,
    SRV_QUEUE_Append_With_Priority,
    SRV_QUEUE_Insert_Before,
    SRV_QUEUE_Insert_After,
    SRV_QUEUE_Read_Or_Remove,
    SRV_QUEUE_Read_Element,
    SRV_QUEUE_Remove_Element,
    SRV_QUEUE_Flush,
    SRV_QUEUE_Set_Capacity,

    SRV_FU_Start,
    SRV_FU_End,
    SRV_FU_CfgRead,
    SRV_FU_CfgWrite,
    SRV_FU_RegisterCallbackMemTransfer,
    SRV_FU_DataRead,
    SRV_FU_DataWrite,
    SRV_FU_RegisterCallbackCrc,
    SRV_FU_CalculateCrc,
    SRV_FU_RegisterCallbackVerify,
    SRV_FU_VerifyImage,
    SRV_FU_GetBitmap,
    SRV_FU_RequestSwapVersion,

    PAL_Initialize,
    PAL_Tasks,
    PAL_Status,
    PAL_CallbackRegister,
    PAL_DataRequest,
    PAL_GetSNR,
    PAL_GetZCT,
    PAL_GetTimer,
    PAL_GetTimerExtended,
    PAL_GetCD,
    PAL_GetNL,
    PAL_GetAGC,
    PAL_SetAGC,
    PAL_GetCCA,
    PAL_GetChannel,
    PAL_SetChannel,
    PAL_ProgramChannelSwitch,
    PAL_GetConfiguration,
    PAL_SetConfiguration,
    PAL_GetSignalCapture,
    PAL_GetMsgDuration,
    PAL_CheckMinimumQuality,
    PAL_GetLessRobustModulation,

    /* New functions must be added at the end */

};
