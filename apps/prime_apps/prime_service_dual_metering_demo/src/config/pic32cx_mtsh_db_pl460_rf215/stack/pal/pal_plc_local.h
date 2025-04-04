/*******************************************************************************
  Company:
    Microchip Technology Inc.

  File Name:
    pal_plc_local.h

  Summary:
    Platform Abstraction Layer (PAL) PLC Interface Local header file.

  Description:
    Platform Abstraction Layer (PAL) PLC Interface Local header file.
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

#ifndef PAL_PLC_LOCAL_H
#define PAL_PLC_LOCAL_H

// *****************************************************************************
// *****************************************************************************
// Section: File includes
// *****************************************************************************
// *****************************************************************************

#include <stdbool.h>
#include <stdint.h>
#include "system/time/sys_time.h"
#include "pal_types.h"
#include "driver/driver_common.h"
#include "driver/plc/phy/drv_plc_phy_comm.h"

// *****************************************************************************
// *****************************************************************************
// Section: Data Types
// *****************************************************************************
// *****************************************************************************
// *****************************************************************************
/* PLC PAL Module Status

  Summary:
    Identifies the current status/state of the PLC PAL module.

  Description:
    This enumeration identifies the current status/state of the PLC PAL module.

  Remarks:
    This enumeration is the return type for the PAL_PLC_Status routine. The
    upper layer must ensure that PAL_PLC_Status returns PAL_PLC_STATUS_READY
    before performing PLC PAL operations.
*/
typedef enum {
    PAL_PLC_STATUS_UNINITIALIZED = SYS_STATUS_UNINITIALIZED,
    PAL_PLC_STATUS_BUSY = SYS_STATUS_BUSY,
    PAL_PLC_STATUS_READY = SYS_STATUS_READY,
    PAL_PLC_STATUS_ERROR = SYS_STATUS_ERROR,
    PAL_PLC_STATUS_INVALID_OBJECT = SYS_STATUS_ERROR_EXTENDED - 1,
    PAL_PLC_STATUS_DETECT_IMPEDANCE = SYS_STATUS_ERROR_EXTENDED - 2,
    PAL_PLC_STATUS_SET_DEFAULT = SYS_STATUS_ERROR_EXTENDED - 3,
} PAL_PLC_STATUS;

/* PAL PLC PHY receiver data structure
 Summary:
    PLC PHY receiver parameters.

 Description:
    This structure contains information about the last received message in the
    PLC PHY layer. This information is needed in the MAC layer for management
    purposes.

 Remarks:
    None.
*/
typedef struct
{
    /* Accumulated Error Vector Magnitude for header */
    uint32_t evmHeaderAcum;
    /* Accumulated Error Vector Magnitude for payload */
    uint32_t evmPayloadAcum;
    /* Reception time (start of message) in 1us */
    uint32_t rxTime;
    /* Error Vector Magnitude for header */
    uint16_t evmHeader;
    /* Error Vector Magnitude for payload */
    uint16_t evmPayload;
    /* Length of the data buffer in bytes */
    uint16_t dataLen;
    /* Modulation scheme of the received message */
    PAL_SCHEME scheme;
    /* Type A, Type B or Type BC */
    uint8_t modType;
    /* Header Type of the received message */
    uint8_t headerType;
    /* Average RSSI (Received Signal Strength Indication) */
    uint8_t rssiAvg;
    /* Average CNIR (Carrier to Interference + Noise ratio) */
    uint8_t cinrAvg;
    /* Minimum CNIR (Carrier to Interference + Noise ratio) */
    uint8_t cinrMin;
    /* Average Soft BER (Bit Error Rate) */
    uint8_t berSoft;
    /* Maximum Soft BER (Bit Error Rate) */
    uint8_t berSoftMax;
    /* Percentage of carriers affected by narrow band noise */
    uint8_t narBandPercent;
    /* Percentage of symbols affected by impulsive noise */
    uint8_t impPercent;
}  PAL_PLC_RX_PHY_PARAMS;

// *****************************************************************************
/* PAL PLC Data

  Summary:
    Holds PAL PLC internal data.

  Description:
    This data type defines all data required to handle the PAL PLC module.

  Remarks:
    None.
*/
typedef struct
{
    PAL_CALLBACKS plcCallbacks;

    DRV_HANDLE drvPhyHandle;

    uint32_t timeRefPlc;

    uint32_t timeRefHost;

    uint32_t syncTimerRelFreq;

    uint32_t syncIntId;

    uint32_t syncDelay;

    SYS_TIME_HANDLE syncHandle;

    uint32_t hiTimerRef;

    uint32_t previousTimerRef;

    DRV_PLC_PHY_PIB_OBJ plcPIB;

    DRV_PLC_PHY_TX_RESULT detectImpedanceResult;

    DRV_PLC_PHY_TRANSMISSION_OBJ phyTxObj;

    PAL_PLC_RX_PHY_PARAMS rxParameters;

    DRV_PLC_PHY_CHANNEL channel;

    SYS_STATUS drvPhyStatus;

    uint16_t channelList;

    PAL_PLC_STATUS status;

    uint8_t palAttenuation;

    uint8_t lastRSSIAvg;

    uint8_t lastCINRMin;

    uint8_t statsErrorUnexpectedKey;

    uint8_t statsErrorReset;

    uint8_t statsErrorDebug;

    uint8_t statsErrorCritical;

    uint8_t errorInfo;

    uint8_t maxNumChannels;

    bool waitingTxCfm;

    bool pvddMonTxEnable;

    bool exceptionPending;

    bool syncEnable;

    bool syncUpdate;

    bool networkDetected;

    PAL_USI_SNIFFER_CB snifferCallback;

    SRV_USI_HANDLE usiHandler;

    uint8_t snifferData[PAL_SNIFFER_DATA_MAX_SIZE];

} PAL_PLC_DATA;

#endif // #ifndef PAL_PLC_LOCAL_H
/*******************************************************************************
 End of File
*/