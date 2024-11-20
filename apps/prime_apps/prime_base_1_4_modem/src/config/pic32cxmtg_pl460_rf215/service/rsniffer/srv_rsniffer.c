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
#define RSNIFFER_RF215_PRIME                          0x13U
#define RSNIFFER_RF215_PRIME_EXTENDED                 0x33U
#define RSNIFFER_RF215_PRIME_SIMULATOR                0xD3U
#define RSNIFFER_VERSION                              0x14U

/* PRIME sniffer message types */
#define RSNIFFER_TYPE_A                               0x20U
#define RSNIFFER_TYPE_B                               0x21U
#define RSNIFFER_TYPE_BC                              0x22U
#define RSNIFFER_TYPE_RF_FSK50                        0x23U
#define RSNIFFER_TYPE_RF_FSK100                       0x24U
#define RSNIFFER_TYPE_RF_FSK150                       0x25U
#define RSNIFFER_TYPE_RF_FSK200                       0x26U
#define RSNIFFER_TYPE_RF_FSK300                       0x27U
#define RSNIFFER_TYPE_RF_FSK400                       0x28U
#define RSNIFFER_TYPE_RF_4FSK50                       0x29U
#define RSNIFFER_TYPE_RF_4FSK100                      0x2AU
#define RSNIFFER_TYPE_RF_4FSK150                      0x2BU
#define RSNIFFER_TYPE_RF_4FSK200                      0x2CU
#define RSNIFFER_TYPE_RF_4FSK300                      0x2DU
#define RSNIFFER_TYPE_RF_4FSK400                      0x2EU
#define RSNIFFER_TYPE_RF_OFDM1                        0x2FU
#define RSNIFFER_TYPE_RF_OFDM2                        0x30U
#define RSNIFFER_TYPE_RF_OFDM3                        0x31U
#define RSNIFFER_TYPE_RF_OFDM4                        0x32U

/* PRIME sniffer PHY modulations */
#define RSNIFFER_MOD_DBPSK                            0x00U
#define RSNIFFER_MOD_DQPSK                            0x01U
#define RSNIFFER_MOD_D8PSK                            0x02U
#define RSNIFFER_MOD_DBPSK_C                          0x04U
#define RSNIFFER_MOD_DQPSK_C                          0x05U
#define RSNIFFER_MOD_D8PSK_C                          0x06U
#define RSNIFFER_MOD_R_DBPSK                          0x0CU
#define RSNIFFER_MOD_R_DQPSK                          0x0DU
#define RSNIFFER_MOD_R_DQPSK                          0x0DU
#define RSNIFFER_MOD_RF_FSK_FEC_OFF                   0x10U
#define RSNIFFER_MOD_RF_FSK_FEC_ON                    0x11U
#define RSNIFFER_MOD_RF_OFDM_MCS0                     0x12U
#define RSNIFFER_MOD_RF_OFDM_MCS1                     0x13U
#define RSNIFFER_MOD_RF_OFDM_MCS2                     0x14U
#define RSNIFFER_MOD_RF_OFDM_MCS3                     0x15U
#define RSNIFFER_MOD_RF_OFDM_MCS4                     0x16U
#define RSNIFFER_MOD_RF_OFDM_MCS5                     0x17U
#define RSNIFFER_MOD_RF_OFDM_MCS6                     0x18U

/* Buffer sizes */
#define RSNIFFER_MSG_HEADER_SIZE                      32U
#define RSNIFFER_MSG_MAX_SIZE                         (DRV_RF215_MAX_PSDU_LEN + RSNIFFER_MSG_HEADER_SIZE)

// *****************************************************************************
// *****************************************************************************
// Section: File Scope Data
// *****************************************************************************
// *****************************************************************************

static uint64_t srvRsnifferPrevSysTime = 0;
static uint32_t srvRsnifferPrevTimeUS = 0;
static uint8_t srvRsnifferRxMsg[RSNIFFER_MSG_MAX_SIZE];
static uint8_t srvRsnifferTxMsg[DRV_RF215_TX_BUFFERS_NUMBER][RSNIFFER_MSG_MAX_SIZE];

