/*******************************************************************************
  Company:
    Microchip Technology Inc.

  File Name:
    pal_plc.c

  Summary:
    PLC Platform Abstraction Layer (PAL) Interface source file.

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

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "configuration.h"
#include "driver/driver.h"
#include "driver/plc/phy/drv_plc_phy_comm.h"
#include "service/pcoup/srv_pcoup.h"
#include "service/time_management/srv_time_management.h"
#include "pal_types.h"
#include "pal_local.h"
#include "pal_plc.h"
#include "pal_plc_local.h"
#include "pal_plc_rm.h"
#include "service/psniffer/srv_psniffer.h"
#include "service/log_report/srv_log_report.h"      
#include "peripheral/trng/plib_trng.h"

// *****************************************************************************
// *****************************************************************************
// Section: Macro Definitions
// *****************************************************************************
// *****************************************************************************
/* Delay in us between read of Host timer and PL360 timer (CS falling edge) */
/* The delay depends on CPU frequency, compiler and optimizations */
/* It doesn't affect to relative time between RX and TX (compensated) */
#define PAL_PLC_TIMER_SYNC_OFFSET    6U

/* Maximum and minimum relative frequency between PL360 and host timers (F_host/F_360 [uQ1.24]) */
/* It is used to detect wrong timer reads */
#define SYNC_TIMER_REL_FREQ_MAX  0x01000D1B /* +200 PPM */
#define SYNC_TIMER_REL_FREQ_MIN  0x00FFF2E5 /* -200 PPM */

/* Time in us of chirp */
#define PHY_CHIRP_TIME                 (2048U)
#define PHY_CHIRP_MODE_B_TIME          (2048U * 4U)
#define PHY_CHIRP_MODE_BC_TIME         ((2048U * 5U) + (2240U * 2U))

/* Time in us of phy symbol */
#define PHY_SYMBOL_TIME                (2240U)

/* Time in us of phy header */
#define PHY_HEADER_TIME                (4480U)
#define PHY_HEADER_B_BC_TIME           (2240U * 4U)

#define DIV_ROUND(a, b)                (((a) + (b >> 1)) / (b))
#define MAX(a, b)                      (((a) > (b)) ?  (a) : (b))
#define MIN(a, b)                      (((a) < (b)) ?  (a) : (b))

/******************************************************************************
 * PRIME PAL PLC interface implementation
 ******************************************************************************/
const PAL_INTERFACE PAL_PLC_Interface =
{
    .PAL_GetSNR = PAL_PLC_GetSNR,
    .PAL_GetZCT = PAL_PLC_GetZCT,
    .PAL_GetTimer = PAL_PLC_GetTimer,
    .PAL_GetTimerExtended = PAL_PLC_GetTimerExtended,
    .PAL_GetCD = PAL_PLC_GetCD,
    .PAL_GetNL = PAL_PLC_GetNL,
    .PAL_GetAGC = PAL_PLC_GetAGC,
    .PAL_SetAGC = PAL_PLC_SetAGC,
    .PAL_GetCCA = PAL_PLC_GetCCA,
    .PAL_GetChannel = PAL_PLC_GetChannel,
    .PAL_SetChannel = PAL_PLC_SetChannel,
    .PAL_DataRequest = PAL_PLC_DataRequest,
    .PAL_ProgramChannelSwitch = PAL_PLC_ProgramChannelSwitch,
    .PAL_GetConfiguration = PAL_PLC_GetConfiguration,
    .PAL_SetConfiguration = PAL_PLC_SetConfiguration,
    .PAL_GetSignalCapture = PAL_PLC_GetSignalCapture,
    .PAL_GetMsgDuration = PAL_PLC_GetMsgDuration,
    .PAL_CheckMinimumQuality = PAL_PLC_RM_CheckMinimumQuality,
    .PAL_GetLessRobustModulation = PAL_PLC_RM_GetLessRobustModulation,
};

// *****************************************************************************
// *****************************************************************************
// Section: File Scope Data
// *****************************************************************************
// *****************************************************************************
extern DRV_PLC_PHY_INIT drvPlcPhyInitData;

static PAL_PLC_DATA palPlcData = {0};

static const uint8_t palPlcSymbolSize[14] = {
    12, 24, 36, 0, 6, 12, 18, 0, 0, 0, 0, 0, 6, 12
};

static const uint32_t palPlcTimeChirpHeader[4] = { 
    PHY_CHIRP_TIME + PHY_HEADER_TIME, 
    0, 
    PHY_CHIRP_MODE_B_TIME + PHY_HEADER_B_BC_TIME,
    PHY_CHIRP_MODE_BC_TIME + PHY_HEADER_B_BC_TIME
};

// *****************************************************************************
// *****************************************************************************
// Section: File Scope Functions
// *****************************************************************************
// *****************************************************************************
static void lPAL_PLC_SysTimeCB( uintptr_t context )
{
    palPlcData.syncUpdate = true;
}

static uint16_t lPAL_PLC_GetPCH(uint8_t channel)
{
    uint16_t pch;

    if (channel <= 8) 
    {
        pch = (uint16_t)(1 << (channel - 1)); /* Single channel */
    } 
    else  
    {
        pch = (uint16_t)(3 << (channel - 9)); /* Double channel */
    }

    return pch;
}

static uint8_t lPAL_PLC_GetChannelNumber(uint16_t pch)
{
    uint8_t channel = 1;

    if ((pch % 3) == 0)
    {
        // double channel
        pch /= 3;
        channel += 8;
    }

    while ((pch & 0x0001) == 0) 
    {
        pch >>= 1;
        channel++;
    }

    return channel;
}

static void lPAL_PLC_SetTxContinuousMode(uint8_t txMode)
{
    uint8_t dummyValue = 0;
    
    palPlcData.phyTxObj.pTransmitData = &dummyValue;
    palPlcData.phyTxObj.timeIni = 0;
    palPlcData.phyTxObj.dataLength = 0;
    palPlcData.phyTxObj.mode = 0;
    palPlcData.phyTxObj.attenuation = 0xFF;
    palPlcData.phyTxObj.csma.disableRx = 1;
    palPlcData.phyTxObj.csma.senseCount = 0;
    palPlcData.phyTxObj.csma.senseDelayMs = 0;
    palPlcData.phyTxObj.bufferId = 0;
    palPlcData.phyTxObj.scheme = SCHEME_DBPSK;
    palPlcData.phyTxObj.frameType = FRAME_TYPE_B;

    /* Check mode */
    if (txMode == 0) 
    {
        /* Disable continuous tx mode */
        palPlcData.phyTxObj.mode = TX_MODE_CANCEL;
    } 
    else 
    {
        /* Enable continuous tx mode */
        palPlcData.phyTxObj.mode = TX_MODE_PREAMBLE_CONTINUOUS | TX_MODE_RELATIVE;
    }

    DRV_PLC_PHY_TxRequest(palPlcData.drvPhyHandle, &palPlcData.phyTxObj);
}

