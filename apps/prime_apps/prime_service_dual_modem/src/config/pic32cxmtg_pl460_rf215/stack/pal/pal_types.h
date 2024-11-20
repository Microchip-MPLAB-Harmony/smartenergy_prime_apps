/*******************************************************************************
  Company:
    Microchip Technology Inc.

  File Name:
    pal_types.h

  Summary:
    Physical Abstraction Layer (PAL) header file with type definitions.

  Description:
    This module provides the interface between the PRIME MAC layer and the
    different physical layers.
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

#ifndef PAL_TYPES_H
#define PAL_TYPES_H

// *****************************************************************************
// *****************************************************************************
// Section: Include Files
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include "system/system.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    extern "C" {

#endif
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Macro Definitions
// *****************************************************************************
// *****************************************************************************

/* Robust Management (RM) value used when there is no information available
about the modulation scheme to be used */
#define PAL_OUTDATED_INF    0x0F

/* Radio Channel Mask */
#define PRIME_PAL_RF_CHN_MASK                  0x0200
/* Macro to configure frequency hopping channel */
#define PRIME_PAL_RF_FREQ_HOPPING_CHANNEL      0x03FF
/* Serial Channel Mask */
#define PRIME_PAL_SERIAL_CHN_MASK              0x0400

// *****************************************************************************
// *****************************************************************************
// Section: Data Types
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* PAL Handle

  Summary:
    Handle to a PAL instance.

  Description:
    This data type is a handle to a PAL instance. It can be used to access
    and control the PAL.

  Remarks:
    Do not rely on the underlying type as it may change in different versions
    or implementations of the PAL service.
*/
typedef uintptr_t PAL_HANDLE;

// *****************************************************************************
/* Invalid PAL handle value to a PAL instance.

  Summary:
    Invalid handle value to a PAL instance.

  Description:
    Defines the invalid handle value to a PAL instance.

  Remarks:
    Do not rely on the actual value as it may change in different versions
    or implementations of the PAL service.
*/
#define PAL_HANDLE_INVALID   ((PAL_HANDLE) (-1))

// *****************************************************************************
/* PAL Module Status

  Summary:
    Identifies the current status/state of the PAL module.

  Description:
    This enumeration identifies the current status/state of the PAL module.

  Remarks:
    This enumeration is the return type for the PAL_xxx_Status routines. The
    upper layer must ensure that PAL_xxx_Status returns PAL_STATUS_READY
    before performing PAL operations.
*/
typedef enum {
    PAL_STATUS_UNINITIALIZED = SYS_STATUS_UNINITIALIZED,
    PAL_STATUS_BUSY = SYS_STATUS_BUSY,
    PAL_STATUS_READY = SYS_STATUS_READY,
    PAL_STATUS_ERROR = SYS_STATUS_ERROR,
    PAL_STATUS_INVALID_OBJECT = SYS_STATUS_ERROR_EXTENDED - 1,
} PAL_STATUS;

typedef enum {
    PAL_FRAME_TYPE_A  = 0,
    PAL_FRAME_TYPE_B  = 2,
    PAL_FRAME_TYPE_BC = 3,
    PAL_FRAME_TYPE_RF = 5,
    PAL_FRAME_NOISE   = 0xFE,  /* Not in PL360 */
    PAL_FRAME_TEST    = 0xFF,  /* Not in PL360 */
} PAL_FRAME;

// *****************************************************************************
/* PAL Modulation schemes

  Summary:
    The list of all modulation schemes supported by PRIME spec.

  Remarks:
    None.
*/

typedef enum {
    PAL_SCHEME_DBPSK = 0x00,
    PAL_SCHEME_DQPSK = 0x01,
    PAL_SCHEME_D8PSK = 0x02,
    PAL_SCHEME_DBPSK_C = 0x04,
    PAL_SCHEME_DQPSK_C = 0x05,
    PAL_SCHEME_D8PSK_C = 0x06,
    PAL_SCHEME_R_DBPSK = 0x0C,
    PAL_SCHEME_R_DQPSK = 0x0D,
    PAL_SCHEME_RF = 0x20,
    PAL_SCHEME_RF_FSK_FEC_OFF = 0x21,
    PAL_SCHEME_RF_FSK_FEC_ON = 0x22,
} PAL_SCHEME;

