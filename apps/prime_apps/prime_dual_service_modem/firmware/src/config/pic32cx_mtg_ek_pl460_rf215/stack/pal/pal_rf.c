/*******************************************************************************
  Company:
    Microchip Technology Inc.

  File Name:
    pal_rf.c

  Summary:
    RF Platform Abstraction Layer (PAL) Interface source file.

  Description:
    This module provides the interface between the PRIME MAC layer and the
    RF physical layer.
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
#include "service/time_management/srv_time_management.h"
#include "pal_types.h"
#include "pal_local.h"
#include "pal_rf.h"
#include "pal_rf_local.h"
#include "pal_rf_rm.h"
#include "service/rsniffer/srv_rsniffer.h"
#include "service/log_report/srv_log_report.h"      

/******************************************************************************
 * PRIME PAL RF interface implementation
 ******************************************************************************/
const PAL_INTERFACE PAL_RF_Interface =
{
    .PAL_GetSNR = PAL_RF_GetSNR,
    .PAL_GetZCT = PAL_RF_GetZCT,
    .PAL_GetTimer = PAL_RF_GetTimer,
    .PAL_GetTimerExtended = PAL_RF_GetTimerExtended,
    .PAL_GetCD = PAL_RF_GetCD,
    .PAL_GetNL = PAL_RF_GetNL,
    .PAL_GetAGC = PAL_RF_GetAGC,
    .PAL_SetAGC = PAL_RF_SetAGC,
    .PAL_GetCCA = PAL_RF_GetCCA,
    .PAL_GetChannel = PAL_RF_GetChannel,
    .PAL_SetChannel = PAL_RF_SetChannel,
    .PAL_DataRequest = PAL_RF_DataRequest,
    .PAL_ProgramChannelSwitch = PAL_RF_ProgramChannelSwitch,
    .PAL_GetConfiguration = PAL_RF_GetConfiguration,
    .PAL_SetConfiguration = PAL_RF_SetConfiguration,
    .PAL_GetSignalCapture = PAL_RF_GetSignalCapture,
    .PAL_GetMsgDuration = PAL_RF_GetMsgDuration,
    .PAL_CheckMinimumQuality = PAL_RF_RM_CheckMinimumQuality,
    .PAL_GetLessRobustModulation = PAL_RF_RM_GetLessRobustModulation,
};

// *****************************************************************************
// *****************************************************************************
// Section: File Scope Variables
// *****************************************************************************
// *****************************************************************************
static PAL_RF_DATA palRfData = {0};

// *****************************************************************************
// *****************************************************************************
// Section: File Scope Functions
// *****************************************************************************
// *****************************************************************************
static PAL_CFG_RESULT lPAL_RF_SetTxHandler(DRV_RF215_TX_HANDLE txHandle, uint8_t *pData)
{
    uint8_t index;

    for (index = 0; index < DRV_RF215_TX_BUFFERS_NUMBER; index++)
    {
        if (palRfData.txHandleData[index].pData == NULL)
        {
            palRfData.txHandleData[index].pData = pData;
            palRfData.txHandleData[index].txHandle = txHandle;
            return PAL_CFG_SUCCESS;
        }
    }

    return PAL_CFG_INVALID_INPUT;
}

static PAL_CFG_RESULT lPAL_RF_GetTxHandler(DRV_RF215_TX_HANDLE *txHandle, uint8_t *pData)
{
    uint8_t index;
    
    *txHandle = DRV_RF215_TX_HANDLE_INVALID;

    for (index = 0; index < DRV_RF215_TX_BUFFERS_NUMBER; index++)
    {
        if (palRfData.txHandleData[index].pData == pData)
        {
            *txHandle = palRfData.txHandleData[index].txHandle;

            palRfData.txHandleData[index].pData = NULL;
            palRfData.txHandleData[index].txHandle = DRV_RF215_TX_HANDLE_INVALID;
            return PAL_CFG_SUCCESS;
        }
    }

    return PAL_CFG_INVALID_INPUT;
}

static uint8_t lPAL_RF_GetTxBuffId(DRV_RF215_TX_HANDLE txHandle)
{
    uint8_t index;

    for (index = 0; index < DRV_RF215_TX_BUFFERS_NUMBER; index++)
    {
        if (palRfData.txHandleData[index].txHandle == txHandle)
        {
            return index;
        }
    }

    return 0xFF;
}