static uint32_t lPAL_PLC_TimerSyncRead(uint32_t *pTimePlc)
{
    uint32_t timeHost;
    uint32_t basePrioPrev = __get_BASEPRI();

    /* Enter critical region. Disable all interrupts except highest priority
     * (<1: 0) to ensure constant delay between timers read */
    __set_BASEPRI(1 << (8 - __NVIC_PRIO_BITS));

    /* Read Host timer */
    timeHost = SRV_TIME_MANAGEMENT_GetTimeUS();

    /* Read PLC timer */
    palPlcData.plcPIB.id = PLC_ID_TIME_REF_ID;
    palPlcData.plcPIB.length = 4;
    palPlcData.plcPIB.pData = (uint8_t *)pTimePlc;
    DRV_PLC_PHY_PIBGet(palPlcData.drvPhyHandle, &palPlcData.plcPIB);

    /* Adjust delay between Host and PLC timers */
    timeHost += PAL_PLC_TIMER_SYNC_OFFSET;

    /* Leave critical region */
    __set_BASEPRI(basePrioPrev);

    return timeHost;
}

__STATIC_INLINE void lPAL_PLC_TimerSyncInitialize(void)
{
    if (!palPlcData.syncEnable) 
    {
        palPlcData.syncUpdate = false;
    } 
    else 
    {
        /* Get initial timer references */
        palPlcData.timeRefHost = lPAL_PLC_TimerSyncRead(&palPlcData.timeRefPlc);

        /* Initialize relative frequency F_host/F_plc to 1 [uQ1.24] */
        palPlcData.syncTimerRelFreq = 1UL << 24;

        /* Program first interrupt after 50 ms (5 us deviation with 100 PPM) */
        palPlcData.syncDelay = 50000;
        SYS_TIME_TimerDestroy(palPlcData.syncHandle);
        palPlcData.syncHandle = SRV_TIME_MANAGEMENT_CallbackRegisterUS(
                lPAL_PLC_SysTimeCB, 0, palPlcData.syncDelay, SYS_TIME_SINGLE);
        if (palPlcData.syncHandle != SYS_TIME_HANDLE_INVALID)
        {
            palPlcData.syncUpdate = false;
        }
        else
        {
            palPlcData.syncDelay = 0;
            palPlcData.syncUpdate = true;
        }
    }
}

__STATIC_INLINE void lPAL_PLC_TimerSyncUpdate(void)
{
    uint32_t timeHost;
    uint32_t timePlc;
    uint32_t delayHost;
    uint32_t delayPlc;
    uint32_t syncTimerRelFreq;

    /* Get current Host and PLC timers */
    timeHost = lPAL_PLC_TimerSyncRead(&timePlc);

    /* Compute delays from references (last read) */
    delayHost = timeHost - palPlcData.timeRefHost;
    delayPlc = timePlc - palPlcData.timeRefPlc;

    /* Compute relative frequency F_host/F_plc [uQ1.24] */
    syncTimerRelFreq = (uint32_t)DIV_ROUND((uint64_t)delayHost << 24, delayPlc);

    /* Check if relative frequency is consistent, otherwise timer read is wrong */
    if ((syncTimerRelFreq >= SYNC_TIMER_REL_FREQ_MIN) && (syncTimerRelFreq <= SYNC_TIMER_REL_FREQ_MAX)) 
    {
        /* Update relative frequency and references */
        palPlcData.syncTimerRelFreq = syncTimerRelFreq;
        palPlcData.timeRefHost = timeHost;
        palPlcData.timeRefPlc = timePlc;

        switch (palPlcData.syncDelay) 
        {
            case 0:
                /* Next sync after 50 ms (5 us deviation with 100 PPM) */
                palPlcData.syncDelay = 50000;
                break;

            case 50000:
                /* Next sync after 250 ms (5 us deviation with 20 PPM) */
                palPlcData.syncDelay = 250000;
                break;

            case 250000:
                /* Next sync after 1 second (5 us deviation with 5 PPM) */
                palPlcData.syncDelay = 1000000;
                break;

            default:
                /* Next sync after 5 seconds (5 us deviation with 1 PPM) */
                palPlcData.syncDelay = 5000000;
                break;
        }

        /* Program next interrupt */
        palPlcData.syncHandle = SRV_TIME_MANAGEMENT_CallbackRegisterUS(
                lPAL_PLC_SysTimeCB, 0, palPlcData.syncDelay, SYS_TIME_SINGLE);
        if (palPlcData.syncHandle != SYS_TIME_HANDLE_INVALID)
        {
            palPlcData.syncUpdate = false;
        }
        else
        {
            palPlcData.syncDelay = 0;
        }
    } 
    else 
    {
        SRV_LOG_REPORT_Message_With_Code(SRV_LOG_REPORT_ERROR, 
                (SRV_LOG_REPORT_CODE)PAL_PLC_TIMER_SYNC_ERROR,
                "PRIME_PAL_PLC: PLC timer syncronization error\r\n");
        lPAL_PLC_TimerSyncInitialize();
    }
}

static uint32_t lPAL_PLC_GetHostTime(uint32_t timePlc)
{
    int64_t delayAux;
    int32_t delayPlc;
    int32_t delayHost;
    uint32_t timeHost;

    /* Compute PLC delay time since last sinchronization */
    delayPlc = (int32_t)(timePlc - palPlcData.timeRefPlc);

    /* Convert PLC delay to Host delay (frequency deviation) */
    delayAux = (int64_t)delayPlc * palPlcData.syncTimerRelFreq;
    delayHost = (int32_t)((delayAux + (1UL << 23)) >> 24);

    /* Compute Host time */
    timeHost = palPlcData.timeRefHost + delayHost;

    return timeHost;
}

