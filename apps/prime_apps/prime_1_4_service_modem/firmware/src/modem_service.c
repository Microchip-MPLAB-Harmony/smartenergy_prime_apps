/*******************************************************************************
  Modem Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    modem_service.c

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

#define MAX_NUM_MSG_RCV    (5)

#define MAX_LENGTH_BUFF    CL_432_MAX_LENGTH_DATA

PRIME_API *gPrimeApi;

SRV_USI_HANDLE gUsiHandle=0;

/* buffer used to tx serialization */
static uint8_t appSerialBuf[MAX_LENGTH_BUFF];

/* Queue of buffers in rx */
typedef struct APP_MODEM_MSG_RCV_tag
{
    uint16_t len;
    uint8_t dataBuf[MAX_LENGTH_BUFF];
} APP_MODEM_MSG_RCV;

static APP_MODEM_MSG_RCV sAppModemMsgRecv[MAX_NUM_MSG_RCV];

static uint8_t outputMsgRecvIndex;
static uint8_t inputMsgRecvIndex;

/** Structure used for USI send request */
//static x_usi_serial_cmd_params_t x_usi_msg;

/** global status node information */
static APP_MODEM_NODE_STATE sAppNodeState;

/** Data transmission indication variable */
static uint8_t sRxdataIndication;
/** Data reception indication variable */
static uint8_t sTxdataIndication;

static void APP_Modem_SetCallbacks(void);

APP_MODEM_NODE_STATE APP_Modem_NodeState(void)
{
	return sAppNodeState;
}

