/******************************************************************************
  RF PHY Sniffer Serialization Implementation.

  Company:
    Microchip Technology Inc.

  File Name:
    srv_rsniffer.c

  Summary:
    Source code for the RF PHY sniffer serialization implementation.

  Description:
    The RF PHY sniffer serialization provides a service to format messages
    through serial connection in order to communicate with Hybrid Sniffer Tool
    provided by Microchip. This file contains the source code for the
    implementation of the RF PHY sniffer serialization.

*******************************************************************************/

//DOM-IGNORE-BEGIN
/*******************************************************************************
* Copyright (C) 2022 Microchip Technology Inc. and its subsidiaries.
*
* Subject to your compliance with these terms, you may use Microchip software
* and any derivatives exclusively with Microchip products. It is your
* responsibility to comply with third party license terms applicable to your
* use of third party software (including open source software) that may
* accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
* EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
* WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
* PARTICULAR PURPOSE.
*
* IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
* INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
* WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
* BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
* FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
* ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
*******************************************************************************/
//DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
#include <string.h>
#include "srv_rsniffer.h"
#include "configuration.h"
#include "system/time/sys_time.h"

// *****************************************************************************
// *****************************************************************************
// Section: Macro definitions
// *****************************************************************************
// *****************************************************************************

/* PRIME sniffer identifiers and version */
#define RSNIFFER_RF215_PRIME                          0x13
#define RSNIFFER_RF215_PRIME_EXTENDED                 0x33
#define RSNIFFER_RF215_PRIME_SIMULATOR                0xD3
#define RSNIFFER_VERSION                              0x14

/* PRIME sniffer message types */
#define RSNIFFER_PHY_MESSAGE_TYPE_A                   0x20
#define RSNIFFER_PHY_MESSAGE_TYPE_B                   0x21
#define RSNIFFER_PHY_MESSAGE_TYPE_BC                  0x22
#define RSNIFFER_PHY_MESSAGE_TYPE_RF_FSK50            0x23
#define RSNIFFER_PHY_MESSAGE_TYPE_RF_FSK100           0x24
#define RSNIFFER_PHY_MESSAGE_TYPE_RF_FSK150           0x25
#define RSNIFFER_PHY_MESSAGE_TYPE_RF_FSK200           0x26
#define RSNIFFER_PHY_MESSAGE_TYPE_RF_FSK300           0x27
#define RSNIFFER_PHY_MESSAGE_TYPE_RF_FSK400           0x28
#define RSNIFFER_PHY_MESSAGE_TYPE_RF_4FSK50           0x29
#define RSNIFFER_PHY_MESSAGE_TYPE_RF_4FSK100          0x2A
#define RSNIFFER_PHY_MESSAGE_TYPE_RF_4FSK150          0x2B
#define RSNIFFER_PHY_MESSAGE_TYPE_RF_4FSK200          0x2C
#define RSNIFFER_PHY_MESSAGE_TYPE_RF_4FSK300          0x2D
#define RSNIFFER_PHY_MESSAGE_TYPE_RF_4FSK400          0x2E
#define RSNIFFER_PHY_MESSAGE_TYPE_RF_OFDM1            0x2F
#define RSNIFFER_PHY_MESSAGE_TYPE_RF_OFDM2            0x30
#define RSNIFFER_PHY_MESSAGE_TYPE_RF_OFDM3            0x31
#define RSNIFFER_PHY_MESSAGE_TYPE_RF_OFDM4            0x32