static uint32_t lPAL_PLC_GetPlcTime(uint32_t timeHost)
{
    int64_t delayAux;
    int32_t delayHost;
    int32_t delayPlc;
    uint32_t timePlc;

    /* Compute Host delay time since last synchronization */
    delayHost = (int32_t)(timeHost - palPlcData.timeRefHost);

    /* Convert Host delay to PLC delay (frequency deviation) */
    delayAux = (int64_t)delayHost << 24;
    delayPlc = (int32_t)DIV_ROUND(delayAux, palPlcData.syncTimerRelFreq);

    /* Compute PLC time */
    timePlc = palPlcData.timeRefPlc + delayPlc;

    return timePlc;
}

static void lPAL_PLC_SetTxRxChannel(DRV_PLC_PHY_CHANNEL channel)
{
    /* Set channel configuration */
    palPlcData.plcPIB.id = PLC_ID_CHANNEL_CFG;
    palPlcData.plcPIB.length = 1;
    palPlcData.plcPIB.pData = &channel;
    DRV_PLC_PHY_PIBSet(palPlcData.drvPhyHandle, &palPlcData.plcPIB);

    /* Set coupling configuration */
    SRV_PCOUP_SetChannelConfig(palPlcData.drvPhyHandle, channel);

    /* Initialize synchronization of PL360-Host timers when channel updated */
    lPAL_PLC_TimerSyncInitialize();

    SRV_PSNIFFER_SetPLCChannel(channel);

}

// *****************************************************************************
// *****************************************************************************
// Section: Callback Functions
// *****************************************************************************
// *****************************************************************************
static void lPAL_PLC_PLC_DataCfmCb(DRV_PLC_PHY_TRANSMISSION_CFM_OBJ *pCfmObj, uintptr_t context)
{
    /* Avoid warning */
    (void)context;

    if (pCfmObj->result == DRV_PLC_PHY_TX_RESULT_TIMEOUT)
    {
        lPAL_PLC_TimerSyncInitialize();
    }

    if (palPlcData.status == PAL_PLC_STATUS_DETECT_IMPEDANCE)
    {
        palPlcData.detectImpedanceResult = pCfmObj->result;
        return;
    }

    if (palPlcData.plcCallbacks.dataConfirm != NULL)
    {
        PAL_MSG_CONFIRM_DATA dataCfm;

        dataCfm.txTime = lPAL_PLC_GetHostTime(pCfmObj->timeIni);
        pCfmObj->timeIni = dataCfm.txTime;  /* For sniffer */
        dataCfm.rmsCalc = (uint16_t)pCfmObj->rmsCalc;
        dataCfm.pch = lPAL_PLC_GetPCH(palPlcData.channel);
        dataCfm.frameType = (uint8_t)pCfmObj->frameType;
        dataCfm.bufId = pCfmObj->bufferId;

        switch (pCfmObj->result)
        {
            case DRV_PLC_PHY_TX_RESULT_BUSY_CH:
            case DRV_PLC_PHY_TX_RESULT_BUSY_RX:
                dataCfm.result = (uint8_t)PAL_TX_RESULT_BUSY_CH;
                break;
            default:
                dataCfm.result = (uint8_t)pCfmObj->result;
                break;
        }

        palPlcData.plcCallbacks.dataConfirm(&dataCfm);
    }

    if (palPlcData.snifferCallback)
    {
        size_t dataLength;
        uint16_t paySymbols;

        palPlcData.plcPIB.id = PLC_ID_TX_PAY_SYMBOLS;
        palPlcData.plcPIB.length = 2;
        palPlcData.plcPIB.pData = (uint8_t *)&paySymbols;
        DRV_PLC_PHY_PIBGet(palPlcData.drvPhyHandle, &palPlcData.plcPIB);
        
        SRV_PSNIFFER_SetTxPayloadSymbols(*(uint16_t *)palPlcData.plcPIB.pData);

        dataLength = SRV_PSNIFFER_SerialCfmMessage(palPlcData.snifferData, pCfmObj);

        palPlcData.snifferCallback(palPlcData.snifferData, dataLength);
    }

}

static void lPAL_PLC_PLC_DataIndCb(DRV_PLC_PHY_RECEPTION_OBJ *pIndObj, uintptr_t context)
{
    PAL_MSG_INDICATION_DATA dataInd;

    /* Avoid warning */
    (void)context;

    /* Store Rx parameters */
    palPlcData.rxParameters.evmHeaderAcum = pIndObj->evmHeaderAcum;
    palPlcData.rxParameters.evmPayloadAcum = pIndObj->evmPayloadAcum;
    palPlcData.rxParameters.rxTime = pIndObj->timeIni;
    palPlcData.rxParameters.evmHeader = pIndObj->evmHeader;
    palPlcData.rxParameters.evmPayload = pIndObj->evmPayload;
    palPlcData.rxParameters.dataLen = pIndObj->dataLength;
    palPlcData.rxParameters.scheme = (uint8_t)pIndObj->scheme;
    palPlcData.rxParameters.modType = (uint8_t)pIndObj->frameType;
    palPlcData.rxParameters.headerType = (uint8_t)pIndObj->headerType;
    palPlcData.rxParameters.rssiAvg = pIndObj->rssiAvg;
    palPlcData.rxParameters.cinrAvg = pIndObj->cinrAvg;
    palPlcData.rxParameters.cinrMin = pIndObj->cinrMin;
    palPlcData.rxParameters.berSoft = pIndObj->berSoftAvg;
    palPlcData.rxParameters.berSoftMax = pIndObj->berSoftMax;
    palPlcData.rxParameters.narBandPercent = pIndObj->narBandPercent;
    palPlcData.rxParameters.impPercent = pIndObj->impNoisePercent;

    /* Fill dataInd */
    dataInd.pData = pIndObj->pReceivedData;
    dataInd.rxTime = lPAL_PLC_GetHostTime(pIndObj->timeIni);
    pIndObj->timeIni = dataInd.rxTime;   /* For sniffer */
    dataInd.dataLength = pIndObj->dataLength;
    dataInd.pch = lPAL_PLC_GetPCH(palPlcData.channel);
    PAL_PLC_RM_GetRobustModulation(pIndObj, &dataInd.estimatedBitrate, &dataInd.lessRobustMod, dataInd.pch);
    dataInd.rssi = pIndObj->rssiAvg;
    dataInd.bufId = 0;
    dataInd.scheme = (uint8_t)pIndObj->scheme;
    dataInd.frameType = (uint8_t)pIndObj->frameType;
    dataInd.headerType = (uint8_t)pIndObj->headerType;
    dataInd.lqi = PAL_PLC_RM_GetLqi(pIndObj->cinrAvg);

    /* Store last values of some fields */
    palPlcData.lastRSSIAvg = pIndObj->rssiAvg;
    palPlcData.lastCINRMin = pIndObj->cinrMin;

    if (palPlcData.plcCallbacks.dataIndication != NULL)
    {
        palPlcData.plcCallbacks.dataIndication(&dataInd);
    }

    if (palPlcData.snifferCallback)
    {
        size_t length;
        uint16_t paySymbols;

        palPlcData.plcPIB.id = PLC_ID_RX_PAY_SYMBOLS;
        palPlcData.plcPIB.length = 2;
        palPlcData.plcPIB.pData = (uint8_t *)&paySymbols;
        DRV_PLC_PHY_PIBGet(palPlcData.drvPhyHandle, &palPlcData.plcPIB);

        SRV_PSNIFFER_SetRxPayloadSymbols(*(uint16_t *)palPlcData.plcPIB.pData);

        length = SRV_PSNIFFER_SerialRxMessage(palPlcData.snifferData, pIndObj);

        palPlcData.snifferCallback(palPlcData.snifferData, length);
    }

}