static void lPAL_RF_UpdatePhyConfiguration(void)
{
	/* Get PHY configuration */
    DRV_RF215_GetPib(palRfData.drvRfPhyHandle, RF215_PIB_PHY_CONFIG, &palRfData.rfPhyConfig);

	/* Always initialize PAL RF to min channel */
	palRfData.pch = (uint16_t)(PRIME_PAL_RF_CHN_MASK | palRfData.rfPhyConfig.chnNumMin);

	palRfData.rfChannelsNumber = palRfData.rfPhyConfig.chnNumMax - palRfData.rfPhyConfig.chnNumMin + 1;
	if (palRfData.rfPhyConfig.chnNumMin2 != 0xFFFF) 
    {
		palRfData.rfChannelsNumber += palRfData.rfPhyConfig.chnNumMax2 - palRfData.rfPhyConfig.chnNumMin2 + 1;
	}
}

// *****************************************************************************
// *****************************************************************************
// Section: Callback Functions
// *****************************************************************************
// *****************************************************************************
static void lPAL_RF_DataCfmCb(DRV_RF215_TX_HANDLE txHandle, 
						      DRV_RF215_TX_CONFIRM_OBJ *pCfmObj, uintptr_t ctxt)
{
    if (palRfData.rfCallbacks.dataConfirm != NULL)
    {
        PAL_MSG_CONFIRM_DATA dataCfm;

        /* Avoid warning */
        (void)ctxt;

        dataCfm.txTime = SRV_TIME_MANAGEMENT_CountToUS(pCfmObj->timeIniCount);
        dataCfm.pch = palRfData.pch;
        dataCfm.rmsCalc = 255;
        dataCfm.frameType = PAL_FRAME_TYPE_RF;
        dataCfm.bufId = lPAL_RF_GetTxBuffId(txHandle);

        switch(pCfmObj->txResult)
        {
            case RF215_TX_SUCCESS:
                dataCfm.result = PAL_TX_RESULT_SUCCESS;
                break;

            case RF215_TX_BUSY_RX:
                dataCfm.result = PAL_TX_RESULT_BUSY_RX;
                break;

            case RF215_TX_BUSY_CHN:
            case RF215_TX_CANCEL_BY_RX:
                dataCfm.result = PAL_TX_RESULT_BUSY_CH;
                break;

            case RF215_TX_BUSY_TX:
            case RF215_TX_FULL_BUFFERS:
            case RF215_TX_TRX_SLEPT:
                dataCfm.result = PAL_TX_RESULT_BUSY_TX;
                break;

            case RF215_TX_INVALID_LEN:
                dataCfm.result = PAL_TX_RESULT_INV_LENGTH;
                break;

            case RF215_TX_INVALID_DRV_HANDLE:
            case RF215_TX_INVALID_PARAM:
                dataCfm.result = PAL_TX_RESULT_INV_PARAM;
                break;

            case RF215_TX_ERROR_UNDERRUN:
            case RF215_TX_TIMEOUT:
                dataCfm.result = PAL_TX_RESULT_TIMEOUT;
                break;

            case RF215_TX_CANCELLED:
                dataCfm.result = PAL_TX_RESULT_CANCELLED;
                break;

            case RF215_TX_ABORTED:
            default:
                dataCfm.result = PAL_TX_RESULT_PHY_ERROR;
                break;
        }

        palRfData.rfCallbacks.dataConfirm(&dataCfm);
    }

    if (palRfData.snifferCallback)
    {
        uint8_t* pRfSnifferData;
        uint16_t paySymbols;
        size_t dataLength;

        DRV_RF215_GetPib(palRfData.drvRfPhyHandle, RF215_PIB_PHY_TX_PAY_SYMBOLS,
                    &paySymbols);
        
        pRfSnifferData = SRV_RSNIFFER_SerialCfmMessage(pCfmObj, txHandle,
                    &palRfData.rfPhyConfig, paySymbols, palRfData.pch, &dataLength);

        palRfData.snifferCallback(pRfSnifferData, dataLength);
    }

}

