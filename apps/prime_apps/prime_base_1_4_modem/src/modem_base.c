/*******************************************************************************
  Modem Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    modem_base.c

  Summary:
    MODEM : Modem Application for PRIME Base Node

  Description:
    This source file handles the serialization of the PRIME primitives through
    the USI for the Base Node.
*******************************************************************************/

#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "definitions.h"
#include "modem.h"

#define MAX_NUM_MSG_RCV    (1)

#define MAX_LENGTH_BUFF    CL_432_MAX_LENGTH_DATA

/* Buffer used to tx serialization */
static uint8_t appSerialBuf[MAX_LENGTH_BUFF];

const PRIME_API *gPrimeApi;

SRV_USI_HANDLE gUsiHandle=0;

APP_MODEM_STATES modemState;

/* Queue of buffers in rx */
typedef struct APP_MODEM_MSG_RCV_tag
{
    uint16_t len;
    uint8_t dataBuf[MAX_LENGTH_BUFF];
} APP_MODEM_MSG_RCV;

static APP_MODEM_MSG_RCV sAppModemMsgRecv[MAX_NUM_MSG_RCV];

static uint8_t outputMsgRecvIndex;
static uint8_t inputMsgRecvIndex;

/* Data transmission indication variable */
static uint8_t sRxdataIndication;
/* Data reception indication variable */
static uint8_t sTxdataIndication;

static void APP_Modem_SetCallbacks(void);

static void APP_Modem_EstablishIndication(uint16_t conHandle, uint8_t *eui48,
        uint8_t type, uint8_t *data, uint16_t dataLen, uint8_t cfbytes,
        uint8_t ae)
{
    uint16_t serialLen = 0U;

    appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_ESTABLISH_INDICATION_CMD;
    appSerialBuf[serialLen++] = (uint8_t)(conHandle >> 8);
    appSerialBuf[serialLen++] = (uint8_t)(conHandle);
    memcpy(&appSerialBuf[serialLen], eui48, 6);
    serialLen += 6;
    appSerialBuf[serialLen++] = type;
    appSerialBuf[serialLen++] = (uint8_t)(dataLen >> 8);
    appSerialBuf[serialLen++] = (uint8_t)(dataLen);
    memcpy(&appSerialBuf[serialLen], data, dataLen);
    serialLen += dataLen;
    appSerialBuf[serialLen++] = cfbytes;
    appSerialBuf[serialLen++] = ae;

    /* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf,
                         serialLen);
}

static void APP_Modem_EstablishConfirm(uint16_t conHandle,
            MAC_ESTABLISH_CONFIRM_RESULT result, uint8_t *eui48, uint8_t type,
            uint8_t *data, uint16_t dataLen, uint8_t ae)
{
    uint16_t serialLen = 0U;

    appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_ESTABLISH_CONFIRM_CMD;
    appSerialBuf[serialLen++] = (uint8_t)(conHandle >> 8);
    appSerialBuf[serialLen++] = (uint8_t)(conHandle);
    appSerialBuf[serialLen++] = result;
    memcpy(&appSerialBuf[serialLen], eui48, 6);
    serialLen += 6;
    appSerialBuf[serialLen++] = type;
    appSerialBuf[serialLen++] = (uint8_t)(dataLen >> 8);
    appSerialBuf[serialLen++] = (uint8_t)(dataLen);
    memcpy(&appSerialBuf[serialLen], data, dataLen);
    serialLen += dataLen;
    appSerialBuf[serialLen++] = ae;

    /* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf,
                         serialLen);
}

static void APP_Modem_ReleaseIndication(uint16_t conHandle,
                        MAC_RELEASE_INDICATION_REASON reason)
{
    uint16_t serialLen = 0U;

    appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_RELEASE_INDICATION_CMD;
    appSerialBuf[serialLen++] = (uint8_t)(conHandle >> 8);
    appSerialBuf[serialLen++] = (uint8_t)(conHandle);
    appSerialBuf[serialLen++] = reason;

    /* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf,
                         serialLen);
}

static void APP_Modem_ReleaseConfirm(uint16_t conHandle,
                                     MAC_RELEASE_CONFIRM_RESULT result)
{
    uint16_t serialLen = 0U;

    appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_RELEASE_CONFIRM_CMD;
    appSerialBuf[serialLen++] = (uint8_t)(conHandle >> 8);
    appSerialBuf[serialLen++] = (uint8_t)(conHandle);
    appSerialBuf[serialLen++] = result;

    /* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf,
                         serialLen);
}

static void APP_Modem_JoinIndication(uint16_t conHandle,
        uint8_t *eui48, uint8_t conType, uint8_t *data, uint16_t dataLen,
        uint8_t ae)
{
    uint16_t serialLen = 0U;

    appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_JOIN_INDICATION_CMD;
    appSerialBuf[serialLen++] = (uint8_t)(conHandle >> 8);
    appSerialBuf[serialLen++] = (uint8_t)(conHandle);
    memcpy(&appSerialBuf[serialLen], eui48, 6);
    serialLen += 6;
    appSerialBuf[serialLen++] = conType;
    appSerialBuf[serialLen++] = (uint8_t)(dataLen >> 8);
    appSerialBuf[serialLen++] = (uint8_t)(dataLen);
    memcpy(&appSerialBuf[serialLen], data, dataLen);
    serialLen += dataLen;
    appSerialBuf[serialLen++] = ae;

    /* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf,
                         serialLen);
}

static void APP_Modem_JoinConfirm(uint16_t conHandle,
        MAC_JOIN_CONFIRM_RESULT result, uint8_t ae)
{
    uint16_t serialLen = 0U;

    appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_JOIN_CONFIRM_CMD;
    appSerialBuf[serialLen++] = (uint8_t)(conHandle >> 8);
    appSerialBuf[serialLen++] = (uint8_t)(conHandle);
    appSerialBuf[serialLen++] = result;
    appSerialBuf[serialLen++] = ae;

    /* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf,
                         serialLen);
}

static void APP_Modem_LeaveConfirm(uint16_t conHandle,
                                   MAC_LEAVE_CONFIRM_RESULT result)
{
    uint16_t serialLen = 0U;

    appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_LEAVE_CONFIRM_CMD;
    appSerialBuf[serialLen++] = (uint8_t)(conHandle >> 8);
    appSerialBuf[serialLen++] = (uint8_t)(conHandle);
    appSerialBuf[serialLen++] = result;

    /* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf,
                         serialLen);
}

static void APP_Modem_LeaveIndication(uint16_t conHandle, uint8_t *eui48)
{
    uint16_t serialLen = 0U;

    appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_LEAVE_INDICATION_CMD;
    appSerialBuf[serialLen++] = (uint8_t)(conHandle >> 8);
    appSerialBuf[serialLen++] = (uint8_t)(conHandle);
    if(eui48 != NULL)
    {
        memcpy(&appSerialBuf[serialLen], eui48, 6);
    }
    else
    {
        memset(&appSerialBuf[serialLen], 0xff, 6);
    }

    serialLen += 6;

    /* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf,
                         serialLen);
}

static void APP_Modem_DataConfirm(uint16_t conHandle, uint8_t *dataBuf,
            MAC_DATA_CONFIRM_RESULT result)
{
    uint8_t serialLen = 0U;

    appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_DATA_CONFIRM_CMD;
    appSerialBuf[serialLen++] = (uint8_t)(conHandle >> 8);
    appSerialBuf[serialLen++] = (uint8_t)(conHandle);
    appSerialBuf[serialLen++] = (uint8_t)((uint32_t)dataBuf >> 24);
    appSerialBuf[serialLen++] = (uint8_t)((uint32_t)dataBuf >> 16);
    appSerialBuf[serialLen++] = (uint8_t)((uint32_t)dataBuf >> 8);
    appSerialBuf[serialLen++] = (uint8_t)((uint32_t)dataBuf);
    appSerialBuf[serialLen++] = result;

    /* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf,
                         serialLen);
}

static void APP_Modem_DataIndication(uint16_t conHandle,
        uint8_t *data, uint16_t dataLen, uint32_t timeRef)
{
    uint16_t serialLen = 0U;

    appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_DATA_INDICATION_CMD;
    appSerialBuf[serialLen++] = (uint8_t)(conHandle >> 8);
    appSerialBuf[serialLen++] = (uint8_t)(conHandle);
    appSerialBuf[serialLen++] = (uint8_t)(dataLen >> 8);
    appSerialBuf[serialLen++] = (uint8_t)(dataLen);
    memcpy(&appSerialBuf[serialLen], data, dataLen);
    serialLen += dataLen;
    appSerialBuf[serialLen++] = (uint8_t)(timeRef >> 24);
    appSerialBuf[serialLen++] = (uint8_t)(timeRef >> 16);
    appSerialBuf[serialLen++] = (uint8_t)(timeRef >> 8);
    appSerialBuf[serialLen++] = (uint8_t)(timeRef);

    /* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf,
                         serialLen);

    /* Rx data indication */
    sRxdataIndication = true;
}