// *****************************************************************************
// *****************************************************************************
// Section: File Scope Functions
// *****************************************************************************
// *****************************************************************************

static uint8_t SRV_RSNIFFER_FrameType(DRV_RF215_PHY_CFG_OBJ* pPhyCfgObj)
{
    uint8_t frameType;

    frameType = (uint8_t) pPhyCfgObj->phyTypeCfg.fsk.symRate;
    if (pPhyCfgObj->phyTypeCfg.fsk.modOrd == FSK_MOD_ORD_2FSK)
    {
        frameType += RSNIFFER_TYPE_RF_FSK50;
    }
    else /* FSK_MOD_ORD_4FSK */
    {
        frameType += RSNIFFER_TYPE_RF_4FSK50;
    }

    return frameType;
}

static uint32_t lSRV_RSNIFFER_SysTimeToUS(uint64_t sysTime)
{
    uint64_t sysTimeDiff;
    uint32_t sysTimeDiffNumHigh, sysTimeDiffRemaining;
    uint32_t timeUS = srvRsnifferPrevTimeUS;

    /* Difference between current and previous system time */
    sysTimeDiff = sysTime - srvRsnifferPrevSysTime;
    sysTimeDiffNumHigh = (uint32_t) (sysTimeDiff / 0x10000000UL);
    sysTimeDiffRemaining = (uint32_t) (sysTimeDiff % 0x10000000UL);

    /* Convert system time to microseconds and add to previous time */
    timeUS += (SYS_TIME_CountToUS(0x10000000UL) * sysTimeDiffNumHigh);
    timeUS += SYS_TIME_CountToUS(sysTimeDiffRemaining);

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

SRV_RSNIFFER_COMMAND SRV_RSNIFFER_GetCommand(uint8_t* pDataSrc)
{
    /* Extract Command */
    return (SRV_RSNIFFER_COMMAND)*pDataSrc;
}

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
    srvRsnifferRxMsg[0] = SRV_RSNIFFER_FrameType(pPhyCfgObj);

    /* Sniffer version and sniffer type */
    srvRsnifferRxMsg[1] = RSNIFFER_VERSION;
    srvRsnifferRxMsg[2] = RSNIFFER_RF215_PRIME;

    /* Frame modulation */
    srvRsnifferRxMsg[3] = (uint8_t) pIndObj->modScheme + RSNIFFER_MOD_RF_FSK_FEC_OFF;

    /* Number of payload symbols */
    srvRsnifferRxMsg[4] = (uint8_t) (paySymbols >> 8);
    srvRsnifferRxMsg[5] = (uint8_t) paySymbols;

    /* Channel */
    srvRsnifferRxMsg[6] = (uint8_t) (channel >> 8);
    srvRsnifferRxMsg[7] = (uint8_t) channel;

    /* Initial and end time of RX frame */
    timeIni = lSRV_RSNIFFER_SysTimeToUS(pIndObj->timeIniCount);
    srvRsnifferRxMsg[19] = (uint8_t) (timeIni >> 24);
    srvRsnifferRxMsg[20] = (uint8_t) (timeIni >> 16);
    srvRsnifferRxMsg[21] = (uint8_t) (timeIni >> 8);
    srvRsnifferRxMsg[22] = (uint8_t) timeIni;
    timeEnd = timeIni + SYS_TIME_CountToUS(pIndObj->ppduDurationCount);
    srvRsnifferRxMsg[23] = (uint8_t) (timeEnd >> 24);
    srvRsnifferRxMsg[24] = (uint8_t) (timeEnd >> 16);
    srvRsnifferRxMsg[25] = (uint8_t) (timeEnd >> 8);
    srvRsnifferRxMsg[26] = (uint8_t) timeEnd;

    /* RSSI */
    rssi = (int16_t) pIndObj->rssiDBm;
    srvRsnifferRxMsg[27] = (uint8_t) ((uint16_t) rssi >> 8);
    srvRsnifferRxMsg[28] = (uint8_t) (rssi);

    /* mac_enable not supported */
    srvRsnifferRxMsg[29] = 0;

    /* Data PSDU length (including PRIME CRC) */
    psduLen = pIndObj->psduLen;
    srvRsnifferRxMsg[30] = (uint8_t) (psduLen >> 8);
    srvRsnifferRxMsg[31] = (uint8_t) psduLen;

    /* Copy PSDU */
    (void) memcpy(srvRsnifferRxMsg + RSNIFFER_MSG_HEADER_SIZE, pIndObj->psdu, psduLen);

    *pMsgLen = (size_t) psduLen + RSNIFFER_MSG_HEADER_SIZE;
    return srvRsnifferRxMsg;
}