/* Data transmission indication function */
uint8_t APP_Modem_TxdataIndication(void)
{
	if(sTxdataIndication) 
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

static void APP_Modem_EstablishIndication(uint16_t conHandle, uint8_t *eui48, 
        uint8_t type, uint8_t *data, uint16_t data_len, uint8_t cfbytes, uint8_t ae)
{
	uint16_t serialLen = 0U; 

    appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_ESTABLISH_INDICATION_CMD;
    appSerialBuf[serialLen++] = (uint8_t)(conHandle >> 8);
    appSerialBuf[serialLen++] = (uint8_t)(conHandle);
    memcpy(&appSerialBuf[serialLen], eui48, 6);
    serialLen += 6;
    appSerialBuf[serialLen++] = type;
    appSerialBuf[serialLen++] = (uint8_t)(data_len >> 8);
    appSerialBuf[serialLen++] = (uint8_t)(data_len);
    memcpy(&appSerialBuf[serialLen], data, data_len);
    serialLen += data_len;
    appSerialBuf[serialLen++] = cfbytes;
    appSerialBuf[serialLen++] = ae;

    /* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf, serialLen);
}

static void APP_Modem_EstablishConfirm(uint16_t con_handle, MAC_ESTABLISH_CONFIRM_RESULT result,
uint8_t *eui48, uint8_t type, uint8_t *data, uint16_t data_len, uint8_t ae)
{
	uint8_t serialLen = 0U;

	appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_ESTABLISH_CONFIRM_CMD;
    appSerialBuf[serialLen++] = (uint8_t)(con_handle >> 8);
    appSerialBuf[serialLen++] = (uint8_t)(con_handle);
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
    appSerialBuf[serialLen++] = (uint8_t)(data_len >> 8);
    appSerialBuf[serialLen++] = (uint8_t)(data_len);
    memcpy(&appSerialBuf[serialLen], data, data_len);
    serialLen += data_len;
    appSerialBuf[serialLen++] = ae;

    /* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf, serialLen);
}

static void APP_Modem_ReleaseIndication(uint16_t con_handle, 
                        MAC_RELEASE_INDICATION_REASON reason)
{
	uint16_t serialLen = 0U; 

    appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_RELEASE_INDICATION_CMD;
    appSerialBuf[serialLen++] = (uint8_t)(con_handle >> 8);
    appSerialBuf[serialLen++] = (uint8_t)(con_handle);
    appSerialBuf[serialLen++] = reason;

    /* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf, serialLen);
}

static void APP_Modem_ReleaseConfirm(uint16_t con_handle, MAC_RELEASE_CONFIRM_RESULT  result)
{
    uint16_t serialLen = 0U; 

    appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_RELEASE_CONFIRM_CMD;
    appSerialBuf[serialLen++] = (uint8_t)(con_handle >> 8);
    appSerialBuf[serialLen++] = (uint8_t)(con_handle);
    appSerialBuf[serialLen++] = result;

    /* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf, serialLen);

	/* Change node state */
	sAppNodeState = APP_MODEM_NODE_REGISTERED;
}

static void APP_Modem_JoinIndication(uint16_t con_handle, 
        uint8_t *eui48, uint8_t con_type, uint8_t *data, uint16_t data_len, uint8_t ae)
{
    uint16_t serialLen = 0U;

    appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_JOIN_INDICATION_CMD;
    appSerialBuf[serialLen++] = (uint8_t)(con_handle >> 8);
    appSerialBuf[serialLen++] = (uint8_t)(con_handle);
    if(eui48 != NULL)
    {
        memcpy(&appSerialBuf[serialLen], eui48, 6);
    }
    else
    {
        memset(&appSerialBuf[serialLen], 0, 6);
    }
	serialLen += 6;
    appSerialBuf[serialLen++] = con_type;
    appSerialBuf[serialLen++] = (uint8_t)(data_len >> 8);
    appSerialBuf[serialLen++] = (uint8_t)(data_len);
    memcpy(&appSerialBuf[serialLen], data, data_len);
    serialLen += data_len;
    appSerialBuf[serialLen++] = ae;

    /* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf, serialLen);
}

static void APP_Modem_JoinConfirm(uint16_t con_handle, 
        MAC_JOIN_CONFIRM_RESULT result, uint8_t ae)
{
    uint16_t serialLen = 0U;

    appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_JOIN_CONFIRM_CMD;
    appSerialBuf[serialLen++] = (uint8_t)(con_handle >> 8);
    appSerialBuf[serialLen++] = (uint8_t)(con_handle);
    appSerialBuf[serialLen++] = result;
    appSerialBuf[serialLen++] = ae;

    /* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf, serialLen);
}

static void APP_Modem_LeaveConfirm(uint16_t con_handle, MAC_LEAVE_CONFIRM_RESULT result)
{
    uint16_t serialLen = 0U;

    appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_LEAVE_CONFIRM_CMD;
    appSerialBuf[serialLen++] = (uint8_t)(con_handle >> 8);
    appSerialBuf[serialLen++] = (uint8_t)(con_handle);
    appSerialBuf[serialLen++] = result;

    /* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf, serialLen);
}


static void APP_Modem_LeaveIndication(uint16_t con_handle, uint8_t *eui48)
{
    uint16_t serialLen = 0U;

    appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_LEAVE_INDICATION_CMD;
    appSerialBuf[serialLen++] = (uint8_t)(con_handle >> 8);
    appSerialBuf[serialLen++] = (uint8_t)(con_handle);
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
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf, serialLen);
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
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf, serialLen);
}

static void APP_Modem_DataIndication(uint16_t con_handle, 
        uint8_t *data, uint16_t data_len, uint32_t time_ref)
{
	uint8_t serialLen = 0U;

    appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_DATA_INDICATION_CMD;
    appSerialBuf[serialLen++] = (uint8_t)(con_handle >> 8);
    appSerialBuf[serialLen++] = (uint8_t)(con_handle);
    appSerialBuf[serialLen++] = (uint8_t)(data_len >> 8);
    appSerialBuf[serialLen++] = (uint8_t)(data_len);
    memcpy(&appSerialBuf[serialLen], data, data_len);
    serialLen += data_len;
    appSerialBuf[serialLen++] = (uint8_t)(time_ref >> 24);
    appSerialBuf[serialLen++] = (uint8_t)(time_ref >> 16);
    appSerialBuf[serialLen++] = (uint8_t)(time_ref >> 8);
    appSerialBuf[serialLen++] = (uint8_t)(time_ref);

    /* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf, serialLen);

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
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf, serialLen);
}


static void APP_Modem_PLME_SleepConfirm(PLME_RESULT x_result, uint16_t pch)
{
    uint16_t serialLen = 0U;

    appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_PLME_SLEEP_CONFIRM_CMD;
    appSerialBuf[serialLen++] = x_result;
    appSerialBuf[serialLen++] = (uint8_t)(pch >> 8);
    appSerialBuf[serialLen++] = (uint8_t)pch;

    /* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf, serialLen);
}

static void APP_Modem_PLME_ResumeConfirm(PLME_RESULT x_result, uint16_t pch)
{
    uint16_t serialLen = 0U;

    appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_PLME_RESUME_CONFIRM_CMD;
    appSerialBuf[serialLen++] = x_result;
    appSerialBuf[serialLen++] = (uint8_t)(pch >> 8);
    appSerialBuf[serialLen++] = (uint8_t)pch;

    /* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf, serialLen);
}

static void APP_Modem_PLME_GetConfirm(PLME_RESULT x_status, 
        uint16_t pib_attrib, void *pib_value, uint8_t pib_size, uint16_t pch)
{
    uint16_t serialLen = 0U;
    uint16_t temp_s;
    uint32_t temp_l;

    appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_PLME_GET_CONFIRM_CMD;
    appSerialBuf[serialLen++] = x_status;
    appSerialBuf[serialLen++] = (uint8_t)(pib_attrib >> 8);
    appSerialBuf[serialLen++] = (uint8_t)(pib_attrib);
    appSerialBuf[serialLen++] = pib_size;

    /* Check size */
    switch (pib_size) 
    {
        case 2:
            /* Extract value */
            temp_s = *((uint16_t *)pib_value);
            /* Copy value into buffer with MSB in MSB */
            appSerialBuf[serialLen++] = (uint8_t)(temp_s >> 8);
            appSerialBuf[serialLen++] = (uint8_t)temp_s;
            break;

        case 4:
            temp_l = *((uint32_t *)pib_value);
            /* Copy value into buffer with MSB in MSB */
            appSerialBuf[serialLen++] = (uint8_t)(temp_l >> 24);
            appSerialBuf[serialLen++] = (uint8_t)(temp_l >> 16);
            appSerialBuf[serialLen++] = (uint8_t)(temp_l >> 8);
            appSerialBuf[serialLen++] = (uint8_t)temp_l;
            break;

        default:
            /* Copy value into buffer */
            memcpy(&appSerialBuf[serialLen], (uint8_t *)pib_value, pib_size);
            /* Increase pointer */
            serialLen += pib_size;
    }

    appSerialBuf[serialLen++] = (uint8_t)(pch >> 8);
    appSerialBuf[serialLen++] = (uint8_t)pch;

    /* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf, serialLen);
}

static void APP_Modem_PLME_SetConfirm(PLME_RESULT x_result, uint16_t pch)
{
    uint16_t serialLen = 0U;

    appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_PLME_SET_CONFIRM_CMD;
    appSerialBuf[serialLen++] = x_result;
    appSerialBuf[serialLen++] = (uint8_t)(pch >> 8);
    appSerialBuf[serialLen++] = (uint8_t)pch;

    /* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf, serialLen);
}

static void APP_Modem_MLME_RegisterConfirm(MLME_RESULT x_result, uint8_t *sna, uint8_t sid)
{
	uint8_t serialLen = 0U;

	appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_MLME_REGISTER_CONFIRM_CMD;
	appSerialBuf[serialLen++] = x_result;
	memcpy(&appSerialBuf[serialLen], sna, 6);
	serialLen += 6;
	appSerialBuf[serialLen++] = sid;

	/* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf, serialLen);
}

static void APP_Modem_MLME_RegisterIndication(uint8_t *sna, uint8_t sid)
{
	uint8_t serialLen = 0U;

	appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_MLME_REGISTER_INDICATION_CMD;
	memcpy(&appSerialBuf[serialLen], sna, 6);
	serialLen += 6;
	appSerialBuf[serialLen++] = sid;

	/* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf, serialLen);

	/* Change node state */
	sAppNodeState = APP_MODEM_NODE_REGISTERED;
}

static void APP_Modem_MLME_UnregisterConfirm(MLME_RESULT x_result)
{
	uint8_t serialLen = 0U;

	appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_MLME_UNREGISTER_CONFIRM_CMD;
	appSerialBuf[serialLen++] = x_result;
	
	/* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf, serialLen);
}

static void APP_Modem_MLME_UnregisterIndication(void)
{
	uint8_t serialLen = 0U;

	appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_MLME_UNREGISTER_INDICATION_CMD;

	/* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf, serialLen);

	/* Change node state */
	sAppNodeState = APP_MODEM_NODE_UNREGISTERED;
}

static void APP_Modem_MLME_PromteConfirm(MLME_RESULT x_result)
{
	uint8_t serialLen = 0U;

	appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_MLME_PROMOTE_CONFIRM_CMD;
	appSerialBuf[serialLen++] = x_result;
	
	/* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf, serialLen);
}

static void APP_Modem_MLME_PromteIndication(void)
{
	uint8_t serialLen = 0U;

	appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_MLME_PROMOTE_INDICATION_CMD;

	/* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf, serialLen);

	/* Change node state */
	sAppNodeState = APP_MODEM_NODE_SWITCH;
}

static void APP_Modem_MLME_MP_PromteConfirm(MLME_RESULT x_result)
{
	uint8_t serialLen = 0U;

	appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_MLME_PROMOTE_CONFIRM_CMD;
	appSerialBuf[serialLen++] = x_result;

	/* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf, serialLen);
}

static void APP_Modem_MLME_MP_PromteIndication(uint16_t us_pch)
{
	uint8_t serialLen = 0U;

	appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_MLME_PROMOTE_INDICATION_CMD;
	appSerialBuf[serialLen++] = (uint8_t)(us_pch >> 8);
	appSerialBuf[serialLen++] = (uint8_t)(us_pch);

	/* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf, serialLen);

	/* Change node state */
	sAppNodeState = APP_MODEM_NODE_SWITCH;
}

static void APP_Modem_MLME_DemoteConfirm(MLME_RESULT x_result)
{
	uint8_t serialLen = 0U;

	appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_MLME_DEMOTE_CONFIRM_CMD;
	appSerialBuf[serialLen++] = x_result;

	/* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf, serialLen);
}

static void APP_Modem_MLME_DemoteIndication(void)
{
	uint8_t serialLen = 0U;

	appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_MLME_DEMOTE_INDICATION_CMD;

	/* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf, serialLen);

	/* Change node state */
	sAppNodeState = APP_MODEM_NODE_REGISTERED;
}

static void APP_Modem_MLME_MP_DemoteConfirm(MLME_RESULT x_result)
{
	uint8_t serialLen = 0U;

	appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_MLME_MP_DEMOTE_CONFIRM_CMD;
	appSerialBuf[serialLen++] = x_result;

	/* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf, serialLen);
}

static void APP_Modem_MLME_MP_DemoteIndication(uint8_t uc_lsid)
{
	uint8_t serialLen = 0U;

	appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_MLME_MP_DEMOTE_INDICATION_CMD;
	appSerialBuf[serialLen++] = uc_lsid;

	/* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf, serialLen);

	/* Change node state */
	sAppNodeState = APP_MODEM_NODE_REGISTERED;
}

static void APP_Modem_MLME_ResetConfirm(MLME_RESULT x_result)
{
	uint8_t serialLen = 0U;

	/* Check result */
	if(x_result == MLME_RESULT_DONE)
    {
		/* Set callback functions */
	  	APP_Modem_SetCallbacks();
	}

	appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_MLME_RESET_CONFIRM_CMD;
	appSerialBuf[serialLen++] = x_result;

	/* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf, serialLen);
}

static void APP_Modem_MLME_GetConfirm(MLME_RESULT x_status, 
        uint16_t pib_attrib, void *pib_value, uint8_t pib_size)
{
    uint16_t serialLen = 0U;
    uint16_t temp_s;
    uint32_t temp_h;

    appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_MLME_GET_CONFIRM_CMD;
    appSerialBuf[serialLen++] = x_status;
    appSerialBuf[serialLen++] = (uint8_t)(pib_attrib >> 8);
    appSerialBuf[serialLen++] = (uint8_t)(pib_attrib);
    appSerialBuf[serialLen++] = pib_size;

    /* Check size */
    switch (pib_size) 
    {
        case 2:
            /* Extract value */
            temp_s = *((uint16_t *)pib_value);
            /* Copy value into buffer with MSB in MSB */
            appSerialBuf[serialLen++] = (uint8_t)(temp_s >> 8);
            appSerialBuf[serialLen++] = (uint8_t)temp_s;
            break;

        case 4:
            temp_h = *((uint32_t *)pib_value);
            /* Copy value into buffer with MSB in MSB */
            appSerialBuf[serialLen++] = (uint8_t)(temp_h >> 24);
            appSerialBuf[serialLen++] = (uint8_t)(temp_h >> 16);
            appSerialBuf[serialLen++] = (uint8_t)(temp_h >> 8);
            appSerialBuf[serialLen++] = (uint8_t)temp_h;
            break;

        default:
            /* Copy value into buffer */
            memcpy(&appSerialBuf[serialLen], (uint8_t *)pib_value, pib_size);
            /* Increase pointer */
            serialLen += pib_size;
    }

    /* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf, serialLen);
}

static void APP_Modem_MLME_ListGetConfirm(MLME_RESULT x_status, 
        uint16_t pib_attrib, uint8_t *pib_buff, uint16_t pib_len)
{
    uint16_t serialLen = 0U;

    appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_MLME_LIST_GET_CONFIRM_CMD;
    appSerialBuf[serialLen++] = x_status;
    appSerialBuf[serialLen++] = (uint8_t)(pib_attrib >> 8);
    appSerialBuf[serialLen++] = (uint8_t)(pib_attrib);
    appSerialBuf[serialLen++] = (uint8_t)(pib_len >> 8);
    appSerialBuf[serialLen++] = (uint8_t)(pib_len);
    memcpy(&appSerialBuf[serialLen], (uint8_t *)pib_buff, pib_len);
    serialLen += pib_len;

    /* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf, serialLen);
}

static void APP_Modem_MLME_SetConfirm(MLME_RESULT x_result)
{
    uint16_t serialLen = 0U;

    appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_MLME_SET_CONFIRM_CMD;
    appSerialBuf[serialLen++] = x_result;

    /* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf, serialLen);
}

static void APP_Modem_CL432_EstablishConfirm(uint8_t *device_id, 
uint8_t device_id_len, uint16_t dst_address, uint16_t base_address, uint8_t ae)
{
	uint8_t serialLen = 0U;

	appSerialBuf[serialLen++] = APP_MODEM_CL_432_ESTABLISH_CONFIRM_CMD;
	appSerialBuf[serialLen++] = device_id_len;
	memcpy(&appSerialBuf[serialLen], device_id, device_id_len);
	serialLen += device_id_len;
	appSerialBuf[serialLen++] = (uint8_t)(dst_address >> 8);
	appSerialBuf[serialLen++] = (uint8_t)(dst_address);
	appSerialBuf[serialLen++] = (uint8_t)(base_address >> 8);
	appSerialBuf[serialLen++] = (uint8_t)(base_address);
	appSerialBuf[serialLen++] = ae;
	
	/* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf, serialLen);
}

static void APP_Modem_CL432_ReleaseConfirm(uint16_t dst_address, DL_432_RESULT result)
{
	uint8_t serialLen = 0U;

	appSerialBuf[serialLen++] = APP_MODEM_CL_432_RELEASE_CONFIRM_CMD;
	appSerialBuf[serialLen++] = (uint8_t)(dst_address >> 8);
	appSerialBuf[serialLen++] = (uint8_t)(dst_address);
	appSerialBuf[serialLen++] = result;

	/* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf, serialLen);
}

static void APP_Modem_CL432_DlDataIndication(uint8_t dst_lsap, uint8_t src_lsap,
        uint16_t dst_address, uint16_t src_address,
        uint8_t *data, uint16_t lsdu_len, uint8_t link_class)
{
    uint16_t serialLen = 0U;

    appSerialBuf[serialLen++] = APP_MODEM_CL_432_DL_DATA_INDICATION_CMD;
    appSerialBuf[serialLen++] = dst_lsap;
    appSerialBuf[serialLen++] = src_lsap;
    appSerialBuf[serialLen++] = (uint8_t)(dst_address >> 8);
    appSerialBuf[serialLen++] = (uint8_t)(dst_address);
    appSerialBuf[serialLen++] = (uint8_t)(src_address >> 8);
    appSerialBuf[serialLen++] = (uint8_t)(src_address);
    appSerialBuf[serialLen++] = (uint8_t)(lsdu_len >> 8);
    appSerialBuf[serialLen++] = (uint8_t)(lsdu_len);
    memcpy(&appSerialBuf[serialLen], data, lsdu_len);
    serialLen += lsdu_len;
    appSerialBuf[serialLen++] = link_class;

    /* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf, serialLen);

    /* Rx data indication */
    sRxdataIndication = true;
}

static void APP_Modem_CL432_DlDataConfirm(uint8_t dst_lsap, uint8_t src_lsap, 
        uint16_t dst_address, DL_432_TX_STATUS tx_status)
{
    uint16_t serialLen = 0U;

    appSerialBuf[serialLen++] = APP_MODEM_CL_432_DL_DATA_CONFIRM_CMD;
    appSerialBuf[serialLen++] = dst_lsap;
    appSerialBuf[serialLen++] = src_lsap;
    appSerialBuf[serialLen++] = (uint8_t)(dst_address >> 8);
    appSerialBuf[serialLen++] = (uint8_t)(dst_address);
    appSerialBuf[serialLen++] = (uint8_t)tx_status;

    /* Send packet */
    SRV_USI_Send_Message(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, appSerialBuf, serialLen);
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
        {/* ERROR ,Message too big */
            SRV_LOG_REPORT_Message_With_Code(SRV_LOG_REPORT_WARNING, APP_MODEM_ERR_MSG_TOO_BIG, "ERROR: Message too big\r\n");
        }
    } 
    else 
    {/* Error, RX queue is full */
        SRV_LOG_REPORT_Message_With_Code(SRV_LOG_REPORT_WARNING, APP_MODEM_ERR_QUEUE_FULL, "ERROR: RX queue full\r\n");
    }

    return;
}