// *****************************************************************************
/* PAL configuration result macros

  Summary:
    PAL configuration results.

  Description:
    PAL configuration output results

  Remarks:
    None.
*/
typedef enum {
    PAL_CFG_SUCCESS = 0,
    PAL_CFG_INVALID_INPUT = 1,
} PAL_CFG_RESULT;

/* PAL information base enumeration
 Summary:
    PHY parameter identifiers requested from the MAC layer

 Description:
    This list contains the PHY parameter identifiers to be used by the MAC
    layer to access PHY parameters.

 Remarks:
    None.
*/
typedef enum
{
    /* Enable/Disable continuous transmission mode in PLC */
    PAL_ID_CONTINUOUS_TX_EN,
    /* Zero Crossing Period in PLC */
    PAL_ID_ZC_PERIOD,
    /* Host controller version identifier in PLC */
    PAL_ID_HOST_VERSION,
    /* Maximum transmission/reception number of channels in PLC */
    PAL_ID_CFG_MAX_TXRX_NUM_CHANNELS,
    /* Attenuation to be applied to every message */
    PAL_ID_CFG_ATTENUATION,
    /* List of available transmission/reception channels, double combination in PLC */
    PAL_ID_CFG_TXRX_DOUBLE_CHANNEL_LIST,
    /* Product identifier */
    PAL_ID_INFO_VERSION,
    /* Enable/disable PHY Sniffer */
    PAL_ID_PHY_SNIFFER_EN,
    /* Flag to enable branch auto detection in PLC */
    PAL_ID_CFG_AUTODETECT_BRANCH,
    /* When branch auto detection disabled, indicate impedance to use in PLC */
    PAL_ID_CFG_IMPEDANCE,
    /* Transmission/Reception channel */
    PAL_ID_CFG_TXRX_CHANNEL,
    /* List of available transmission/reception channels
    (depends on coupling and band plan) in PLC */
    PAL_ID_CFG_TXRX_CHANNEL_LIST,
    /* RX payload length in OFDM symbols */
    PAL_ID_RX_PAYLOAD_LEN_SYM,
    /* Duration of channel senses in CSMA RF */
    PAL_ID_CSMA_RF_SENSE_TIME,
    /* Duration of a unit backoff period in CSMA RF */
    PAL_ID_UNIT_BACKOFF_PERIOD,
    /* Network detection in PLC*/
    PAL_ID_NETWORK_DETECTION,
    /* Device information in PLC*/
    PAL_ID_INFO_DEVICE,
    /* Remaining duration of present frame in PLC */
    PAL_ID_REMAINING_FRAME_DURATION,
    /* Default scheme for RF */
    PAL_ID_RF_DEFAULT_SCHEME,
    /* Physical parameters for a received PLC message */
    PAL_ID_PLC_RX_PHY_PARAMS,
    /* RF channels used to generate the main hopping sequence */
    PAL_ID_RF_BITS_HOPPING_SEQUENCE,
    /* RF channels used to generate the beacon hopping sequence */
    PAL_ID_RF_BITS_BCN_HOPPING_SEQUENCE,
    /* Number of channels in the hopping sequence. */
    PAL_ID_RF_MAC_HOPPING_SEQUENCE_LENGTH,
    /* Number of channels in the Beacon hopping sequence */
    PAL_ID_RF_MAC_HOPPING_BCN_SEQUENCE_LENGTH,
    /* Number of supported RF channels */
    PAL_ID_RF_NUM_CHANNELS,
    /* The maximum PSDU (PHY Service Data Unit) in octets */
    PAL_ID_MAX_PHY_PACKET_SIZE,
    /* Turn around time for the SUN (Smart Utility Network) FSK PHY layer */
    PAL_ID_TURNAROUND_TIME,
    /* The transmit power of the device in dBm */
    PAL_ID_PHY_TX_POWER,
    /* Status of FEC (Forward Error Correction) */
    PAL_ID_PHY_FSK_FEC_ENABLED,
    /* Status of the RSC (Recursive and Systematic Code) interleaving */
    PAL_ID_PHY_FSK_FEC_INTERLEAVING_RSC,
    /* FEC scheme */
    PAL_ID_PHY_FSK_FEC_SCHEME,
    /* Length of the preamble pattern in FSK */
    PAL_ID_PHY_FSK_PREAMBLE_LENGTH,
    /* Used group of SFDs (Start of Frame Delimiter) */
    PAL_ID_PHY_SUN_FSK_SFD,
    /* Status of PSDU (PHY Service Data Unit) data whitening */
    PAL_ID_PHY_FSK_SCRAMBLE_PSDU,
    /* The duration for CCA (Clear Channel Assessment) */
    PAL_ID_PHY_CCA_DURATION,
    /* Number of dB above the specified receiver sensitivity for the RF PHY layer */
    PAL_ID_PHY_CCA_THRESHOLD,
    /* RF band and operating mode */
    PAL_ID_RF_PHY_BAND_OPERATING_MODE,
} PAL_ATTRIBUTE_ID;

