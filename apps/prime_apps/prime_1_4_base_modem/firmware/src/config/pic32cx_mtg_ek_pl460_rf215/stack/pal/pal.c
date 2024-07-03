/*******************************************************************************
  Company:
    Microchip Technology Inc.

  File Name:
    pal.c

  Summary:
    Platform Abstraction Layer (PAL) Interface source file.

  Description:
    This module provides the interface between the PRIME MAC layer and the
    PLC physical layer.
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
// Section: File includes
// *****************************************************************************
// *****************************************************************************

#include <string.h>
#include "configuration.h"
#include "pal.h"
#include "pal_types.h"
#include "pal_local.h"
#include "pal_plc.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global data
// *****************************************************************************
// *****************************************************************************

static PAL_DATA palData;

extern const PAL_INTERFACE PAL_PLC_Interface;

// *****************************************************************************
// *****************************************************************************
// Section: File scope functions
// *****************************************************************************
// *****************************************************************************
static void lPAL_PlcDataConfirmCallback(PAL_MSG_CONFIRM_DATA *pData)
{
    if (palData.dataConfirmCallback) {
        palData.dataConfirmCallback(pData);
    }
}

static void lPAL_PlcDataIndicationCallback(PAL_MSG_INDICATION_DATA *pData)
{
    if (palData.dataIndicationCallback) {
        palData.dataIndicationCallback(pData);
    }
}

static PAL_INTERFACE * lPAL_GetInterface(uint16_t pch)
{
    (void)pch;
    return (PAL_INTERFACE *)&PAL_PLC_Interface;
}

// *****************************************************************************
// *****************************************************************************
// Section: PAL Interface Implementation
// *****************************************************************************
// *****************************************************************************

SYS_MODULE_OBJ PAL_Initialize(const SYS_MODULE_INDEX index)
{
    if (index != PRIME_PAL_INDEX)
    {
        return SYS_MODULE_OBJ_INVALID;
    }

    palData.snifferEnabled = 0;

    if (PAL_PLC_Initialize() == SYS_MODULE_OBJ_INVALID)
    {
        return SYS_MODULE_OBJ_INVALID;
    }

    PAL_PLC_DataConfirmCallbackRegister(lPAL_PlcDataConfirmCallback);
    PAL_PLC_DataIndicationCallbackRegister(lPAL_PlcDataIndicationCallback);


    return (SYS_MODULE_OBJ)PRIME_PAL_INDEX;
}

void PAL_Tasks(SYS_MODULE_OBJ object)
{
    if (object != PRIME_PAL_INDEX)
    {
        return;
    }

    PAL_PLC_Tasks();

}

SYS_STATUS PAL_Status(SYS_MODULE_OBJ object)
{
    if (object != PRIME_PAL_INDEX)
    {
        return SYS_STATUS_ERROR;
    }

    if (PAL_PLC_Status() != SYS_STATUS_READY)
    {
        return SYS_STATUS_BUSY;
    }

    return SYS_STATUS_READY;
}

void PAL_CallbackRegister(PAL_CALLBACKS *pCallbacks)
{
    palData.dataConfirmCallback = pCallbacks->dataConfirm;
    palData.dataIndicationCallback = pCallbacks->dataIndication;
}

uint8_t PAL_DataRequest(PAL_MSG_REQUEST_DATA *pData)
{
    PAL_INTERFACE *palIface = lPAL_GetInterface(pData->pch);
    return palIface->PAL_DataRequest(pData);
}

uint8_t PAL_GetSNR(uint16_t pch, uint8_t *snr, uint8_t qt)
{
    PAL_INTERFACE *palIface = lPAL_GetInterface(pch);
    return palIface->PAL_GetSNR(snr, qt);
}

uint8_t PAL_GetZCT(uint16_t pch, uint32_t *zct)
{
    PAL_INTERFACE *palIface = lPAL_GetInterface(pch);
    return palIface->PAL_GetZCT(zct);
}

uint8_t PAL_GetTimer(uint16_t pch, uint32_t *timer)
{
    PAL_INTERFACE *palIface = lPAL_GetInterface(pch);
    return palIface->PAL_GetTimer(timer);
}

uint8_t PAL_GetTimerExtended(uint16_t pch, uint64_t *timer)
{
    PAL_INTERFACE *palIface = lPAL_GetInterface(pch);
    return palIface->PAL_GetTimerExtended(timer);
}

uint8_t PAL_GetCD(uint16_t pch, uint8_t *cd, uint8_t *rssi, uint32_t *time, uint8_t *header)
{
    PAL_INTERFACE *palIface = lPAL_GetInterface(pch);
    return palIface->PAL_GetCD(cd, rssi, time, header);
}

uint8_t PAL_GetNL(uint16_t pch, uint8_t *noise)
{
    PAL_INTERFACE *palIface = lPAL_GetInterface(pch);
    return palIface->PAL_GetNL(noise);
}

uint8_t PAL_GetAGC(uint16_t pch, uint8_t *frameType, uint8_t *gain)
{
    PAL_INTERFACE *palIface = lPAL_GetInterface(pch);
    return palIface->PAL_GetAGC(frameType, gain);
}

uint8_t PAL_SetAGC(uint16_t pch, PAL_FRAME frameType, uint8_t gain)
{
    PAL_INTERFACE *palIface = lPAL_GetInterface(pch);
    return palIface->PAL_SetAGC(frameType, gain);
}

uint8_t PAL_GetCCA(uint16_t pch, uint8_t *pState)
{
    PAL_INTERFACE *palIface = lPAL_GetInterface(pch);
    return palIface->PAL_GetCCA(pState);
}

uint8_t PAL_GetChannel(uint16_t *pPch, uint16_t channelReference)
{
    PAL_INTERFACE *palIface = lPAL_GetInterface(channelReference);
    return palIface->PAL_GetChannel(pPch);
}

uint8_t PAL_SetChannel(uint16_t pch)
{
    PAL_INTERFACE *palIface = lPAL_GetInterface(pch);
    return palIface->PAL_SetChannel(pch);
}

void PAL_ProgramChannelSwitch(uint16_t pch, uint32_t timeSync, uint8_t timeMode)
{
    PAL_INTERFACE *palIface = lPAL_GetInterface(pch);
    palIface->PAL_ProgramChannelSwitch(timeSync, pch, timeMode);
}

uint8_t PAL_GetConfiguration(uint16_t pch, uint16_t id, void *val, uint16_t length)
{
    PAL_INTERFACE *palIface = lPAL_GetInterface(pch);

    if (id == PAL_ID_PHY_SNIFFER_EN)
    {
        *(uint8_t *)val = 0;
        return(PAL_CFG_SUCCESS);        
    }

    return palIface->PAL_GetConfiguration(id, val, length);
}

uint8_t PAL_SetConfiguration(uint16_t pch, uint16_t id, void *val, uint16_t length)
{
    PAL_INTERFACE *palIface = lPAL_GetInterface(pch);

    if (id == PAL_ID_PHY_SNIFFER_EN)
    {
        palData.snifferEnabled = 0;
        return(PAL_CFG_SUCCESS);        
    }

    return palIface->PAL_SetConfiguration(id, val, length);
}

uint16_t PAL_GetSignalCapture(uint16_t pch, uint8_t *noiseCapture, PAL_FRAME frameType, 
                              uint32_t timeStart, uint32_t duration)
{
    PAL_INTERFACE *palIface = lPAL_GetInterface(pch);
    return palIface->PAL_GetSignalCapture(noiseCapture, frameType, timeStart, duration);
}

uint8_t PAL_GetMsgDuration(uint16_t pch, uint16_t length, PAL_SCHEME scheme, PAL_FRAME frameType, uint32_t *duration)
{
    PAL_INTERFACE *palIface = lPAL_GetInterface(pch);
    return palIface->PAL_GetMsgDuration(length, scheme, frameType, duration);
}

bool PAL_CheckMinimumQuality(uint16_t pch, uint8_t reference, uint8_t modulation)
{
    PAL_INTERFACE *palIface = lPAL_GetInterface(pch);
    return palIface->PAL_CheckMinimumQuality(reference, modulation);
}

uint8_t PAL_GetLessRobustModulation(uint16_t pch, uint8_t mod1, uint8_t mod2)
{
    PAL_INTERFACE *palIface = lPAL_GetInterface(pch);
    return palIface->PAL_GetLessRobustModulation(mod1, mod2);
}