/**
 * \brief Set callback functions
 *
 */
static void APP_Modem_SetCallbacks(void)
{
	MAC_CALLBACKS mac_callbacks;

	CL_432_CALLBACKS cl432_callbacks;

    mac_callbacks.mac_data_cfm = APP_Modem_DataConfirm;
    mac_callbacks.mac_data_ind = APP_Modem_DataIndication;
    mac_callbacks.mac_establish_cfm = APP_Modem_EstablishConfirm;
    mac_callbacks.mac_establish_ind = APP_Modem_EstablishIndication;
    mac_callbacks.mac_join_cfm = APP_Modem_JoinConfirm;
    mac_callbacks.mac_join_ind = APP_Modem_JoinIndication;
    mac_callbacks.mac_leave_cfm = APP_Modem_LeaveConfirm;
    mac_callbacks.mac_leave_ind = APP_Modem_LeaveIndication;
    mac_callbacks.mac_release_cfm = APP_Modem_ReleaseConfirm;
    mac_callbacks.mac_release_ind = APP_Modem_ReleaseIndication;

	mac_callbacks.mlme_demote_cfm = APP_Modem_MLME_DemoteConfirm;
	mac_callbacks.mlme_demote_ind = APP_Modem_MLME_DemoteIndication;
	mac_callbacks.mlme_mp_demote_cfm = APP_Modem_MLME_MP_DemoteConfirm;
	mac_callbacks.mlme_mp_demote_ind = APP_Modem_MLME_MP_DemoteIndication;
	mac_callbacks.mlme_get_cfm = APP_Modem_MLME_GetConfirm;
	mac_callbacks.mlme_list_get_cfm = APP_Modem_MLME_ListGetConfirm;
	mac_callbacks.mlme_promote_cfm = APP_Modem_MLME_PromteConfirm;
	mac_callbacks.mlme_promote_ind = APP_Modem_MLME_PromteIndication;
	mac_callbacks.mlme_mp_promote_cfm = APP_Modem_MLME_MP_PromteConfirm;
	mac_callbacks.mlme_mp_promote_ind = APP_Modem_MLME_MP_PromteIndication;
	mac_callbacks.mlme_register_cfm = APP_Modem_MLME_RegisterConfirm;
	mac_callbacks.mlme_register_ind = APP_Modem_MLME_RegisterIndication;
	mac_callbacks.mlme_reset_cfm = APP_Modem_MLME_ResetConfirm;
	mac_callbacks.mlme_set_cfm = APP_Modem_MLME_SetConfirm;
	mac_callbacks.mlme_unregister_cfm = APP_Modem_MLME_UnregisterConfirm;
	mac_callbacks.mlme_unregister_ind = APP_Modem_MLME_UnregisterIndication;

    mac_callbacks.plme_get_cfm = APP_Modem_PLME_GetConfirm;
    mac_callbacks.plme_reset_cfm = APP_Modem_PLME_ResetConfirm;
    mac_callbacks.plme_resume_cfm = APP_Modem_PLME_ResumeConfirm;
    mac_callbacks.plme_set_cfm = APP_Modem_PLME_SetConfirm;
    mac_callbacks.plme_sleep_cfm = APP_Modem_PLME_SleepConfirm;
    mac_callbacks.plme_testmode_cfm = NULL;

    gPrimeApi->MacSetCallbacks(&mac_callbacks);


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
    uint16_t data_len;
    uint8_t *data;
    uint8_t lArq;
    uint8_t lCfbytes;
    uint8_t ae;

    /* Extract parameters */
    lMessage = recvMsg;
    memcpy(eui48, lMessage, 6);
    lMessage += 6;
    lType = *lMessage++;
    data_len = ((uint16_t)(*lMessage++)) << 8;
    data_len += *lMessage++;
    data = lMessage;
    lMessage += data_len;
    lArq = *lMessage++;
    lCfbytes = *lMessage++;
    ae = *lMessage++;

    gPrimeApi->MacEstablishRequest(eui48, lType, data, data_len, lArq, lCfbytes, ae);
}