static void APP_Modem_PLME_ResetConfirm(PLME_RESULT result, uint16_t pch)
{
    uint16_t serialLen = 0U;

    appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_PLME_RESET_CONFIRM_CMD;
    appSerialBuf[serialLen++] = result;
    appSerialBuf[serialLen++] = (uint8_t)(pch >> 8);
    appSerialBuf[serialLen++] = (uint8_t)pch;

    /* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf,
                         serialLen);
}

static void APP_Modem_PLME_SleepConfirm(PLME_RESULT result, uint16_t pch)
{
    uint16_t serialLen = 0U;

    appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_PLME_SLEEP_CONFIRM_CMD;
    appSerialBuf[serialLen++] = result;
    appSerialBuf[serialLen++] = (uint8_t)(pch >> 8);
    appSerialBuf[serialLen++] = (uint8_t)pch;

    /* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf,
                         serialLen);
}

static void APP_Modem_PLME_ResumeConfirm(PLME_RESULT result, uint16_t pch)
{
    uint16_t serialLen = 0U;

    appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_PLME_RESUME_CONFIRM_CMD;
    appSerialBuf[serialLen++] = result;
    appSerialBuf[serialLen++] = (uint8_t)(pch >> 8);
    appSerialBuf[serialLen++] = (uint8_t)pch;

    /* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf,
                         serialLen);
}

static void APP_Modem_PLME_GetConfirm(PLME_RESULT status,
        uint16_t pibAttrib, void *pibValue, uint8_t pibSize, uint16_t pch)
{
    uint16_t serialLen = 0U;
    uint16_t temp16;
    uint32_t temp32;

    appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_PLME_GET_CONFIRM_CMD;
    appSerialBuf[serialLen++] = status;
    appSerialBuf[serialLen++] = (uint8_t)(pibAttrib >> 8);
    appSerialBuf[serialLen++] = (uint8_t)(pibAttrib);
    appSerialBuf[serialLen++] = pibSize;

    /* Check size */
    switch (pibSize)
    {
        case 2:
            /* Extract value */
            temp16 = *((uint16_t *)pibValue);
            /* Copy value into buffer with MSB in MSB */
            appSerialBuf[serialLen++] = (uint8_t)(temp16 >> 8);
            appSerialBuf[serialLen++] = (uint8_t)temp16;
            break;

        case 4:
            temp32 = *((uint32_t *)pibValue);
            /* Copy value into buffer with MSB in MSB */
            appSerialBuf[serialLen++] = (uint8_t)(temp32 >> 24);
            appSerialBuf[serialLen++] = (uint8_t)(temp32 >> 16);
            appSerialBuf[serialLen++] = (uint8_t)(temp32 >> 8);
            appSerialBuf[serialLen++] = (uint8_t)temp32;
            break;

        default:
            /* Copy value into buffer */
            memcpy(&appSerialBuf[serialLen], (uint8_t *)pibValue, pibSize);
            /* Increase pointer */
            serialLen += pibSize;
    }

    appSerialBuf[serialLen++] = (uint8_t)(pch >> 8);
    appSerialBuf[serialLen++] = (uint8_t)pch;

    /* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf,
                         serialLen);
}

static void APP_Modem_PLME_SetConfirm(PLME_RESULT result, uint16_t pch)
{
    uint16_t serialLen = 0U;

    appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_PLME_SET_CONFIRM_CMD;
    appSerialBuf[serialLen++] = result;
    appSerialBuf[serialLen++] = (uint8_t)(pch >> 8);
    appSerialBuf[serialLen++] = (uint8_t)pch;

    /* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf,
                         serialLen);
}

static void APP_Modem_MLME_PromoteConfirm(MLME_RESULT result)
{
    uint16_t serialLen = 0U;

    appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_MLME_PROMOTE_CONFIRM_CMD;
    appSerialBuf[serialLen++] = result;

    /* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf,
                         serialLen);
}

static void APP_Modem_MLME_MP_PromoteConfirm(MLME_RESULT result)
{
    uint16_t serialLen = 0U;

    appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_MLME_MP_PROMOTE_CONFIRM_CMD;
    appSerialBuf[serialLen++] = result;

    /* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf,
                         serialLen);
}

static void APP_Modem_MLME_ResetConfirm(MLME_RESULT result)
{
    uint16_t serialLen = 0U;

    /* Check result */
    if (result == MLME_RESULT_DONE)
    {
        /* Set callback functions */
        APP_Modem_SetCallbacks();
    }

    appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_MLME_RESET_CONFIRM_CMD;
    appSerialBuf[serialLen++] = result;

    /* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf,
                         serialLen);
}

static void APP_Modem_MLME_GetConfirm(MLME_RESULT status, uint16_t pibAttrib,
                                      void *pibValue, uint8_t pibSize)
{
    uint16_t serialLen = 0U;
    uint16_t temp16;
    uint32_t temp32;

    appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_MLME_GET_CONFIRM_CMD;
    appSerialBuf[serialLen++] = status;
    appSerialBuf[serialLen++] = (uint8_t)(pibAttrib >> 8);
    appSerialBuf[serialLen++] = (uint8_t)(pibAttrib);
    appSerialBuf[serialLen++] = pibSize;

    /* Check size */
    switch (pibSize)
    {
        case 2:
            /* Extract value */
            temp16 = *((uint16_t *)pibValue);
            /* Copy value into buffer with MSB in MSB */
            appSerialBuf[serialLen++] = (uint8_t)(temp16 >> 8);
            appSerialBuf[serialLen++] = (uint8_t)temp16;
            break;

        case 4:
            temp32 = *((uint32_t *)pibValue);
            /* Copy value into buffer with MSB in MSB */
            appSerialBuf[serialLen++] = (uint8_t)(temp32 >> 24);
            appSerialBuf[serialLen++] = (uint8_t)(temp32 >> 16);
            appSerialBuf[serialLen++] = (uint8_t)(temp32 >> 8);
            appSerialBuf[serialLen++] = (uint8_t)temp32;
            break;

        default:
            /* Copy value into buffer */
            memcpy(&appSerialBuf[serialLen], (uint8_t *)pibValue, pibSize);
            /* Increase pointer */
            serialLen += pibSize;
    }

    /* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf,
                         serialLen);
}

static void APP_Modem_MLME_ListGetConfirm(MLME_RESULT status, uint16_t pibAttrib,
                                          uint8_t *pibBuff, uint16_t pibLen)
{
    uint16_t serialLen = 0U;

    appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_MLME_LIST_GET_CONFIRM_CMD;
    appSerialBuf[serialLen++] = status;
    appSerialBuf[serialLen++] = (uint8_t)(pibAttrib >> 8);
    appSerialBuf[serialLen++] = (uint8_t)(pibAttrib);
    appSerialBuf[serialLen++] = (uint8_t)(pibLen >> 8);
    appSerialBuf[serialLen++] = (uint8_t)(pibLen);
    memcpy(&appSerialBuf[serialLen], (uint8_t *)pibBuff, pibLen);
    serialLen += pibLen;

    /* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf,
                         serialLen);
}

static void APP_Modem_MLME_SetConfirm(MLME_RESULT result)
{
    uint16_t serialLen = 0U;

    appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_MLME_SET_CONFIRM_CMD;
    appSerialBuf[serialLen++] = result;

    /* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf,
                         serialLen);
}

static void APP_Modem_CL432_DlDataIndication(uint8_t dstLsap, uint8_t srcLsap,
        uint16_t dstAddress, uint16_t srcAddress, uint8_t *data,
        uint16_t lsduLen, uint8_t linkClass)
{
    uint16_t serialLen = 0U;

    appSerialBuf[serialLen++] = APP_MODEM_CL_432_DL_DATA_INDICATION_CMD;
    appSerialBuf[serialLen++] = dstLsap;
    appSerialBuf[serialLen++] = srcLsap;
    appSerialBuf[serialLen++] = (uint8_t)(dstAddress >> 8);
    appSerialBuf[serialLen++] = (uint8_t)(dstAddress);
    appSerialBuf[serialLen++] = (uint8_t)(srcAddress >> 8);
    appSerialBuf[serialLen++] = (uint8_t)(srcAddress);
    appSerialBuf[serialLen++] = (uint8_t)(lsduLen >> 8);
    appSerialBuf[serialLen++] = (uint8_t)(lsduLen);
    memcpy(&appSerialBuf[serialLen], data, lsduLen);
    serialLen += lsduLen;
    appSerialBuf[serialLen++] = linkClass;

    /* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf,
                         serialLen);

    /* Rx data indication */
    sRxdataIndication = true;
}