void SRV_RSNIFFER_SetTxMessage (
    DRV_RF215_TX_REQUEST_OBJ* pReqObj,
    DRV_RF215_PHY_CFG_OBJ* pPhyCfgObj,
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
    txBufIndex = (uint8_t) txHandle;
    pMsgDest = srvRsnifferTxMsg[txBufIndex];

    /* Frame modulation */
    (void)pPhyCfgObj;
    pMsgDest[3] = (uint8_t) pReqObj->modScheme + RSNIFFER_MOD_RF_FSK_FEC_OFF;

    /* RSSI */
    rssi = 14 - (int16_t) pReqObj->txPwrAtt;
    pMsgDest[27] = (uint8_t) ((uint16_t) rssi >> 8);
    pMsgDest[28] = (uint8_t) rssi;

    /* mac_enable not supported */
    pMsgDest[29] = 0;

    /* Data PSDU length (including PRIME CRC) */
    psduLen = pReqObj->psduLen;
    pMsgDest[30] = (uint8_t) (psduLen >> 8);
    pMsgDest[31] = (uint8_t) psduLen;

    /* Copy PHY data message */
    (void) memcpy(pMsgDest + RSNIFFER_MSG_HEADER_SIZE, pReqObj->psdu, psduLen);
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
    txBufIndex = (uint8_t) txHandle;
    pMsgDest = srvRsnifferTxMsg[txBufIndex];

    /* Frame type depending on RF PHY configuration */
    pMsgDest[0] = SRV_RSNIFFER_FrameType(pPhyCfgObj);

    /* Sniffer version and sniffer type */
    pMsgDest[1] = RSNIFFER_VERSION;
    pMsgDest[2] = RSNIFFER_RF215_PRIME;

    /* Number of payload symbols */
    pMsgDest[4] = (uint8_t) (paySymbols >> 8);
    pMsgDest[5] = (uint8_t) (paySymbols);

    /* Channel */
    pMsgDest[6] = (uint8_t) (channel >> 8);
    pMsgDest[7] = (uint8_t) (channel);

    /* Initial and end time of RX frame */
    timeIni = lSRV_RSNIFFER_SysTimeToUS(pCfmObj->timeIniCount);
    pMsgDest[19] = (uint8_t) (timeIni >> 24);
    pMsgDest[20] = (uint8_t) (timeIni >> 16);
    pMsgDest[21] = (uint8_t) (timeIni >> 8);
    pMsgDest[22] = (uint8_t) (timeIni);
    timeEnd = timeIni + SYS_TIME_CountToUS(pCfmObj->ppduDurationCount);
    pMsgDest[23] = (uint8_t) (timeEnd >> 24);
    pMsgDest[24] = (uint8_t) (timeEnd >> 16);
    pMsgDest[25] = (uint8_t) (timeEnd >> 8);
    pMsgDest[26] = (uint8_t) (timeEnd);

    psduLen = ((uint16_t) pMsgDest[30] << 8) + pMsgDest[31];
    *pMsgLen = (size_t) psduLen + RSNIFFER_MSG_HEADER_SIZE;
    return pMsgDest;
}

void SRV_RSNIFFER_ParseConfigCommand (
    uint8_t* pDataSrc,
    uint16_t* pBandOpMode,
    uint16_t* pChannel
)
{
    /* Extract Band and Operating Mode */
    *pBandOpMode = ((uint16_t) pDataSrc[1] << 8) + pDataSrc[2];

    /* Extract Channel */
    *pChannel = ((uint16_t) pDataSrc[3] << 8) + pDataSrc[4];
}