static void APP_Modem_MacEstablishResponseCmd(uint8_t *recvMsg)
{
    uint8_t *lMessage;
    uint16_t con_handle;
    uint8_t answer;
    uint16_t data_len;
    uint8_t *data;
    uint8_t ae;

    /* Extract parameters */
    lMessage = recvMsg;
    con_handle = ((uint16_t)(*lMessage++)) << 8;
    con_handle += *lMessage++;
    answer = *lMessage++;
    data_len = ((uint16_t)(*lMessage++)) << 8;
    data_len += *lMessage++;
    data = lMessage;
    lMessage += data_len;
    ae = *lMessage++;

    gPrimeApi->MacEstablishResponse(con_handle, 
            (MAC_ESTABLISH_RESPONSE_ANSWER)answer, data, data_len, ae);
}

static void APP_Modem_MacReleaseRequestCmd(uint8_t *recvMsg)
{
    uint8_t *lMessage;
    uint16_t con_handle;

    /* Extract parameters */
    lMessage = recvMsg;
    con_handle = ((uint16_t)(*lMessage++)) << 8;
    con_handle += *lMessage++;

    gPrimeApi->MacReleaseRequest(con_handle);
}

static void APP_Modem_MacReleaseResponseCmd(uint8_t *recvMsg)
{
    uint8_t *lMessage;
    uint16_t con_handle;
    uint8_t answer;

    /* Extract parameters */
    lMessage = recvMsg;
    con_handle = ((uint16_t)(*lMessage++)) << 8;
    con_handle += *lMessage++;
    answer = *lMessage++;

    gPrimeApi->MacReleaseResponse(con_handle, (MAC_RELEASE_RESPONSE_ANSWER)answer);
}