/* PRIME sniffer PHY modulations */
#define RSNIFFER_PHY_MESSAGE_MOD_DBPSK                0x00
#define RSNIFFER_PHY_MESSAGE_MOD_DQPSK                0x01
#define RSNIFFER_PHY_MESSAGE_MOD_D8PSK                0x02
#define RSNIFFER_PHY_MESSAGE_MOD_DBPSK_C              0x04
#define RSNIFFER_PHY_MESSAGE_MOD_DQPSK_C              0x05
#define RSNIFFER_PHY_MESSAGE_MOD_D8PSK_C              0x06
#define RSNIFFER_PHY_MESSAGE_MOD_R_DBPSK              0x0C
#define RSNIFFER_PHY_MESSAGE_MOD_R_DQPSK              0x0D
#define RSNIFFER_PHY_MESSAGE_MOD_R_DQPSK              0x0D
#define RSNIFFER_PHY_MESSAGE_MOD_RF_FSK_FEC_OFF       0x10
#define RSNIFFER_PHY_MESSAGE_MOD_RF_FSK_FEC_ON        0x11
#define RSNIFFER_PHY_MESSAGE_MOD_RF_OFDM_MCS0         0x12
#define RSNIFFER_PHY_MESSAGE_MOD_RF_OFDM_MCS1         0x13
#define RSNIFFER_PHY_MESSAGE_MOD_RF_OFDM_MCS2         0x14
#define RSNIFFER_PHY_MESSAGE_MOD_RF_OFDM_MCS3         0x15
#define RSNIFFER_PHY_MESSAGE_MOD_RF_OFDM_MCS4         0x16
#define RSNIFFER_PHY_MESSAGE_MOD_RF_OFDM_MCS5         0x17
#define RSNIFFER_PHY_MESSAGE_MOD_RF_OFDM_MCS6         0x18

/* Buffer sizes */
#define RSNIFFER_MSG_HEADER_SIZE                      32
#define RSNIFFER_MSG_MAX_SIZE                         (DRV_RF215_MAX_PSDU_LEN + RSNIFFER_MSG_HEADER_SIZE)

// *****************************************************************************
// *****************************************************************************
// Section: File Scope Data
// *****************************************************************************
// *****************************************************************************

static uint64_t srvRsnifferPrevSysTime;
static uint32_t srvRsnifferPrevTimeUS;
static uint8_t srvRsnifferRxMsg[RSNIFFER_MSG_MAX_SIZE];
static uint8_t srvRsnifferTxMsg[DRV_RF215_TX_BUFFERS_NUMBER][RSNIFFER_MSG_MAX_SIZE];

// *****************************************************************************
// *****************************************************************************
// Section: File Scope Functions
// *****************************************************************************
// *****************************************************************************

static uint8_t _SRV_RSNIFFER_FrameType(DRV_RF215_PHY_CFG_OBJ* pPhyCfgObj)
{
    uint8_t frameType;

    frameType = (uint8_t) pPhyCfgObj->phyTypeCfg.fsk.symRate;
    if (pPhyCfgObj->phyTypeCfg.fsk.modOrd == FSK_MOD_ORD_2FSK)
    {
        frameType += RSNIFFER_PHY_MESSAGE_TYPE_RF_FSK50;
    }
    else /* FSK_MOD_ORD_4FSK */
    {
        frameType += RSNIFFER_PHY_MESSAGE_TYPE_RF_4FSK50;
    }

    return frameType;
}

static uint32_t _SRV_RSNIFFER_SysTimeToUS(uint64_t sysTime)
{
    int64_t sysTimeDiff;
    uint32_t timeUS = srvRsnifferPrevTimeUS;

    /* Difference between current and previous system time */
    sysTimeDiff = (int64_t) sysTime - srvRsnifferPrevSysTime;

    /* Convert system time to microseconds and add to previous time */
    while (sysTimeDiff > 0x10000000)
    {
        timeUS += SYS_TIME_CountToUS(0x10000000);
        sysTimeDiff -= 0x10000000;
    }
    while (sysTimeDiff < -0x10000000)
    {
        timeUS -= SYS_TIME_CountToUS(0x10000000);
        sysTimeDiff += 0x10000000;
    }

    if (sysTimeDiff >= 0)
    {
        timeUS += SYS_TIME_CountToUS((uint32_t) sysTimeDiff);
    }
    else
    {
        timeUS -= SYS_TIME_CountToUS((uint32_t) (-sysTimeDiff));
    }

    /* Store times for next computation */
    srvRsnifferPrevSysTime = sysTime;
    srvRsnifferPrevTimeUS = timeUS;

    return timeUS;
}