static void lPAL_PLC_PLC_ExceptionCb(DRV_PLC_PHY_EXCEPTION exception, uintptr_t context)
{
    uint8_t numErrors;

    /* Avoid warning */
    (void)context;

    if (exception == DRV_PLC_PHY_EXCEPTION_UNEXPECTED_KEY)
    {
        palPlcData.statsErrorUnexpectedKey++;
    }

    if (exception == DRV_PLC_PHY_EXCEPTION_RESET)
    {
        palPlcData.statsErrorReset++;
        palPlcData.exceptionPending = true;
    }

    if (exception == DRV_PLC_PHY_EXCEPTION_DEBUG)
    {
        palPlcData.statsErrorDebug++;
    }

    if (exception == DRV_PLC_PHY_EXCEPTION_CRITICAL_ERROR)
    {
        palPlcData.statsErrorCritical++;
    }

    palPlcData.status = PAL_PLC_STATUS_ERROR;

    /* Store error info */
    palPlcData.errorInfo |= (1 << exception);

    /* Increase number of errors, if possible */
    numErrors = palPlcData.errorInfo >> 4;
    if (numErrors < 0xF) {
        ++numErrors;
        palPlcData.errorInfo &= 0x0F;
        palPlcData.errorInfo += (numErrors << 4);
    }
}

// *****************************************************************************
// *****************************************************************************
// Section: PAL PLC Interface Implementation
// *****************************************************************************
// *****************************************************************************
SYS_MODULE_OBJ PAL_PLC_Initialize(void)
{
    /* Check previously initialized */
    if (palPlcData.status != PAL_PLC_STATUS_UNINITIALIZED)
    {
        return SYS_MODULE_OBJ_INVALID;
    }

    /* Clear exceptions statistics */
    palPlcData.statsErrorUnexpectedKey = 0;
    palPlcData.statsErrorReset = 0;
    palPlcData.statsErrorDebug = 0;
    palPlcData.statsErrorCritical = 0;

    /* Initialize data fields */
    palPlcData.waitingTxCfm = false;
    palPlcData.exceptionPending = false;
    palPlcData.hiTimerRef = 0;
    palPlcData.previousTimerRef = 0;
    palPlcData.errorInfo = 0;
    palPlcData.palAttenuation = 0;
    palPlcData.syncEnable = false;
    palPlcData.syncHandle = SYS_TIME_HANDLE_INVALID;

    /* Read Default Channel */
    palPlcData.channel = SRV_PCOUP_GetDefaultChannel();

    palPlcData.drvPhyStatus = DRV_PLC_PHY_Status(DRV_PLC_PHY_INDEX);
    if (palPlcData.drvPhyStatus == SYS_STATUS_UNINITIALIZED)
    {
        /* Initialize PLC Driver Instance */
        (void) DRV_PLC_PHY_Initialize(DRV_PLC_PHY_INDEX, (SYS_MODULE_INIT *)&drvPlcPhyInitData);
    }

    /* Open PLC driver */
    palPlcData.drvPhyHandle = DRV_PLC_PHY_Open(DRV_PLC_PHY_INDEX, NULL);

    if (palPlcData.drvPhyHandle != DRV_HANDLE_INVALID)
    {
        palPlcData.status = PAL_PLC_STATUS_BUSY;
        palPlcData.syncEnable = true;
        return (SYS_MODULE_OBJ)DRV_PLC_PHY_INDEX;
    }
    else
    {
        SRV_LOG_REPORT_Message_With_Code(SRV_LOG_REPORT_ERROR, 
                (SRV_LOG_REPORT_CODE)PHY_LAYER_PLC_NOT_AVAILABLE,
                "PRIME_PAL_PLC: PLC PHY layer not available\r\n");
        palPlcData.status = PAL_PLC_STATUS_ERROR;
        return SYS_MODULE_OBJ_INVALID;
    }
}

SYS_STATUS PAL_PLC_Status(void)
{
    /* Return the PAL PLC status */
    return ((SYS_STATUS)palPlcData.status);
}

