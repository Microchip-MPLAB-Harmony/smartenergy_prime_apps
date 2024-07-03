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

/* buffer used to tx serialization */
static uint8_t appSerialBuf[MAX_LENGTH_BUFF];

PRIME_API *gPrimeApi;

SRV_USI_HANDLE gUsiHandle=0;

/* Queue of buffers in rx and pointers */
static struct 
{
    uint16_t len;
    uint8_t dataBuf[MAX_LENGTH_BUFF];
} APP_MODEM_MSG_RCV[MAX_NUM_MSG_RCV];

static uint8_t outputMsgRecvIndex;
static uint8_t inputMsgRecvIndex;

/* Data transmission indication variable */
static uint8_t rxdata_index;
/* Data reception indication variable */
static uint8_t txdata_index;

static void APP_Modem_SetCallbacks(void);

static void APP_Modem_establishIndication(uint16_t conHandle, uint8_t *eui48, uint8_t type, uint8_t *data, uint16_t data_len, uint8_t cfbytes, uint8_t ae)
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
    SRV_USI_Send_Message(gUsiHandle,SRV_USI_PROT_ID_PRIME_API,appSerialBuf,serialLen);
}


static void APP_Modem_establishConfirm(uint16_t con_handle, MAC_ESTABLISH_CONFIRM_RESULT result,
uint8_t *eui48, uint8_t type, uint8_t *data, uint16_t data_len, uint8_t ae)
{
	uint16_t serialLen = 0U; 
    
    appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_ESTABLISH_CONFIRM_CMD;
	appSerialBuf[serialLen++] = (uint8_t)(con_handle >> 8);
	appSerialBuf[serialLen++] = (uint8_t)(con_handle);
	appSerialBuf[serialLen++] = result;
	memcpy(&appSerialBuf[serialLen], eui48, 6);
	serialLen += 6;
	appSerialBuf[serialLen++] = type;
	appSerialBuf[serialLen++] = (uint8_t)(data_len >> 8);
	appSerialBuf[serialLen++] = (uint8_t)(data_len);
	memcpy(&appSerialBuf[serialLen], data, data_len);
	serialLen += data_len;
	appSerialBuf[serialLen++] = ae;

	/* Send packet */
    SRV_USI_Send_Message(gUsiHandle,SRV_USI_PROT_ID_PRIME_API,appSerialBuf,serialLen);
}

static void APP_Modem_releaseIndication(uint16_t con_handle, MAC_RELEASE_INDICATION_REASON reason)
{
	uint16_t serialLen = 0U; 
    
    appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_RELEASE_INDICATION_CMD;
	appSerialBuf[serialLen++] = (uint8_t)(con_handle >> 8);
	appSerialBuf[serialLen++] = (uint8_t)(con_handle);
	appSerialBuf[serialLen++] = reason;

	/* Send packet */
    SRV_USI_Send_Message(gUsiHandle,SRV_USI_PROT_ID_PRIME_API,appSerialBuf,serialLen);
}

static void APP_Modem_releaseConfirm(uint16_t con_handle, MAC_RELEASE_CONFIRM_RESULT  result)
{
	uint16_t serialLen = 0U; 
    
    appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_RELEASE_CONFIRM_CMD;
	appSerialBuf[serialLen++] = (uint8_t)(con_handle >> 8);
	appSerialBuf[serialLen++] = (uint8_t)(con_handle);
	appSerialBuf[serialLen++] = result;

	/* Send packet */
    SRV_USI_Send_Message(gUsiHandle,SRV_USI_PROT_ID_PRIME_API,appSerialBuf,serialLen);
}

static void APP_Modem_joinIndication(uint16_t con_handle, uint8_t *eui48, uint8_t con_type, uint8_t *data, uint16_t data_len, uint8_t ae)
{
	uint16_t serialLen = 0U;

	appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_JOIN_INDICATION_CMD;
	appSerialBuf[serialLen++] = (uint8_t)(con_handle >> 8);
	appSerialBuf[serialLen++] = (uint8_t)(con_handle);
	memcpy(&appSerialBuf[serialLen], eui48, 6);
	serialLen += 6;
	appSerialBuf[serialLen++] = con_type;
	appSerialBuf[serialLen++] = (uint8_t)(data_len >> 8);
	appSerialBuf[serialLen++] = (uint8_t)(data_len);
	memcpy(&appSerialBuf[serialLen], data, data_len);
	serialLen += data_len;
	appSerialBuf[serialLen++] = ae;

	/* Send packet */
	SRV_USI_Send_Message(gUsiHandle,SRV_USI_PROT_ID_PRIME_API,appSerialBuf,serialLen);
}

static void APP_Modem_joinConfirm(uint16_t con_handle, MAC_JOIN_CONFIRM_RESULT result, uint8_t ae)
{
	uint16_t serialLen = 0U;

	appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_JOIN_CONFIRM_CMD;
	appSerialBuf[serialLen++] = (uint8_t)(con_handle >> 8);
	appSerialBuf[serialLen++] = (uint8_t)(con_handle);
	appSerialBuf[serialLen++] = result;
	appSerialBuf[serialLen++] = ae;

	/* Send packet */
	SRV_USI_Send_Message(gUsiHandle,SRV_USI_PROT_ID_PRIME_API,appSerialBuf,serialLen);
}

static void APP_Modem_leaveConfirm(uint16_t con_handle, MAC_LEAVE_CONFIRM_RESULT result)
{
	uint16_t serialLen = 0U;

	appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_LEAVE_CONFIRM_CMD;
	appSerialBuf[serialLen++] = (uint8_t)(con_handle >> 8);
	appSerialBuf[serialLen++] = (uint8_t)(con_handle);
	appSerialBuf[serialLen++] = result;

	/* Send packet */
	SRV_USI_Send_Message(gUsiHandle,SRV_USI_PROT_ID_PRIME_API,appSerialBuf,serialLen);
}

static void APP_Modem_leaveIndication(uint16_t con_handle, uint8_t *eui48)
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
	SRV_USI_Send_Message(gUsiHandle,SRV_USI_PROT_ID_PRIME_API,appSerialBuf,serialLen);
}