static void APP_Modem_MacJoinRequestCmd(uint8_t *recvMsg)
{
	uint8_t *lMessage;
	uint8_t broadcast;
	uint8_t con_type;
	uint16_t data_len;
	uint8_t dataBuf[256];
	uint8_t ae;

	/* Extract parameters */
	lMessage = recvMsg;
	broadcast = *lMessage++;
	lMessage += 2; /* Skip con handler */
	lMessage += 6; /* Skip mac */
	con_type = *lMessage++;
	data_len = ((uint16_t)(*lMessage++)) << 8;
	data_len += *lMessage++;
	memcpy(dataBuf, lMessage, data_len);
	lMessage += data_len;
	ae = *lMessage++;

	gPrimeApi->MacJoinRequest((MAC_JOIN_REQUEST_MODE)broadcast, 0, NULL, (MAC_CONNECTION_TYPE)con_type, dataBuf, data_len, ae);
}

static void APP_Modem_MacJoinResponseCmd(uint8_t *recvMsg)
{
	uint8_t *lMessage;
    uint16_t con_handle;
    uint8_t answer;
	uint8_t ae;

    /* Extract parameters */
    lMessage = recvMsg;
    con_handle = ((uint16_t)(*lMessage++)) << 8;
    con_handle += *lMessage++;
    lMessage += 6;
    answer = *lMessage++;
    ae = *lMessage++;

    gPrimeApi->MacJoinResponse(con_handle, NULL, (MAC_JOIN_RESPONSE_ANSWER)answer, ae);
}