void PAL_PLC_Tasks(void)
{
    switch (palPlcData.status)
    {
        case PAL_PLC_STATUS_UNINITIALIZED:
        {
            /* Do Nothing */
            break;
        }

        case PAL_PLC_STATUS_BUSY:
        {
            /* Check PLC Driver status until it is READY */
            palPlcData.drvPhyStatus = DRV_PLC_PHY_Status(DRV_PLC_PHY_INDEX);
            if (palPlcData.drvPhyStatus == SYS_STATUS_READY)
            {
                /* Perform initial configuration */
                DRV_PLC_PHY_ExceptionCallbackRegister(palPlcData.drvPhyHandle, lPAL_PLC_PLC_ExceptionCb, DRV_PLC_PHY_INDEX);
                DRV_PLC_PHY_TxCfmCallbackRegister(palPlcData.drvPhyHandle, lPAL_PLC_PLC_DataCfmCb, DRV_PLC_PHY_INDEX);

                /* Update Channel list */
                palPlcData.channelList = SRV_PCOUP_GetChannelList();

                /* Enable TX */
                DRV_PLC_PHY_EnableTX(palPlcData.drvPhyHandle, true);
                palPlcData.pvddMonTxEnable = true;

                /* Set Channel for impedance detection */
                palPlcData.channel = SRV_PCOUP_GetChannelImpedanceDetection();
                lPAL_PLC_SetTxRxChannel(palPlcData.channel);
                palPlcData.detectImpedanceResult = DRV_PLC_PHY_TX_RESULT_NO_TX;
                palPlcData.status = PAL_PLC_STATUS_DETECT_IMPEDANCE;
            }
            break;
        }

        case PAL_PLC_STATUS_DETECT_IMPEDANCE:
        {
            if (palPlcData.detectImpedanceResult != DRV_PLC_PHY_TX_RESULT_PROCESS)
            {
                if (palPlcData.detectImpedanceResult == DRV_PLC_PHY_TX_RESULT_SUCCESS)
                {
                    /* Set Default configuration */
                    palPlcData.status = PAL_PLC_STATUS_SET_DEFAULT;
                }
                else
                {
                    // Send Dummy Message
                    DRV_PLC_PHY_TRANSMISSION_OBJ txObj;
                    uint8_t pData[8];
                    
                    memset(pData, 0xAA, sizeof(pData));
                    
                    txObj.bufferId = TX_BUFFER_0;
                    txObj.csma.disableRx = true;
                    txObj.csma.senseCount = 0;
                    txObj.csma.senseDelayMs = 0;
                    txObj.frameType = PAL_FRAME_TYPE_A;
                    txObj.scheme = SCHEME_DBPSK;
                    txObj.mode = TX_MODE_RELATIVE;
                    txObj.timeIni = TRNG_ReadData() % 100000;
                    txObj.dataLength = sizeof(pData);
                    txObj.pTransmitData = pData;
                    txObj.attenuation = 7;
                    
                    palPlcData.detectImpedanceResult = DRV_PLC_PHY_TX_RESULT_PROCESS;
                    DRV_PLC_PHY_TxRequest(palPlcData.drvPhyHandle, &txObj);
                }
            }
            break;
        }

        case PAL_PLC_STATUS_SET_DEFAULT:
        {
            /* Set Data Indication Callback */
            DRV_PLC_PHY_DataIndCallbackRegister(palPlcData.drvPhyHandle, lPAL_PLC_PLC_DataIndCb, DRV_PLC_PHY_INDEX);

            /* Apply PLC coupling configuration for the default channel */
            palPlcData.channel = SRV_PCOUP_GetDefaultChannel();
            lPAL_PLC_SetTxRxChannel(palPlcData.channel);

            /* Set PAL status to ready */
            palPlcData.status = PAL_PLC_STATUS_READY;
            break;
        }

        case PAL_PLC_STATUS_READY:
        {
            if (palPlcData.syncUpdate) {
                /* Update synchronization between Host and PL360 timers */
                lPAL_PLC_TimerSyncUpdate();
            }
            break;
        }

        case PAL_PLC_STATUS_ERROR:
        default:
        {
            /* Restore parameters in case of PL360/PL460 reset */
            if (palPlcData.exceptionPending == true)
            {
                /* Restart exception flag */
                palPlcData.exceptionPending = false;

                /* Set Channel for impedance detection */
                palPlcData.channel = SRV_PCOUP_GetChannelImpedanceDetection();
                lPAL_PLC_SetTxRxChannel(palPlcData.channel);
                palPlcData.detectImpedanceResult = DRV_PLC_PHY_TX_RESULT_NO_TX;
                palPlcData.status = PAL_PLC_STATUS_DETECT_IMPEDANCE;

                // lSetCorrelationThresholds();

                palPlcData.hiTimerRef = 0;
                palPlcData.previousTimerRef = 0;
            }
            break;
        }
    }
}

void PAL_PLC_DataConfirmCallbackRegister(PAL_DATA_CONFIRM_CB callback)
{
    palPlcData.plcCallbacks.dataConfirm = callback;
}

void PAL_PLC_DataIndicationCallbackRegister(PAL_DATA_INDICATION_CB callback)
{
    palPlcData.plcCallbacks.dataIndication = callback;
}

uint8_t PAL_PLC_DataRequest(PAL_MSG_REQUEST_DATA *pMessageData)
{
    if (palPlcData.status != PAL_PLC_STATUS_READY)
    {
        return ((uint8_t)PAL_TX_RESULT_PHY_ERROR);
    }

    /* Adapt Timer mode */
    if (pMessageData->timeMode == TX_MODE_ABSOLUTE) 
    {
        palPlcData.phyTxObj.timeIni = lPAL_PLC_GetPlcTime(pMessageData->timeDelay);
    }
    else
    {
        palPlcData.phyTxObj.timeIni = pMessageData->timeDelay;
    }

    palPlcData.phyTxObj.dataLength = pMessageData->dataLength;
    palPlcData.phyTxObj.mode = pMessageData->timeMode;
    palPlcData.phyTxObj.attenuation = palPlcData.palAttenuation + pMessageData->attLevel;
    palPlcData.phyTxObj.csma.disableRx = pMessageData->disableRx;
    palPlcData.phyTxObj.csma.senseCount = pMessageData->numSenses;
    palPlcData.phyTxObj.csma.senseDelayMs = pMessageData->senseDelayMs;
    palPlcData.phyTxObj.bufferId = pMessageData->buffId;
    palPlcData.phyTxObj.scheme = pMessageData->scheme;
    palPlcData.phyTxObj.frameType = pMessageData->frameType;
    palPlcData.phyTxObj.pTransmitData = pMessageData->pData;

    SRV_PSNIFFER_SetTxMessage(&palPlcData.phyTxObj);

    DRV_PLC_PHY_TxRequest(palPlcData.drvPhyHandle, &palPlcData.phyTxObj);

    return ((uint8_t)PAL_TX_RESULT_PROCESS);
}

void PAL_PLC_ProgramChannelSwitch(uint32_t timeSync, uint16_t pch, uint8_t timeMode)
{
    (void)timeSync;
    (void)pch;
    (void)timeMode;
}

uint8_t PAL_PLC_GetTimer(uint32_t *pTimer)
{
    *pTimer = SRV_TIME_MANAGEMENT_GetTimeUS();

    return PAL_CFG_SUCCESS;
}

uint8_t PAL_PLC_GetTimerExtended(uint64_t *pTimeExtended)
{
    *pTimeExtended = SRV_TIME_MANAGEMENT_GetTimeUS64();

    return PAL_CFG_SUCCESS;
}