static void APP_Modem_dataConfirm(uint16_t con_handle, uint8_t *data, MAC_DATA_CONFIRM_RESULT result)
{
    uint8_t serialLen = 0U; 
    
	appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_DATA_CONFIRM_CMD;
	appSerialBuf[serialLen++] = (uint8_t)(con_handle >> 8);
	appSerialBuf[serialLen++] = (uint8_t)(con_handle);
	appSerialBuf[serialLen++] = (uint8_t)((uint32_t)data >> 24);
	appSerialBuf[serialLen++] = (uint8_t)((uint32_t)data >> 16);
	appSerialBuf[serialLen++] = (uint8_t)((uint32_t)data >> 8);
	appSerialBuf[serialLen++] = (uint8_t)((uint32_t)data);
	appSerialBuf[serialLen++] = result;

	/* Send packet */
    SRV_USI_Send_Message(gUsiHandle,SRV_USI_PROT_ID_PRIME_API,appSerialBuf,serialLen);
}

static void APP_Modem_dataIndication(uint16_t con_handle, uint8_t *data, uint16_t data_len, uint32_t time_ref)
{
	uint16_t serialLen = 0U; 
    
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
    SRV_USI_Send_Message(gUsiHandle,SRV_USI_PROT_ID_PRIME_API,appSerialBuf,serialLen);

	/* Rx data indication */
	rxdata_index = true;
}

static void APP_Modem_PLME_resetConfirm(PLME_RESULT result, uint16_t pch)
{
	uint16_t serialLen = 0U; 
    
    appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_PLME_RESET_CONFIRM_CMD;
	appSerialBuf[serialLen++] = result;
	appSerialBuf[serialLen++] = (uint8_t)(pch >> 8);
	appSerialBuf[serialLen++] = (uint8_t)pch;

	/* Send packet */
    SRV_USI_Send_Message(gUsiHandle,SRV_USI_PROT_ID_PRIME_API,appSerialBuf,serialLen);
}

static void APP_Modem_PLME_SleepConfirm(PLME_RESULT x_result, uint16_t pch)
{
	uint16_t serialLen = 0U;

	appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_PLME_SLEEP_CONFIRM_CMD;
	appSerialBuf[serialLen++] = x_result;
	appSerialBuf[serialLen++] = (uint8_t)(pch >> 8);
	appSerialBuf[serialLen++] = (uint8_t)pch;

	/* Send packet */
	SRV_USI_Send_Message(gUsiHandle,SRV_USI_PROT_ID_PRIME_API,appSerialBuf,serialLen);
}

static void APP_Modem_PLME_ResumeConfirm(PLME_RESULT x_result, uint16_t pch)
{
	uint16_t serialLen = 0U;

	appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_PLME_RESUME_CONFIRM_CMD;
	appSerialBuf[serialLen++] = x_result;
	appSerialBuf[serialLen++] = (uint8_t)(pch >> 8);
	appSerialBuf[serialLen++] = (uint8_t)pch;

	/* Send packet */
	SRV_USI_Send_Message(gUsiHandle,SRV_USI_PROT_ID_PRIME_API,appSerialBuf,serialLen);
}

static void APP_Modem_PLME_GetConfirm(PLME_RESULT x_status, uint16_t pib_attrib, void *pib_value, uint8_t pib_size, uint16_t pch)
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
	SRV_USI_Send_Message(gUsiHandle,SRV_USI_PROT_ID_PRIME_API,appSerialBuf,serialLen);
}

static void APP_Modem_PLME_SetConfirm(PLME_RESULT x_result, uint16_t pch)
{
	uint16_t serialLen = 0U;

	appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_PLME_SET_CONFIRM_CMD;
	appSerialBuf[serialLen++] = x_result;
	appSerialBuf[serialLen++] = (uint8_t)(pch >> 8);
	appSerialBuf[serialLen++] = (uint8_t)pch;

	/* Send packet */
	SRV_USI_Send_Message(gUsiHandle,SRV_USI_PROT_ID_PRIME_API,appSerialBuf,serialLen);
}

static void APP_Modem_MLME_PromteConfirm(MLME_RESULT x_result)
{
	uint16_t serialLen = 0U;

	appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_MLME_PROMOTE_CONFIRM_CMD;
	appSerialBuf[serialLen++] = x_result;

	/* Send packet */
	SRV_USI_Send_Message(gUsiHandle,SRV_USI_PROT_ID_PRIME_API,appSerialBuf,serialLen);
}

static void APP_Modem_MLME_MP_PromteConfirm(MLME_RESULT x_result)
{
	uint16_t serialLen = 0U;

	appSerialBuf[serialLen++] = APP_MODEM_MLME_MP_PROMOTE_CONFIRM_CMD;
	appSerialBuf[serialLen++] = x_result;

	/* Send packet */
	SRV_USI_Send_Message(gUsiHandle,SRV_USI_PROT_ID_PRIME_API,appSerialBuf,serialLen);
}

static void APP_Modem_MLME_ResetConfirm(MLME_RESULT x_result)
{
	uint16_t serialLen = 0U;

	/* Check result */
	if (x_result == MLME_RESULT_DONE)
    {
		/* Set callback functions */
		APP_Modem_SetCallbacks();
	}


	appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_MLME_RESET_CONFIRM_CMD;
	appSerialBuf[serialLen++] = x_result;

	/* Send packet */
	SRV_USI_Send_Message(gUsiHandle,SRV_USI_PROT_ID_PRIME_API,appSerialBuf,serialLen);
}

static void APP_Modem_MLME_GetConfirm(MLME_RESULT x_status, uint16_t pib_attrib, void *pib_value, uint8_t pib_size)
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
	SRV_USI_Send_Message(gUsiHandle,SRV_USI_PROT_ID_PRIME_API,appSerialBuf,serialLen);
}

static void APP_Modem_MLME_ListGetConfirm(MLME_RESULT x_status, uint16_t pib_attrib, uint8_t *pib_buff, uint16_t pib_len)
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
	SRV_USI_Send_Message(gUsiHandle,SRV_USI_PROT_ID_PRIME_API,appSerialBuf,serialLen);
}