static void lPAL_RF_DataIndCb(DRV_RF215_RX_INDICATION_OBJ* pIndObj, uintptr_t ctxt)
{
    if (palRfData.rfCallbacks.dataIndication != NULL)
    {
        PAL_MSG_INDICATION_DATA dataInd;
        DRV_RF215_TX_HANDLE txHandle;

        /* Avoid warning */
        (void)ctxt;

        /* Fill dataInd */
        dataInd.pData = pIndObj->psdu;
        dataInd.rxTime = SRV_TIME_MANAGEMENT_CountToUS(pIndObj->timeIniCount);
        dataInd.dataLength = pIndObj->psduLen;
        dataInd.pch = palRfData.pch;
        PAL_RF_RM_GetRobustModulation(pIndObj, &dataInd.estimatedBitrate, &dataInd.lessRobustMod, dataInd.pch);
        dataInd.rssi = pIndObj->rssiDBm;
        dataInd.scheme = (uint8_t)pIndObj->modScheme;
        dataInd.frameType = (uint8_t)PAL_FRAME_TYPE_RF;
        dataInd.headerType = (dataInd.pData[0] >> 4) & 0x03;
        dataInd.lqi = PAL_RF_RM_GetLqi(dataInd.rssi);

        lPAL_RF_GetTxHandler(&txHandle, dataInd.pData);
        dataInd.bufId = lPAL_RF_GetTxBuffId(txHandle);

        palRfData.rfCallbacks.dataIndication(&dataInd);
    }

    if (palRfData.snifferCallback)
    {
        uint8_t* pRfSnifferData=NULL;
        uint16_t paySymbols=0;
        size_t dataLength=0;

        DRV_RF215_GetPib(palRfData.drvRfPhyHandle, RF215_PIB_PHY_RX_PAY_SYMBOLS,
                    &paySymbols);
        
        pRfSnifferData = SRV_RSNIFFER_SerialRxMessage(pIndObj, &palRfData.rfPhyConfig, 
                    paySymbols, palRfData.pch, &dataLength);
                    
        palRfData.snifferCallback(pRfSnifferData, dataLength);
    }

}

static void lPAL_RF_InitCallback(uintptr_t context, SYS_STATUS status)
{
    if (status == SYS_STATUS_ERROR)
    {
        palRfData.status = PAL_RF_STATUS_ERROR;
        return;
    }

    palRfData.drvRfPhyHandle = DRV_RF215_Open(DRV_RF215_INDEX_0, RF215_TRX_ID_RF09);

    if (palRfData.drvRfPhyHandle == DRV_HANDLE_INVALID)
    {
        SRV_LOG_REPORT_Message_With_Code(SRV_LOG_REPORT_ERROR, 
                (SRV_LOG_REPORT_CODE)PHY_LAYER_RF_NOT_AVAILABLE,
                "PRIME_PAL_RF: RF PHY layer not vailable\r\n");
        palRfData.status = PAL_RF_STATUS_ERROR;
        return;
    }

    /* Register RF PHY driver callbacks */
    DRV_RF215_RxIndCallbackRegister(palRfData.drvRfPhyHandle, lPAL_RF_DataIndCb, 0);
    DRV_RF215_TxCfmCallbackRegister(palRfData.drvRfPhyHandle, lPAL_RF_DataCfmCb, 0);

    palRfData.status = PAL_RF_STATUS_READY;
}

SYS_MODULE_OBJ PAL_RF_Initialize(void)
{
    /* Check previously initialized */
    if (palRfData.status != PAL_RF_STATUS_UNINITIALIZED)
    {
        return SYS_MODULE_OBJ_INVALID;
    }

    palRfData.status = PAL_RF_STATUS_BUSY;
    palRfData.drvRfPhyHandle = DRV_HANDLE_INVALID;

    DRV_RF215_ReadyStatusCallbackRegister(DRV_RF215_INDEX_0, lPAL_RF_InitCallback, 0);

    return (SYS_MODULE_OBJ)DRV_RF215_INDEX_0;
}

SYS_STATUS PAL_RF_Status(void)
{
    /* Return the PAL RF status */
    return ((SYS_STATUS)palRfData.status);
}

void PAL_RF_Tasks(void)
{
    /* Do Nothing */
}