// *****************************************************************************
// *****************************************************************************
// Section: RF PHY Sniffer Serialization Interface Implementation
// *****************************************************************************
// *****************************************************************************

uint8_t* SRV_RSNIFFER_SerialRxMessage (
    DRV_RF215_RX_INDICATION_OBJ* pIndObj,
    DRV_RF215_PHY_CFG_OBJ* pPhyCfgObj,
    uint16_t paySymbols,
    uint16_t channel,
    size_t* pMsgLen
)
{
    uint32_t timeIni, timeEnd;
    uint16_t psduLen;
    int16_t rssi;

    /* Frame type depending on RF PHY configuration */
    srvRsnifferRxMsg[0] = _SRV_RSNIFFER_FrameType(pPhyCfgObj);

    /* Sniffer version and sniffer type */
    srvRsnifferRxMsg[1] = RSNIFFER_VERSION;
    srvRsnifferRxMsg[2] = RSNIFFER_RF215_PRIME;

    /* Frame modulation */
    srvRsnifferRxMsg[3] = (uint8_t) pIndObj->modScheme + RSNIFFER_PHY_MESSAGE_MOD_RF_FSK_FEC_OFF;

    /* Number of payload symbols */
    srvRsnifferRxMsg[4] = (uint8_t) (paySymbols >> 8);
    srvRsnifferRxMsg[5] = (uint8_t) (paySymbols);

    /* Channel */
    srvRsnifferRxMsg[6] = (uint8_t) (channel >> 8);
    srvRsnifferRxMsg[7] = (uint8_t) (channel);

    /* Initial and end time of RX frame */
    timeIni = _SRV_RSNIFFER_SysTimeToUS(pIndObj->timeIni);
    srvRsnifferRxMsg[19] = (uint8_t) (timeIni >> 24);
    srvRsnifferRxMsg[20] = (uint8_t) (timeIni >> 16);
    srvRsnifferRxMsg[21] = (uint8_t) (timeIni >> 8);
    srvRsnifferRxMsg[22] = (uint8_t) (timeIni);
    timeEnd = timeIni + pIndObj->ppduDurationUS;
    srvRsnifferRxMsg[23] = (uint8_t) (timeEnd >> 24);
    srvRsnifferRxMsg[24] = (uint8_t) (timeEnd >> 16);
    srvRsnifferRxMsg[25] = (uint8_t) (timeEnd >> 8);
    srvRsnifferRxMsg[26] = (uint8_t) (timeEnd);

    /* RSSI */
    rssi = (int16_t) pIndObj->rssiDBm;
    srvRsnifferRxMsg[27] = (uint8_t) (rssi >> 8);
    srvRsnifferRxMsg[28] = (uint8_t) (rssi);

    /* mac_enable not supported */
    srvRsnifferRxMsg[29] = 0;

    /* Data PSDU length (including PRIME CRC) */
    psduLen = pIndObj->psduLen;
    srvRsnifferRxMsg[30] = (uint8_t) (psduLen >> 8);
    srvRsnifferRxMsg[31] = (uint8_t) (psduLen);

    /* Copy PSDU */
    memcpy(srvRsnifferRxMsg + RSNIFFER_MSG_HEADER_SIZE, pIndObj->psdu, psduLen);

    *pMsgLen = (size_t) psduLen + RSNIFFER_MSG_HEADER_SIZE;
    return srvRsnifferRxMsg;
}