static void APP_Modem_CL432_DlDataConfirm(uint8_t dstLsap, uint8_t srcLsap,
        uint16_t dstAddress, DL_432_TX_STATUS txStatus)
{
    uint16_t serialLen = 0U;

    appSerialBuf[serialLen++] = APP_MODEM_CL_432_DL_DATA_CONFIRM_CMD;
    appSerialBuf[serialLen++] = dstLsap;
    appSerialBuf[serialLen++] = srcLsap;
    appSerialBuf[serialLen++] = (uint8_t)(dstAddress >> 8);
    appSerialBuf[serialLen++] = (uint8_t)(dstAddress);
    appSerialBuf[serialLen++] = (uint8_t)txStatus;

    /* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf,
                         serialLen);
}

static void APP_Modem_CL432_JoinIndication(uint8_t *deviceId,
        uint8_t deviceIdLen, uint16_t dstAddress, uint8_t *mac, uint8_t ae)
{
    uint16_t serialLen = 0U;
    uint8_t temp;

    appSerialBuf[serialLen++] = APP_MODEM_CL_432_DL_JOIN_INDICATION_CMD;
    appSerialBuf[serialLen++] = deviceIdLen;
    for (temp = 0; temp < deviceIdLen; temp++)
    {
        appSerialBuf[serialLen++] = *deviceId++;
    }

    appSerialBuf[serialLen++] = (uint8_t)(dstAddress >> 8);
    appSerialBuf[serialLen++] = (uint8_t)(dstAddress);
    for (temp = 0; temp < 8; temp++)
    {
        appSerialBuf[serialLen++] = *mac++;
    }

    appSerialBuf[serialLen++] = ae;

    /* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf,
                         serialLen);
}

static void APP_Modem_CL432_LeaveIndication(uint16_t dstAddress)
{
    uint16_t serialLen = 0U;

    appSerialBuf[serialLen++] = APP_MODEM_CL_432_DL_LEAVE_INDICATION_CMD;
    appSerialBuf[serialLen++] = (uint8_t)(dstAddress >> 8);
    appSerialBuf[serialLen++] = (uint8_t)(dstAddress);

    /* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf,
                         serialLen);
}

static void APP_Modem_BMNG_FupAck(uint8_t cmd, BMNG_FUP_ACK ackCode,
        uint16_t extraInfo)
{
    uint16_t serialLen = 0U;

    appSerialBuf[serialLen++] = APP_MODEM_BMNG_FUP_ACK_CMD;
    appSerialBuf[serialLen++] = cmd;
    appSerialBuf[serialLen++] = ackCode;
    appSerialBuf[serialLen++] = (uint8_t)(extraInfo >> 8);
    appSerialBuf[serialLen++] = (uint8_t)(extraInfo);

    /* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf,
                         serialLen);
}

static void APP_Modem_BMNG_FupStatusIndication(BMNG_FUP_NODE_STATE fupNodeState,
        uint16_t pages, uint8_t *eui48)
{
    uint16_t serialLen = 0U;

    appSerialBuf[serialLen++] = APP_MODEM_BMNG_FUP_STATUS_INDICATION_CMD;
    appSerialBuf[serialLen++] = fupNodeState;
    appSerialBuf[serialLen++] = (uint8_t)(pages >> 8);
    appSerialBuf[serialLen++] = (uint8_t)(pages);
    memcpy(&appSerialBuf[serialLen], eui48, 6);
    serialLen += 6;

    /* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf,
                         serialLen);
}

static void APP_Modem_BMNG_FupErrorIndication(BMNG_FUP_ERROR errorCode,
                                              uint8_t *eui48)
{
    uint16_t serialLen = 0U;


    appSerialBuf[serialLen++] = APP_MODEM_BMNG_FUP_STATUS_ERROR_INDICATION_CMD;
    appSerialBuf[serialLen++] = errorCode;
    memcpy(&appSerialBuf[serialLen], eui48, 6);
    serialLen += 6;

    /* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf,
                         serialLen);
}

static void APP_Modem_BMNG_FupVersionIndication(uint8_t *eui48,
        uint8_t vendorLen, char *vendor, uint8_t modelLen, char *model,
		uint8_t versionlLen, char *version)
{
    uint16_t serialLen = 0U;

    appSerialBuf[serialLen++] = APP_MODEM_BMNG_FUP_VERSION_INDICATION_CMD;
    memcpy(&appSerialBuf[serialLen], eui48, 6);
    serialLen += 6;
    appSerialBuf[serialLen++] = vendorLen;
    memcpy(&appSerialBuf[serialLen], vendor, vendorLen);
    serialLen += vendorLen;
    appSerialBuf[serialLen++] = modelLen;
    memcpy(&appSerialBuf[serialLen], model, modelLen);
    serialLen += modelLen;
    appSerialBuf[serialLen++] = versionlLen;
    memcpy(&appSerialBuf[serialLen], version, versionlLen);
    serialLen += versionlLen;

    /* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf,
                         serialLen);
}

static void APP_Modem_BMNG_FupKillIndication(uint8_t *eui48)
{
    uint16_t serialLen = 0U;

    appSerialBuf[serialLen++] = APP_MODEM_BMNG_FUP_KILL_INDICATION_CMD;
    memcpy(&appSerialBuf[serialLen], eui48, 6);
    serialLen += 6;

    /* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf,
                         serialLen);
}

static void APP_Modem_BMNG_NetEventIndication(BMNG_NET_EVENT_INFO *netEvent)
{
    uint16_t serialLen = 0U;

    appSerialBuf[serialLen++] = APP_MODEM_BMNG_NETWORK_EVENT_CMD;
    appSerialBuf[serialLen++] = netEvent->netEvent;
    memcpy(&appSerialBuf[serialLen], netEvent->eui48, 6);
    serialLen += 6;
    appSerialBuf[serialLen++] = netEvent->sid;
    appSerialBuf[serialLen++] = (uint8_t)(netEvent->lnid >> 8);
    appSerialBuf[serialLen++] = (uint8_t)(netEvent->lnid);
    appSerialBuf[serialLen++] = netEvent->lsid;
    appSerialBuf[serialLen++] = netEvent->alvRxCnt;
    appSerialBuf[serialLen++] = netEvent->alvTxCnt;
    appSerialBuf[serialLen++] = netEvent->alvTime;
    appSerialBuf[serialLen++] = (uint8_t)(netEvent->pch >> 8);
    appSerialBuf[serialLen++] = (uint8_t)(netEvent->pch);
    appSerialBuf[serialLen++] = (uint8_t)(netEvent->pchLsid >> 8);
    appSerialBuf[serialLen++] = (uint8_t)(netEvent->pchLsid);

    /* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf,
                         serialLen);
}

static void APP_Modem_BMNG_PprofAck(uint8_t cmd, BMNG_PPROF_ACK ackCode)
{
    uint16_t serialLen = 0U;

    appSerialBuf[serialLen++] = APP_MODEM_BMNG_PPROF_ACK_CMD;
    appSerialBuf[serialLen++] = cmd;
    appSerialBuf[serialLen++] = ackCode;

    /* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf,
                         serialLen);
}

static void APP_Modem_BMNG_PprofGetResponse(uint8_t *eui48, uint16_t dataLen,
                                            uint8_t *data)
{
    uint16_t serialLen = 0U;

    appSerialBuf[serialLen++] = APP_MODEM_BMNG_PPROF_GET_RESPONSE_CMD;
    memcpy(&appSerialBuf[serialLen], eui48, 6);
    serialLen += 6;
    appSerialBuf[serialLen++] = (uint8_t)(dataLen >> 8);
    appSerialBuf[serialLen++] = (uint8_t)(dataLen);
    memcpy(&appSerialBuf[serialLen], data, dataLen);
    serialLen += dataLen;

    /* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf,
                         serialLen);
}

static void APP_Modem_BMNG_PprofGetEnhancedResponse(uint8_t *eui48,
            uint16_t dataLen, uint8_t *data)
{
    uint16_t serialLen = 0U;

    appSerialBuf[serialLen++] = APP_MODEM_BMNG_PPROF_GET_ENHANCED_RESPONSE_CMD;
    memcpy(&appSerialBuf[serialLen], eui48, 6);
    serialLen += 6;
    appSerialBuf[serialLen++] = (uint8_t)(dataLen >> 8);
    appSerialBuf[serialLen++] = (uint8_t)(dataLen);
    memcpy(&appSerialBuf[serialLen], data, dataLen);
    serialLen += dataLen;

    /* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf,
                         serialLen);
}

static void APP_Modem_BMNG_PprofGetZCResponse(uint8_t *eui48, uint8_t zcStatus,
                                              uint32_t zcTime)
{
    uint16_t serialLen = 0U;

    appSerialBuf[serialLen++] = APP_MODEM_BMNG_PPROF_GET_ZC_RESPONSE_CMD;
    memcpy(&appSerialBuf[serialLen], eui48, 6);
    serialLen += 6;
    appSerialBuf[serialLen++] = zcStatus;
    appSerialBuf[serialLen++] = (uint8_t)(zcTime >> 24);
    appSerialBuf[serialLen++] = (uint8_t)(zcTime >> 16);
    appSerialBuf[serialLen++] = (uint8_t)(zcTime >> 8);
    appSerialBuf[serialLen++] = (uint8_t)(zcTime);

    /* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf,
            serialLen);
}

static void APP_Modem_BMNG_PprofDiffZCResponse(uint8_t *eui48,
            uint32_t timeFreq, uint32_t timeDiff)
{
    uint16_t serialLen = 0U;

    appSerialBuf[serialLen++] = APP_MODEM_BMNG_PPROF_ZC_DIFF_RESPONSE_CMD;
    memcpy(&appSerialBuf[serialLen], eui48, 6);
    serialLen += 6;
    appSerialBuf[serialLen++] = (uint8_t)(timeFreq >> 24);
    appSerialBuf[serialLen++] = (uint8_t)(timeFreq >> 16);
    appSerialBuf[serialLen++] = (uint8_t)(timeFreq >> 8);
    appSerialBuf[serialLen++] = (uint8_t)(timeFreq);
    appSerialBuf[serialLen++] = (uint8_t)(timeDiff >> 24);
    appSerialBuf[serialLen++] = (uint8_t)(timeDiff >> 16);
    appSerialBuf[serialLen++] = (uint8_t)(timeDiff >> 8);
    appSerialBuf[serialLen++] = (uint8_t)(timeDiff);

    /* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf,
                         serialLen);
}

static void APP_Modem_BMNG_WhitelistAck(uint8_t cmd, BMNG_WHITELIST_ACK ackCode)
{
    uint16_t serialLen = 0U;

    appSerialBuf[serialLen++] = APP_MODEM_BMNG_WHITELIST_ACK_CMD;
    appSerialBuf[serialLen++] = cmd;
    appSerialBuf[serialLen++] = ackCode;

    /* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf,
                         serialLen);
}

static void APP_Modem_USI_PRIME_ApiHandler(uint8_t *rxMsg, size_t inputLen)
{
    if (!sAppModemMsgRecv[inputMsgRecvIndex].len)
    {
        if (inputLen < MAX_LENGTH_BUFF)
        {
            memcpy(sAppModemMsgRecv[inputMsgRecvIndex].dataBuf, rxMsg, inputLen);
            sAppModemMsgRecv[inputMsgRecvIndex].len = inputLen;

            if (++inputMsgRecvIndex == MAX_NUM_MSG_RCV)
            {
                inputMsgRecvIndex = 0;
            }

            return;
        }
        else
        {
            /* ERROR ,Message too big */
            SRV_LOG_REPORT_Message_With_Code(SRV_LOG_REPORT_WARNING,
                    APP_MODEM_ERR_MSG_TOO_BIG, "ERROR: Message too big\r\n");
        }
    }
    else
    {
        /* Error, RX queue is full */
        SRV_LOG_REPORT_Message_With_Code(SRV_LOG_REPORT_WARNING,
                       APP_MODEM_ERR_QUEUE_FULL, "ERROR: RX queue full\r\n");
    }

    return;
}