uint8_t PAL_PLC_GetCD(uint8_t *pCD, uint8_t *pRSSI, uint32_t *pTime, uint8_t *pHeader)
{
    DRV_PLC_PHY_CD_INFO cdData;
    uint64_t tempValue;
    uint32_t hiTimerRef;
    uint32_t rxTimeEnd;
    uint32_t time10us;
    uint8_t cd;
    uint8_t header;
    uint8_t rssi;

    /* Read Carrier Detect information from PL360 */
    palPlcData.plcPIB.id = PLC_ID_RX_CD_INFO;
    palPlcData.plcPIB.length = sizeof(cdData);
    palPlcData.plcPIB.pData = (uint8_t *)&cdData;
    DRV_PLC_PHY_PIBGet(palPlcData.drvPhyHandle, &palPlcData.plcPIB);

    /* Convert RSSI to format from PRIME spec: 0: <=70 dBuV; 1: <=72 dBuV; ... 15: >98 dBuV */
    rssi = MAX(cdData.rssiAvg, 69);
    rssi -= 69;
    rssi >>= 1;
    *pRSSI = MIN(rssi, 15);

    if (cdData.cdRxState == CD_RX_IDLE) 
    {
        /* Free channel */
        cd = 0;
        header = 0;
        time10us = 0;
    } 
    else 
    {
        /* Busy channel */
        cd = 1;
        header = (cdData.cdRxState == CD_RX_PAYLOAD)? 0 : 1;

        /* Check if there is overflow since last time ref read (assumed that last read time ref is previous to Rx end time) */
        rxTimeEnd = lPAL_PLC_GetHostTime(cdData.rxTimeEnd);
        hiTimerRef = palPlcData.hiTimerRef;
        if (rxTimeEnd < palPlcData.previousTimerRef) {
            hiTimerRef++;
        }

        /* Convert time to extended mode (64 bits) */
        tempValue = ((uint64_t)hiTimerRef << 32) + rxTimeEnd;

        /* To adjust more, make round instead floor */
        tempValue += 5;

        /* Convert time to 10us base */
        time10us = (uint32_t)((tempValue / 10) & 0xffffffff);
    }

    /* Write output parameters */
    *pCD = cd;
    *pTime = time10us;
    *pHeader = header;

    return(PAL_CFG_SUCCESS);
}

uint8_t PAL_PLC_GetZCT(uint32_t *pZcTime)
{
    /* Read ZC data */
    uint64_t tempValue;
    uint32_t timeRef;
    uint32_t zcTime1us;
    uint32_t zcTime10us;

    palPlcData.plcPIB.id = PLC_ID_ZC_TIME;
    palPlcData.plcPIB.length = sizeof(zcTime1us);
    palPlcData.plcPIB.pData = (uint8_t *)&zcTime1us;
    DRV_PLC_PHY_PIBGet(palPlcData.drvPhyHandle, &palPlcData.plcPIB);

    zcTime1us = lPAL_PLC_GetHostTime(zcTime1us);

    PAL_PLC_GetTimerExtended(&tempValue);

    timeRef = (uint32_t)(tempValue & 0xffffffff);

    tempValue = tempValue & (0xffffffff00000000);
    if (timeRef < zcTime1us) {
        tempValue -= 0x100000000;
    }

    tempValue += zcTime1us;

    /* To adjust more, make round instead floor */
    tempValue += 5;

    zcTime10us = (uint32_t)((tempValue / 10) & 0xffffffff);
    *pZcTime = zcTime10us;

    return(PAL_CFG_SUCCESS);
}

uint8_t PAL_PLC_GetAGC(uint8_t *pMode, uint8_t *pGain)
{
    *pMode = 0;
    *pGain = 0;

    return(PAL_CFG_INVALID_INPUT);
}

uint8_t PAL_PLC_SetAGC(uint8_t mode, uint8_t gain)
{
    (void)(mode);
    (void)(gain);

    return(PAL_CFG_INVALID_INPUT);
}

uint8_t PAL_PLC_GetCCA(uint8_t *channelState)
{
    return PAL_CFG_INVALID_INPUT;
}

uint8_t PAL_PLC_GetNL(uint8_t *pNoise)
{
    /* CINR is in 1/4 db. */
    *pNoise = (palPlcData.lastRSSIAvg - (palPlcData.lastCINRMin >> 2));

    return(PAL_CFG_SUCCESS);
}

uint8_t PAL_PLC_GetChannel(uint16_t *pPch)
{
    if (palPlcData.status != PAL_PLC_STATUS_READY)
    {
        return PAL_CFG_INVALID_INPUT;
    }

    *pPch = lPAL_PLC_GetPCH(palPlcData.channel);
    return(PAL_CFG_SUCCESS);
}

uint8_t PAL_PLC_SetChannel(uint16_t pch)
{
    uint8_t channel;

    if (palPlcData.status != PAL_PLC_STATUS_READY)
    {
        return PAL_CFG_INVALID_INPUT;
    }

    channel = lPAL_PLC_GetChannelNumber(pch);

    lPAL_PLC_SetTxRxChannel(channel);
    
    return(PAL_CFG_SUCCESS);
}