void SRV_RSNIFFER_SetTxMessage (
    DRV_RF215_TX_REQUEST_OBJ* pReqObj,
    DRV_RF215_TX_HANDLE txHandle
)
{
    uint8_t* pMsgDest;
    uint16_t psduLen;
    int16_t rssi;
    uint8_t txBufIndex;

    if (txHandle == DRV_RF215_TX_HANDLE_INVALID)
    {
        return;
    }

    /* Get buffer index from TX handle */
    txBufIndex = (uint8_t) (txHandle & 0xFF);
    pMsgDest = srvRsnifferTxMsg[txBufIndex];

    /* Frame modulation */
    srvRsnifferRxMsg[3] = (uint8_t) pReqObj->modScheme + RSNIFFER_PHY_MESSAGE_MOD_RF_FSK_FEC_OFF;

    /* RSSI */
    rssi = 14 - (int16_t) pReqObj->txPwrAtt;
    pMsgDest[27] = (uint8_t) (rssi >> 8);
    pMsgDest[28] = (uint8_t) (rssi);

    /* mac_enable not supported */
    srvRsnifferRxMsg[29] = 0;

    /* Data PSDU length (including PRIME CRC) */
    psduLen = pReqObj->psduLen;
    pMsgDest[30] = (uint8_t) (psduLen >> 8);
    pMsgDest[31] = (uint8_t) (psduLen);

    /* Copy PHY data message */
    memcpy(pMsgDest + RSNIFFER_MSG_HEADER_SIZE, pReqObj->psdu, psduLen);
}

uint8_t* SRV_RSNIFFER_SerialCfmMessage (
    DRV_RF215_TX_CONFIRM_OBJ* pCfmObj,
    DRV_RF215_TX_HANDLE txHandle,
    DRV_RF215_PHY_CFG_OBJ* pPhyCfgObj,
    uint16_t paySymbols,
    uint16_t channel,
    size_t* pMsgLen
)
{
    uint8_t* pMsgDest;
    uint32_t timeIni, timeEnd;
    uint16_t psduLen;
    uint8_t txBufIndex;

    if (pCfmObj->txResult != RF215_TX_SUCCESS)
    {
        /* Error in transmission: No report */
        *pMsgLen = 0;
        return NULL;
    }

    /* Get buffer index from TX handle */
    txBufIndex = (uint8_t) (txHandle & 0xFF);
    pMsgDest = srvRsnifferTxMsg[txBufIndex];

    /* Frame type depending on RF PHY configuration */
    pMsgDest[0] = _SRV_RSNIFFER_FrameType(pPhyCfgObj);

    /* Sniffer version and sniffer type */
    pMsgDest[1] = RSNIFFER_VERSION;
    pMsgDest[2] = RSNIFFER_RF215_PRIME;

    /* Number of payload symbols */
    pMsgDest[4] = (uint8_t) (paySymbols >> 8);
    pMsgDest[5] = (uint8_t) (paySymbols);

    /* Channel */
    srvRsnifferRxMsg[6] = (uint8_t) (channel >> 8);
    srvRsnifferRxMsg[7] = (uint8_t) (channel);

    /* Initial and end time of RX frame */
    timeIni = _SRV_RSNIFFER_SysTimeToUS(pCfmObj->timeIni);
    pMsgDest[19] = (uint8_t) (timeIni >> 24);
    pMsgDest[20] = (uint8_t) (timeIni >> 16);
    pMsgDest[21] = (uint8_t) (timeIni >> 8);
    pMsgDest[22] = (uint8_t) (timeIni);
    timeEnd = timeIni + pCfmObj->ppduDurationUS;
    pMsgDest[23] = (uint8_t) (timeEnd >> 24);
    pMsgDest[24] = (uint8_t) (timeEnd >> 16);
    pMsgDest[25] = (uint8_t) (timeEnd >> 8);
    pMsgDest[26] = (uint8_t) (timeEnd);

    psduLen = (uint16_t) (pMsgDest[23] << 8) + pMsgDest[24];
    *pMsgLen = (size_t) psduLen + RSNIFFER_MSG_HEADER_SIZE;
    return pMsgDest;
}