static void APP_Modem_MLME_SetConfirm(MLME_RESULT x_result)
{
	uint16_t serialLen = 0U;

	appSerialBuf[serialLen++] = APP_MODEM_CL_NULL_MLME_SET_CONFIRM_CMD;
	appSerialBuf[serialLen++] = x_result;

	/* Send packet */
	SRV_USI_Send_Message(gUsiHandle,SRV_USI_PROT_ID_PRIME_API,appSerialBuf,serialLen);
}

static void APP_Modem_CL432_DlDataIndication(uint8_t dst_lsap, uint8_t src_lsap, uint16_t dst_address, uint16_t src_address,
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
	SRV_USI_Send_Message(gUsiHandle,SRV_USI_PROT_ID_PRIME_API,appSerialBuf,serialLen);

	/* Rx data indication */
	rxdata_index = true;
}

static void APP_Modem_CL432_DlDataConfirm(uint8_t dst_lsap, uint8_t src_lsap, uint16_t dst_address, DL_432_TX_STATUS tx_status)
{
	uint16_t serialLen = 0U;

	appSerialBuf[serialLen++] = APP_MODEM_CL_432_DL_DATA_CONFIRM_CMD;
	appSerialBuf[serialLen++] = dst_lsap;
	appSerialBuf[serialLen++] = src_lsap;
	appSerialBuf[serialLen++] = (uint8_t)(dst_address >> 8);
	appSerialBuf[serialLen++] = (uint8_t)(dst_address);
	appSerialBuf[serialLen++] = (uint8_t)tx_status;

	/* Send packet */
	SRV_USI_Send_Message(gUsiHandle,SRV_USI_PROT_ID_PRIME_API,appSerialBuf,serialLen);
}

static void APP_Modem_CL432_JoinIndication(uint8_t *device_id, uint8_t device_id_len, uint16_t dst_address, uint8_t *puc_mac, uint8_t ae)
{
	uint16_t serialLen = 0U;
	uint8_t tmp_c;

	appSerialBuf[serialLen++] = APP_MODEM_CL_432_DL_JOIN_INDICATION_CMD;
	appSerialBuf[serialLen++] = device_id_len;
	for (tmp_c = 0; tmp_c < device_id_len; tmp_c++) 
    {
		appSerialBuf[serialLen++] = *device_id++;
	}

	appSerialBuf[serialLen++] = (uint8_t)(dst_address >> 8);
	appSerialBuf[serialLen++] = (uint8_t)(dst_address);
	for (tmp_c = 0; tmp_c < 8; tmp_c++) 
    {
		appSerialBuf[serialLen++] = *puc_mac++;
	}

	appSerialBuf[serialLen++] = ae;
    
	/* Send packet */
	SRV_USI_Send_Message(gUsiHandle,SRV_USI_PROT_ID_PRIME_API,appSerialBuf,serialLen);
}

static void APP_Modem_CL432_LeaveIndiication(uint16_t dst_address)
{
	uint16_t serialLen = 0U;

	appSerialBuf[serialLen++] = APP_MODEM_CL_432_DL_LEAVE_INDICATION_CMD;
	appSerialBuf[serialLen++] = (uint8_t)(dst_address >> 8);
	appSerialBuf[serialLen++] = (uint8_t)(dst_address);

	/* Send packet */
	SRV_USI_Send_Message(gUsiHandle,SRV_USI_PROT_ID_PRIME_API,appSerialBuf,serialLen);
}

static void APP_Modem_BMNG_FupAck(uint8_t cmd, BMNG_FUP_ACK ack_code, uint16_t extra_info)
{
	uint16_t serialLen = 0U;

	appSerialBuf[serialLen++] = APP_MODEM_BMNG_FUP_ACK_CMD;
	appSerialBuf[serialLen++] = cmd;
	appSerialBuf[serialLen++] = ack_code;
	appSerialBuf[serialLen++] = (uint8_t)(extra_info >> 8);
	appSerialBuf[serialLen++] = (uint8_t)(extra_info);
	
	/* Send packet */
	SRV_USI_Send_Message(gUsiHandle,SRV_USI_PROT_ID_PRIME_API,appSerialBuf,serialLen);
}

static void APP_Modem_BMNG_FupStatusIndication(BMNG_FUP_NODE_STATE fup_node_state, uint16_t pages, uint8_t *eui48)
{
	uint16_t serialLen = 0U;
	
	appSerialBuf[serialLen++] = APP_MODEM_BMNG_FUP_STATUS_INDICATION_CMD;
	appSerialBuf[serialLen++] = fup_node_state;
	appSerialBuf[serialLen++] = (uint8_t)(pages >> 8);
	appSerialBuf[serialLen++] = (uint8_t)(pages);
	memcpy(&appSerialBuf[serialLen], eui48, 6);
	serialLen += 6;

	/* Send packet */
	SRV_USI_Send_Message(gUsiHandle,SRV_USI_PROT_ID_PRIME_API,appSerialBuf,serialLen);
}

static void APP_Modem_BMNG_FupErrorIndication(BMNG_FUP_ERROR error_code, uint8_t *eui48)
{
	uint16_t serialLen = 0U;

	

	appSerialBuf[serialLen++] = APP_MODEM_BMNG_FUP_STATUS_ERROR_INDICATION_CMD;
	appSerialBuf[serialLen++] = error_code;
	memcpy(&appSerialBuf[serialLen], eui48, 6);
	serialLen += 6;

	/* Send packet */
	SRV_USI_Send_Message(gUsiHandle,SRV_USI_PROT_ID_PRIME_API,appSerialBuf,serialLen);
}

static void APP_Modem_BMNG_FupVersionIndication(uint8_t *eui48, uint8_t vendor_len, char *vendor, uint8_t model_len, char *model,
		uint8_t version_len, char *version)
{
	uint16_t serialLen = 0U;

	appSerialBuf[serialLen++] = APP_MODEM_BMNG_FUP_VERSION_INDICATION_CMD;
	memcpy(&appSerialBuf[serialLen], eui48, 6);
	serialLen += 6;
	appSerialBuf[serialLen++] = vendor_len;
	memcpy(&appSerialBuf[serialLen], vendor, vendor_len);
	serialLen += vendor_len;
	appSerialBuf[serialLen++] = model_len;
	memcpy(&appSerialBuf[serialLen], model, model_len);
	serialLen += model_len;
	appSerialBuf[serialLen++] = version_len;
	memcpy(&appSerialBuf[serialLen], version, version_len);
	serialLen += version_len;

	/* Send packet */
	SRV_USI_Send_Message(gUsiHandle,SRV_USI_PROT_ID_PRIME_API,appSerialBuf,serialLen);
}