static void APP_Modem_SetCallbacks(void)
{
    MAC_CALLBACKS macCallbacks;
    CL_432_CALLBACKS cl432Callbacks;
    BMNG_CALLBACKS bmngCallbacks;

    macCallbacks.mac_data_cfm = APP_Modem_DataConfirm;
    macCallbacks.mac_data_ind = APP_Modem_DataIndication;
    macCallbacks.mac_establish_cfm = APP_Modem_EstablishConfirm;
    macCallbacks.mac_establish_ind = APP_Modem_EstablishIndication;
    macCallbacks.mac_join_cfm = APP_Modem_JoinConfirm;
    macCallbacks.mac_join_ind = APP_Modem_JoinIndication;
    macCallbacks.mac_leave_cfm = APP_Modem_LeaveConfirm;
    macCallbacks.mac_leave_ind = APP_Modem_LeaveIndication;
    macCallbacks.mac_release_cfm = APP_Modem_ReleaseConfirm;
    macCallbacks.mac_release_ind = APP_Modem_ReleaseIndication;

    macCallbacks.mlme_get_cfm = APP_Modem_MLME_GetConfirm;
    macCallbacks.mlme_list_get_cfm = APP_Modem_MLME_ListGetConfirm;
    macCallbacks.mlme_promote_cfm = APP_Modem_MLME_PromoteConfirm;
    macCallbacks.mlme_mp_promote_cfm = APP_Modem_MLME_MP_PromoteConfirm;
    macCallbacks.mlme_reset_cfm = APP_Modem_MLME_ResetConfirm;
    macCallbacks.mlme_set_cfm = APP_Modem_MLME_SetConfirm;

    macCallbacks.plme_get_cfm = APP_Modem_PLME_GetConfirm;
    macCallbacks.plme_reset_cfm = APP_Modem_PLME_ResetConfirm;
    macCallbacks.plme_resume_cfm = APP_Modem_PLME_ResumeConfirm;
    macCallbacks.plme_set_cfm = APP_Modem_PLME_SetConfirm;
    macCallbacks.plme_sleep_cfm = APP_Modem_PLME_SleepConfirm;
    macCallbacks.plme_testmode_cfm = NULL;

    gPrimeApi->MacSetCallbacks(&macCallbacks);

    cl432Callbacks.cl_432_dl_data_cfm = APP_Modem_CL432_DlDataConfirm;
    cl432Callbacks.cl_432_dl_data_ind = APP_Modem_CL432_DlDataIndication;
    cl432Callbacks.cl_432_join_ind = APP_Modem_CL432_JoinIndication;
    cl432Callbacks.cl_432_leave_ind = APP_Modem_CL432_LeaveIndication;

    gPrimeApi->Cl432SetCallbacks(&cl432Callbacks);

    bmngCallbacks.fup_ack = APP_Modem_BMNG_FupAck;
    bmngCallbacks.fup_error_ind = APP_Modem_BMNG_FupErrorIndication;
    bmngCallbacks.fup_kill_ind = APP_Modem_BMNG_FupKillIndication;
    bmngCallbacks.fup_status_ind = APP_Modem_BMNG_FupStatusIndication;
    bmngCallbacks.fup_version_ind = APP_Modem_BMNG_FupVersionIndication;
    bmngCallbacks.network_event_ind = APP_Modem_BMNG_NetEventIndication;
    bmngCallbacks.pprof_ack = APP_Modem_BMNG_PprofAck;
    bmngCallbacks.pprof_get_response = APP_Modem_BMNG_PprofGetResponse;
    bmngCallbacks.pprof_get_enhanced_response =
                                     APP_Modem_BMNG_PprofGetEnhancedResponse;
    bmngCallbacks.pprof_get_zc_response = APP_Modem_BMNG_PprofGetZCResponse;
    bmngCallbacks.pprof_zc_diff_response = APP_Modem_BMNG_PprofDiffZCResponse;
    bmngCallbacks.whitelist_ack = APP_Modem_BMNG_WhitelistAck;

    gPrimeApi->BmngSetCallbacks(&bmngCallbacks);
}

static void APP_Modem_MacEstablishRequestCmd(uint8_t *recvMsg)
{
    uint8_t *lMessage;
    uint8_t eui48[6];
    uint8_t lType;
    uint16_t dataLen;
    uint8_t *data;
    uint8_t lArq;
    uint8_t lCfbytes;
    uint8_t ae;

    /* Extract parameters */
    lMessage = recvMsg;
    memcpy(eui48, lMessage, 6);
    lMessage += 6;
    lType = *lMessage++;
    dataLen = ((uint16_t)(*lMessage++)) << 8;
    dataLen += *lMessage++;
    data = lMessage;
    lMessage += dataLen;
    lArq = *lMessage++;
    lCfbytes = *lMessage++;
    ae = *lMessage++;

    gPrimeApi->MacEstablishRequest(eui48, lType, data, dataLen, lArq,
                                   lCfbytes, ae);
}

static void APP_Modem_MacEstablishResponseCmd(uint8_t *recvMsg)
{
    uint8_t *lMessage;
    uint16_t conHandle;
    uint8_t answer;
    uint16_t dataLen;
    uint8_t *data;
    uint8_t ae;

    /* Extract parameters */
    lMessage = recvMsg;
    conHandle = ((uint16_t)(*lMessage++)) << 8;
    conHandle += *lMessage++;
    answer = *lMessage++;
    dataLen = ((uint16_t)(*lMessage++)) << 8;
    dataLen += *lMessage++;
    data = lMessage;
    lMessage += dataLen;
    ae = *lMessage++;

    gPrimeApi->MacEstablishResponse(conHandle,
            (MAC_ESTABLISH_RESPONSE_ANSWER)answer, data, dataLen, ae);
}