void PAL_RF_DataConfirmCallbackRegister(PAL_DATA_CONFIRM_CB callback)
{
    palRfData.rfCallbacks.dataConfirm = callback;
}

void PAL_RF_DataIndicationCallbackRegister(PAL_DATA_INDICATION_CB callback)
{
    palRfData.rfCallbacks.dataIndication = callback;
}

uint8_t PAL_RF_DataRequest(PAL_MSG_REQUEST_DATA *pMessageData)
{
    DRV_RF215_TX_REQUEST_OBJ *txReqObj = &palRfData.txReqObj[pMessageData->buffId];
    DRV_RF215_TX_HANDLE rfPhyTxReqHandle;
    DRV_RF215_TX_RESULT txResult;

    if (palRfData.status != PAL_RF_STATUS_READY)
    {
        return PAL_CFG_INVALID_INPUT;
    }

    if (pMessageData->timeMode == PAL_TX_MODE_CANCEL)
    {
        if (lPAL_RF_GetTxHandler(&rfPhyTxReqHandle, pMessageData->pData) == PAL_CFG_SUCCESS)
        {
            DRV_RF215_TxCancel(palRfData.drvRfPhyHandle, rfPhyTxReqHandle);
        }

        return (uint8_t)rfPhyTxReqHandle;
    }

    txReqObj->psdu = pMessageData->pData;
    txReqObj->psduLen = pMessageData->dataLength;
    txReqObj->timeMode = (DRV_RF215_TX_TIME_MODE)pMessageData->timeMode;
    txReqObj->txPwrAtt = pMessageData->attLevel;
    txReqObj->modScheme = pMessageData->scheme;
    txReqObj->timeCount = SRV_TIME_MANAGEMENT_USToCount(pMessageData->timeDelay);

    if (pMessageData->disableRx == false)
    {
        /* CSMA used: Energy above threshold and carrier sense CCA Mode */
        txReqObj->ccaMode = PHY_CCA_MODE_3;
        /* Programmed TX canceled once RX frame detected */
        txReqObj->cancelByRx = true;
        /* Set number of CCA */
        txReqObj->ccaContentionWindow = pMessageData->numSenses;
    }
    else
    {
		/* Forced mode: At least Energy above threshold CCA Mode is
		 * needed to comply with RF regulations */
		txReqObj->ccaMode = PHY_CCA_MODE_1;
        /* Programmed TX canceled once RX frame detected */
        txReqObj->cancelByRx = false;
        /* Set number of CCA */
        txReqObj->ccaContentionWindow = pMessageData->numSenses;
    }

    rfPhyTxReqHandle = DRV_RF215_TxRequest(palRfData.drvRfPhyHandle, txReqObj, &txResult);

    if (rfPhyTxReqHandle == DRV_RF215_TX_HANDLE_INVALID)
    {
        DRV_RF215_TX_CONFIRM_OBJ pCfmObj;

        pCfmObj.txResult = txResult;
        pCfmObj.timeIniCount = pMessageData->timeDelay;
        pCfmObj.ppduDurationCount = 0;
        lPAL_RF_DataCfmCb(DRV_RF215_TX_HANDLE_INVALID, &pCfmObj, pMessageData->buffId);
    }
    else
    {
        (void)lPAL_RF_SetTxHandler(rfPhyTxReqHandle, pMessageData->pData);

        if (palRfData.snifferCallback)
        {
            SRV_RSNIFFER_SetTxMessage(txReqObj, &palRfData.rfPhyConfig, rfPhyTxReqHandle);
        }

    }

    return (uint8_t)rfPhyTxReqHandle;
}

void PAL_RF_ProgramChannelSwitch(uint32_t timeSync, uint16_t pch, uint8_t timeMode)
{
    (void)timeSync;
    (void)pch;
    (void)timeMode;
}

uint8_t PAL_RF_GetTimer(uint32_t *pTimer)
{
    *pTimer = SRV_TIME_MANAGEMENT_GetTimeUS();

    return PAL_CFG_SUCCESS;
}

uint8_t PAL_RF_GetTimerExtended(uint64_t *pTimeExtended)
{
    *pTimeExtended = SRV_TIME_MANAGEMENT_GetTimeUS64();

    return PAL_CFG_SUCCESS;
}