static void APP_Modem_BMNG_FupKillIndication(uint8_t *eui48)
{
	uint16_t serialLen = 0U;

	appSerialBuf[serialLen++] = APP_MODEM_BMNG_FUP_KILL_INDICATION_CMD;
	memcpy(&appSerialBuf[serialLen], eui48, 6);
	serialLen += 6;
	
	/* Send packet */
	SRV_USI_Send_Message(gUsiHandle,SRV_USI_PROT_ID_PRIME_API,appSerialBuf,serialLen);
}

static void APP_Modem_BMNG_NetEventIndication(BMNG_NET_EVENT_INFO *net_event)
{
	uint16_t serialLen = 0U;

	appSerialBuf[serialLen++] = APP_MODEM_BMNG_NETWORK_EVENT_CMD;
	appSerialBuf[serialLen++] = net_event->netEvent;
	memcpy(&appSerialBuf[serialLen], net_event->eui48, 6);
	serialLen += 6;
	appSerialBuf[serialLen++] = net_event->sid;
	appSerialBuf[serialLen++] = (uint8_t)(net_event->lnid >> 8);
	appSerialBuf[serialLen++] = (uint8_t)(net_event->lnid);
	appSerialBuf[serialLen++] = net_event->lsid;
	appSerialBuf[serialLen++] = net_event->alvRxCnt;
	appSerialBuf[serialLen++] = net_event->alvTxCnt;
	appSerialBuf[serialLen++] = net_event->alvTime;
	appSerialBuf[serialLen++] = (uint8_t)(net_event->pch >> 8);
	appSerialBuf[serialLen++] = (uint8_t)(net_event->pch);
	appSerialBuf[serialLen++] = (uint8_t)(net_event->pchLsid >> 8);
	appSerialBuf[serialLen++] = (uint8_t)(net_event->pchLsid);

	/* Send packet */
	SRV_USI_Send_Message(gUsiHandle,SRV_USI_PROT_ID_PRIME_API,appSerialBuf,serialLen);
}

void APP_Modem_MDM_BMNG_NetEventIndication(void *net_event)
{
	APP_Modem_BMNG_NetEventIndication((BMNG_NET_EVENT_INFO *)net_event);
}

static void APP_Modem_BMNG_PPROFAck(uint8_t cmd, BMNG_PPROF_ACK ack_code)
{
	uint16_t serialLen = 0U;

	appSerialBuf[serialLen++] = APP_MODEM_BMNG_PPROF_ACK_CMD;
	appSerialBuf[serialLen++] = cmd;
	appSerialBuf[serialLen++] = ack_code;

	/* Send packet */
	SRV_USI_Send_Message(gUsiHandle,SRV_USI_PROT_ID_PRIME_API,appSerialBuf,serialLen);
}

static void APP_Modem_BMNG_PPROFGetResponse(uint8_t *eui48, uint16_t data_len, uint8_t *data)
{
	uint16_t serialLen = 0U;

	appSerialBuf[serialLen++] = APP_MODEM_BMNG_PPROF_GET_RESPONSE_CMD;
	memcpy(&appSerialBuf[serialLen], eui48, 6);
	serialLen += 6;
	appSerialBuf[serialLen++] = (uint8_t)(data_len >> 8);
	appSerialBuf[serialLen++] = (uint8_t)(data_len);
	memcpy(&appSerialBuf[serialLen], data, data_len);
	serialLen += data_len;

	/* Send packet */
	SRV_USI_Send_Message(gUsiHandle,SRV_USI_PROT_ID_PRIME_API,appSerialBuf,serialLen);
}

static void APP_Modem_BMNG_PPROFGetEnhancedResponse(uint8_t *eui48, uint16_t data_len, uint8_t *data)
{
	uint16_t serialLen = 0U;

	appSerialBuf[serialLen++] = APP_MODEM_BMNG_PPROF_GET_ENHANCED_RESPONSE_CMD;
	memcpy(&appSerialBuf[serialLen], eui48, 6);
	serialLen += 6;
	appSerialBuf[serialLen++] = (uint8_t)(data_len >> 8);
	appSerialBuf[serialLen++] = (uint8_t)(data_len);
	memcpy(&appSerialBuf[serialLen], data, data_len);
	serialLen += data_len;

	/* Send packet */
	SRV_USI_Send_Message(gUsiHandle,SRV_USI_PROT_ID_PRIME_API,appSerialBuf,serialLen);
}

static void APP_Modem_BMNG_PPROFGetZCResponse(uint8_t *eui48, uint8_t zc_status, uint32_t zc_time)
{
	uint16_t serialLen = 0U;

	appSerialBuf[serialLen++] = APP_MODEM_BMNG_PPROF_GET_ZC_RESPONSE_CMD;
	memcpy(&appSerialBuf[serialLen], eui48, 6);
	serialLen += 6;
	appSerialBuf[serialLen++] = zc_status;
	appSerialBuf[serialLen++] = (uint8_t)(zc_time >> 24);
	appSerialBuf[serialLen++] = (uint8_t)(zc_time >> 16);
	appSerialBuf[serialLen++] = (uint8_t)(zc_time >> 8);
	appSerialBuf[serialLen++] = (uint8_t)(zc_time);

	/* Send packet */
	SRV_USI_Send_Message(gUsiHandle,SRV_USI_PROT_ID_PRIME_API,appSerialBuf,serialLen);
}