uint8_t PAL_PLC_GetConfiguration(uint16_t id, void *pValue, uint16_t length)
{
    uint16_t plcID;

    /* Check identifier */
    switch (id) 
    {
        case PAL_ID_CONTINUOUS_TX_EN:
            /* Not needed */
            return(PAL_CFG_INVALID_INPUT);

        case PAL_ID_ZC_PERIOD:
            plcID = PLC_ID_ZC_PERIOD;
            break;

        case PAL_ID_HOST_VERSION:
            plcID = PLC_ID_HOST_VERSION_ID;
            break;

        case PAL_ID_CFG_MAX_TXRX_NUM_CHANNELS:
            *(uint8_t *)pValue = 2;

            return(PAL_CFG_SUCCESS);

        case PAL_ID_CFG_TXRX_DOUBLE_CHANNEL_LIST:
            *(uint8_t *)pValue = (uint8_t)((palPlcData.channelList >> 8) & 0xff);
            return(PAL_CFG_SUCCESS);

        case PAL_ID_INFO_VERSION:
            plcID = PLC_ID_VERSION_NUM;
            break;

        case PAL_ID_CFG_AUTODETECT_BRANCH:
            plcID = PLC_ID_CFG_AUTODETECT_IMPEDANCE;
            break;

        case PAL_ID_CFG_IMPEDANCE:
            plcID = PLC_ID_CFG_IMPEDANCE;
            break;

        case PAL_ID_CFG_TXRX_CHANNEL:
            if (palPlcData.status != PAL_PLC_STATUS_READY)
            {
                return PAL_CFG_INVALID_INPUT;
            }
            *(uint8_t *)pValue = palPlcData.channel;
            return(PAL_CFG_SUCCESS);

        case PAL_ID_CFG_TXRX_CHANNEL_LIST:
            *(uint8_t *)pValue = (uint8_t)(palPlcData.channelList & 0xff);
            return(PAL_CFG_SUCCESS);

        case PAL_ID_NETWORK_DETECTION:
            *(uint8_t *)pValue = palPlcData.networkDetected;
            return(PAL_CFG_SUCCESS);

        case PAL_ID_CFG_ATTENUATION:
            *(uint8_t *)pValue = palPlcData.palAttenuation;
            return (PAL_CFG_SUCCESS);

        case PAL_ID_RX_PAYLOAD_LEN_SYM:
            plcID = PLC_ID_RX_PAY_SYMBOLS;
            break;

        case PAL_ID_INFO_DEVICE:
            plcID = PLC_ID_HOST_PRODUCT_ID;
            break;

        case PAL_ID_REMAINING_FRAME_DURATION:
        {
            DRV_PLC_PHY_CD_INFO cdData;

            palPlcData.plcPIB.id = PLC_ID_RX_CD_INFO;
            palPlcData.plcPIB.length = sizeof(cdData);
            palPlcData.plcPIB.pData = (uint8_t *)&cdData;
            DRV_PLC_PHY_PIBGet(palPlcData.drvPhyHandle, &palPlcData.plcPIB);

            if (cdData.cdRxState == CD_RX_IDLE) 
            {
                *(uint32_t *)pValue = 0;
            } 
            else 
            {
                *(uint32_t *)pValue = cdData.rxTimeEnd - cdData.currentTime;
            }

            return(PAL_CFG_SUCCESS);
        }

        case PAL_ID_PLC_RX_PHY_PARAMS:
            memcpy(pValue, &palPlcData.rxParameters, length);
            return(PAL_CFG_SUCCESS);

        default:
            if (id >= 0xFD00) 
            {
                if (id != 0xFEEE) 
                {
                    plcID = (id & 0x00FF) | 0x4000;
                } 
                else 
                {
                    *(uint8_t *)pValue = palPlcData.errorInfo;
                    return(PAL_CFG_SUCCESS);
                }
            } 
            else 
            {
                *(uint8_t *)pValue = 0;
                return(PAL_CFG_SUCCESS);
            }
    }

    /* Get in phy layer */
    palPlcData.plcPIB.id = plcID;
    palPlcData.plcPIB.length = length;
    palPlcData.plcPIB.pData = (uint8_t *)pValue;
    DRV_PLC_PHY_PIBGet(palPlcData.drvPhyHandle, &palPlcData.plcPIB);

    return(PAL_CFG_SUCCESS);
}

uint8_t PAL_PLC_SetConfiguration(uint16_t id, void *pValue, uint16_t length)
{
    uint16_t plcID;

    /* Check identifier */
    switch (id) 
    {
        case PAL_ID_CONTINUOUS_TX_EN:
        {
            uint8_t txMode;
            
            txMode = *(uint8_t *)pValue;
            lPAL_PLC_SetTxContinuousMode(txMode);
            return(PAL_CFG_SUCCESS);
        }

        case PAL_ID_ZC_PERIOD:
            plcID = PLC_ID_ZC_PERIOD;
            break;

        case PAL_ID_HOST_VERSION:
            plcID = PLC_ID_HOST_VERSION_ID;
            break;

        case PAL_ID_CFG_TXRX_DOUBLE_CHANNEL_LIST:
        {
            uint16_t channelList;
            
            channelList = (*(uint16_t *)pValue);
            
            palPlcData.channelList &= 0x00FF;
            palPlcData.channelList |= (channelList << 8);
            return(PAL_CFG_SUCCESS);
        }

        case PAL_ID_INFO_VERSION:
            plcID = PLC_ID_VERSION_NUM;
            break;

        case PAL_ID_CFG_AUTODETECT_BRANCH:
            plcID = PLC_ID_CFG_AUTODETECT_IMPEDANCE;
            break;

        case PAL_ID_CFG_IMPEDANCE:
            plcID = PLC_ID_CFG_IMPEDANCE;
            break;

        case PAL_ID_CFG_TXRX_CHANNEL:
        {
            uint8_t chn;
            
            if (palPlcData.status != PAL_PLC_STATUS_READY)
            {
                return PAL_CFG_INVALID_INPUT;
            }

            chn = (*(uint8_t *)pValue);
            lPAL_PLC_SetTxRxChannel(chn);
            return(PAL_CFG_SUCCESS);
        }

        case PAL_ID_CFG_TXRX_CHANNEL_LIST:
        {
            palPlcData.channelList &= 0xFF00;
            palPlcData.channelList |= (*(uint8_t *)pValue);
            return(PAL_CFG_SUCCESS);
        }

        case PAL_ID_RX_PAYLOAD_LEN_SYM:
            plcID = PLC_ID_RX_PAY_SYMBOLS;
            break;

        case PAL_ID_CFG_MAX_TXRX_NUM_CHANNELS:
        case PAL_ID_INFO_DEVICE:
        case PAL_ID_REMAINING_FRAME_DURATION:
        case PAL_ID_PLC_RX_PHY_PARAMS:
            /* Read only */
            return(PAL_CFG_INVALID_INPUT);

        case PAL_ID_NETWORK_DETECTION:
            palPlcData.networkDetected = (*(uint8_t *)pValue);
            return(PAL_CFG_SUCCESS);

        case PAL_ID_CFG_ATTENUATION:
            palPlcData.palAttenuation = (*(uint8_t *)pValue);
            return (PAL_CFG_SUCCESS);

        default:
            return(PAL_CFG_INVALID_INPUT);
    }

    /* Set in phy layer */
    palPlcData.plcPIB.id = plcID;
    palPlcData.plcPIB.length = length;
    palPlcData.plcPIB.pData = (uint8_t *)pValue;
    DRV_PLC_PHY_PIBSet(palPlcData.drvPhyHandle, &palPlcData.plcPIB);
    
    return(PAL_CFG_SUCCESS);
}