static void APP_Modem_MacLeaveRequestCmd(uint8_t *recvMsg)
{
	uint8_t *lMessage;
    uint16_t con_handle;

	/* Extract parameters */
	lMessage = recvMsg;
	con_handle = ((uint16_t)(*lMessage++)) << 8;
    con_handle += *lMessage++;

	gPrimeApi->MacLeaveRequest(con_handle, NULL);
}

static void APP_Modem_MacDataRequestCmd(uint8_t *recvMsg)
{
	uint8_t *lMessage;
    uint16_t con_handle;
    uint16_t data_len;
    uint8_t *data;
    uint8_t prio;
    uint32_t time_ref;

    /* Extract parameters */
    lMessage = recvMsg;
    con_handle = ((uint16_t)(*lMessage++)) << 8;
    con_handle += *lMessage++;
    data_len = ((uint16_t)(*lMessage++)) << 8;
    data_len += *lMessage++;
    data = lMessage;
    lMessage += data_len;
    prio = *lMessage++;
    time_ref = ((uint32_t)(*lMessage++)) << 24;
    time_ref += ((uint32_t)(*lMessage++)) << 16;
    time_ref += ((uint32_t)(*lMessage++)) << 8;
    time_ref += *lMessage++;

    gPrimeApi->MacDataRequest(con_handle, data, data_len, prio, time_ref);

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
    uint16_t pib_attrib;
    uint16_t pch;

    /* Extract parameters */
    lMessage = recvMsg;
    pib_attrib = ((uint16_t)(*lMessage++)) << 8;
    pib_attrib += *lMessage++;
    pch = ((uint16_t)(*lMessage++)) << 8;
    pch += *lMessage++;

    gPrimeApi->PlmeGetRequest(pib_attrib, pch);
}