static void APP_Modem_BMNG_PPROFDiffZCResponse(uint8_t *eui48, uint32_t time_freq, uint32_t time_diff)
{
	uint16_t serialLen = 0U;

	appSerialBuf[serialLen++] = APP_MODEM_BMNG_PPROF_ZC_DIFF_RESPONSE_CMD;
	memcpy(&appSerialBuf[serialLen], eui48, 6);
	serialLen += 6;
	appSerialBuf[serialLen++] = (uint8_t)(time_freq >> 24);
	appSerialBuf[serialLen++] = (uint8_t)(time_freq >> 16);
	appSerialBuf[serialLen++] = (uint8_t)(time_freq >> 8);
	appSerialBuf[serialLen++] = (uint8_t)(time_freq);
	appSerialBuf[serialLen++] = (uint8_t)(time_diff >> 24);
	appSerialBuf[serialLen++] = (uint8_t)(time_diff >> 16);
	appSerialBuf[serialLen++] = (uint8_t)(time_diff >> 8);
	appSerialBuf[serialLen++] = (uint8_t)(time_diff);

	/* Send packet */
	SRV_USI_Send_Message(gUsiHandle,SRV_USI_PROT_ID_PRIME_API,appSerialBuf,serialLen);
}

static void APP_Modem_BMNG_WhitelistAck(uint8_t cmd, BMNG_WHITELIST_ACK ack_code)
{
	uint16_t serialLen = 0U;

	appSerialBuf[serialLen++] = APP_MODEM_BMNG_WHITELIST_ACK_CMD;
	appSerialBuf[serialLen++] = cmd;
	appSerialBuf[serialLen++] = ack_code;
	
	/* Send packet */
	SRV_USI_Send_Message(gUsiHandle,SRV_USI_PROT_ID_PRIME_API,appSerialBuf,serialLen);
}

static void APP_Modem_USI_PRIME_ApiHandler(uint8_t *rxMsg, size_t inputLen)
{
	if (!APP_MODEM_MSG_RCV[inputMsgRecvIndex].len) 
    {
		if (inputLen < MAX_LENGTH_BUFF) 
        {
			memcpy(APP_MODEM_MSG_RCV[inputMsgRecvIndex].dataBuf, rxMsg, inputLen);
			APP_MODEM_MSG_RCV[inputMsgRecvIndex].len = inputLen;

			if (++inputMsgRecvIndex == MAX_NUM_MSG_RCV) 
            {
				inputMsgRecvIndex = 0;
			}

			return;
		} 
        else 
        {/* ERROR ,Message too big */
			PRIME_HAL_WRP_DebugReport(SRV_LOG_REPORT_INFO, APP_MODEM_ERR_MSG_TOO_BIG, "ERROR ,Message too big\r\n"); 
		}
	} 
    else 
    {/* Error, RX queue is full */
		PRIME_HAL_WRP_DebugReport(SRV_LOG_REPORT_INFO, APP_MODEM_ERR_QUEUE_FULL,"ERROR ,RX queue full\r\n" );
	}

	return;
}

static void APP_Modem_SetCallbacks(void)
{
	MAC_CALLBACKS mac_callbacks;

	CL_432_CALLBACKS cl432_callbacks;

	BMNG_CALLBACKS bmng_callbacks;

	mac_callbacks.mac_data_cfm = APP_Modem_dataConfirm;
	mac_callbacks.mac_data_ind = APP_Modem_dataIndication;
	mac_callbacks.mac_establish_cfm = APP_Modem_establishConfirm;
	mac_callbacks.mac_establish_ind = APP_Modem_establishIndication;
	mac_callbacks.mac_join_cfm = APP_Modem_joinConfirm;
	mac_callbacks.mac_join_ind = APP_Modem_joinIndication;
	mac_callbacks.mac_leave_cfm = APP_Modem_leaveConfirm;
	mac_callbacks.mac_leave_ind = APP_Modem_leaveIndication;
	mac_callbacks.mac_release_cfm = APP_Modem_releaseConfirm;
	mac_callbacks.mac_release_ind = APP_Modem_releaseIndication;

	mac_callbacks.mlme_get_cfm = APP_Modem_MLME_GetConfirm;
	mac_callbacks.mlme_list_get_cfm = APP_Modem_MLME_ListGetConfirm;
	mac_callbacks.mlme_promote_cfm = APP_Modem_MLME_PromteConfirm;
	mac_callbacks.mlme_mp_promote_cfm = APP_Modem_MLME_MP_PromteConfirm;
	mac_callbacks.mlme_reset_cfm = APP_Modem_MLME_ResetConfirm;
	mac_callbacks.mlme_set_cfm = APP_Modem_MLME_SetConfirm;

	mac_callbacks.plme_get_cfm = APP_Modem_PLME_GetConfirm;
	mac_callbacks.plme_reset_cfm = APP_Modem_PLME_resetConfirm;
	mac_callbacks.plme_resume_cfm = APP_Modem_PLME_ResumeConfirm;
	mac_callbacks.plme_set_cfm = APP_Modem_PLME_SetConfirm;
	mac_callbacks.plme_sleep_cfm = APP_Modem_PLME_SleepConfirm;
	mac_callbacks.plme_testmode_cfm = NULL;

	gPrimeApi->MacSetCallbacks(&mac_callbacks);

	cl432_callbacks.cl_432_dl_data_cfm = APP_Modem_CL432_DlDataConfirm;
	cl432_callbacks.cl_432_dl_data_ind = APP_Modem_CL432_DlDataIndication;
	cl432_callbacks.cl_432_join_ind = APP_Modem_CL432_JoinIndication;
	cl432_callbacks.cl_432_leave_ind = APP_Modem_CL432_LeaveIndiication;

	gPrimeApi->Cl432SetCallbacks(&cl432_callbacks);


	bmng_callbacks.fup_ack = APP_Modem_BMNG_FupAck;
	bmng_callbacks.fup_error_ind = APP_Modem_BMNG_FupErrorIndication;
	bmng_callbacks.fup_kill_ind = APP_Modem_BMNG_FupKillIndication;
	bmng_callbacks.fup_status_ind = APP_Modem_BMNG_FupStatusIndication;
	bmng_callbacks.fup_version_ind = APP_Modem_BMNG_FupVersionIndication;
	bmng_callbacks.network_event_ind = APP_Modem_BMNG_NetEventIndication;
	bmng_callbacks.pprof_ack = APP_Modem_BMNG_PPROFAck;
	bmng_callbacks.pprof_get_response = APP_Modem_BMNG_PPROFGetResponse;
	bmng_callbacks.pprof_get_enhanced_response = APP_Modem_BMNG_PPROFGetEnhancedResponse;
	bmng_callbacks.pprof_get_zc_response = APP_Modem_BMNG_PPROFGetZCResponse;
	bmng_callbacks.pprof_zc_diff_response = APP_Modem_BMNG_PPROFDiffZCResponse;
	bmng_callbacks.whitelist_ack = APP_Modem_BMNG_WhitelistAck;

	gPrimeApi->BmngSetCallbacks(&bmng_callbacks);
}