// *****************************************************************************
/* PAL Tx Time Mode Enum

  Summary:
    PAL different Tx Time modes.

  Description:
    These Tx Time modes are used by MAC layer.

  Remarks:
    None.
*/

typedef enum
{
    /* Absolute TX scheduling mode (absolute TX time specified) */
    PAL_TX_MODE_ABSOLUTE = 0,
    /* Relative TX scheduling mode (delay for TX time specified) */
    PAL_TX_MODE_RELATIVE = 1,
    /* Cancel TX scheduling mode (cancel TX) */
    PAL_TX_MODE_CANCEL   = 2,
} PAL_TX_TIME_MODE;

// *****************************************************************************
/* PAL TX result

  Summary:
    PAL transmission results.

  Description:
    PAL Tx result is used as the return type for data transmission.

  Remarks:
    None.
*/

typedef enum
{
    /* Transmission result: already in process */
    PAL_TX_RESULT_PROCESS        = 0,
    /* Transmission result: end successfully */
    PAL_TX_RESULT_SUCCESS        = 1,
    /* Transmission result: invalid length error */
    PAL_TX_RESULT_INV_LENGTH     = 2,
    /* Transmission result: busy channel error */
    PAL_TX_RESULT_BUSY_CH        = 3,
    /* Transmission result: busy transmission error */
    PAL_TX_RESULT_BUSY_TX        = 4,
    /* Transmission result: busy reception error */
    PAL_TX_RESULT_BUSY_RX        = 5,
    /* Transmission result: invalid scheme error */
    PAL_TX_RESULT_INV_SCHEME     = 6,
    /* Transmission result: timeout error */
    PAL_TX_RESULT_TIMEOUT        = 7,
    /* Transmission result: invalid buffer identifier error */
    PAL_TX_RESULT_INV_BUFFER     = 8,
    /* Transmission result: invalid PRIME Mode error */
    PAL_TX_RESULT_INV_PRIME_MODE = 9,
    /* Transmission result: invalid transmission mode error */
    PAL_TX_RESULT_INV_TX_MODE    = 10,
    /* Transmission result: transmission cancelled */
    PAL_TX_RESULT_CANCELLED      = 11,
    /* Transmission result: high temperature
    (>120 Degree Centigrade) error (only with PL460) */
    PAL_TX_RESULT_HIGH_TEMP_120  = 12,
    /* Transmission result: high temperature
    (>110 Degree Centigrade) error (only with PL460) */
    PAL_TX_RESULT_HIGH_TEMP_110  = 13,
    /* Transmission Result (only RF): invalid parameter */
    PAL_TX_RESULT_INV_PARAM      = 20,
    /* Transmission result: error in tx */
    PAL_TX_RESULT_PHY_ERROR      = 0xFE,
} PAL_TX_RESULT;

/* PAL transmission data structure
 Summary:
    Data structure used for transmission

 Description:
    This structure contains the MAC Protocol Data Unit (MPDU) to be transmitted
    and its associated transmission parameters. The PHY Abstraction Layer
    forwards this information to the medium selected by the MAC layer.

 Remarks:
    None.
*/
typedef struct
{
    /* Pointer to data buffer */
    uint8_t *pData;
    /* Delay for transmission in us */
    uint32_t timeDelay;
    /* Length of the data buffer */
    uint16_t dataLength;
    /* Physical Physical channel to transmit the message */
    uint16_t pch;
    /* Buffer identifier */
    uint8_t buffId;
    /* Attenuation level with which the message must be transmitted */
    uint8_t attLevel;
    /* Modulation scheme of last transmitted message */
    PAL_SCHEME scheme;
    /* TX Forced */
    uint8_t disableRx;
    /* Type A, Type B, Type BC, Type Radio */
    PAL_FRAME frameType;
    /* Time mode: 0: Absolute mode, 1: Differential mode, 2: Cancel TX */
    PAL_TX_TIME_MODE timeMode;
    /* Number of channel senses */
    uint8_t numSenses;
    /* Delay between channel senses in ms */
    uint8_t senseDelayMs;
} PAL_MSG_REQUEST_DATA;

