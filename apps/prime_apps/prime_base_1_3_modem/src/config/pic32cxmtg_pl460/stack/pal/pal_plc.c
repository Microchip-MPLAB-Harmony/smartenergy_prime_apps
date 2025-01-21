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
#include "service/pvddmon/srv_pvddmon.h"
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
#define SYNC_TIMER_REL_FREQ_MAX  0x01000D1BU /* +200 PPM */
#define SYNC_TIMER_REL_FREQ_MIN  0x00FFF2E5U /* -200 PPM */

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
    .MPAL_GetSNR = PAL_PLC_GetSNR,
    .MPAL_GetZCT = PAL_PLC_GetZCT,
    .MPAL_GetTimer = PAL_PLC_GetTimer,
    .MPAL_GetTimerExtended = PAL_PLC_GetTimerExtended,
    .MPAL_GetCD = PAL_PLC_GetCD,
    .MPAL_GetNL = PAL_PLC_GetNL,
    .MPAL_GetAGC = PAL_PLC_GetAGC,
    .MPAL_SetAGC = PAL_PLC_SetAGC,
    .MPAL_GetCCA = PAL_PLC_GetCCA,
    .MPAL_GetChannel = PAL_PLC_GetChannel,
    .MPAL_SetChannel = PAL_PLC_SetChannel,
    .MPAL_DataRequest = PAL_PLC_DataRequest,
    .MPAL_ProgramChannelSwitch = PAL_PLC_ProgramChannelSwitch,
    .MPAL_GetConfiguration = PAL_PLC_GetConfiguration,
    .MPAL_SetConfiguration = PAL_PLC_SetConfiguration,
    .MPAL_GetSignalCapture = PAL_PLC_GetSignalCapture,
    .MPAL_GetMsgDuration = PAL_PLC_GetMsgDuration,
    .MPAL_CheckMinimumQuality = PAL_PLC_RM_CheckMinimumQuality,
    .MPAL_GetLessRobustModulation = PAL_PLC_RM_GetLessRobustModulation,
};

// *****************************************************************************
// *****************************************************************************
// Section: File Scope Data
// *****************************************************************************
// *****************************************************************************
extern DRV_PLC_PHY_INIT drvPlcPhyInitData;

static uint32_t timePlcSync = 0;

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

static uint16_t lPAL_PLC_GetPCH(DRV_PLC_PHY_CHANNEL channel)
{
    uint16_t pch;

    if ((uint8_t)channel <= 8U)
    {
        pch = (uint16_t)(1U) << ((uint8_t)channel - 1U); /* Single channel */
    }
    else
    {
        pch = (uint16_t)(3U) << ((uint8_t)channel - 9U); /* Double channel */
    }

    return pch;
}