static void APP_Modem_MacEstablishReuestCmd(uint8_t *recvMsg)
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

    gPrimeApi->MacEstablishResponse(con_handle, (MAC_ESTABLISH_RESPONSE_ANSWER)answer, data, data_len, ae);

}

static void APP_Modem_MacRedirectResponseCmd(uint8_t *recvMsg)
{
	uint8_t *lMessage;
	uint16_t con_handle;
	uint8_t eui48[6];
	uint8_t *data;
	uint16_t data_len;

	lMessage = recvMsg;
	con_handle = ((uint16_t)(*lMessage++)) << 8;
	con_handle += *lMessage++;
	memcpy(eui48, lMessage, 6);
	data_len = ((uint16_t)(*lMessage++)) << 8;
	data_len += *lMessage++;
	data = lMessage;

    gPrimeApi->MacRedirectResponse(con_handle, eui48, data, data_len);
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
	uint8_t  broadcast;
	uint8_t con_type;
	uint16_t data_len;
	uint16_t con_handle;
	uint8_t data[256];
	uint8_t eui48[6];
	uint8_t ae;

	/* Extract parameters */
	lMessage = recvMsg;
	broadcast = *lMessage++;
	con_handle = ((uint16_t)(*lMessage++)) << 8;
	con_handle += *lMessage++;
	memcpy(eui48, lMessage, 6);
	lMessage += 6;
	con_type = *lMessage++;
	data_len = ((uint16_t)(*lMessage++)) << 8;
	data_len += *lMessage++;
	memcpy(data, lMessage, data_len);
	lMessage += data_len;
	ae = *lMessage++;

	gPrimeApi->MacJoinRequest((MAC_JOIN_REQUEST_MODE)broadcast, con_handle, eui48, con_type, data, data_len, ae);
}

static void APP_Modem_MacJoinResponseCmd(uint8_t *recvMsg)
{
	uint8_t *lMessage;
	uint16_t con_handle;
	uint8_t answer;
	uint8_t eui48[6];
	uint8_t ae;

	/* Extract parameters */
	lMessage = recvMsg;
	con_handle = ((uint16_t)(*lMessage++)) << 8;
	con_handle += *lMessage++;
	memcpy(eui48, lMessage, 6);
	lMessage += 6;
	answer = *lMessage++;
	ae = *lMessage++;

	gPrimeApi->MacJoinResponse(con_handle, eui48, (MAC_JOIN_RESPONSE_ANSWER)answer, ae);
}