/* PAL confirm data structure
 Summary:
    Data structure used to confirm the result of transmission request

 Description:
    This structure is used to indicate to the MAC layer the result of the
    transmission request of the MAC Protocol Data Unit (MPDU).

 Remarks:
    None.
*/
typedef struct
{
    /* Transmission time in us */
    uint32_t txTime;
    /* Physical channel where the message has been transmitted */
    uint16_t pch;
    /* RMS value */
    uint16_t rmsCalc;
    /* Type mode: Type A, Type B, Type BC, Type Radio  */
    PAL_FRAME frameType;
    /* Buffer identifier */
    uint8_t bufId;
    /* Result */
    PAL_TX_RESULT result;
} PAL_MSG_CONFIRM_DATA;

/* PAL indication data structure
 Summary:
    Data structure used for PHY data indication

 Description:
    This structure contains the received MAC Protocol Data Unit (MPDU) and its
    associated reception parameters. The PHY Abstraction Layer forwards this
    information to the MAC layer.

 Remarks:
    None.
*/
typedef struct
{
    /* Pointer to data buffer */
    uint8_t *pData;
    /* Reception time in us */
    uint32_t rxTime;
    /* Length of the data buffer */
    uint16_t dataLength;
    /* Physical channel where the message has been received */
    uint16_t pch;
    /* Bitrate estimation in Kbs */
    uint16_t estimatedBitrate;
    /* RSSI (Received Signal Strength Indicator) coded as specified */
    int16_t rssi;
    /* Buffer identifier */
    uint8_t bufId;
    /* Modulation scheme of the last received message */
    PAL_SCHEME scheme;
    /* Type A, Type B, Type BC, Type Radio */
    PAL_FRAME frameType;
    /* Header type of the last received message */
    uint8_t headerType;
    /* Less robust modulation */
    uint8_t lessRobustMod;
    /* SNR (Signal Noise Ratio) /LQI (Link Quality Indicator) */
    uint8_t lqi;
} PAL_MSG_INDICATION_DATA;

/* PHY Abstraction Layer confirm data transmission function pointer

  Summary:
    Callback used for confirm data transmission.

  Description:
    The confirm attribute is used  to tell the MAC layer if MAC Protocol Data
    Unit (MPDU) request has been successfully transmitted.

  Remarks:
    None.
*/
typedef void (*PAL_DATA_CONFIRM_CB)(PAL_MSG_CONFIRM_DATA *pData);

/* PHY Abstraction Layer Indication Data Transmission Function Pointer

  Summary:
    Callback used for indication data transmission.

  Description:
    This callback is used to indicate the reception of a message to the MAC layer.

  Remarks:
    None.
*/
typedef void (*PAL_DATA_INDICATION_CB)(PAL_MSG_INDICATION_DATA *pData);

/* PHY Abstraction Layer switch RF channel function pointer

  Summary:
    Callback designed to inform the MAC layer the execution of a programmed
    change of the RF channel.

  Description:
    This callback is used to inform the MAC layer upon the execution of a
    programmed change of the RF (Radio Frequency) channel.

  Remarks:
    None.
*/
typedef void (*PAL_SWITCH_RF_CH_CB)(uint16_t pch);

// ****************************************************************************
/* PRIME PAL handlers data

  Summary:
    Defines the handlers required to manage the PRIME PAL module.

  Description:
    This data type defines the handlers required to manage the PRIME PAL module.

  Remarks:
    None.
*/
typedef struct
{
    PAL_DATA_CONFIRM_CB dataConfirm;
    PAL_DATA_INDICATION_CB dataIndication;
    PAL_SWITCH_RF_CH_CB switchRfChannel;
} PAL_CALLBACKS;

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    }

#endif
// DOM-IGNORE-END

#endif //PAL_TYPES_H