static void APP_Modem_MacRedirectResponseCmd(uint8_t *recvMsg)
{
    uint8_t *lMessage;
    uint16_t conHandle;
    uint8_t eui48[6];
    uint8_t *data;
    uint16_t dataLen;

    lMessage = recvMsg;
    conHandle = ((uint16_t)(*lMessage++)) << 8;
    conHandle += *lMessage++;
    memcpy(eui48, lMessage, 6);
    dataLen = ((uint16_t)(*lMessage++)) << 8;
    dataLen += *lMessage++;
    data = lMessage;

    gPrimeApi->MacRedirectResponse(conHandle, eui48, data, dataLen);
}

static void APP_Modem_MacReleaseRequestCmd(uint8_t *recvMsg)
{
    uint8_t *lMessage;
    uint16_t conHandle;

    /* Extract parameters */
    lMessage = recvMsg;
    conHandle = ((uint16_t)(*lMessage++)) << 8;
    conHandle += *lMessage++;

    gPrimeApi->MacReleaseRequest(conHandle);
}

static void APP_Modem_MacReleaseResponseCmd(uint8_t *recvMsg)
{
    uint8_t *lMessage;
    uint16_t conHandle;
    uint8_t answer;

    /* Extract parameters */
    lMessage = recvMsg;
    conHandle = ((uint16_t)(*lMessage++)) << 8;
    conHandle += *lMessage++;
    answer = *lMessage++;

    gPrimeApi->MacReleaseResponse(conHandle, (MAC_RELEASE_RESPONSE_ANSWER)answer);
}

static void APP_Modem_MacJoinRequestCmd(uint8_t *recvMsg)
{
    uint8_t *lMessage;
    uint8_t  broadcast;
    uint8_t conType;
    uint16_t dataLen;
    uint16_t conHandle;
    uint8_t data[256];
    uint8_t eui48[6];
    uint8_t ae;

    /* Extract parameters */
    lMessage = recvMsg;
    broadcast = *lMessage++;
    conHandle = ((uint16_t)(*lMessage++)) << 8;
    conHandle += *lMessage++;
    memcpy(eui48, lMessage, 6);
    lMessage += 6;
    conType = *lMessage++;
    dataLen = ((uint16_t)(*lMessage++)) << 8;
    dataLen += *lMessage++;
    memcpy(data, lMessage, dataLen);
    lMessage += dataLen;
    ae = *lMessage++;

    gPrimeApi->MacJoinRequest((MAC_JOIN_REQUEST_MODE)broadcast, conHandle,
                                eui48, conType, data, dataLen, ae);
}

static void APP_Modem_MacJoinResponseCmd(uint8_t *recvMsg)
{
    uint8_t *lMessage;
    uint16_t conHandle;
    uint8_t answer;
    uint8_t eui48[6];
    uint8_t ae;

    /* Extract parameters */
    lMessage = recvMsg;
    conHandle = ((uint16_t)(*lMessage++)) << 8;
    conHandle += *lMessage++;
    memcpy(eui48, lMessage, 6);
    lMessage += 6;
    answer = *lMessage++;
    ae = *lMessage++;

    gPrimeApi->MacJoinResponse(conHandle, eui48,
                               (MAC_JOIN_RESPONSE_ANSWER)answer, ae);
}

static void APP_Modem_MacLeaveRequestCmd(uint8_t *recvMsg)
{
    uint8_t *lMessage;
    uint16_t conHandle;
    uint8_t eui48[6];

    /* Extract parameters */
    lMessage = recvMsg;
    conHandle = ((uint16_t)(*lMessage++)) << 8;
    conHandle += *lMessage++;
    memcpy(eui48, lMessage, 6);

    gPrimeApi->MacLeaveRequest(conHandle, eui48);
}


static void APP_Modem_MacDataRequestCmd(uint8_t *recvMsg)
{
    uint8_t *lMessage;
    uint16_t conHandle;
    uint16_t dataLen;
    uint8_t *data;
    uint8_t prio;
    uint32_t time_ref;

    /* Extract parameters */
    lMessage = recvMsg;
    conHandle = ((uint16_t)(*lMessage++)) << 8;
    conHandle += *lMessage++;
    dataLen = ((uint16_t)(*lMessage++)) << 8;
    dataLen += *lMessage++;
    data = lMessage;
    lMessage += dataLen;
    prio = *lMessage++;
    time_ref = ((uint32_t)(*lMessage++)) << 24;
    time_ref += ((uint32_t)(*lMessage++)) << 16;
    time_ref += ((uint32_t)(*lMessage++)) << 8;
    time_ref += *lMessage++;

    gPrimeApi->MacDataRequest(conHandle, data, dataLen, prio, time_ref);

    /* Tx data indication */
    sTxdataIndication = true;
}

static void APP_Modem_PLME_ResetRequestCmd(uint8_t *recvMsg)
{
    uint8_t *lMessage;
    uint16_t pch;

    /* Extract parameters */
    lMessage = recvMsg;
    pch = ((uint16_t)(*lMessage++)) << 8;
    pch += *lMessage++;

    gPrimeApi->PlmeResetRequest(pch);
}

static void APP_Modem_PLME_SleepRequestCmd(uint8_t *recvMsg)
{
    uint8_t *lMessage;
    uint16_t pch;

    /* Extract parameters */
    lMessage = recvMsg;
    pch = ((uint16_t)(*lMessage++)) << 8;
    pch += *lMessage++;

    gPrimeApi->PlmeSleepRequest(pch);
}

static void APP_Modem_PLME_ResumeRequestCmd(uint8_t *recvMsg)
{
    uint8_t *lMessage;
    uint16_t pch;

    /* Extract parameters */
    lMessage = recvMsg;
    pch = ((uint16_t)(*lMessage++)) << 8;
    pch += *lMessage++;

    gPrimeApi->PlmeResumeRequest(pch);
}

static void APP_Modem_PLME_GetRequestCmd(uint8_t *recvMsg)
{
    uint8_t *lMessage;
    uint16_t pibAttrib;
    uint16_t pch;

    /* Extract parameters */
    lMessage = recvMsg;
    pibAttrib = ((uint16_t)(*lMessage++)) << 8;
    pibAttrib += *lMessage++;
    pch = ((uint16_t)(*lMessage++)) << 8;
    pch += *lMessage++;

    gPrimeApi->PlmeGetRequest(pibAttrib, pch);
}

static void APP_Modem_PLME_SetRequestCmd(uint8_t *recvMsg)
{
    uint8_t *lMessage;
    uint16_t pibAttrib;
    uint8_t  pibValueBuf[256];
    uint8_t pibSize;
    void *pibValue;
    uint16_t temp16;
    uint32_t temp32;
    uint16_t pch;

    /* Extract parameters */
    lMessage = recvMsg;
    pibAttrib = ((uint16_t)(*lMessage++)) << 8;
    pibAttrib += *lMessage++;
    pibSize = *lMessage++;
    /* Check PIB size */
    switch (pibSize)
    {
        case 2: /* sizeof(uint16_t) */
            /* Extract PIB value */
            temp16 = ((uint16_t)(*lMessage++)) << 8;
            temp16 += *lMessage++;
            pibValue = (void *)(&temp16);
            break;

        case 4: /* sizeof(uint32_t) */
            /* Extract PIB value */
            temp32 = ((uint32_t)(*lMessage++) << 24);
            temp32 += ((uint32_t)(*lMessage++) << 16);
            temp32 += ((uint32_t)(*lMessage++) << 8);
            temp32 += (uint32_t)*lMessage++;
            pibValue = (void *)(&temp32);
            break;

        case 1: /* sizeof(uint8_t) */
        default: /* arrays */
            memcpy(pibValueBuf, lMessage, pibSize);
            pibValue = (void *)pibValueBuf;
            break;
    }

    pch = ((uint16_t)(*lMessage++)) << 8;
    pch += *lMessage++;

    gPrimeApi->PlmeSetRequest(pibAttrib, pibValue, pibSize, pch);
}

static void APP_Modem_MLME_PromoteRequestCmd(uint8_t *recvMsg)
{
    uint8_t *lMessage;
    uint8_t eui48[6];
    uint8_t bcnMode;

    /* Extract parameters */
    lMessage = recvMsg;
    memcpy(eui48, lMessage, 6);
    lMessage += 6;
	bcnMode = *lMessage++;

    gPrimeApi->MlmePromoteRequest(eui48, bcnMode);
}

static void APP_Modem_MLME_MP_PromoteRequestCmd(uint8_t *recvMsg)
{
    uint8_t *lMessage;
    uint8_t eui48[6];
    uint16_t pch;
    uint8_t bcnMode;

    /* Extract parameters */
    lMessage = recvMsg;
    memcpy(eui48, lMessage, 6);
    lMessage += 6;
    bcnMode = *lMessage++;
    pch = ((uint16_t)(*lMessage++)) << 8;
    pch += *lMessage++;

    gPrimeApi->MlmeMpPromoteRequest(eui48, bcnMode, pch);
}

