/*******************************************************************************
  Modem Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    modem_service.c

  Summary:
    MODEM : Modem Application for PRIME Service Node

  Description:
    This source file handles the serialization of the PRIME primitives through
    the USI for the Service Node.
*******************************************************************************/

#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "definitions.h"
#include "modem.h"

#define MAX_NUM_MSG_RCV    (5)

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

/* Global status node information */
static APP_MODEM_NODE_STATE sAppNodeState;

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
	uint8_t serialLen = 0U;

    appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_ESTABLISH_CONFIRM_CMD;
    appSerialBuf[serialLen++] = (uint8_t)(conHandle >> 8);
    appSerialBuf[serialLen++] = (uint8_t)(conHandle);
    appSerialBuf[serialLen++] = result;
    if(eui48 != NULL)
    {
        memcpy(&appSerialBuf[serialLen], eui48, 6);
    }
    else
    {
        memset(&appSerialBuf[serialLen], 0, 6);
    }

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
	/* Change node state */
	sAppNodeState = APP_MODEM_NODE_REGISTERED;
}

static void APP_Modem_JoinIndication(uint16_t conHandle,
        uint8_t *eui48, uint8_t conType, uint8_t *data, uint16_t dataLen,
        uint8_t ae)
{
    uint16_t serialLen = 0U;

    appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_JOIN_INDICATION_CMD;
    appSerialBuf[serialLen++] = (uint8_t)(conHandle >> 8);
    appSerialBuf[serialLen++] = (uint8_t)(conHandle);
    if(eui48 != NULL)
    {
        memcpy(&appSerialBuf[serialLen], eui48, 6);
    }
    else
    {
        memset(&appSerialBuf[serialLen], 0, 6);
    }
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
	uint8_t serialLen = 0U;

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

static void APP_Modem_MLME_RegisterConfirm(MLME_RESULT result, uint8_t *sna,
                                           uint8_t sid)
{
    uint8_t serialLen = 0U;

    appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_MLME_REGISTER_CONFIRM_CMD;
    appSerialBuf[serialLen++] = result;
    memcpy(&appSerialBuf[serialLen], sna, 6);
    serialLen += 6;
    appSerialBuf[serialLen++] = sid;

    /* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf,
                         serialLen);
}

static void APP_Modem_MLME_RegisterIndication(uint8_t *sna, uint8_t sid)
{
    uint8_t serialLen = 0U;

    appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_MLME_REGISTER_INDICATION_CMD;
    memcpy(&appSerialBuf[serialLen], sna, 6);
    serialLen += 6;
    appSerialBuf[serialLen++] = sid;

	/* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf,
                         serialLen);

    /* Change node state */
    sAppNodeState = APP_MODEM_NODE_REGISTERED;
}

static void APP_Modem_MLME_UnregisterConfirm(MLME_RESULT result)
{
    uint8_t serialLen = 0U;

    appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_MLME_UNREGISTER_CONFIRM_CMD;
    appSerialBuf[serialLen++] = result;

    /* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf,
                         serialLen);
}

static void APP_Modem_MLME_UnregisterIndication(void)
{
    uint8_t serialLen = 0U;

    appSerialBuf[serialLen++] =
            APP_MODEM_CL_NULL_MLME_UNREGISTER_INDICATION_CMD;

    /* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf,
                         serialLen);

    /* Change node state */
    sAppNodeState = APP_MODEM_NODE_UNREGISTERED;
}

static void APP_Modem_MLME_PromoteConfirm(MLME_RESULT result)
{
    uint8_t serialLen = 0U;

    appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_MLME_PROMOTE_CONFIRM_CMD;
    appSerialBuf[serialLen++] = result;

    /* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf,
                         serialLen);
}

static void APP_Modem_MLME_PromoteIndication(void)
{
    uint8_t serialLen = 0U;

    appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_MLME_PROMOTE_INDICATION_CMD;

    /* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf,
                         serialLen);

    /* Change node state */
    sAppNodeState = APP_MODEM_NODE_SWITCH;
}

static void APP_Modem_MLME_MP_PromoteConfirm(MLME_RESULT result)
{
    uint8_t serialLen = 0U;

    appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_MLME_MP_PROMOTE_CONFIRM_CMD;
    appSerialBuf[serialLen++] = result;

    /* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf,
                         serialLen);
}

static void APP_Modem_MLME_MP_PromoteIndication(uint16_t us_pch)
{
    uint8_t serialLen = 0U;

    appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_MLME_MP_PROMOTE_INDICATION_CMD;
    appSerialBuf[serialLen++] = (uint8_t)(us_pch >> 8);
    appSerialBuf[serialLen++] = (uint8_t)(us_pch);

    /* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf,
                         serialLen);

    /* Change node state */
    sAppNodeState = APP_MODEM_NODE_SWITCH;
}

static void APP_Modem_MLME_DemoteConfirm(MLME_RESULT result)
{
    uint8_t serialLen = 0U;

    appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_MLME_DEMOTE_CONFIRM_CMD;
    appSerialBuf[serialLen++] = result;

    /* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf,
                         serialLen);
}

static void APP_Modem_MLME_DemoteIndication(void)
{
    uint8_t serialLen = 0U;

    appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_MLME_DEMOTE_INDICATION_CMD;

    /* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf,
                         serialLen);

    /* Change node state */
    sAppNodeState = APP_MODEM_NODE_REGISTERED;
}

static void APP_Modem_MLME_MP_DemoteConfirm(MLME_RESULT result)
{
    uint8_t serialLen = 0U;

    appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_MLME_MP_DEMOTE_CONFIRM_CMD;
    appSerialBuf[serialLen++] = result;

    /* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf,
                         serialLen);
}

static void APP_Modem_MLME_MP_DemoteIndication(uint8_t uc_lsid)
{
    uint8_t serialLen = 0U;

    appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_MLME_MP_DEMOTE_INDICATION_CMD;
    appSerialBuf[serialLen++] = uc_lsid;

    /* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf,
                         serialLen);

    /* Change node state */
    sAppNodeState = APP_MODEM_NODE_REGISTERED;
}

static void APP_Modem_MLME_ResetConfirm(MLME_RESULT result)
{
    uint8_t serialLen = 0U;

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

static void APP_Modem_CL432_EstablishConfirm(uint8_t *deviceId,
            uint8_t deviceIdLen, uint16_t dstAddress, uint16_t baseAddress,
            uint8_t ae)
{
    uint8_t serialLen = 0U;

    appSerialBuf[serialLen++] = APP_MODEM_CL_432_ESTABLISH_CONFIRM_CMD;
    appSerialBuf[serialLen++] = deviceIdLen;
    memcpy(&appSerialBuf[serialLen], deviceId, deviceIdLen);
    serialLen += deviceIdLen;
    appSerialBuf[serialLen++] = (uint8_t)(dstAddress >> 8);
    appSerialBuf[serialLen++] = (uint8_t)(dstAddress);
    appSerialBuf[serialLen++] = (uint8_t)(baseAddress >> 8);
    appSerialBuf[serialLen++] = (uint8_t)(baseAddress);
    appSerialBuf[serialLen++] = ae;

    /* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf,
                         serialLen);
}

static void APP_Modem_CL432_ReleaseConfirm(uint16_t dstAddress,
                                           DL_432_RESULT result)
{
    uint8_t serialLen = 0U;

    appSerialBuf[serialLen++] = APP_MODEM_CL_432_RELEASE_CONFIRM_CMD;
    appSerialBuf[serialLen++] = (uint8_t)(dstAddress >> 8);
    appSerialBuf[serialLen++] = (uint8_t)(dstAddress);
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
    CL_432_CALLBACKS cl432_callbacks;

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

    macCallbacks.mlme_demote_cfm = APP_Modem_MLME_DemoteConfirm;
    macCallbacks.mlme_demote_ind = APP_Modem_MLME_DemoteIndication;
    macCallbacks.mlme_mp_demote_cfm = APP_Modem_MLME_MP_DemoteConfirm;
    macCallbacks.mlme_mp_demote_ind = APP_Modem_MLME_MP_DemoteIndication;
    macCallbacks.mlme_get_cfm = APP_Modem_MLME_GetConfirm;
    macCallbacks.mlme_list_get_cfm = APP_Modem_MLME_ListGetConfirm;
    macCallbacks.mlme_promote_cfm = APP_Modem_MLME_PromoteConfirm;
    macCallbacks.mlme_promote_ind = APP_Modem_MLME_PromoteIndication;
    macCallbacks.mlme_mp_promote_cfm = APP_Modem_MLME_MP_PromoteConfirm;
    macCallbacks.mlme_mp_promote_ind = APP_Modem_MLME_MP_PromoteIndication;
    macCallbacks.mlme_register_cfm = APP_Modem_MLME_RegisterConfirm;
    macCallbacks.mlme_register_ind = APP_Modem_MLME_RegisterIndication;
    macCallbacks.mlme_reset_cfm = APP_Modem_MLME_ResetConfirm;
    macCallbacks.mlme_set_cfm = APP_Modem_MLME_SetConfirm;
    macCallbacks.mlme_unregister_cfm = APP_Modem_MLME_UnregisterConfirm;
    macCallbacks.mlme_unregister_ind = APP_Modem_MLME_UnregisterIndication;

    macCallbacks.plme_get_cfm = APP_Modem_PLME_GetConfirm;
    macCallbacks.plme_reset_cfm = APP_Modem_PLME_ResetConfirm;
    macCallbacks.plme_resume_cfm = APP_Modem_PLME_ResumeConfirm;
    macCallbacks.plme_set_cfm = APP_Modem_PLME_SetConfirm;
    macCallbacks.plme_sleep_cfm = APP_Modem_PLME_SleepConfirm;
    macCallbacks.plme_testmode_cfm = NULL;

    gPrimeApi->MacSetCallbacks(&macCallbacks);

    cl432_callbacks.cl_432_dl_data_cfm = APP_Modem_CL432_DlDataConfirm;
    cl432_callbacks.cl_432_dl_data_ind = APP_Modem_CL432_DlDataIndication;
    cl432_callbacks.cl_432_establish_cfm = APP_Modem_CL432_EstablishConfirm;
    cl432_callbacks.cl_432_release_cfm = APP_Modem_CL432_ReleaseConfirm;

    gPrimeApi->Cl432SetCallbacks(&cl432_callbacks);
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
    uint8_t broadcast;
    uint8_t conType;
    uint16_t dataLen;
    uint8_t dataBuf[256];
    uint8_t ae;

    /* Extract parameters */
    lMessage = recvMsg;
    broadcast = *lMessage++;
    lMessage += 2; /* Skip con handler */
    lMessage += 6; /* Skip mac */
    conType = *lMessage++;
    dataLen = ((uint16_t)(*lMessage++)) << 8;
    dataLen += *lMessage++;
    memcpy(dataBuf, lMessage, dataLen);
    lMessage += dataLen;
    ae = *lMessage++;

    gPrimeApi->MacJoinRequest((MAC_JOIN_REQUEST_MODE)broadcast, 0, NULL,
               (MAC_CONNECTION_TYPE)conType, dataBuf, dataLen, ae);
}

static void APP_Modem_MacJoinResponseCmd(uint8_t *recvMsg)
{
    uint8_t *lMessage;
    uint16_t conHandle;
    uint8_t answer;
    uint8_t ae;

    /* Extract parameters */
    lMessage = recvMsg;
    conHandle = ((uint16_t)(*lMessage++)) << 8;
    conHandle += *lMessage++;
    lMessage += 6;
    answer = *lMessage++;
    ae = *lMessage++;

    gPrimeApi->MacJoinResponse(conHandle, NULL,
                               (MAC_JOIN_RESPONSE_ANSWER)answer, ae);
}

static void APP_Modem_MacLeaveRequestCmd(uint8_t *recvMsg)
{
    uint8_t *lMessage;
    uint16_t conHandle;

    /* Extract parameters */
    lMessage = recvMsg;
    conHandle = ((uint16_t)(*lMessage++)) << 8;
    conHandle += *lMessage++;

    gPrimeApi->MacLeaveRequest(conHandle, NULL);
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

static void APP_Modem_MLME_RegisterRequestCmd(uint8_t *recvMsg)
{
    uint8_t *lMessage;
    uint8_t sna[6];
    uint8_t sid;
    uint8_t index;
    bool snaValid = false;

    /* Extract parameters */
    lMessage = recvMsg;
    memcpy(sna, lMessage, 6);
    lMessage += 6;
    sid = *lMessage++;

    /* Check if SNA is invalid */
    for (index = 0; index < 6; index++)
    {
        if (sna[index] != 0xFF)
        {
            snaValid = true;
            break;
        }
    }

    if(snaValid)
    {
        gPrimeApi->MlmeRegisterRequest(sna, sid);
    }
    else
    {
        gPrimeApi->MlmeRegisterRequest(NULL, sid);
    }
}

static void APP_Modem_MLME_PromoteRequestCmd(uint8_t *recvMsg)
{
    uint8_t *lMessage;
    uint8_t bcnMode;

    /* Extract parameters */
    lMessage = recvMsg;
    lMessage += 6; /* Skip mac */
    bcnMode = *lMessage++;

    gPrimeApi->MlmePromoteRequest(NULL, bcnMode);
}

static void APP_Modem_MLME_MP_PromoteRequestCmd(uint8_t *recvMsg)
{
    uint8_t *lMessage;
    uint16_t pch;
    uint8_t bcnMode;

    /* Extract parameters */
    lMessage = recvMsg;
    lMessage += 6; /* Skip mac */
    bcnMode = *lMessage++;
    pch = ((uint16_t)(*lMessage++)) << 8;
    pch += *lMessage++;

    gPrimeApi->MlmeMpPromoteRequest(NULL, bcnMode, pch);
}

static void APP_Modem_MLME_MP_DemoteRequestCmd(uint8_t *recvMsg)
{
    uint8_t *lMessage;
    uint8_t lsid;

    /* Extract parameters */
    lMessage = recvMsg;
    lsid = *lMessage++;

    gPrimeApi->MlmeMpDemoteRequest(lsid);
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

static void APP_Modem_CL432EstablishRequestCmd(uint8_t *recvMsg)
{
    uint8_t *lMessage;
    uint8_t *deviceId;
    uint8_t deviceIdLen;
    uint8_t ae;

    /* Extract parameters */
    lMessage = recvMsg;
    deviceIdLen = *lMessage++;
    deviceId = lMessage;
    lMessage += deviceIdLen;
    ae = *lMessage++;

    gPrimeApi->Cl432EstablishRequest(deviceId, deviceIdLen, ae);
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

void APP_Modem_Initialize(void)
{
    SRV_STORAGE_PRIME_MODE_INFO_CONFIG boardInfo;

    /* Initialize the reception queue */
    inputMsgRecvIndex = 0;
    outputMsgRecvIndex = 0;

    (void) memset(sAppModemMsgRecv, 0, sizeof(sAppModemMsgRecv));

    /* Initialize TxRx data indicators */
    sRxdataIndication = false;
    sTxdataIndication = false;

    /* Reset node state */
    sAppNodeState = APP_MODEM_NODE_UNREGISTERED;

    (void) memset(&boardInfo, 0, sizeof(boardInfo));

    /* Get the PRIME version */
    SRV_STORAGE_GetConfigInfo(SRV_STORAGE_TYPE_MODE_PRIME, (uint8_t)sizeof(boardInfo),
                              (void *)&boardInfo);

    /* Get PRIME API pointer */
    switch (boardInfo.primeVersion)
    {
        case PRIME_VERSION_1_3:
            PRIME_API_GetPrime13API(&gPrimeApi);
            break;

        case PRIME_VERSION_1_4:
        default:
            PRIME_API_GetPrime14API(&gPrimeApi);
            break;
    }

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

                    case APP_MODEM_CL_NULL_MLME_REGISTER_REQUEST_CMD:
                        APP_Modem_MLME_RegisterRequestCmd(recvBuf);
                        break;

                    case APP_MODEM_CL_NULL_MLME_UNREGISTER_REQUEST_CMD:
                        gPrimeApi->MlmeUnregisterRequest();
                        break;

                    case APP_MODEM_CL_NULL_MLME_PROMOTE_REQUEST_CMD:
                        APP_Modem_MLME_PromoteRequestCmd(recvBuf);
                        break;

                    case APP_MODEM_CL_NULL_MLME_DEMOTE_REQUEST_CMD:
                        gPrimeApi->MlmeDemoteRequest();
                        break;

                    case APP_MODEM_CL_NULL_MLME_MP_PROMOTE_REQUEST_CMD:
                        APP_Modem_MLME_MP_PromoteRequestCmd(recvBuf);
                        break;

                    case APP_MODEM_CL_NULL_MLME_MP_DEMOTE_REQUEST_CMD:
                        APP_Modem_MLME_MP_DemoteRequestCmd(recvBuf);
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

                    case APP_MODEM_CL_432_ESTABLISH_REQUEST_CMD:
                        APP_Modem_CL432EstablishRequestCmd(recvBuf);
                        break;

                    case APP_MODEM_CL_432_RELEASE_REQUEST_CMD:
                        APP_Modem_CL432ReleaseRequestCmd(recvBuf);
                        break;

                    case APP_MODEM_CL_432_DL_DATA_REQUEST_CMD:
                        APP_Modem_CL432DataRequestCmd(recvBuf);
                        break;

                    default:
                        SRV_LOG_REPORT_Message_With_Code(SRV_LOG_REPORT_INFO,
                            APP_MODEM_ERR_UNKNOWN_CMD, "ERROR: unknown command\r\n" );
                    break;
                }

            sAppModemMsgRecv[outputMsgRecvIndex].len = 0;
            if(++outputMsgRecvIndex == MAX_NUM_MSG_RCV)
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