static void APP_Modem_PLME_SetRequestCmd(uint8_t *recvMsg)
{
	 uint8_t *lMessage;
    uint16_t pib_attrib;
    uint8_t  pibValueBuf[256];
    uint8_t pib_size;
    void *pibValue;
    uint16_t tmp_s;
    uint32_t tmp_l;
    uint16_t pch;

    /* Extract parameters */
    lMessage = recvMsg;
    pib_attrib = ((uint16_t)(*lMessage++)) << 8;
    pib_attrib += *lMessage++;
    pib_size = *lMessage++;
    /* Check PIB size */
    switch (pib_size) 
    {
        case 2: /* sizeof(uint16_t) */
            /* Extract PIB value */
            tmp_s = ((uint16_t)(*lMessage++)) << 8;
            tmp_s += *lMessage++;
            pibValue = (void *)(&tmp_s);
            break;

        case 4: /* sizeof(uint32_t) */
            /* Extract PIB value */
            tmp_l = ((uint32_t)(*lMessage++) << 24);
            tmp_l += ((uint32_t)(*lMessage++) << 16);
            tmp_l += ((uint32_t)(*lMessage++) << 8);
            tmp_l += (uint32_t)*lMessage++;
            pibValue = (void *)(&tmp_l);
            break;

        case 1: /* sizeof(uint8_t) */
        default: /* arrays */
            memcpy(pibValueBuf, lMessage, pib_size);
            pibValue = (void *)pibValueBuf;
            break;
    }

    pch = ((uint16_t)(*lMessage++)) << 8;
    pch += *lMessage++;

    gPrimeApi->PlmeSetRequest(pib_attrib, pibValue, pib_size, pch);
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
    uint8_t bcn_mode;

    /* Extract parameters */
    lMessage = recvMsg;
    lMessage += 6; /* Skip mac */
    bcn_mode = *lMessage++;
    pch = ((uint16_t)(*lMessage++)) << 8;
    pch += *lMessage++;

    gPrimeApi->MlmeMpPromoteRequest(NULL, bcn_mode, pch);
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
    uint16_t pib_attrib;

    /* Extract parameters */
    lMessage = recvMsg;
    pib_attrib = ((uint16_t)(*lMessage++)) << 8;
    pib_attrib += *lMessage++;

    gPrimeApi->MlmeGetRequest(pib_attrib);
}

static void APP_Modem_MLME_ListGetRequestCmd(uint8_t *recvMsg)
{
    uint8_t *lMessage;
    uint16_t pib_attrib;

    /* Extract parameters */
    lMessage = recvMsg;
    pib_attrib = ((uint16_t)(*lMessage++)) << 8;
    pib_attrib += *lMessage++;

    gPrimeApi->MlmeListGetRequest(pib_attrib);
}