uint8_t PAL_RF_GetCD(uint8_t *pCD, uint8_t *pRSSI, uint32_t *pTime, uint8_t *pHeader)
{
    (void)pCD;
    (void)pRSSI;
    (void)pTime;
    (void)pHeader;

    return PAL_CFG_INVALID_INPUT;
}

uint8_t PAL_RF_GetZCT(uint32_t *pZcTime)
{
    *pZcTime = 0;

    return PAL_CFG_INVALID_INPUT;
}

uint8_t PAL_RF_GetCCA(uint8_t *channelState)
{
    *channelState = 1;

    return PAL_CFG_INVALID_INPUT;
}

uint8_t PAL_RF_GetNL(uint8_t *pNoise)
{
    *pNoise = 0;

    return PAL_CFG_INVALID_INPUT;
}

uint8_t PAL_RF_GetAGC(uint8_t *pMode, uint8_t *pGain)
{
    *pMode = 0;
    *pGain = 0;

    return(PAL_CFG_INVALID_INPUT);
}

uint8_t PAL_RF_SetAGC(uint8_t mode, uint8_t gain)
{
    (void)(mode);
    (void)(gain);

    return(PAL_CFG_INVALID_INPUT);
}

uint8_t PAL_RF_GetChannel(uint16_t *pPch)
{
    if (palRfData.status != PAL_RF_STATUS_READY)
    {
        return PAL_CFG_INVALID_INPUT;
    }

    *pPch = palRfData.pch | PRIME_PAL_RF_CHN_MASK;

    return PAL_CFG_SUCCESS;
}

uint8_t PAL_RF_SetChannel(uint16_t pch)
{
    uint16_t channel;
	channel = pch & (~PRIME_PAL_RF_CHN_MASK);

    if (palRfData.status != PAL_RF_STATUS_READY)
    {
        return PAL_CFG_INVALID_INPUT;
    }

    if (channel == PRIME_PAL_RF_FREQ_HOPPING_CHANNEL)
    {
        return PAL_CFG_INVALID_INPUT;
    }
    else
    {
        /* Set in RF215 driver */
        if (DRV_RF215_SetPib(palRfData.drvRfPhyHandle, 
            RF215_PIB_PHY_CHANNEL_NUM, &channel) == RF215_PIB_RESULT_SUCCESS)
        {
            palRfData.pch = channel;
            return PAL_CFG_SUCCESS;
        }
    }

    return PAL_CFG_INVALID_INPUT;
}