static void APP_Modem_MLME_GetRequestCmd(uint8_t *recvMsg)
{
    uint8_t *lMessage;
    uint16_t pibAttrib;

    /* Extract parameters */
    lMessage = recvMsg;
    pibAttrib = ((uint16_t)(*lMessage++)) << 8;
    pibAttrib += *lMessage++;

    gPrimeApi->MlmeGetRequest(pibAttrib);
}

static void APP_Modem_MLME_ListGetRequestCmd(uint8_t *recvMsg)
{
    uint8_t *lMessage;
    uint16_t pibAttrib;

    /* Extract parameters */
    lMessage = recvMsg;
    pibAttrib = ((uint16_t)(*lMessage++)) << 8;
    pibAttrib += *lMessage++;

    gPrimeApi->MlmeListGetRequest(pibAttrib);
}

static void APP_Modem_MLME_SetRequestCmd(uint8_t *recvMsg)
{
    uint8_t *lMessage;
    void *pibValue;
    uint16_t pibAttrib;
    uint8_t pibValueBuf[256];
    uint8_t pibSize;
    uint16_t temp16;
    uint32_t temp32;

    /* Extract parameters */
    lMessage = recvMsg;
    pibAttrib = ((uint16_t)(*lMessage++)) << 8;
    pibAttrib += *lMessage++;
    pibSize = *lMessage++;
    /* Check PIB size */
    switch (pibSize)
    {
        case 2: /* sizeof(uint16_t) */
            /* Extract PIB value */
            temp16 = ((uint16_t)(*lMessage++)) << 8;
            temp16 += *lMessage++;
            pibValue = (void *)(&temp16);
            break;

        case 4: /* sizeof(uint32_t) */
            /* Extract PIB value */
            temp32 = ((uint32_t)(*lMessage++)) << 24;
            temp32 += ((uint32_t)(*lMessage++)) << 16;
            temp32 += ((uint32_t)(*lMessage++)) << 8;
            temp32 += *lMessage++;
            pibValue = (void *)(&temp32);
            break;

        case 1: /* sizeof(uint8_t) */
        default: /* arrays */
            memcpy(pibValueBuf, lMessage, pibSize);
            pibValue = (void *)pibValueBuf;
            break;
    }

    gPrimeApi->MlmeSetRequest(pibAttrib, pibValue, pibSize);
}

static void APP_Modem_CL432ReleaseRequestCmd(uint8_t *recvMsg)
{
    uint8_t *lMessage;
    uint16_t dstAddress;

    /* Extract parameters */
    lMessage = recvMsg;
    dstAddress = ((uint16_t)(*lMessage++)) << 8;
    dstAddress += *lMessage++;

    gPrimeApi->Cl432ReleaseRequest(dstAddress);
}

static void APP_Modem_CL432DataRequestCmd(uint8_t *recvMsg)
{
    uint8_t *lMessage;
    uint16_t dstAddress, lsduLen;
    DL_432_BUFFER buff432;
    uint8_t linkClass, dstLsap, srcLsap;

    /* Extract parameters */
    lMessage = recvMsg;
    dstLsap = *lMessage++;
    srcLsap = *lMessage++;
    dstAddress = ((uint16_t)(*lMessage++)) << 8;
    dstAddress += *lMessage++;
    lsduLen = ((uint16_t)(*lMessage++)) << 8;
    lsduLen += *lMessage++;
    if(lsduLen <= CL_432_MAX_LENGTH_DATA)
    {
        memcpy(buff432.dl.buff, lMessage, lsduLen);
        lMessage += lsduLen;
        linkClass = *lMessage++;

        gPrimeApi->Cl432DlDataRequest(dstLsap, srcLsap, dstAddress,
                &buff432, lsduLen, linkClass);
    }

    /* Tx data indication */
    sTxdataIndication = true;
}

static void APP_Modem_BMNG_FupAddTargetRequestCmd(uint8_t *recvMsg)
{
    uint8_t *lMessage;
    uint8_t eui48[6];

    /* Extract parameters */
    lMessage = recvMsg;
    memcpy(eui48, lMessage, 6);

    gPrimeApi->BmngFupAddTargetRequest(APP_MODEM_BMNG_FUP_ADD_TARGET_REQUEST_CMD,
                eui48);
}

static void APP_Modem_BMNG_FupSetFwDataRequestCmd(uint8_t *recvMsg)
{
    uint8_t *lMessage;
    char vendor[32], model[32], version[32];
    uint8_t vendorLen, modelLen, versionlLen;

    /* Extract parameters */
    lMessage = recvMsg;
    vendorLen = *lMessage++;
    memcpy(vendor, lMessage, vendorLen);
    lMessage += vendorLen;
    modelLen = *lMessage++;
    memcpy(model, lMessage, modelLen);
    lMessage += modelLen;
    versionlLen = *lMessage++;
    memcpy(version, lMessage, versionlLen);

    gPrimeApi->BmngFupSetFwDataRequest(
    APP_MODEM_BMNG_FUP_SET_FW_DATA_REQUEST_CMD, vendorLen, vendor, modelLen,
            model, versionlLen, version);
}

static void APP_Modem_BMNG_FupSetUpgOptionRequestCmd(uint8_t *recvMsg)
{
    uint8_t *lMessage;
    uint32_t delayRestart;
    uint32_t safetyTimer;
    BMNG_FUP_PAGE_SIZE pageSize;
    uint8_t arqEn;
    uint8_t multicastEn;

    /* Extract parameters */
    lMessage = recvMsg;
    arqEn = *lMessage++;
    pageSize = (BMNG_FUP_PAGE_SIZE)*lMessage++;
    multicastEn = *lMessage++;
    delayRestart = ((uint32_t)(*lMessage++)) << 24;
    delayRestart += ((uint32_t)(*lMessage++)) << 16;
    delayRestart += ((uint32_t)(*lMessage++)) << 8;
    delayRestart += *lMessage++;
    safetyTimer = ((uint32_t)(*lMessage++)) << 24;
    safetyTimer += ((uint32_t)(*lMessage++)) << 16;
    safetyTimer += ((uint32_t)(*lMessage++)) << 8;
    safetyTimer += *lMessage++;

    gPrimeApi->BmngFupSetUpgradeOptionsRequest(
    APP_MODEM_BMNG_FUP_SET_UPGRADE_REQUEST_CMD, arqEn, pageSize, multicastEn,
            delayRestart, safetyTimer);
}

static void APP_Modem_BMNG_FupInitFileTxRequestCmd(uint8_t *recvMsg)
{
    uint8_t *lMessage;
    uint32_t fileSize, crc;
    uint16_t frameNumber, frameSize;

    /* Extract parameters */
    lMessage = recvMsg;
    frameNumber = ((uint16_t)(*lMessage++)) << 8;
    frameNumber += *lMessage++;
    fileSize = ((uint32_t)(*lMessage++)) << 24;
    fileSize += ((uint32_t)(*lMessage++)) << 16;
    fileSize += ((uint32_t)(*lMessage++)) << 8;
    fileSize += *lMessage++;
    frameSize = ((uint16_t)(*lMessage++)) << 8;
    frameSize += *lMessage++;
    crc = ((uint32_t)(*lMessage++)) << 24;
    crc += ((uint32_t)(*lMessage++)) << 16;
    crc += ((uint32_t)(*lMessage++)) << 8;
    crc += *lMessage++;

    gPrimeApi->BmngFupInitFileTxRequest(
    APP_MODEM_BMNG_FUP_INIT_FILE_TX_REQUEST_CMD, frameNumber, fileSize,
            frameSize, crc);
}

static void APP_Modem_BMNG_FupDataFrameRequestCmd(uint8_t *recvMsg)
{
    uint8_t *lMessage;
    uint8_t data[1000];
    uint16_t frameNumber, dataLen;

    /* Extract parameters */
    lMessage = recvMsg;
    frameNumber = ((uint16_t)(*lMessage++)) << 8;
    frameNumber += *lMessage++;
    dataLen = ((uint16_t)(*lMessage++)) << 8;
    dataLen += *lMessage++;
    memcpy(data, lMessage, dataLen);

    gPrimeApi->BmngFupDataFrameRequest(APP_MODEM_BMNG_FUP_DATA_FRAME_REQUEST_CMD,
                                                frameNumber, dataLen, data);
}

static void APP_Modem_BMNG_FupAbortFuRequestCmd(uint8_t *recvMsg)
{
    uint8_t *lMessage;
    uint8_t eui48[6];

    /* Extract parameters */
    lMessage = recvMsg;
    memcpy(eui48, lMessage, 6);

    gPrimeApi->BmngFupAbortFuRequest(APP_MODEM_BMNG_FUP_ABORT_FU_REQUEST_CMD,
                                        eui48);
}