static void APP_Modem_MLME_SetRequestCmd(uint8_t *recvMsg)
{
	uint8_t *lMessage;
    void *pibValue;
    uint16_t pib_attrib;
    uint8_t pibValueBuf[256];
    uint8_t pib_size;
    uint16_t us_tmp;
    uint32_t ul_tmp;

    /* Extract parameters */
    lMessage = recvMsg;
    pib_attrib = ((uint16_t)(*lMessage++)) << 8;
    pib_attrib += *lMessage++;
    pib_size = *lMessage++;
    /* Check PIB size */
    switch (pib_size) 
    {
        case 2: /* sizeof(uint16_t) */
            /* Extract PIB value */
            us_tmp = ((uint16_t)(*lMessage++)) << 8;
            us_tmp += *lMessage++;
            pibValue = (void *)(&us_tmp);
            break;

        case 4: /* sizeof(uint32_t) */
            /* Extract PIB value */
            ul_tmp = ((uint32_t)(*lMessage++)) << 24;
            ul_tmp += ((uint32_t)(*lMessage++)) << 16;
            ul_tmp += ((uint32_t)(*lMessage++)) << 8;
            ul_tmp += *lMessage++;
            pibValue = (void *)(&ul_tmp);
            break;

        case 1: /* sizeof(uint8_t) */
        default: /* arrays */
            memcpy(pibValueBuf, lMessage, pib_size);
            pibValue = (void *)pibValueBuf;
            break;
    }

    gPrimeApi->MlmeSetRequest(pib_attrib, pibValue, pib_size);
}

static void APP_Modem_CL432EstablishRequestCmd(uint8_t *recvMsg)
{
	uint8_t *lMessage;
	uint8_t *device_id;
	uint8_t device_id_len;
	uint8_t ae;

	/* Extract parameters */
	lMessage = recvMsg;
	device_id_len = *lMessage++;
	device_id = lMessage;
	lMessage += device_id_len;
	ae = *lMessage++;

	gPrimeApi->Cl432EstablishRequest(device_id, device_id_len, ae);
}

static void APP_Modem_CL432ReleaseRequestCmd(uint8_t *recvMsg)
{
	uint8_t *lMessage;
	uint16_t dst_address;

	/* Extract parameters */
	lMessage = recvMsg;
	dst_address = ((uint16_t)(*lMessage++)) << 8;
	dst_address += *lMessage++;

	gPrimeApi->Cl432ReleaseRequest(dst_address);
}

static void APP_Modem_CL432DLDataRequestCmd(uint8_t *recvMsg)
{
	uint8_t *lMessage;
    uint16_t dst_address, lsdu_len;
    DL_432_BUFFER buff_432;
    uint8_t link_class, dst_lsap, src_lsap;

    /* Extract parameters */
    lMessage = recvMsg;
    dst_lsap = *lMessage++;
    src_lsap = *lMessage++;
    dst_address = ((uint16_t)(*lMessage++)) << 8;
    dst_address += *lMessage++;
    lsdu_len = ((uint16_t)(*lMessage++)) << 8;
    lsdu_len += *lMessage++;
    if(lsdu_len <= CL_432_MAX_LENGTH_DATA) 
    {
        memcpy(buff_432.dl.buff, lMessage, lsdu_len);
        lMessage += lsdu_len;
        link_class = *lMessage++;

        gPrimeApi->Cl432DlDataRequest(dst_lsap, src_lsap, dst_address, 
                &buff_432, lsdu_len, link_class);
    }

    /* Tx data indication */
    sTxdataIndication = true;
}

void APP_Modem_Initialize(void)
{
     /* Get the PRIME version */
    SRV_STORAGE_PRIME_MODE_INFO_CONFIG boardInfo;
    /* Initialize the reception queue */
    inputMsgRecvIndex = 0;
    outputMsgRecvIndex = 0;

    SRV_STORAGE_GetConfigInfo(SRV_STORAGE_TYPE_MODE_PRIME, sizeof(boardInfo), 
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

    memset(sAppModemMsgRecv,0,sizeof(sAppModemMsgRecv));
    
    /* Set callback functions */
    APP_Modem_SetCallbacks();

    /* Open USI */
    gUsiHandle = SRV_USI_Open(SRV_USI_INDEX_0);

    /* Configure USI protocol handler */
    SRV_USI_CallbackRegister(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, 
                                APP_Modem_USI_PRIME_ApiHandler);

    /* Initialize TxRx data indicators */
    sRxdataIndication = false;
    sTxdataIndication = false;

	/* Reset node state */
	sAppNodeState = APP_MODEM_NODE_UNREGISTERED;

}

void APP_Modem_Tasks(void)
{
	/* Check data reception */
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
                APP_Modem_CL432DLDataRequestCmd(recvBuf);
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
}