uint8_t PAL_RF_GetConfiguration(uint16_t id, void *pValue, uint16_t length)
{
    uint16_t drvRfId;

    /* Check identifier */
    switch (id) {
        case PAL_ID_INFO_VERSION:
            drvRfId = RF215_PIB_FW_VERSION;
            break;

        case PAL_ID_CFG_TXRX_CHANNEL:
            return PAL_RF_GetChannel((uint16_t *)pValue);

        case PAL_ID_RX_PAYLOAD_LEN_SYM:
            drvRfId = RF215_PIB_PHY_RX_PAY_SYMBOLS;
            break;

        case PAL_ID_CSMA_RF_SENSE_TIME:
            drvRfId = RF215_PIB_PHY_CCA_ED_DURATION_US;
            break;

        case PAL_ID_UNIT_BACKOFF_PERIOD:
            drvRfId = RF215_PIB_MAC_UNIT_BACKOFF_PERIOD;
            break;

        case PAL_ID_INFO_DEVICE:
            drvRfId = RF215_PIB_DEVICE_ID;
            break;

        case PAL_ID_RF_DEFAULT_SCHEME:
            *(uint8_t *)pValue = (uint8_t)PAL_RF_RM_GetScheme();
            return PAL_CFG_SUCCESS;

        case PAL_ID_RF_NUM_CHANNELS:
            *(uint16_t *)pValue = palRfData.rfChannelsNumber;
            return PAL_CFG_SUCCESS;

        case PAL_ID_MAX_PHY_PACKET_SIZE:
            *(uint16_t *)pValue = DRV_RF215_MAX_PSDU_LEN;
            return PAL_CFG_SUCCESS;

        case PAL_ID_TURNAROUND_TIME:
            drvRfId = RF215_PIB_PHY_TURNAROUND_TIME;
            break;

        case PAL_ID_PHY_TX_POWER:
            *(int16_t *)pValue = 14;
            return PAL_CFG_SUCCESS;

        case PAL_ID_PHY_FSK_FEC_ENABLED:
            if (palRfData.rfPhyConfig.phyType == PHY_TYPE_FSK)
            {
                if (PAL_RF_RM_GetScheme() == PAL_SCHEME_RF_FSK_FEC_ON)
                {
                    *(uint8_t *)pValue = 1;
                }
                else
                {
                    *(uint8_t *)pValue = 0;
                }
                
                return PAL_CFG_SUCCESS;
            }
            return PAL_CFG_INVALID_INPUT;

        case PAL_ID_PHY_FSK_FEC_INTERLEAVING_RSC:
            *(uint8_t *)pValue = false;
            return PAL_CFG_SUCCESS;

        case PAL_ID_PHY_FSK_FEC_SCHEME:
            *(uint16_t *)pValue = 0;
            return PAL_CFG_SUCCESS;

        case PAL_ID_PHY_FSK_PREAMBLE_LENGTH:
            *(uint16_t *)pValue = 8;
            return PAL_CFG_SUCCESS;

        case PAL_ID_PHY_SUN_FSK_SFD:
            *(uint16_t *)pValue = 0;
            return PAL_CFG_SUCCESS;

        case PAL_ID_PHY_FSK_SCRAMBLE_PSDU:
            *(uint8_t *)pValue = true;
            return PAL_CFG_SUCCESS;

        case PAL_ID_PHY_CCA_DURATION:
            drvRfId = RF215_PIB_PHY_CCA_ED_DURATION_SYMBOLS;
            break;

        case PAL_ID_PHY_CCA_THRESHOLD:
            drvRfId = RF215_PIB_PHY_CCA_ED_THRESHOLD_SENSITIVITY;
            break;

        case PAL_ID_RF_PHY_BAND_OPERATING_MODE:
            drvRfId = RF215_PIB_PHY_BAND_OPERATING_MODE;
            break;

        default:
            return PAL_CFG_INVALID_INPUT;
	}

    /* Get in phy layer */
    if(DRV_RF215_GetPib(palRfData.drvRfPhyHandle, drvRfId, pValue) == RF215_PIB_RESULT_SUCCESS)
    {
        return PAL_CFG_SUCCESS;
    }

    return PAL_CFG_INVALID_INPUT;
}