static void APP_Modem_BMNG_FupStartFuRequestCmd(uint8_t *recvMsg)
{
    uint8_t *lMessage;
    uint8_t enable;

    /* Extract parameters */
    lMessage = recvMsg;
    enable = *lMessage++;

    gPrimeApi->BmngFupStartFuRequest(APP_MODEM_BMNG_FUP_START_FU_REQUEST_CMD,
                                     enable);
}

static void APP_Modem_BMNG_FupSetMatchRuleRequestCmd(uint8_t *recvMsg)
{
    uint8_t *lMessage;
    uint8_t rules;

    /* Extract parameters */
    lMessage = recvMsg;
    rules = *lMessage++;

    gPrimeApi->BmngFupSetMatchRuleRequest(
                    APP_MODEM_BMNG_FUP_SET_MATCH_RULE_REQUEST_CMD, rules);
}

static void APP_Modem_BMNG_FupGetVersionRequestCmd(uint8_t *recvMsg)
{
    uint8_t *lMessage;
    uint8_t eui48[6];

    /* Extract parameters */
    lMessage = recvMsg;
    memcpy(eui48, lMessage, 6);

    gPrimeApi->BmngFupGetVersionRequest(
                            APP_MODEM_BMNG_FUP_GET_VERSION_REQUEST_CMD, eui48);
}

static void APP_Modem_BMNG_FupGetStateRequestCmd(uint8_t *recvMsg)
{
    uint8_t *lMessage;
    uint8_t eui48[6];

    /* Extract parameters */
    lMessage = recvMsg;
    memcpy(eui48, lMessage, 6);

    gPrimeApi->BmngFupGetStateRequest(APP_MODEM_BMNG_FUP_GET_STATE_REQUEST_CMD,
                                        eui48);
}

static void APP_Modem_BMNG_FupSetSigDataRequestCmd(uint8_t *recvMsg)
{
    uint8_t *lMessage;
    uint16_t len;
    uint8_t algorithm;

    /* Extract parameters */
    lMessage = recvMsg;
    algorithm = *lMessage++;
    len = ((uint16_t)(*lMessage++)) << 8;
    len += *lMessage++;

    gPrimeApi->BmngFupSetSignatureDataRequest(
            APP_MODEM_BMNG_FUP_SET_SIGNATURE_DATA_REQUEST_CMD, algorithm, len);
}

static void APP_Modem_BMNG_PprofGetRequestCmd(uint8_t *recvMsg)
{
    uint8_t *lMessage;
    uint8_t data[1024];
    uint8_t eui48[6];
    uint16_t dataLen;

    /* Extract parameters */
    lMessage = recvMsg;
    memcpy(eui48, lMessage, 6);
    lMessage += 6;
    dataLen = ((uint16_t)(*lMessage++)) << 8;
    dataLen += *lMessage++;
    memcpy(data, lMessage, dataLen);

    gPrimeApi->BmngPprofGetRequest(APP_MODEM_BMNG_PPROF_GET_REQUEST_CMD, eui48,
                                   dataLen, data);
}

static void APP_Modem_BMNG_PprofSetRequestCmd(uint8_t *recvMsg)
{
    uint8_t *lMessage;
    uint8_t data[1024];
    uint8_t eui48[6];
    uint16_t dataLen;

    /* Extract parameters */
    lMessage = recvMsg;
    memcpy(eui48, lMessage, 6);
    lMessage += 6;
    dataLen = ((uint16_t)(*lMessage++)) << 8;
    dataLen += *lMessage++;
    memcpy(data, lMessage, dataLen);

    gPrimeApi->BmngPprofSetRequest(APP_MODEM_BMNG_PPROF_SET_REQUEST_CMD, eui48,
                                   dataLen, data);
}

static void APP_Modem_BMNG_PprofResetRequestCmd(uint8_t *recvMsg)
{
    uint8_t *lMessage;
    uint8_t eui48[6];

    /* Extract parameters */
    lMessage = recvMsg;
    memcpy(eui48, lMessage, 6);

    gPrimeApi->BmngPprofResetRequest(APP_MODEM_BMNG_PPROF_RESET_REQUEST_CMD,
                                     eui48);
}

static void APP_Modem_BMNG_PprofRebootRequestCmd(uint8_t *recvMsg)
{
    uint8_t *lMessage;
    uint8_t eui48[6];

    /* Extract parameters */
    lMessage = recvMsg;
    memcpy(eui48, lMessage, 6);

    gPrimeApi->BmngPprofRebootRequest(APP_MODEM_BMNG_PPROF_REBOOT_REQUEST_CMD,
                                      eui48);
}

static void APP_Modem_BMNG_PprofGetEnhancedRequestCmd(uint8_t *recvMsg)
{
    uint8_t *lMessage;
    uint8_t data[1024];
    uint8_t eui48[6];
    uint16_t dataLen;

    /* Extract parameters */
    lMessage = recvMsg;
    memcpy(eui48, lMessage, 6);
    lMessage += 6;
    dataLen = ((uint16_t)(*lMessage++)) << 8;
    dataLen += *lMessage++;
    memcpy(data, lMessage, dataLen);

    gPrimeApi->BmngPprofGetEnhancedRequest(
        APP_MODEM_BMNG_PPROF_GET_ENHANCED_RESPONSE_CMD, eui48, dataLen, data);
}

static void APP_Modem_BMNG_PprofZcDiffRequestCmd(uint8_t *recvMsg)
{
    uint8_t *lMessage;
    uint8_t eui48[6];

    /* Extract parameters */
    lMessage = recvMsg;
    memcpy(eui48, lMessage, 6);

    gPrimeApi->BmngPprofGetZcDiffRequest(
                        APP_MODEM_BMNG_PPROF_ZC_DIFF_REQUEST_CMD, eui48);
}

static void APP_Modem_BMNG_WhitelistAddRequestCmd(uint8_t *recvMsg)
{
    uint8_t *lMessage;
    uint8_t eui48[6];

    /* Extract parameters */
    lMessage = recvMsg;
    memcpy(eui48, lMessage, 6);

    gPrimeApi->BmngWhitelistAddRequest(
                APP_MODEM_BMNG_WHITELIST_ADD_REQUEST_CMD, eui48);
}

static void APP_Modem_BMNG_WhitelistRemoveRequestCmd(uint8_t *recvMsg)
{
    uint8_t *lMessage;
    uint8_t eui48[6];

    /* Extract parameters */
    lMessage = recvMsg;
    memcpy(eui48, lMessage, 6);

    gPrimeApi->BmngWhitelistRemoveRequest(
               APP_MODEM_BMNG_WHITELIST_REMOVE_REQUEST_CMD, eui48);
}

void APP_Modem_Initialize(void)
{
    /* Initialize the reception queue */
    inputMsgRecvIndex = 0;
    outputMsgRecvIndex = 0;

    (void) memset(sAppModemMsgRecv, 0, sizeof(sAppModemMsgRecv));

    /* Initialize TxRx data indicators */
    sRxdataIndication = false;
    sTxdataIndication = false;

    /* Get PRIME API pointer */
    PRIME_API_GetPrimeAPI(&gPrimeApi);

    /* Set state */
    modemState = APP_MODEM_STATE_CONFIGURE;
}

