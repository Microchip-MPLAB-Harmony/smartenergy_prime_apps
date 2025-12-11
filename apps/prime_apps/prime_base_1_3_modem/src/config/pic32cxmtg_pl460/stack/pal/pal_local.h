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
#include "service/usi/srv_usi.h"

// *****************************************************************************
// *****************************************************************************
// Section: Data Types
// *****************************************************************************
// *****************************************************************************
#define PRIME_PAL_USI_INSTANCE        SRV_USI_INDEX_0
#define PAL_SNIFFER_DATA_MAX_SIZE     (512 + 32) /* Sniffer header is 32 bytes */
/* PHY Abstraction Layer PAL USI Sniffer Callback

  Summary:
    Callback designed to inform the USI Sniffer

  Description:
    This callback is used to inform the USI Sniffer

  Remarks:
    None.
*/
typedef void (*PAL_USI_SNIFFER_CB)(uint8_t *pData, uint16_t length);


typedef struct PAL_INTERFACE_TYPE
{
    uint8_t   (*MPAL_GetSNR)(uint8_t *snr, uint8_t qt);
    uint8_t   (*MPAL_GetZCT)(uint32_t *zct);
    uint8_t   (*MPAL_GetTimer)(uint32_t *timer);
    uint8_t   (*MPAL_GetTimerExtended)(uint64_t *timer);
    uint8_t   (*MPAL_GetCD)(uint8_t *cd, uint8_t *rssi, uint32_t *time, uint8_t *header);
    uint8_t   (*MPAL_GetNL)(uint8_t *noise);
    uint8_t   (*MPAL_GetCCA)(uint8_t *pState);
    uint8_t   (*MPAL_GetAGC)(uint8_t *mode, uint8_t *gain);
    uint8_t   (*MPAL_SetAGC)(uint8_t mode, uint8_t gain);
    uint8_t   (*MPAL_GetChannel)(uint16_t *pPch);
    uint8_t   (*MPAL_SetChannel)(uint16_t pch);
    uint8_t   (*MPAL_DataRequest)(PAL_MSG_REQUEST_DATA *pData);
    void      (*MPAL_ProgramChannelSwitch)(uint32_t timeSync, uint16_t pch, uint8_t timeMode);
    uint8_t   (*MPAL_GetConfiguration)(uint16_t id, void *val, uint16_t len);
    uint8_t   (*MPAL_SetConfiguration)(uint16_t id, void *val, uint16_t len);
    uint16_t  (*MPAL_GetSignalCapture)(uint8_t *noiseCapture, PAL_FRAME frameType, uint32_t timeStart, uint32_t duration);
    uint8_t   (*MPAL_GetMsgDuration)(uint16_t msgLen, PAL_SCHEME scheme, PAL_FRAME frameType, uint32_t *duration);
    bool      (*MPAL_CheckMinimumQuality)(PAL_SCHEME reference, PAL_SCHEME modulation);
    uint8_t   (*MPAL_GetLessRobustModulation)(PAL_SCHEME mod1, PAL_SCHEME mod2);

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

    SRV_USI_HANDLE usiHandler;

    uint8_t snifferEnabled;
} PAL_DATA;


// ****************************************************************************
// ****************************************************************************
// Section: Interface Routines
// ****************************************************************************
// ****************************************************************************

uint8_t MPAL_PLC_GetSNR(uint8_t *snr, uint8_t qt);
uint8_t MPAL_PLC_GetZCT(uint32_t *zct);
uint8_t MPAL_PLC_GetTimer(uint32_t *timer);
uint8_t MPAL_PLC_GetTimerExtended(uint64_t *timer);
uint8_t MPAL_PLC_GetCD(uint8_t *cd, uint8_t *rssi, uint32_t *time, uint8_t *header);
uint8_t MPAL_PLC_GetNL(uint8_t *noise);
uint8_t MPAL_PLC_GetCCA(uint8_t *pState);
uint8_t MPAL_PLC_GetAGC(uint8_t *mode, uint8_t *gain);
uint8_t MPAL_PLC_SetAGC(uint8_t mode, uint8_t gain);
uint8_t MPAL_PLC_GetChannel(uint16_t *pPch);
uint8_t MPAL_PLC_SetChannel(uint16_t pch);
uint8_t MPAL_PLC_DataRequest(PAL_MSG_REQUEST_DATA *pData);
void MPAL_PLC_ProgramChannelSwitch(uint32_t timeSync, uint16_t pch, uint8_t timeMode);
uint8_t MPAL_PLC_GetConfiguration(uint16_t id, void *val, uint16_t len);
uint8_t MPAL_PLC_SetConfiguration(uint16_t id, void *val, uint16_t len);
uint16_t MPAL_PLC_GetSignalCapture(uint8_t *noiseCapture, PAL_FRAME frameType, uint32_t timeStart, uint32_t duration);
uint8_t MPAL_PLC_GetMsgDuration(uint16_t msgLen, PAL_SCHEME scheme, PAL_FRAME frameType, uint32_t *duration);
bool MPAL_PLC_RM_CheckMinimumQuality(PAL_SCHEME reference, PAL_SCHEME modulation);
uint8_t MPAL_PLC_RM_GetLessRobustModulation(PAL_SCHEME mod1, PAL_SCHEME mod2);

#endif // #ifndef PAL_LOCAL_H
/*******************************************************************************
 End of File
*/