uint8_t PAL_PLC_GetSNR(uint8_t *pSnr, uint8_t qt)
{
    if (qt == 0) 
    {
        *pSnr = 0;
        return(PAL_CFG_INVALID_INPUT);
    } 
    else 
    {
        *pSnr = qt / 12 + 1;
        if (*pSnr > 7) 
        {
            *pSnr  = 7;
        }

        return(PAL_CFG_SUCCESS);
    }
}

uint16_t PAL_PLC_GetSignalCapture(uint8_t *pData, uint8_t frameType, uint32_t timeStart, uint32_t duration)
{
    uint8_t *pDataPointer;
    DRV_PLC_PHY_SIGNAL_CAPTURE signalCapture;
    uint8_t captureParameters[9];
    uint8_t index;
    SYS_TIME_HANDLE timer = SYS_TIME_HANDLE_INVALID;

    palPlcData.plcPIB.id = PLC_ID_SIGNAL_CAPTURE_STATUS;
    palPlcData.plcPIB.length = sizeof(signalCapture);
    palPlcData.plcPIB.pData = (uint8_t *)&signalCapture;
    DRV_PLC_PHY_PIBGet(palPlcData.drvPhyHandle, &palPlcData.plcPIB);

    /* Check status */
    while (signalCapture.status == SIGNAL_CAPTURE_RUNNING) 
    {
        DRV_PLC_PHY_PIBGet(palPlcData.drvPhyHandle, &palPlcData.plcPIB);
        if (SYS_TIME_DelayMS(5, &timer) == SYS_TIME_SUCCESS)
        {
            if(SYS_TIME_DelayIsComplete(timer) != true)
            {
                while (SYS_TIME_DelayIsComplete(timer) == false);
            }
        }
    }

    /* Start Capture */
    pDataPointer = captureParameters;
    *pDataPointer++ = frameType;
    *pDataPointer++ = (uint8_t)(timeStart >> 24);
    *pDataPointer++ = (uint8_t)(timeStart >> 16);
    *pDataPointer++ = (uint8_t)(timeStart >> 8);
    *pDataPointer++ = (uint8_t)(timeStart);
    *pDataPointer++ = (uint8_t)(duration >> 24);
    *pDataPointer++ = (uint8_t)(duration >> 16);
    *pDataPointer++ = (uint8_t)(duration >> 8);
    *pDataPointer++ = (uint8_t)(duration);

    palPlcData.plcPIB.id = PLC_ID_SIGNAL_CAPTURE_START;
    palPlcData.plcPIB.length = sizeof(captureParameters);
    palPlcData.plcPIB.pData = (uint8_t *)&captureParameters;
    DRV_PLC_PHY_PIBSet(palPlcData.drvPhyHandle, &palPlcData.plcPIB);

    /* Check status */
    palPlcData.plcPIB.id = PLC_ID_SIGNAL_CAPTURE_STATUS;
    palPlcData.plcPIB.length = sizeof(signalCapture);
    palPlcData.plcPIB.pData = (uint8_t *)&signalCapture;
    DRV_PLC_PHY_PIBGet(palPlcData.drvPhyHandle, &palPlcData.plcPIB);

    while (signalCapture.status != SIGNAL_CAPTURE_READY) 
    {
        DRV_PLC_PHY_PIBGet(palPlcData.drvPhyHandle, &palPlcData.plcPIB);
        if (SYS_TIME_DelayMS(5, &timer) == SYS_TIME_SUCCESS)
        {
            if(SYS_TIME_DelayIsComplete(timer) != true)
            {
                while (SYS_TIME_DelayIsComplete(timer) == false);
            }
        }
    }

    /* Read Noise Data */
    pDataPointer = pData;
    index = 0;
    while (index < signalCapture.numFrags) 
    {
        palPlcData.plcPIB.id = PLC_ID_SIGNAL_CAPTURE_FRAGMENT;
        palPlcData.plcPIB.length = 1;
        palPlcData.plcPIB.pData = (uint8_t *)&index;
        DRV_PLC_PHY_PIBSet(palPlcData.drvPhyHandle, &palPlcData.plcPIB);

        palPlcData.plcPIB.id = PLC_ID_SIGNAL_CAPTURE_DATA;
        palPlcData.plcPIB.length = SIGNAL_CAPTURE_FRAG_SIZE;
        palPlcData.plcPIB.pData = pDataPointer;
        DRV_PLC_PHY_PIBGet(palPlcData.drvPhyHandle, &palPlcData.plcPIB);

        index++;
        pDataPointer += SIGNAL_CAPTURE_FRAG_SIZE;
    }

    return (uint16_t)(pDataPointer - pData);
}

uint8_t PAL_PLC_GetMsgDuration(uint16_t length, PAL_SCHEME scheme, uint8_t frameType, uint32_t *pDuration)
{
    uint32_t frameDuration;
    uint16_t frameLen;
    uint8_t symbolSize = 0;

    if (length == 0)
    {
        *pDuration = 0;
        return(PAL_CFG_INVALID_INPUT);
    }

    frameDuration = 0;
    frameLen = length;

    if (frameType == PAL_FRAME_TYPE_A) 
    {
        /* There are 7 bytes inside the header */
        if (frameLen < 7) 
        {
            frameLen = 0;
        } 
        else 
        {
            frameLen -= 7;
        }
    }

    symbolSize = palPlcSymbolSize[scheme];
    if (scheme >= PAL_SCHEME_DBPSK_C)
    {
        /* Increase a byte for flushing */
        frameLen++;
    }

    /* Update tx frame duration */
    frameDuration =  frameLen / symbolSize;
    if (frameLen % symbolSize) {
        frameDuration++;
    }

    /* adjust ROB scheme */
    if (scheme & 0x08) {
        frameDuration <<= 2;
    }

    frameDuration *= PHY_SYMBOL_TIME;
    /* Adjust chirp and header for PHY frame */
    frameDuration += palPlcTimeChirpHeader[frameType];

    *pDuration = frameDuration;

    return(PAL_CFG_SUCCESS);
}

void PAL_PLC_USISnifferCallbackRegister(SRV_USI_HANDLE usiHandler, PAL_USI_SNIFFER_CB callback)
{
    palPlcData.usiHandler = usiHandler;
    palPlcData.snifferCallback = callback;
}