void APP_Modem_Tasks(void)
{
    switch (modemState)
    {
        case APP_MODEM_STATE_CONFIGURE:
            /* Check if PRIME stack is ready */
            if (gPrimeApi->Status() == SYS_STATUS_READY)
            {
                /* Set callback functions */
                APP_Modem_SetCallbacks();

                /* Open USI */
                gUsiHandle = SRV_USI_Open(SRV_USI_INDEX_0);

                /* Configure USI protocol handler */
                SRV_USI_CallbackRegister(gUsiHandle, SRV_USI_PROT_ID_PRIME_API,
                                         APP_Modem_USI_PRIME_ApiHandler);

                modemState = APP_MODEM_STATE_TASKS;
            }

            break;

        case APP_MODEM_STATE_TASKS:
            /* Check data reception */
            while(sAppModemMsgRecv[outputMsgRecvIndex].len)
            {
                APP_MODEM_PRIME_API_CMD apiCmd;
                uint8_t *recvBuf;

                /* Extract command */
                recvBuf = sAppModemMsgRecv[outputMsgRecvIndex].dataBuf;
                apiCmd = (APP_MODEM_PRIME_API_CMD)*recvBuf++;
                switch (apiCmd)
                {
                    case APP_MODEM_CL_NULL_ESTABLISH_REQUEST_CMD:
                        APP_Modem_MacEstablishRequestCmd(recvBuf);
                        break;

                    case APP_MODEM_CL_NULL_ESTABLISH_RESPONSE_CMD:
                        APP_Modem_MacEstablishResponseCmd(recvBuf);
                        break;

                    case APP_MODEM_CL_NULL_RELEASE_REQUEST_CMD:
                        APP_Modem_MacReleaseRequestCmd(recvBuf);
                        break;

                    case APP_MODEM_CL_NULL_RELEASE_RESPONSE_CMD:
                        APP_Modem_MacReleaseResponseCmd(recvBuf);
                        break;

                    case APP_MODEM_CL_432_REDIRECT_RESPONSE_CMD:
                        APP_Modem_MacRedirectResponseCmd(recvBuf);
                        break;

                    case APP_MODEM_CL_NULL_JOIN_REQUEST_CMD:
                        APP_Modem_MacJoinRequestCmd(recvBuf);
                        break;

                    case APP_MODEM_CL_NULL_JOIN_RESPONSE_CMD:
                        APP_Modem_MacJoinResponseCmd(recvBuf);
                        break;

                    case APP_MODEM_CL_NULL_LEAVE_REQUEST_CMD:
                        APP_Modem_MacLeaveRequestCmd(recvBuf);
                        break;

                    case APP_MODEM_CL_NULL_DATA_REQUEST_CMD:
                        APP_Modem_MacDataRequestCmd(recvBuf);
                        break;

                    case APP_MODEM_CL_NULL_PLME_RESET_REQUEST_CMD:
                        APP_Modem_PLME_ResetRequestCmd(recvBuf);
                        break;

                    case APP_MODEM_CL_NULL_PLME_SLEEP_REQUEST_CMD:
                        APP_Modem_PLME_SleepRequestCmd(recvBuf);
                        break;

                    case APP_MODEM_CL_NULL_PLME_RESUME_REQUEST_CMD:
                        APP_Modem_PLME_ResumeRequestCmd(recvBuf);
                        break;

                    case APP_MODEM_CL_NULL_PLME_TESTMODE_REQUEST_CMD:
                        /* Not implemented */
                        break;

                    case APP_MODEM_CL_NULL_PLME_GET_REQUEST_CMD:
                        APP_Modem_PLME_GetRequestCmd(recvBuf);
                        break;

                    case APP_MODEM_CL_NULL_PLME_SET_REQUEST_CMD:
                        APP_Modem_PLME_SetRequestCmd(recvBuf);
                        break;

                    case APP_MODEM_CL_NULL_MLME_PROMOTE_REQUEST_CMD:
                        APP_Modem_MLME_PromoteRequestCmd(recvBuf);
                        break;

                    case APP_MODEM_CL_NULL_MLME_MP_PROMOTE_REQUEST_CMD:
                        APP_Modem_MLME_MP_PromoteRequestCmd(recvBuf);
                        break;

                    case APP_MODEM_CL_NULL_MLME_RESET_REQUEST_CMD:
                        gPrimeApi->MlmeResetRequest();
                        break;

                    case APP_MODEM_CL_NULL_MLME_GET_REQUEST_CMD:
                        APP_Modem_MLME_GetRequestCmd(recvBuf);
                        break;

                    case APP_MODEM_CL_NULL_MLME_LIST_GET_REQUEST_CMD:
                        APP_Modem_MLME_ListGetRequestCmd(recvBuf);
                        break;

                    case APP_MODEM_CL_NULL_MLME_SET_REQUEST_CMD:
                        APP_Modem_MLME_SetRequestCmd(recvBuf);
                        break;

                    case APP_MODEM_CL_432_RELEASE_REQUEST_CMD:
                        APP_Modem_CL432ReleaseRequestCmd(recvBuf);
                        break;

                    case APP_MODEM_CL_432_DL_DATA_REQUEST_CMD:
                        APP_Modem_CL432DataRequestCmd(recvBuf);
                        break;

                    case APP_MODEM_BMNG_FUP_CLEAR_TARGET_REQUEST_CMD:
                        gPrimeApi->BmngFupClearTargetListRequest(
                                    APP_MODEM_BMNG_FUP_CLEAR_TARGET_REQUEST_CMD);
                        break;

                    case APP_MODEM_BMNG_FUP_ADD_TARGET_REQUEST_CMD:
                        APP_Modem_BMNG_FupAddTargetRequestCmd(recvBuf);
                        break;

                    case APP_MODEM_BMNG_FUP_SET_FW_DATA_REQUEST_CMD:
                        APP_Modem_BMNG_FupSetFwDataRequestCmd(recvBuf);
                        break;

                    case APP_MODEM_BMNG_FUP_SET_UPGRADE_REQUEST_CMD:
                        APP_Modem_BMNG_FupSetUpgOptionRequestCmd(recvBuf);
                        break;

                    case APP_MODEM_BMNG_FUP_INIT_FILE_TX_REQUEST_CMD:
                        APP_Modem_BMNG_FupInitFileTxRequestCmd(recvBuf);
                        break;

                    case APP_MODEM_BMNG_FUP_DATA_FRAME_REQUEST_CMD:
                        APP_Modem_BMNG_FupDataFrameRequestCmd(recvBuf);
                        break;

                    case APP_MODEM_BMNG_FUP_CHECK_CRC_REQUEST_CMD:
                        gPrimeApi->BmngFupCheckCrcRequest(
                                APP_MODEM_BMNG_FUP_CHECK_CRC_REQUEST_CMD);
                        break;

                    case APP_MODEM_BMNG_FUP_ABORT_FU_REQUEST_CMD:
                        APP_Modem_BMNG_FupAbortFuRequestCmd(recvBuf);
                        break;

                    case APP_MODEM_BMNG_FUP_START_FU_REQUEST_CMD:
                        APP_Modem_BMNG_FupStartFuRequestCmd(recvBuf);
                        break;

                    case APP_MODEM_BMNG_FUP_SET_MATCH_RULE_REQUEST_CMD:
                        APP_Modem_BMNG_FupSetMatchRuleRequestCmd(recvBuf);
                        break;

                    case APP_MODEM_BMNG_FUP_GET_VERSION_REQUEST_CMD:
                        APP_Modem_BMNG_FupGetVersionRequestCmd(recvBuf);
                        break;

                    case APP_MODEM_BMNG_FUP_GET_STATE_REQUEST_CMD:
                        APP_Modem_BMNG_FupGetStateRequestCmd(recvBuf);
                        break;

                    case APP_MODEM_BMNG_FUP_SET_SIGNATURE_DATA_REQUEST_CMD:
                        APP_Modem_BMNG_FupSetSigDataRequestCmd(recvBuf);
                        break;

                    case APP_MODEM_BMNG_PPROF_GET_REQUEST_CMD:
                        APP_Modem_BMNG_PprofGetRequestCmd(recvBuf);
                        break;

                    case APP_MODEM_BMNG_PPROF_SET_REQUEST_CMD:
                        APP_Modem_BMNG_PprofSetRequestCmd(recvBuf);
                        break;

                    case APP_MODEM_BMNG_PPROF_RESET_REQUEST_CMD:
                        APP_Modem_BMNG_PprofResetRequestCmd(recvBuf);
                        break;

                    case APP_MODEM_BMNG_PPROF_REBOOT_REQUEST_CMD:
                        APP_Modem_BMNG_PprofRebootRequestCmd(recvBuf);
                        break;

                    case APP_MODEM_BMNG_PPROF_GET_ENHANCED_REQUEST_CMD:
                        APP_Modem_BMNG_PprofGetEnhancedRequestCmd(recvBuf);
                        break;

                    case APP_MODEM_BMNG_PPROF_ZC_DIFF_REQUEST_CMD:
                        APP_Modem_BMNG_PprofZcDiffRequestCmd(recvBuf);
                        break;

                    case APP_MODEM_BMNG_WHITELIST_ADD_REQUEST_CMD:
                        APP_Modem_BMNG_WhitelistAddRequestCmd(recvBuf);
                        break;

                    case APP_MODEM_BMNG_WHITELIST_REMOVE_REQUEST_CMD:
                        APP_Modem_BMNG_WhitelistRemoveRequestCmd(recvBuf);
                        break;

                    default:
                        SRV_LOG_REPORT_Message_With_Code(SRV_LOG_REPORT_INFO,
                            APP_MODEM_ERR_UNKNOWN_CMD, "ERROR: unknown command\r\n" );
                        break;
                }

                sAppModemMsgRecv[outputMsgRecvIndex].len = 0;
                if (++outputMsgRecvIndex == MAX_NUM_MSG_RCV)
                {
                    outputMsgRecvIndex = 0;
                }
            }

            break;

        default:
            break;
    }
}

/* Data transmission indication function */
uint8_t APP_Modem_TxdataIndication(void)
{
    if (sTxdataIndication)
    {
        sTxdataIndication = false;
        return true;
    }
    else
    {
        return false;
    }
}

/* Data reception indication function */
uint8_t APP_Modem_RxdataIndication(void)
{
    if(sRxdataIndication)
    {
        sRxdataIndication = false;
        return true;
    }
    else
    {
        return false;
    }
}