static uint8_t lPAL_PLC_GetChannelNumber(uint16_t pch)
{
    uint8_t channel = 1U;

    if ((pch % 3U) == 0U)
    {
        // double channel
        pch /= 3U;
        channel += 8U;
    }

    while ((pch & 0x0001U) == 0U)
    {
        pch >>= 1U;
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
    palPlcData.phyTxObj.mode = TX_MODE_ABSOLUTE;
    palPlcData.phyTxObj.attenuation = 0xFF;
    palPlcData.phyTxObj.csma.disableRx = 1;
    palPlcData.phyTxObj.csma.senseCount = 0;
    palPlcData.phyTxObj.csma.senseDelayMs = 0;
    palPlcData.phyTxObj.bufferId = TX_BUFFER_0;
    palPlcData.phyTxObj.scheme = SCHEME_DBPSK;
    palPlcData.phyTxObj.frameType = FRAME_TYPE_B;

    /* Check mode */
    if (txMode == 0U)
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
    __set_BASEPRI(1U << (uint8_t)(8U - (uint8_t)__NVIC_PRIO_BITS));

    /* Read Host timer */
    timeHost = SRV_TIME_MANAGEMENT_GetTimeUS();

    /* Read PLC timer */
    palPlcData.plcPIB.id = PLC_ID_TIME_REF_ID;
    palPlcData.plcPIB.length = 4;
    palPlcData.plcPIB.pData = (uint8_t *)pTimePlc;
    (void)DRV_PLC_PHY_PIBGet(palPlcData.drvPhyHandle, &palPlcData.plcPIB);

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
        (void)SYS_TIME_TimerDestroy(palPlcData.syncHandle);
        palPlcData.syncHandle = SRV_TIME_MANAGEMENT_CbRegisterUS(
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
    uint32_t delayHost;
    uint32_t delayPlc;
    uint32_t syncTimerRelFreq;
    uint64_t delayAux;

    /* Get current Host and PLC timers */
    timeHost = lPAL_PLC_TimerSyncRead(&timePlcSync);

    /* Compute delays from references (last read) */
    delayHost = timeHost - palPlcData.timeRefHost;
    delayPlc = timePlcSync - palPlcData.timeRefPlc;

    /* Compute relative frequency F_host/F_plc [uQ1.24] */
    delayAux = DIV_ROUND((uint64_t)delayHost << 24, (uint64_t)(delayPlc));
    syncTimerRelFreq = (uint32_t)(delayAux);

    /* Check if relative frequency is consistent, otherwise timer read is wrong */
    if ((syncTimerRelFreq >= SYNC_TIMER_REL_FREQ_MIN) && (syncTimerRelFreq <= SYNC_TIMER_REL_FREQ_MAX))
    {
        /* Update relative frequency and references */
        palPlcData.syncTimerRelFreq = syncTimerRelFreq;
        palPlcData.timeRefHost = timeHost;
        palPlcData.timeRefPlc = timePlcSync;

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
        palPlcData.syncHandle = SRV_TIME_MANAGEMENT_CbRegisterUS(
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
                "PRIME_PAL_PLC: PLC timer synchronization error\r\n");
        lPAL_PLC_TimerSyncInitialize();
    }
}

static uint32_t lPAL_PLC_GetHostTime(uint32_t timePlc)
{
    int64_t delayAux;
    int64_t delayPlc;
    int64_t delayHost;
    int64_t timeHost;

    /* Compute PLC delay time since last sinchronization */
    delayPlc = (int64_t)(timePlc) - (int64_t)(palPlcData.timeRefPlc);

    /* Convert PLC delay to Host delay (frequency deviation) */
    delayAux = delayPlc * (int64_t)palPlcData.syncTimerRelFreq;

/* MISRA C-2012 deviation block start */
/* MISRA C-2012 Rule 10.1 deviated once. Deviation record ID - H3_MISRAC_2012_R_10_1_DR_1 */
    delayHost = (delayAux + (1L << 23)) >> 24;
/* MISRA C-2012 deviation block end */

    /* Compute Host time */
    timeHost = (int64_t)palPlcData.timeRefHost + delayHost;

    return (uint32_t)(timeHost);
}

static uint32_t lPAL_PLC_GetPlcTime(uint32_t timeHost)
{
    int64_t delayPlc;
    int64_t delayHost;
    int64_t timePlc;

    /* Compute Host delay time since last synchronization */
    delayHost = (int64_t)(timeHost) - (int64_t)(palPlcData.timeRefHost);

/* MISRA C-2012 deviation block start */
/* MISRA C-2012 Rule 10.1 deviated twice. Deviation record ID - H3_MISRAC_2012_R_10_1_DR_1 */
/* Convert Host delay to PLC delay (frequency deviation) */
    delayPlc = delayHost << 24;
    delayPlc = DIV_ROUND(delayPlc, (int64_t)(palPlcData.syncTimerRelFreq));
/* MISRA C-2012 deviation block end */

    /* Compute PLC time */
    timePlc = (int64_t)(palPlcData.timeRefPlc) + delayPlc;

    return (uint32_t)(timePlc);
}

static void lPAL_PLC_SetTxRxChannel(DRV_PLC_PHY_CHANNEL channel)
{
    /* Set channel configuration */
    palPlcData.plcPIB.id = PLC_ID_CHANNEL_CFG;
    palPlcData.plcPIB.length = 1;
    palPlcData.plcPIB.pData = &channel;
    (void)DRV_PLC_PHY_PIBSet(palPlcData.drvPhyHandle, &palPlcData.plcPIB);

    /* Set coupling configuration */
    (void)SRV_PCOUP_SetChannelConfig(palPlcData.drvPhyHandle, channel);

    /* Initialize synchronization of PL360-Host timers when channel updated */
    lPAL_PLC_TimerSyncInitialize();

    SRV_PSNIFFER_SetPLCChannel((uint8_t)channel);

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
        dataCfm.frameType = (PAL_FRAME)pCfmObj->frameType;
        dataCfm.bufId = (uint8_t)pCfmObj->bufferId;

        switch (pCfmObj->result)
        {
            case DRV_PLC_PHY_TX_RESULT_BUSY_CH:
            case DRV_PLC_PHY_TX_RESULT_BUSY_RX:
                dataCfm.result = PAL_TX_RESULT_BUSY_CH;
                break;
            default:
                dataCfm.result = (PAL_TX_RESULT)pCfmObj->result;
                break;
        }

        palPlcData.plcCallbacks.dataConfirm(&dataCfm);
    }

    if ((palPlcData.snifferCallback) != NULL)
    {
        size_t dataLength;
        uint16_t paySymbols;
        uint16_t payloadSymbols;

        palPlcData.plcPIB.id = PLC_ID_TX_PAY_SYMBOLS;
        palPlcData.plcPIB.length = 2;
        palPlcData.plcPIB.pData = (uint8_t *)&paySymbols;
        (void)DRV_PLC_PHY_PIBGet(palPlcData.drvPhyHandle, &palPlcData.plcPIB);

        payloadSymbols = (uint16_t)palPlcData.plcPIB.pData[0] | ((uint16_t)palPlcData.plcPIB.pData[1] << 8);
        SRV_PSNIFFER_SetTxPayloadSymbols(payloadSymbols);

        dataLength = SRV_PSNIFFER_SerialCfmMessage(palPlcData.snifferData, pCfmObj);

        if (dataLength != 0U)
        {
            palPlcData.snifferCallback(palPlcData.snifferData, dataLength);
        }
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
    palPlcData.rxParameters.scheme = (PAL_SCHEME)pIndObj->scheme;
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
    dataInd.rssi = (int16_t)(pIndObj->rssiAvg);
    dataInd.bufId = 0;
    dataInd.scheme = (PAL_SCHEME)pIndObj->scheme;
    dataInd.frameType = (PAL_FRAME)pIndObj->frameType;
    dataInd.headerType = (uint8_t)pIndObj->headerType;
    dataInd.lqi = PAL_PLC_RM_GetLqi(pIndObj->cinrAvg);

    /* Store last values of some fields */
    palPlcData.lastRSSIAvg = pIndObj->rssiAvg;
    palPlcData.lastCINRMin = pIndObj->cinrMin;

    if (palPlcData.plcCallbacks.dataIndication != NULL)
    {
        palPlcData.plcCallbacks.dataIndication(&dataInd);
    }

    if ((palPlcData.snifferCallback) != NULL)
    {
        size_t length;
        uint16_t paySymbols;

        palPlcData.plcPIB.id = PLC_ID_RX_PAY_SYMBOLS;
        palPlcData.plcPIB.length = 2;
        palPlcData.plcPIB.pData = (uint8_t *)&paySymbols;
        (void)DRV_PLC_PHY_PIBGet(palPlcData.drvPhyHandle, &palPlcData.plcPIB);

        SRV_PSNIFFER_SetRxPayloadSymbols(paySymbols);

        length = SRV_PSNIFFER_SerialRxMessage(palPlcData.snifferData, pIndObj);

        if (length != 0U)
        {
            palPlcData.snifferCallback(palPlcData.snifferData, length);
        }
    }

}

static void lPAL_PLC_PLC_PVDDMonitorCb(SRV_PVDDMON_CMP_MODE cmpMode, uintptr_t context)
{
    /* Avoid warning */
    (void)context;

    if (cmpMode == SRV_PVDDMON_CMP_MODE_OUT)
    {
        /* PLC Transmission is not permitted */
        DRV_PLC_PHY_EnableTX(palPlcData.drvPhyHandle, false);
        palPlcData.pvddMonTxEnable = false;
        /* Restart PVDD Monitor to check when VDD is within the comparison window */
        SRV_PVDDMON_Restart(SRV_PVDDMON_CMP_MODE_IN);
    }
    else
    {
        /* PLC Transmission is permitted again */
        DRV_PLC_PHY_EnableTX(palPlcData.drvPhyHandle, true);
        palPlcData.pvddMonTxEnable = true;
        /* Restart PVDD Monitor to check when VDD is out of the comparison window */
        SRV_PVDDMON_Restart(SRV_PVDDMON_CMP_MODE_OUT);
    }
}

static void lPAL_PLC_PLC_ExceptionCb(DRV_PLC_PHY_EXCEPTION exception, uintptr_t context)
{
    uint8_t numErrors;

    /* Avoid warning */
    (void)context;

    /* Set Error Status */
    palPlcData.status = PAL_PLC_STATUS_ERROR;

    if (exception == DRV_PLC_PHY_EXCEPTION_CRITICAL_ERROR)
    {
        palPlcData.statsErrorCritical++;
        palPlcData.exceptionPending = false;
    }
    else
    {
        /* Set exception pending flag to manage reset from tasks */
        palPlcData.exceptionPending = true;

        /* Update stats counter */
        if (exception == DRV_PLC_PHY_EXCEPTION_UNEXPECTED_KEY)
        {
            palPlcData.statsErrorUnexpectedKey++;
        }

        if (exception == DRV_PLC_PHY_EXCEPTION_RESET)
        {
            palPlcData.statsErrorReset++;
        }

        if (exception == DRV_PLC_PHY_EXCEPTION_DEBUG)
        {
            palPlcData.statsErrorDebug++;
        }
    }

    /* Store error info */
    palPlcData.errorInfo |= (1U << (uint8_t)exception);

    /* Increase number of errors, if possible */
    numErrors = palPlcData.errorInfo >> 4;
    if (numErrors < 0xFU) {
        ++numErrors;
        palPlcData.errorInfo &= 0x0FU;
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
    /* MISRA C-2012 deviation block start */
/* MISRA C-2012 Rule 11.3 deviated twice. Deviation record ID - H3_MISRAC_2012_R_11_3_DR_1 */
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

                /* Disable TX Enable at the beginning */
                DRV_PLC_PHY_EnableTX(palPlcData.drvPhyHandle, false);
                palPlcData.pvddMonTxEnable = false;
                /* Enable PLC PVDD Monitor Service */
                SRV_PVDDMON_CallbackRegister(lPAL_PLC_PLC_PVDDMonitorCb, 0);
                SRV_PVDDMON_Start(SRV_PVDDMON_CMP_MODE_IN);

                /* Set Channel for impedance detection */
                palPlcData.channel = SRV_PCOUP_GetChannelImpedanceDetection();
                lPAL_PLC_SetTxRxChannel(palPlcData.channel);
                palPlcData.detectImpedanceResult = DRV_PLC_PHY_TX_RESULT_NO_TX;
                palPlcData.status = PAL_PLC_STATUS_DETECT_IMPEDANCE;
            }
            else
            {
                if (palPlcData.drvPhyStatus == SYS_STATUS_ERROR)
                {
                    palPlcData.status = PAL_PLC_STATUS_ERROR;
                }
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

                    (void)memset(pData, 0xAA, sizeof(pData));

                    txObj.bufferId = TX_BUFFER_0;
                    txObj.csma.disableRx = 1; /* true */
                    txObj.csma.senseCount = 0;
                    txObj.csma.senseDelayMs = 0;
                    txObj.frameType = FRAME_TYPE_A;
                    txObj.scheme = SCHEME_DBPSK;
                    txObj.mode = (uint8_t)(TX_MODE_RELATIVE);
                    txObj.timeIni = TRNG_ReadData() % 100000U;
                    txObj.dataLength = (uint16_t)(sizeof(pData));
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
                /* Check PLC Driver status until it is READY */
                palPlcData.drvPhyStatus = DRV_PLC_PHY_Status(DRV_PLC_PHY_INDEX);
                if (palPlcData.drvPhyStatus == SYS_STATUS_READY)
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
                else
                {
                    if (palPlcData.drvPhyStatus == SYS_STATUS_ERROR)
                    {
                        palPlcData.exceptionPending = false;
                    }
                }
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
    if (pMessageData->timeMode == PAL_TX_MODE_ABSOLUTE)
    {
        palPlcData.phyTxObj.timeIni = lPAL_PLC_GetPlcTime(pMessageData->timeDelay);
    }
    else
    {
        palPlcData.phyTxObj.timeIni = pMessageData->timeDelay;
    }

    palPlcData.phyTxObj.dataLength = pMessageData->dataLength;
    palPlcData.phyTxObj.mode = (uint8_t)(pMessageData->timeMode);
    palPlcData.phyTxObj.attenuation = palPlcData.palAttenuation + pMessageData->attLevel;
    palPlcData.phyTxObj.csma.disableRx = pMessageData->disableRx;
    palPlcData.phyTxObj.csma.senseCount = pMessageData->numSenses;
    palPlcData.phyTxObj.csma.senseDelayMs = pMessageData->senseDelayMs;
    palPlcData.phyTxObj.bufferId = (DRV_PLC_PHY_BUFFER_ID)(pMessageData->buffId);
    palPlcData.phyTxObj.scheme = (DRV_PLC_PHY_SCH)pMessageData->scheme;
    palPlcData.phyTxObj.frameType = (DRV_PLC_PHY_FRAME_TYPE)pMessageData->frameType;
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

    return (uint8_t)PAL_CFG_SUCCESS;
}

uint8_t PAL_PLC_GetTimerExtended(uint64_t *pTimerExtended)
{
    *pTimerExtended = SRV_TIME_MANAGEMENT_GetTimeUS64();

    return (uint8_t)PAL_CFG_SUCCESS;
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
    palPlcData.plcPIB.length = (uint16_t)(sizeof(cdData));
    palPlcData.plcPIB.pData = (uint8_t *)&cdData;
    (void)DRV_PLC_PHY_PIBGet(palPlcData.drvPhyHandle, &palPlcData.plcPIB);

    /* Convert RSSI to format from PRIME spec: 0: <=70 dBuV; 1: <=72 dBuV; ... 15: >98 dBuV */
    rssi = MAX(cdData.rssiAvg, 69U);
    rssi -= 69U;
    rssi >>= 1;
    *pRSSI = MIN(rssi, 15U);

    if (cdData.cdRxState == CD_RX_IDLE)
    {
        /* Free channel */
        cd = 0;
        header = 0U;
        time10us = 0;
    }
    else
    {
        /* Busy channel */
        cd = 1;
        header = (cdData.cdRxState == CD_RX_PAYLOAD)? 0U : 1U;

        /* Check if there is overflow since last time ref read (assumed that last read time ref is previous to Rx end time) */
        rxTimeEnd = lPAL_PLC_GetHostTime(cdData.rxTimeEnd);
        hiTimerRef = palPlcData.hiTimerRef;
        if (rxTimeEnd < palPlcData.previousTimerRef) {
            hiTimerRef++;
        }

        /* Convert time to extended mode (64 bits) */
        tempValue = ((uint64_t)hiTimerRef << 32) + rxTimeEnd;

        /* To adjust more, make round instead floor */
        tempValue += 5U;

        /* Convert time to 10us base */
        time10us = (uint32_t)((tempValue / 10U) & 0xffffffffU);
    }

    /* Write output parameters */
    *pCD = cd;
    *pTime = time10us;
    *pHeader = header;

    return((uint8_t)PAL_CFG_SUCCESS);
}

uint8_t PAL_PLC_GetZCT(uint32_t *pZcTime)
{
    /* Read ZC data */
    uint64_t tempValue;
    uint32_t timeRef;
    uint32_t zcTime1us;
    uint32_t zcTime10us;

    palPlcData.plcPIB.id = PLC_ID_ZC_TIME;
    palPlcData.plcPIB.length = (uint16_t)(sizeof(zcTime1us));
    palPlcData.plcPIB.pData = (uint8_t *)&zcTime1us;
    (void)DRV_PLC_PHY_PIBGet(palPlcData.drvPhyHandle, &palPlcData.plcPIB);

    zcTime1us = lPAL_PLC_GetHostTime(zcTime1us);

    (void)PAL_PLC_GetTimerExtended(&tempValue);

    timeRef = (uint32_t)(tempValue & 0xffffffffU);

    tempValue = tempValue & (0xffffffff00000000U);
    if (timeRef < zcTime1us) {
        tempValue -= 0x100000000U;
    }

    tempValue += zcTime1us;

    /* To adjust more, make round instead floor */
    tempValue += 5U;

    zcTime10us = (uint32_t)((tempValue / 10U) & 0xffffffffU);
    *pZcTime = zcTime10us;

    return((uint8_t)PAL_CFG_SUCCESS);
}

uint8_t PAL_PLC_GetAGC(uint8_t *pMode, uint8_t *pGain)
{
    *pMode = 0;
    *pGain = 0;

    return((uint8_t)PAL_CFG_INVALID_INPUT);
}

uint8_t PAL_PLC_SetAGC(uint8_t mode, uint8_t gain)
{
    (void)(mode);
    (void)(gain);

    return((uint8_t)PAL_CFG_INVALID_INPUT);
}

uint8_t PAL_PLC_GetCCA(uint8_t *channelState)
{
    return ((uint8_t)PAL_CFG_INVALID_INPUT);
}

uint8_t PAL_PLC_GetNL(uint8_t *pNoise)
{
    /* CINR is in 1/4 db. */
    *pNoise = (palPlcData.lastRSSIAvg - (palPlcData.lastCINRMin >> 2));

    return((uint8_t)PAL_CFG_SUCCESS);
}

uint8_t PAL_PLC_GetChannel(uint16_t *pPch)
{
    if (palPlcData.status != PAL_PLC_STATUS_READY)
    {
        return ((uint8_t)PAL_CFG_INVALID_INPUT);
    }

    *pPch = lPAL_PLC_GetPCH(palPlcData.channel);
    return((uint8_t)PAL_CFG_SUCCESS);
}

uint8_t PAL_PLC_SetChannel(uint16_t pch)
{
    uint8_t channel;

    if (palPlcData.status != PAL_PLC_STATUS_READY)
    {
        return ((uint8_t)PAL_CFG_INVALID_INPUT);
    }

    channel = lPAL_PLC_GetChannelNumber(pch);

    lPAL_PLC_SetTxRxChannel((DRV_PLC_PHY_CHANNEL)channel);

    return((uint8_t)PAL_CFG_SUCCESS);
}

uint8_t PAL_PLC_GetConfiguration(uint16_t id, void *pValue, uint16_t length)
{
    DRV_PLC_PHY_ID plcID;
    PAL_CFG_RESULT result = PAL_CFG_INVALID_INPUT;
    bool askPhy = false;

    /* Check identifier */
    switch ((PAL_ATTRIBUTE_ID)id)
    {
        case PAL_ID_CONTINUOUS_TX_EN:
            /* Not needed */
            result = PAL_CFG_INVALID_INPUT;
            break;

        case PAL_ID_ZC_PERIOD:
            plcID = PLC_ID_ZC_PERIOD;
            askPhy = true;
            break;

        case PAL_ID_HOST_VERSION:
            plcID = PLC_ID_HOST_VERSION_ID;
            askPhy = true;
            break;

        case PAL_ID_CFG_MAX_TXRX_NUM_CHANNELS:
            *(uint8_t *)pValue = 2;
            result = PAL_CFG_SUCCESS;
            break;

        case PAL_ID_CFG_TXRX_DOUBLE_CHANNEL_LIST:
            *(uint8_t *)pValue = (uint8_t)((palPlcData.channelList >> 8U) & 0xffU);
            result = PAL_CFG_SUCCESS;
            break;

        case PAL_ID_INFO_VERSION:
            plcID = PLC_ID_VERSION_NUM;
            askPhy = true;
            break;

        case PAL_ID_CFG_AUTODETECT_BRANCH:
            plcID = PLC_ID_CFG_AUTODETECT_IMPEDANCE;
            askPhy = true;
            break;

        case PAL_ID_CFG_IMPEDANCE:
            plcID = PLC_ID_CFG_IMPEDANCE;
            askPhy = true;
            break;

        case PAL_ID_CFG_TXRX_CHANNEL:
            if (palPlcData.status != PAL_PLC_STATUS_READY)
            {
                result = PAL_CFG_INVALID_INPUT;
                break;
            }
            *(uint8_t *)pValue = (uint8_t)palPlcData.channel;
            result = PAL_CFG_SUCCESS;
            break;

        case PAL_ID_CFG_TXRX_CHANNEL_LIST:
            *(uint8_t *)pValue = (uint8_t)(palPlcData.channelList & 0xffU);
            result = PAL_CFG_SUCCESS;
            break;

        case PAL_ID_NETWORK_DETECTION:
            *(uint8_t *)pValue = (uint8_t)palPlcData.networkDetected;
            result = PAL_CFG_SUCCESS;
            break;

        case PAL_ID_CFG_ATTENUATION:
            *(uint8_t *)pValue = palPlcData.palAttenuation;
            result = PAL_CFG_SUCCESS;
            break;

        case PAL_ID_RX_PAYLOAD_LEN_SYM:
            plcID = PLC_ID_RX_PAY_SYMBOLS;
            askPhy = true;
            break;

        case PAL_ID_INFO_DEVICE:
            plcID = PLC_ID_HOST_PRODUCT_ID;
            askPhy = true;
            break;

        case PAL_ID_REMAINING_FRAME_DURATION:
        {
            DRV_PLC_PHY_CD_INFO cdData;

            palPlcData.plcPIB.id = PLC_ID_RX_CD_INFO;
            palPlcData.plcPIB.length = (uint16_t)(sizeof(cdData));
            palPlcData.plcPIB.pData = (uint8_t *)&cdData;
            (void)DRV_PLC_PHY_PIBGet(palPlcData.drvPhyHandle, &palPlcData.plcPIB);

            if (cdData.cdRxState == CD_RX_IDLE)
            {
                *(uint32_t *)pValue = 0;
            }
            else
            {
                *(uint32_t *)pValue = cdData.rxTimeEnd - cdData.currentTime;
            }
            result = PAL_CFG_SUCCESS;
            break;
        }

        case PAL_ID_PLC_RX_PHY_PARAMS:
            (void *)memcpy(pValue,(void *)&palPlcData.rxParameters, length);
            result = PAL_CFG_SUCCESS;
            break;

        default:
            if (id >= 0xFD00U)
            {
                if (id != 0xFEEEU)
                {
                    uint16_t plcIDaux;
                    plcIDaux = (id & 0x00FFU) | 0x4000U;
                    plcID = (DRV_PLC_PHY_ID)(plcIDaux);
                    askPhy = true;
                }
                else
                {
                    *(uint8_t *)pValue = palPlcData.errorInfo;
                    result = PAL_CFG_SUCCESS;
                    break;
                }
            }
            else
            {
                *(uint8_t *)pValue = 0;
                result = PAL_CFG_SUCCESS;
                break;
            }
            break;
    }

    if(askPhy)
    {
        /* Get in phy layer */
        palPlcData.plcPIB.id = plcID;
        palPlcData.plcPIB.length = length;
        palPlcData.plcPIB.pData = (uint8_t *)pValue;
        (void)DRV_PLC_PHY_PIBGet(palPlcData.drvPhyHandle, &palPlcData.plcPIB);
        result = PAL_CFG_SUCCESS;
    }

    return((uint8_t)result);
}

uint8_t PAL_PLC_SetConfiguration(uint16_t id, void *pValue, uint16_t length)
{
    DRV_PLC_PHY_ID plcID;
    PAL_CFG_RESULT result = PAL_CFG_INVALID_INPUT;
    bool updatePhy = false;

    /* Check identifier */
    switch ((PAL_ATTRIBUTE_ID)id)
    {
        case PAL_ID_CONTINUOUS_TX_EN:
        {
            uint8_t txMode;

            txMode = *(uint8_t *)pValue;
            lPAL_PLC_SetTxContinuousMode(txMode);
            result = PAL_CFG_SUCCESS;
            break;
        }

        case PAL_ID_ZC_PERIOD:
            plcID = PLC_ID_ZC_PERIOD;
            updatePhy = true;
            break;

        case PAL_ID_HOST_VERSION:
            plcID = PLC_ID_HOST_VERSION_ID;
            updatePhy = true;
            break;

        case PAL_ID_CFG_TXRX_DOUBLE_CHANNEL_LIST:
        {
            uint16_t channelList;

            channelList = (*(uint16_t *)pValue);

            palPlcData.channelList &= 0x00FFU;
            palPlcData.channelList |= (channelList << 8);
            result = PAL_CFG_SUCCESS;
            break;
        }

        case PAL_ID_INFO_VERSION:
            plcID = PLC_ID_VERSION_NUM;
            updatePhy = true;
            break;

        case PAL_ID_CFG_AUTODETECT_BRANCH:
            plcID = PLC_ID_CFG_AUTODETECT_IMPEDANCE;
            updatePhy = true;
            break;

        case PAL_ID_CFG_IMPEDANCE:
            plcID = PLC_ID_CFG_IMPEDANCE;
            updatePhy = true;
            break;

        case PAL_ID_CFG_TXRX_CHANNEL:
        {
            uint8_t chn;

            if (palPlcData.status != PAL_PLC_STATUS_READY)
            {
                return ((uint8_t)PAL_CFG_INVALID_INPUT);
            }

            chn = (*(uint8_t *)pValue);
            lPAL_PLC_SetTxRxChannel((DRV_PLC_PHY_CHANNEL)chn);
            result = PAL_CFG_SUCCESS;
            break;
        }

        case PAL_ID_CFG_TXRX_CHANNEL_LIST:
        {
            palPlcData.channelList &= 0xFF00U;
            palPlcData.channelList |= (*(uint8_t *)pValue);
            result = PAL_CFG_SUCCESS;
            break;
        }

        case PAL_ID_RX_PAYLOAD_LEN_SYM:
            plcID = PLC_ID_RX_PAY_SYMBOLS;
            updatePhy = true;
            break;

        case PAL_ID_CFG_MAX_TXRX_NUM_CHANNELS:
        case PAL_ID_INFO_DEVICE:
        case PAL_ID_REMAINING_FRAME_DURATION:
        case PAL_ID_PLC_RX_PHY_PARAMS:
            /* Read only */
            result = PAL_CFG_INVALID_INPUT;
            break;

        case PAL_ID_NETWORK_DETECTION:
            palPlcData.networkDetected = (bool)(*(uint8_t *)pValue);
            result = PAL_CFG_SUCCESS;
            break;

        case PAL_ID_CFG_ATTENUATION:
            palPlcData.palAttenuation = (*(uint8_t *)pValue);
            result = PAL_CFG_SUCCESS;
            break;

        default:
            result = PAL_CFG_INVALID_INPUT;
            break;
    }

    if(updatePhy)
    {
        /* Set in phy layer */
        palPlcData.plcPIB.id = plcID;
        palPlcData.plcPIB.length = length;
        palPlcData.plcPIB.pData = (uint8_t *)pValue;
        (void)DRV_PLC_PHY_PIBSet(palPlcData.drvPhyHandle, &palPlcData.plcPIB);
        result = PAL_CFG_SUCCESS;
    }


    return((uint8_t)result);
}

uint8_t PAL_PLC_GetSNR(uint8_t *pSnr, uint8_t qt)
{
    if (qt == 0U)
    {
        *pSnr = 0;
        return((uint8_t)PAL_CFG_INVALID_INPUT);
    }
    else
    {
        *pSnr = (uint8_t)(qt / 12U + 1U);
        if (*pSnr > 7U)
        {
            *pSnr  = 7;
        }

        return((uint8_t)PAL_CFG_SUCCESS);
    }
}

uint16_t PAL_PLC_GetSignalCapture(uint8_t *pData, PAL_FRAME frameType, uint32_t timeStart, uint32_t duration)
{
    uint8_t *pDataPointer;
    DRV_PLC_PHY_SIGNAL_CAPTURE signalCapture;
    uint8_t captureParameters[9];
    uint8_t index;
    SYS_TIME_HANDLE timer = SYS_TIME_HANDLE_INVALID;

    palPlcData.plcPIB.id = PLC_ID_SIGNAL_CAPTURE_STATUS;
    palPlcData.plcPIB.length = (uint16_t)sizeof(signalCapture);
    palPlcData.plcPIB.pData = (uint8_t *)&signalCapture;
    (void)DRV_PLC_PHY_PIBGet(palPlcData.drvPhyHandle, &palPlcData.plcPIB);

    /* Check status */
    while (signalCapture.status == (uint8_t)SIGNAL_CAPTURE_RUNNING)
    {
        (void)DRV_PLC_PHY_PIBGet(palPlcData.drvPhyHandle, &palPlcData.plcPIB);
        if (SYS_TIME_DelayMS(5, &timer) == SYS_TIME_SUCCESS)
        {
            if(SYS_TIME_DelayIsComplete(timer) != true)
            {
                while (SYS_TIME_DelayIsComplete(timer) == false)
                {
                    /* Empty body */
                }
            }
        }
    }

    /* Start Capture */
    pDataPointer = captureParameters;
    *pDataPointer++ = (uint8_t)(frameType);
    *pDataPointer++ = (uint8_t)(timeStart >> 24U);
    *pDataPointer++ = (uint8_t)(timeStart >> 16U);
    *pDataPointer++ = (uint8_t)(timeStart >> 8U);
    *pDataPointer++ = (uint8_t)(timeStart);
    *pDataPointer++ = (uint8_t)(duration >> 24U);
    *pDataPointer++ = (uint8_t)(duration >> 16U);
    *pDataPointer++ = (uint8_t)(duration >> 8U);
    *pDataPointer++ = (uint8_t)(duration);

    palPlcData.plcPIB.id = PLC_ID_SIGNAL_CAPTURE_START;
    palPlcData.plcPIB.length = (uint16_t)sizeof(captureParameters);
    palPlcData.plcPIB.pData = (uint8_t *)&captureParameters;
    (void)DRV_PLC_PHY_PIBSet(palPlcData.drvPhyHandle, &palPlcData.plcPIB);

    /* Check status */
    palPlcData.plcPIB.id = PLC_ID_SIGNAL_CAPTURE_STATUS;
    palPlcData.plcPIB.length = (uint16_t)sizeof(signalCapture);
    palPlcData.plcPIB.pData = (uint8_t *)&signalCapture;
    (void)DRV_PLC_PHY_PIBGet(palPlcData.drvPhyHandle, &palPlcData.plcPIB);

    while (signalCapture.status != (uint8_t)SIGNAL_CAPTURE_READY)
    {
        (void)DRV_PLC_PHY_PIBGet(palPlcData.drvPhyHandle, &palPlcData.plcPIB);
        if (SYS_TIME_DelayMS(5, &timer) == SYS_TIME_SUCCESS)
        {
            if(SYS_TIME_DelayIsComplete(timer) != true)
            {
                while (SYS_TIME_DelayIsComplete(timer) == false)
                {
                    /* Empty body */
                }
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
        (void)DRV_PLC_PHY_PIBSet(palPlcData.drvPhyHandle, &palPlcData.plcPIB);

        palPlcData.plcPIB.id = PLC_ID_SIGNAL_CAPTURE_DATA;
        palPlcData.plcPIB.length = SIGNAL_CAPTURE_FRAG_SIZE;
        palPlcData.plcPIB.pData = pDataPointer;
        (void)DRV_PLC_PHY_PIBGet(palPlcData.drvPhyHandle, &palPlcData.plcPIB);

        index++;
        pDataPointer += SIGNAL_CAPTURE_FRAG_SIZE;
    }

    ptrdiff_t diff;
    diff = pDataPointer - pData;
    if(diff > 0)
    {
        return (uint16_t)(diff);
    }
    else
    {
        return 0;
    }
}

uint8_t PAL_PLC_GetMsgDuration(uint16_t length, PAL_SCHEME scheme, PAL_FRAME frameType, uint32_t *pDuration)
{
    uint32_t frameDuration;
    uint16_t frameLen;
    uint8_t symbolSize = 0;

    if (length == 0U)
    {
        *pDuration = 0;
        return((uint8_t)PAL_CFG_INVALID_INPUT);
    }

    frameLen = length;

    if (frameType == PAL_FRAME_TYPE_A)
    {
        /* There are 7 bytes inside the header */
        if (frameLen < 7U)
        {
            frameLen = 0;
        }
        else
        {
            frameLen -= 7U;
        }
    }

    symbolSize = palPlcSymbolSize[scheme];
    if (scheme >= PAL_SCHEME_DBPSK_C)
    {
        /* Increase a byte for flushing */
        frameLen++;
    }

    /* Update tx frame duration */
    frameDuration =  (uint32_t)((uint32_t)(frameLen) / symbolSize);
    if ((frameLen % symbolSize) > 0U) {
        frameDuration++;
    }

    /* adjust ROB scheme */
    if (((uint8_t)(scheme) & 0x08U) > 0U) {
        frameDuration <<= 2;
    }

    frameDuration *= PHY_SYMBOL_TIME;
    /* Adjust chirp and header for PHY frame */
    frameDuration += palPlcTimeChirpHeader[frameType];

    *pDuration = frameDuration;

    return((uint8_t)PAL_CFG_SUCCESS);
}

void PAL_PLC_USISnifferCallbackRegister(SRV_USI_HANDLE usiHandler, PAL_USI_SNIFFER_CB callback)
{
    palPlcData.usiHandler = usiHandler;
    palPlcData.snifferCallback = callback;
}