static void APP_Modem_MacLeaveRequestCmd(uint8_t *recvMsg)
{
	uint8_t *lMessage;
	uint16_t con_handle;
	uint8_t eui48[6];

	/* Extract parameters */
	lMessage = recvMsg;
	con_handle = ((uint16_t)(*lMessage++)) << 8;
	con_handle += *lMessage++;
	memcpy(eui48, lMessage, 6);

	gPrimeApi->MacLeaveRequest(con_handle, eui48);
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
	txdata_index = true;
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
	switch (pib_size) {
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

static void APP_Modem_MLME_PromoteRequestCmd(uint8_t *recvMsg)
{
	uint8_t *lMessage;
	uint8_t eui48[6];
	uint8_t bcn_mode;

	/* Extract parameters */
	lMessage = recvMsg;
	memcpy(eui48, lMessage, 6);
	lMessage += 6;
	bcn_mode = *lMessage++;

	gPrimeApi->MlmePromoteRequest(eui48, bcn_mode);
}

static void APP_Modem_MLME_MP_PromoteRequestCmd(uint8_t *recvMsg)
{
	uint8_t *lMessage;
	uint8_t eui48[6];
	uint16_t pch;
	uint8_t bcn_mode;

	/* Extract parameters */
	lMessage = recvMsg;
	memcpy(eui48, lMessage, 6);
	lMessage += 6;
	bcn_mode = *lMessage++;
	pch = ((uint16_t)(*lMessage++)) << 8;
	pch += *lMessage++;

	gPrimeApi->MlmeMpPromoteRequest(eui48, bcn_mode, pch);
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
	switch (pib_size) {
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

static void APP_Modem_CL432DataRequestCmd(uint8_t *recvMsg)
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
	if (lsdu_len <= CL_432_MAX_LENGTH_DATA) {
		memcpy(buff_432.dl.buff, lMessage, lsdu_len);
		lMessage += lsdu_len;
		link_class = *lMessage++;

		gPrimeApi->Cl432DlDataRequest(dst_lsap, src_lsap, dst_address, &buff_432, lsdu_len, link_class);
	}

	/* Tx data indication */
	txdata_index = true;
}

static void APP_Modem_BMNG_FupAddTargetRequestCmd(uint8_t *recvMsg)
{
	uint8_t *lMessage;
	uint8_t eui48[6];

	/* Extract parameters */
	lMessage = recvMsg;
	memcpy(eui48, lMessage, 6);

	gPrimeApi->BmngFupAddTargetRequest(APP_MODEM_BMNG_FUP_ADD_TARGET_REQUEST_CMD, eui48);
}

static void APP_Modem_BMNG_FupSetFwDataRequestCmd(uint8_t *recvMsg)
{
	uint8_t *lMessage;
	char vendor[32], model[32], version[32];
	uint8_t vendor_len, model_len, version_len;

	/* Extract parameters */
	lMessage = recvMsg;
	vendor_len = *lMessage++;
	memcpy(vendor, lMessage, vendor_len);
	lMessage += vendor_len;
	model_len = *lMessage++;
	memcpy(model, lMessage, model_len);
	lMessage += model_len;
	version_len = *lMessage++;
	memcpy(version, lMessage, version_len);

	gPrimeApi->BmngFupSetFwDataRequest(APP_MODEM_BMNG_FUP_SET_FW_DATA_REQUEST_CMD, vendor_len, vendor, model_len, model, version_len,
			version);
}

static void APP_Modem_BMNG_FupSetUpgOptionRequestCmd(uint8_t *recvMsg)
{
	uint8_t *lMessage;
	uint32_t delayRestart;
	uint32_t safetyTimer;
	BMNG_FUP_PAGE_SIZE page_size;
	uint8_t arqEn;
	uint8_t multicastEn;

	/* Extract parameters */
	lMessage = recvMsg;
	arqEn = *lMessage++;
	page_size = (BMNG_FUP_PAGE_SIZE)*lMessage++;
	multicastEn = *lMessage++;
	delayRestart = ((uint32_t)(*lMessage++)) << 24;
	delayRestart += ((uint32_t)(*lMessage++)) << 16;
	delayRestart += ((uint32_t)(*lMessage++)) << 8;
	delayRestart += *lMessage++;
	safetyTimer = ((uint32_t)(*lMessage++)) << 24;
	safetyTimer += ((uint32_t)(*lMessage++)) << 16;
	safetyTimer += ((uint32_t)(*lMessage++)) << 8;
	safetyTimer += *lMessage++;

	gPrimeApi->BmngFupSetUpgradeOptionsRequest(APP_MODEM_BMNG_FUP_SET_UPGRADE_REQUEST_CMD, arqEn, page_size, multicastEn, delayRestart,
			safetyTimer);
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

	gPrimeApi->BmngFupInitFileTxRequest(APP_MODEM_BMNG_FUP_INIT_FILE_TX_REQUEST_CMD, frameNumber, fileSize, frameSize, crc);
}

static void APP_Modem_BMNG_FupDataFrameRequestCmd(uint8_t *recvMsg)
{
	uint8_t *lMessage;
	uint8_t data[1000];
	uint16_t frameNumber, data_len;

	/* Extract parameters */
	lMessage = recvMsg;
	frameNumber = ((uint16_t)(*lMessage++)) << 8;
	frameNumber += *lMessage++;
	data_len = ((uint16_t)(*lMessage++)) << 8;
	data_len += *lMessage++;
	memcpy(data, lMessage, data_len);

	gPrimeApi->BmngFupDataFrameRequest(APP_MODEM_BMNG_FUP_DATA_FRAME_REQUEST_CMD, frameNumber, data_len, data);
}

static void APP_Modem_BMNG_FupAbortFuRequestCmd(uint8_t *recvMsg)
{
	uint8_t *lMessage;
	uint8_t eui48[6];

	/* Extract parameters */
	lMessage = recvMsg;
	memcpy(eui48, lMessage, 6);

	gPrimeApi->BmngFupAbortFuRequest(APP_MODEM_BMNG_FUP_ABORT_FU_REQUEST_CMD, eui48);
}

static void APP_Modem_BMNG_FupStartFuRequestCmd(uint8_t *recvMsg)
{
	uint8_t *lMessage;
	uint8_t enable;

	/* Extract parameters */
	lMessage = recvMsg;
	enable = *lMessage++;

	gPrimeApi->BmngFupStartFuRequest(APP_MODEM_BMNG_FUP_START_FU_REQUEST_CMD, enable);
}

static void APP_Modem_BMNG_FupSetMatchRuleRequestCmd(uint8_t *recvMsg)
{
	uint8_t *lMessage;
	uint8_t rules;

	/* Extract parameters */
	lMessage = recvMsg;
	rules = *lMessage++;

	gPrimeApi->BmngFupSetMatchRuleRequest(APP_MODEM_BMNG_FUP_SET_MATCH_RULE_REQUEST_CMD, rules);
}

static void APP_Modem_BMNG_FupGetVersionRequestCmd(uint8_t *recvMsg)
{
	uint8_t *lMessage;
	uint8_t eui48[6];

	/* Extract parameters */
	lMessage = recvMsg;
	memcpy(eui48, lMessage, 6);

	gPrimeApi->BmngFupGetVersionRequest(APP_MODEM_BMNG_FUP_GET_VERSION_REQUEST_CMD, eui48);
}

static void APP_Modem_BMNG_FupGetStateRequestCmd(uint8_t *recvMsg)
{
	uint8_t *lMessage;
	uint8_t eui48[6];

	/* Extract parameters */
	lMessage = recvMsg;
	memcpy(eui48, lMessage, 6);

	gPrimeApi->BmngFupGetStateRequest(APP_MODEM_BMNG_FUP_GET_STATE_REQUEST_CMD, eui48);
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

	gPrimeApi->BmngFupSetSignatureDataRequest(APP_MODEM_BMNG_FUP_SET_SIGNATURE_DATA_REQUEST_CMD, algorithm, len);
}

static void APP_Modem_BMNG_PprofGetRequestCmd(uint8_t *recvMsg)
{
	uint8_t *lMessage;
	uint8_t data[1024];
	uint8_t eui48[6];
	uint16_t data_len;

	/* Extract parameters */
	lMessage = recvMsg;
	memcpy(eui48, lMessage, 6);
	lMessage += 6;
	data_len = ((uint16_t)(*lMessage++)) << 8;
	data_len += *lMessage++;
	memcpy(data, lMessage, data_len);

	gPrimeApi->BmngPprofGetRequest(APP_MODEM_BMNG_PPROF_GET_REQUEST_CMD, eui48, data_len, data);
}

static void APP_Modem_BMNG_PprofSetRequestCmd(uint8_t *recvMsg)
{
	uint8_t *lMessage;
	uint8_t data[1024];
	uint8_t eui48[6];
	uint16_t data_len;

	/* Extract parameters */
	lMessage = recvMsg;
	memcpy(eui48, lMessage, 6);
	lMessage += 6;
	data_len = ((uint16_t)(*lMessage++)) << 8;
	data_len += *lMessage++;
	memcpy(data, lMessage, data_len);

	gPrimeApi->BmngPprofSetRequest(APP_MODEM_BMNG_PPROF_SET_REQUEST_CMD, eui48, data_len, data);
}

static void APP_Modem_BMNG_PprofResetRequestCmd(uint8_t *recvMsg)
{
	uint8_t *lMessage;
	uint8_t eui48[6];

	/* Extract parameters */
	lMessage = recvMsg;
	memcpy(eui48, lMessage, 6);

	gPrimeApi->BmngPprofResetRequest(APP_MODEM_BMNG_PPROF_RESET_REQUEST_CMD, eui48);
}

static void APP_Modem_BMNG_PprofRebootRequestCmd(uint8_t *recvMsg)
{
	uint8_t *lMessage;
	uint8_t eui48[6];

	/* Extract parameters */
	lMessage = recvMsg;
	memcpy(eui48, lMessage, 6);

	gPrimeApi->BmngPprofRebootRequest(APP_MODEM_BMNG_PPROF_REBOOT_REQUEST_CMD, eui48);
}

static void APP_Modem_BMNG_PprofGetEnhancedRequestCmd(uint8_t *recvMsg)
{
	uint8_t *lMessage;
	uint8_t data[1024];
	uint8_t eui48[6];
	uint16_t data_len;

	/* Extract parameters */
	lMessage = recvMsg;
	memcpy(eui48, lMessage, 6);
	lMessage += 6;
	data_len = ((uint16_t)(*lMessage++)) << 8;
	data_len += *lMessage++;
	memcpy(data, lMessage, data_len);

	gPrimeApi->BmngPprofGetEnhancedRequest(APP_MODEM_BMNG_PPROF_GET_ENHANCED_RESPONSE_CMD, eui48, data_len, data);
}

static void APP_Modem_BMNG_PprofZcDiffRequestCmd(uint8_t *recvMsg)
{
	uint8_t *lMessage;
	uint8_t eui48[6];

	/* Extract parameters */
	lMessage = recvMsg;
	memcpy(eui48, lMessage, 6);

	gPrimeApi->BmngPprofGetZcDiffRequest(APP_MODEM_BMNG_PPROF_ZC_DIFF_REQUEST_CMD, eui48);
}

static void APP_Modem_BMNG_WhitelistAddRequestCmd(uint8_t *recvMsg)
{
	uint8_t *lMessage;
	uint8_t eui48[6];

	/* Extract parameters */
	lMessage = recvMsg;
	memcpy(eui48, lMessage, 6);

	gPrimeApi->BmngWhitelistAddRequest(APP_MODEM_BMNG_WHITELIST_ADD_REQUEST_CMD, eui48);
}

static void APP_Modem_BMNG_WhitelistRemoveRequestCmd(uint8_t *recvMsg)
{
	uint8_t *lMessage;
	uint8_t eui48[6];

	/* Extract parameters */
	lMessage = recvMsg;
	memcpy(eui48, lMessage, 6);

	gPrimeApi->BmngWhitelistRemoveRequest(APP_MODEM_BMNG_WHITELIST_REMOVE_REQUEST_CMD, eui48);
}

void modem_debug_report(uint32_t ul_err_type)
{
	uint16_t serialLen = 0U;

	appSerialBuf[serialLen++] = APP_MODEM_DEBUG_REPORT_CMD;
	appSerialBuf[serialLen++] = (uint8_t)((uint32_t)ul_err_type >> 24);
	appSerialBuf[serialLen++] = (uint8_t)((uint32_t)ul_err_type >> 16);
	appSerialBuf[serialLen++] = (uint8_t)((uint32_t)ul_err_type >> 8);
	appSerialBuf[serialLen++] = (uint8_t)((uint32_t)ul_err_type);

	/* Send packet */
	SRV_USI_Send_Message(gUsiHandle,SRV_USI_PROT_ID_PRIME_API,appSerialBuf,serialLen);
}

void APP_Modem_Initialize(void)
{
	uint8_t index=0;
    
    
	/* Initialize the reception queue */
	inputMsgRecvIndex = 0;
	outputMsgRecvIndex = 0;
	for (index = 0; index < MAX_NUM_MSG_RCV; index++) 
    {
		APP_MODEM_MSG_RCV[index].len = 0;
	}

    /* Get PRIME API */
    PRIME_API_GetPrimeAPI(&gPrimeApi);
    
    /* Set callback functions */
    APP_Modem_SetCallbacks();

    /* Open USI */
    gUsiHandle = SRV_USI_Open(SRV_USI_INDEX_0);
    
    /* Configure USI protocol handler */
    SRV_USI_CallbackRegister(gUsiHandle, SRV_USI_PROT_ID_PRIME_API, 
                             APP_Modem_USI_PRIME_ApiHandler);

	/* Initialize TxRx data indicators */
	rxdata_index = false;
	txdata_index = false;
} 

void App_Modem_Tasks(void)
{
	/* Check data reception */
	while(APP_MODEM_MSG_RCV[outputMsgRecvIndex].len) 
    {
		APP_MODEM_PRIME_API_CMD apiCmd;
		uint8_t *recvBuf;

		/* Extract cmd */
		recvBuf = APP_MODEM_MSG_RCV[outputMsgRecvIndex].dataBuf;
		apiCmd = (APP_MODEM_PRIME_API_CMD)*recvBuf++;
		switch (apiCmd) {
		case APP_MODEM_CL_NULL_ESTABLISH_REQUEST_CMD:
			APP_Modem_MacEstablishReuestCmd(recvBuf);
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

		case APP_MODEM_MLME_MP_PROMOTE_REQUEST_CMD:
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
            gPrimeApi->BmngFupClearTargetListRequest(APP_MODEM_BMNG_FUP_CLEAR_TARGET_REQUEST_CMD);
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
            gPrimeApi->BmngFupCheckCrcRequest(APP_MODEM_BMNG_FUP_CHECK_CRC_REQUEST_CMD);
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

		APP_MODEM_MSG_RCV[outputMsgRecvIndex].len = 0;
		if (++outputMsgRecvIndex == MAX_NUM_MSG_RCV) {
			outputMsgRecvIndex = 0;
		}
	}
}
