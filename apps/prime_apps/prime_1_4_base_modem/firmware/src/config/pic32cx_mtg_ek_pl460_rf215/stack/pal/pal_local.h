/*******************************************************************************
  Company:
    Microchip Technology Inc.

  File Name:
    pal_local.h

  Summary:
    Platform Abstraction Layer (PAL) Interface Local header file.

  Description:
    Platform Abstraction Layer (PAL) Interface Local header file.
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

#ifndef PAL_LOCAL_H
#define PAL_LOCAL_H

// *****************************************************************************
// *****************************************************************************
// Section: File includes
// *****************************************************************************
// *****************************************************************************

#include <stdbool.h>
#include <stdint.h>
#include "pal.h"
#include "pal_types.h"

// *****************************************************************************
// *****************************************************************************
// Section: Data Types
// *****************************************************************************
// *****************************************************************************
typedef struct PAL_INTERFACE_TYPE
{
    uint8_t   (*PAL_GetSNR)(uint8_t *snr, uint8_t qt);
    uint8_t   (*PAL_GetZCT)(uint32_t *zct);
    uint8_t   (*PAL_GetTimer)(uint32_t *timer);
    uint8_t   (*PAL_GetTimerExtended)(uint64_t *timer);
    uint8_t   (*PAL_GetCD)(uint8_t *cd, uint8_t *rssi, uint32_t *time, uint8_t *header);
    uint8_t   (*PAL_GetNL)(uint8_t *noise);
    uint8_t   (*PAL_GetCCA)(uint8_t *pState);
    uint8_t   (*PAL_GetAGC)(uint8_t *mode, uint8_t *gain);
    uint8_t   (*PAL_SetAGC)(uint8_t mode, uint8_t gain);
    uint8_t   (*PAL_GetChannel)(uint16_t *pPch);
    uint8_t   (*PAL_SetChannel)(uint16_t pch);
    uint8_t   (*PAL_DataRequest)(PAL_MSG_REQUEST_DATA *pData);
    void      (*PAL_ProgramChannelSwitch)(uint32_t timeSync, uint16_t pch, uint8_t timeMode);
    uint8_t   (*PAL_GetConfiguration)(uint16_t id, void *val, uint16_t len);
    uint8_t   (*PAL_SetConfiguration)(uint16_t id, void *val, uint16_t len);
    uint16_t  (*PAL_GetSignalCapture)(uint8_t *noiseCapture, PAL_FRAME frameType, uint32_t timeStart, uint32_t duration);
    uint8_t   (*PAL_GetMsgDuration)(uint16_t msgLen, PAL_SCHEME scheme, PAL_FRAME frameType, uint32_t *duration);
    bool      (*PAL_CheckMinimumQuality)(PAL_SCHEME reference, PAL_SCHEME modulation);
    uint8_t   (*PAL_GetLessRobustModulation)(PAL_SCHEME mod1, PAL_SCHEME mod2);

} PAL_INTERFACE;        // PRIME PAL interface descriptor

// *****************************************************************************
/* PAL Data

  Summary:
    Holds PAL internal data.

  Description:
    This data type defines all data required to handle the PAL module.

  Remarks:
    None.
*/
typedef struct
{
    PAL_DATA_CONFIRM_CB dataConfirmCallback;

    PAL_DATA_INDICATION_CB dataIndicationCallback;

    PAL_SWITCH_RF_CH_CB channelSwitchCallback;

    uint8_t snifferEnabled;
} PAL_DATA;

#endif // #ifndef PAL_LOCAL_H
/*******************************************************************************
 End of File
*/