uint8_t PAL_RF_SetConfiguration(uint16_t id, void *pValue, uint16_t length)
{
    uint16_t drvRfId;

    /* Check identifier */
    switch (id) {
	case PAL_ID_INFO_VERSION:
		drvRfId = RF215_PIB_FW_VERSION;
		break;

	case PAL_ID_CFG_TXRX_CHANNEL:
    {
        uint16_t pch = *(uint16_t *)pValue;
        return PAL_RF_SetChannel(pch);
    }

	case PAL_ID_RX_PAYLOAD_LEN_SYM:
		drvRfId = RF215_PIB_PHY_RX_PAY_SYMBOLS;
		break;

	case PAL_ID_PHY_FSK_FEC_ENABLED:
        if (palRfData.rfPhyConfig.phyType == PHY_TYPE_FSK)
        {
            PAL_RF_RM_SetScheme(*(PAL_SCHEME *)pValue);
            return PAL_CFG_SUCCESS;
        }
        return PAL_CFG_INVALID_INPUT;

	case PAL_ID_PHY_CCA_DURATION:
		drvRfId = RF215_PIB_PHY_CCA_ED_DURATION_SYMBOLS;
		break;

	case PAL_ID_PHY_CCA_THRESHOLD:
		drvRfId = RF215_PIB_PHY_CCA_ED_THRESHOLD_SENSITIVITY;
		break;

	case PAL_ID_RF_PHY_BAND_OPERATING_MODE:
		drvRfId = RF215_PIB_PHY_BAND_OPERATING_MODE;
		break;

	case PAL_ID_INFO_DEVICE:
	case PAL_ID_CSMA_RF_SENSE_TIME:
	case PAL_ID_UNIT_BACKOFF_PERIOD:
	case PAL_ID_RF_DEFAULT_SCHEME:
	case PAL_ID_RF_NUM_CHANNELS:
	case PAL_ID_RF_BITS_HOPPING_SEQUENCE:
	case PAL_ID_RF_BITS_BCN_HOPPING_SEQUENCE:
	case PAL_ID_RF_MAC_HOPPING_SEQUENCE_LENGTH:
	case PAL_ID_RF_MAC_HOPPING_BCN_SEQUENCE_LENGTH:
	case PAL_ID_MAX_PHY_PACKET_SIZE:
	case PAL_ID_TURNAROUND_TIME:
	case PAL_ID_PHY_TX_POWER:
	case PAL_ID_PHY_FSK_FEC_INTERLEAVING_RSC:
	case PAL_ID_PHY_FSK_FEC_SCHEME:
	case PAL_ID_PHY_FSK_PREAMBLE_LENGTH:
	case PAL_ID_PHY_SUN_FSK_SFD:
	case PAL_ID_PHY_FSK_SCRAMBLE_PSDU:
		/* Read only */
		return PAL_CFG_INVALID_INPUT;

	default:
		return PAL_CFG_INVALID_INPUT;
	}
    
    /* Set in phy layer */
    if(DRV_RF215_SetPib(palRfData.drvRfPhyHandle, drvRfId, pValue) == RF215_PIB_RESULT_SUCCESS)
    {
        if (drvRfId == RF215_PIB_PHY_BAND_OPERATING_MODE) {
			/* Get PHY configuration and update variables accordingly */
			lPAL_RF_UpdatePhyConfiguration();
		}
        return PAL_CFG_SUCCESS;
    }
    
    return PAL_CFG_INVALID_INPUT;
}

uint8_t PAL_RF_GetMsgDuration(uint16_t length, PAL_SCHEME scheme, PAL_FRAME frameType, uint32_t *pDuration)
{
    // Only Implemented for FSK (FEC OFF): SUN_FSK_BAND_863_870_OPM1
    uint8_t shrSymbol;
    uint8_t symbolsOctet;
    uint8_t phrSymbols;
    uint16_t paySymbols;
    uint16_t symbRateKHz;
    uint16_t totalSymbols;
    uint32_t frameDuration;

    DRV_RF215_PHY_CFG_OBJ phyConfig;

    DRV_RF215_GetPib(palRfData.drvRfPhyHandle, RF215_PIB_PHY_CONFIG, &phyConfig);

    /* SHR (Preamble + SFD): Preamble fixed to 8 octets, SFD 2 octets. 8
	 * symbols per octect (not affected by modulation order and FEC) */
	shrSymbol = 10 << 3;

	/* Compute number of FSK symbols per octet (PHR + Payload) */
	symbolsOctet = 8;

	/* PHR: 2 octets. Symbols depends on modulation order and FEC */
	phrSymbols = symbolsOctet << 1;

	/* Payload: PSDU + tail + padding */
    /* FEC disabled: No tail / padding added */

	/* Payload: PSDU + tail + padding */
	paySymbols = length * symbolsOctet;

	/* Symbol rate in kHz */
	symbRateKHz = 50;

	/* Compute frame duration in us */
	totalSymbols = shrSymbol + phrSymbols + paySymbols;
	frameDuration = ((uint32_t)totalSymbols * 1000) / symbRateKHz;

    *pDuration = frameDuration;
    
    return PAL_CFG_SUCCESS;
}

uint8_t PAL_RF_GetSNR(uint8_t *pSnr, uint8_t qt)
{
    (void)pSnr;
    (void)qt;

    return PAL_CFG_SUCCESS;
}

uint16_t PAL_RF_GetSignalCapture(uint8_t *pData, PAL_FRAME frameType, uint32_t timeStart, uint32_t duration)
{
    (void)pData;
    (void)frameType;
    (void)timeStart;
    (void)duration;

    return PAL_CFG_INVALID_INPUT;
}

void PAL_RF_USISnifferCallbackRegister(SRV_USI_HANDLE usiHandler, PAL_USI_SNIFFER_CB callback)
{
    palRfData.usiHandler = usiHandler;
    palRfData.snifferCallback = callback;
}

