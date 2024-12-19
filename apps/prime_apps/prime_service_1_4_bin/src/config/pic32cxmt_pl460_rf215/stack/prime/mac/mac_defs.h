/*******************************************************************************
  PRIME MAC Definitions Header

  Company:
    Microchip Technology Inc.

  File Name:
    mac_defs.h

  Summary:
    PRIME MAC Definitions Header File

  Description:
    This file contains definitions of the PRIME MAC primitives to be used by the
    PRIME application.
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

#ifndef MAC_DEFS_H_INCLUDE
#define MAC_DEFS_H_INCLUDE

// *****************************************************************************
// *****************************************************************************
// Section: File includes
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

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

/* MAC invalid values */
#define MAC_INVALID_SID                         0xFF
#define MAC_INVALID_LNID                        0x3FFF
#define MAC_BROADCAST_LNID                      0x3FFF
#define MAC_MULTICAST_LNID                      0x3FFE
#define MAC_MUL_SW_LEAVE_LNID                   0x0
#define MAC_INVALID_HANDLER                     0xFFFF
#define MAC_INVALID_LCID                        0x7FFF

/* MAC reserved LCID */
#define MAC_LCI_CL_IPV4_BROADCAST               1
#define MAC_LCI_CL_432_BROADCAST                2

/* MAC reserved handlers*/
#define MAC_MULTICAST_CON_HANDLER               0
#define MAC_BN_DIRECT_CON_HANDLER               3
#define MAC_BN_DIRECT_CON_ERROR_HANDLER         4
#define MAC_SN_DIRECT_CON_HANDLER_MSK           0x0080
#define MAC_RESERVED_HANDLER                    0

// *****************************************************************************
// *****************************************************************************
// Section: Data Types
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* PRIME version customer values

 Summary:
    Data structure of the PRIME version customer values.

 Description:
    This structure contains the PRIME version customer values.

 Remarks:
    None.
*/
typedef struct {
	char fwVersion[16];
	char fwModel[16];
	char fwVendor[16];
	uint16_t pibVendor;
	uint16_t pibModel;
} MAC_VERSION_INFO;

// *****************************************************************************
/* PLME result values

 Summary:
    Defines the PLME result values.

 Description:
    This enumeration defines the PLME result values.

 Remarks:
    None.
*/
typedef enum {
	PLME_RESULT_SUCCESS     = 0,
	PLME_RESULT_FAILED      = 1,
	PLME_RESULT_REJECTED    = 2,
	PLME_RESULT_BADATTR     = 2,
} PLME_RESULT;

// *****************************************************************************
/* MLME result values

 Summary:
    Defines the MLME result values.

 Description:
    This enumeration defines the MLME result values.

 Remarks:
    None.
*/
typedef enum {
	MLME_RESULT_DONE          = 0,
	MLME_RESULT_FAILED        = 1,
	MLME_RESULT_REJECTED      = 1,
	MLME_RESULT_TIMEOUT       = 2,
	MLME_RESULT_NOSUCHDEVICE  = 4,
	MLME_RESULT_NOSNA         = 8,
	MLME_RESULT_NOSWITCH      = 9,
	MLME_RESULT_WRONGMEDIUM   = 9,
	MLME_RESULT_REDUNDANT     = 10,
	MLME_RESULT_BADATTR       = 11,
	MLME_RESULT_OUTOFRANGE    = 12,
	MLME_RESULT_READONLY      = 13,
	MLME_RESULT_WRONGLSID     = 15
} MLME_RESULT;

// *****************************************************************************
/* MAC connection types

 Summary:
    Defines the PRIME MAC connection types.

 Description:
    This enumeration defines the PRIME MAC connection types.

 Remarks:
    None.
*/
typedef enum {
	MAC_CONNECTION_TYPE_INVALID         = 0,
	MAC_CONNECTION_TYPE_IPV4_AR         = 1,
	MAC_CONNECTION_TYPE_IPV4_UNICAST    = 2,
	MAC_CONNECTION_TYPE_CL_432          = 3,
	MAC_CONNECTION_TYPE_MNGT            = 4,
	MAC_CONNECTION_TYPE_IPV6_AR         = 5,
	MAC_CONNECTION_TYPE_IPV6_UNICAST    = 6,
} MAC_CONNECTION_TYPE;

// *****************************************************************************
/* MAC SAP result values for MAC_ESTABLISH.confirm primitive

 Summary:
    Defines MAC SAP result values for the MAC_ESTABLISH.confirm primitive.

 Description:
    This enumeration defines MAC SAP result values for the MAC_ESTABLISH.confirm
    primitive.

 Remarks:
    None.
*/
typedef enum {
	MAC_ESTABLISH_CFM_RESULT_SUCCESS            = 0,
	MAC_ESTABLISH_CFM_RESULT_REJECT             = 1,
	MAC_ESTABLISH_CFM_RESULT_TIMEOUT            = 2,
	MAC_ESTABLISH_CFM_RESULT_NO_BANDWIDTH       = 3,
	MAC_ESTABLISH_CFM_RESULT_NO_SUCH_DEVICE     = 4,
	MAC_ESTABLISH_CFM_RESULT_REDIRECT_FAILED    = 5,
	MAC_ESTABLISH_CFM_RESULT_NOT_REGISTERED     = 6,
	MAC_ESTABLISH_CFM_RESULT_NO_MORE_LCIDS      = 7,
	MAC_ESTABLISH_CFM_RESULT_DC_NO_SUPPORTED    = 8,
	MAC_ESTABLISH_CFM_RESULT_UNSUPPORTED_SP     = 14,
	MAC_ESTABLISH_CFM_RESULT_PROCCESS_ACTIVE    = 0x80
} MAC_ESTABLISH_CONFIRM_RESULT;

// *****************************************************************************
/* MAC SAP values for the answer parameter in MAC_ESTABLISH.response primitive

 Summary:
    Defines MAC SAP values for the answer parameter in the MAC_ESTABLISH.response
    primitive.

 Description:
    This enumeration defines values for the answer parameter in the
    MAC_ESTABLISH.response primitive.

 Remarks:
    None.
*/
typedef enum {
	MAC_ESTABLISH_RESPONSE_ANSWER_ACCEPT    = 0,
	MAC_ESTABLISH_RESPONSE_ANSWER_REJECT    = 1,
} MAC_ESTABLISH_RESPONSE_ANSWER;

// *****************************************************************************
/* MAC SAP values for the reason parameter in MAC_RELEASE.indication primitive

 Summary:
    Defines MAC SAP values for the reason parameter in the MAC_RELEASE.indication
    primitive.

 Description:
    This enumeration defines values for the reason parameter in the
    MAC_RELEASE.indication primitive.

 Remarks:
    None.
*/
typedef enum {
	MAC_RELEASE_INDICATION_REASON_SUCCESS   = 0,
	MAC_RELEASE_INDICATION_REASON_ERROR     = 1,
} MAC_RELEASE_INDICATION_REASON;

// *****************************************************************************
/* MAC SAP values for the answer parameter in MAC_RELEASE.response primitive

 Summary:
    Defines MAC SAP values for the answer parameter in the MAC_RELEASE.response
    primitive.

 Description:
    This enumeration defines values for the answer parameter in the
    MAC_RELEASE.response primitive.

 Remarks:
    None.
*/
typedef enum {
	MAC_RELEASE_RESPONSE_ANSWER_ACCEPT = 0,
} MAC_RELEASE_RESPONSE_ANSWER;

// *****************************************************************************
/* MAC SAP result values for MAC_RELEASE.confirm primitive

 Summary:
    Defines MAC SAP result values for the MAC_RELEASE.confirm primitive.

 Description:
    This enumeration defines MAC SAP result values for the MAC_RELEASE.confirm
    primitive.

 Remarks:
    None.
*/
typedef enum {
	MAC_RELEASE_CFM_RESULT_SUCCESS          = 0,
	MAC_RELEASE_CFM_RESULT_TIMEOUT          = 2,
	MAC_RELEASE_CFM_RESULT_NOT_REGISTERED   = 6,
	MAC_RELEASE_CFM_RESULT_PROCCESS_ACTIVE  = 0x80,
	MAC_RELEASE_CFM_RESULT_BAD_HANDLER      = 0x81,
	MAC_RELEASE_CFM_RESULT_NOT_OPEN_CONN    = 0x82,
	MAC_RELEASE_CFM_RESULT_ERROR_SENDING    = 0x83,
	MAC_RELEASE_CFM_RESULT_BAD_FLOW_MODE    = 0x84,
} MAC_RELEASE_CONFIRM_RESULT;

// *****************************************************************************
/* MAC SAP result values for MAC_DATA.confirm primitive

 Summary:
    Defines MAC SAP result values for the MAC_DATA.confirm primitive.

 Description:
    This enumeration defines MAC SAP result values for the MAC_DATA.confirm
    primitive.

 Remarks:
    None.
*/
typedef enum {
	MAC_DATA_CFM_RESULT_SUCCESS                            = 0,
	MAC_DATA_CFM_RESULT_TIMEOUT                            = 2,
	MAC_DATA_CFM_RESULT_ERROR_SENDING                      = 0x80,
	MAC_DATA_CFM_RESULT_ERROR_PROCESSING_PREVIOUS_REQUEST  = 0x81,
	MAC_DATA_CFM_RESULT_ERROR_NO_FREE_BUFFERS              = 0x82,
	MAC_DATA_CFM_RESULT_ERROR_CON_CLOSED                   = 0x83,
	MAC_DATA_CFM_RESULT_ERROR_RECEIVING_DATA               = 0x84
} MAC_DATA_CONFIRM_RESULT;

// *****************************************************************************
/* Modes of join request

 Summary:
    Defines the modes of join request.

 Description:
    This enumeration defines the modes of join request.

 Remarks:
    None.
*/
typedef enum {
	MAC_JOIN_REQUEST_MODE_MULTICAST       = 0,
	MAC_JOIN_REQUEST_MODE_BROADCAST       = 1,
} MAC_JOIN_REQUEST_MODE;

// *****************************************************************************
/* MAC SAP result values for MAC_JOIN.confirm primitive

 Summary:
    Defines MAC SAP result values for the MAC_JOIN.confirm primitive.

 Description:
    This enumeration defines MAC SAP result values for the MAC_JOIN.confirm
    primitive.

 Remarks:
    None.
*/
typedef enum {
	MAC_JOIN_CFM_RESULT_SUCCESS          = 0,
	MAC_JOIN_CFM_RESULT_FAILURE          = 1,
	MAC_JOIN_CFM_RESULT_TIMEOUT          = 2,
	MAC_JOIN_CFM_RESULT_NOT_REGISTERED   = 6,
	MAC_JOIN_CFM_RESULT_UNSUPPORTED_SP   = 14
} MAC_JOIN_CONFIRM_RESULT;

// *****************************************************************************
/* MAC SAP values for the answer parameter in MAC_JOIN.response primitive

 Summary:
    Defines MAC SAP values for the answer parameter in the MAC_JOIN.response
    primitive.

 Description:
    This enumeration defines values for the answer parameter in the
    MAC_JOIN.response primitive.

 Remarks:
    None.
*/
typedef enum {
	MAC_JOIN_RESPONSE_ANSWER_ACCEPT    = 0,
	MAC_JOIN_RESPONSE_ANSWER_REJECT    = 1,
} MAC_JOIN_RESPONSE_ANSWER;

// *****************************************************************************
/* MAC SAP result values for MAC_LEAVE.confirm primitive

 Summary:
    Defines MAC SAP result values for the MAC_LEAVE.confirm primitive.

 Description:
    This enumeration defines MAC SAP result values for the MAC_LEAVE.confirm
    primitive.

 Remarks:
    None.
*/
typedef enum {
	MAC_LEAVE_CFM_RESULT_ACCEPT           = 0,
	MAC_LEAVE_CFM_RESULT_TIMEOUT          = 1,
	MAC_LEAVE_CFM_RESULT_PROCCESS_ACTIVE  = 0x80,
	MAC_LEAVE_CFM_RESULT_BAD_HANDLER      = 0x81,
	MAC_LEAVE_CFM_RESULT_NOT_OPEN_CONN    = 0x82,
	MAC_LEAVE_CFM_RESULT_ERROR_SENDING    = 0x83,
	MAC_LEAVE_CFM_RESULT_BAD_FLOW_MODE    = 0x84,
	MAC_LEAVE_CFM_RESULT_NOT_REGISTERED   = 0x85,
} MAC_LEAVE_CONFIRM_RESULT;

// *****************************************************************************
/* MAC certification parameters to send a message.

 Summary:
    Defines the PRIME MAC certification parameters to send a message.

 Description:
    This structure defines the PRIME MAC certification parameters to send a
    message.

 Remarks:
    None.
*/
typedef struct {
	uint16_t msgCount;
	uint8_t modulation;
	uint8_t signalAtt;
	uint8_t dutyCycle;
	uint8_t frameType;
} MAC_CERT_PARAMETER_SEND_MSG;

// *****************************************************************************
/* MAC certification modes.

 Summary:
    Defines the PRIME MAC certification modes.

 Description:
    This enumeration defines the PRIME MAC certification modes.

 Remarks:
    None.
*/
typedef enum {
	MAC_CERT_MODE_NONE      = 0,
	MAC_CERT_MODE_PHY       = 1,
	MAC_CERT_MODE_MAC       = 2,
	MAC_CERT_MODE_PHY_1_4   = 3,
	MAC_CERT_MODE_PHY_RF    = 4,
} MAC_CERT_MODE;

// *****************************************************************************
/* MAC certification reject messages.

 Summary:
    Defines the PRIME MAC certification reject messages.

 Description:
    This enumeration defines the PRIME MAC certification reject messages.

 Remarks:
    None.
*/
typedef enum {
	MAC_CERT_REJECT_MSG_PRO_REQ_S      = 0,
	MAC_CERT_REJECT_MSG_PRM            = 1,
	MAC_CERT_REJECT_MSG_CON_REQ_S      = 2,
	MAC_CERT_REJECT_MSG_REG_REQ        = 3,
} MAC_CERT_REJECT_MSG;

// *****************************************************************************
/* MAC connection establishment request

  Summary:
    Function pointer to request a MAC connection establishment.

  Description:
    This function pointer is used to request a MAC connection establishment.
    eui48       - Pointer to the address of the node to which this connection
                  will be addressed
    type        - Convergence Layer type of the connection
    data        - Data associated with the connection establishment procedure
    dataLen     - Length of the data in bytes
    arq         - Flag to indicate whether or not the ARQ mechanism should be
                  used for this connection
    cfBytes     - Flag to indicate whether or not the connection should use the
                  contention or contention-free channel access scheme
    ae          - Flag to indicate that authentication and encryption is
                  requested (v1.4)

  Remarks:
    None.
*/
typedef void (*MAC_ESTABLISH_REQUEST)(uint8_t *eui48, uint8_t type, uint8_t *data,
    uint16_t dataLen, uint8_t arq, uint8_t cfBytes, uint8_t ae);

// *****************************************************************************
/* MAC connection establishment indication

  Summary:
    Callback function pointer for the MAC connection establishment indication.

  Description:
    This callback is used for the MAC connection establishment indication.
    conHandle   - Unique identifier of the connection
    eui48       - Pointer to the address of the node to which this connection
                  will be addressed
    type        - Convergence Layer type of the connection
    data        - Data associated with the connection establishment procedure
    dataLen     - Length of the data in bytes
    cfBytes     - Flag to indicate whether or not the connection should use the
                  contention or contention-free channel access scheme
    ae          - Flag to indicate that authentication and encryption is
                  requested (v1.4)

  Remarks:
    None.
*/
typedef void (*MAC_ESTABLISH_INDICATION_CB)(uint16_t conHandle, uint8_t *eui48,
    uint8_t type, uint8_t *data, uint16_t dataLen, uint8_t cfBytes, uint8_t ae);

// *****************************************************************************
/* MAC connection establishment confirm

  Summary:
    Callback function pointer for the MAC connection establishment confirm.

  Description:
    This callback is used for the MAC connection establishment confirm.
    conHandle   - Unique identifier of the connection
    result      - Result of the connection establishment process
    eui48       - Pointer to the address of the node to which this connection
                  will be addressed
    type        - Convergence Layer type of the connection
    data        - Data associated with the connection establishment procedure
    dataLen     - Length of the data in bytes
    ae          - Flag to indicate that authentication and encryption is
                  requested (v1.4)

  Remarks:
    None.
*/
typedef void (*MAC_ESTABLISH_CONFIRM_CB)(uint16_t conHandle,
    MAC_ESTABLISH_CONFIRM_RESULT result, uint8_t *eui48, uint8_t type,
    uint8_t *data, uint16_t dataLen, uint8_t ae);

// *****************************************************************************
/* MAC connection establishment response

  Summary:
    Function pointer to respond to a MAC connection establishment request.

  Description:
    This function pointer is used to respond to a MAC connection establishment request.
    conHandle   - Unique identifier of the connection
    answer:     - Action to be taken for this connection establishment
    data        - Data associated with the connection establishment procedure
    dataLen     - Length of the data in bytes
    ae          - Flag to indicate that authentication and encryption is
                  requested (v1.4)

  Remarks:
    None.
*/
typedef void (*MAC_ESTABLISH_RESPONSE)(uint16_t conHandle,
    MAC_ESTABLISH_RESPONSE_ANSWER answer, uint8_t *data, uint16_t dataLen,
    uint8_t ae);

// *****************************************************************************
/* MAC release request

  Summary:
    Function pointer to request a MAC connection release.

  Description:
    This function pointer is used to request a MAC connection release.
    conHandle   - Unique identifier of the connection

  Remarks:
    None.
*/
typedef void (*MAC_RELEASE_REQUEST)(uint16_t conHandle);

// *****************************************************************************
/* MAC release indication

  Summary:
    Callback function pointer for the MAC release indication.

  Description:
    This callback is used for the MAC release indication.
    conHandle   - Unique identifier of the connection
    reason      - Cause of the connection release

  Remarks:
    None.
*/
typedef void (*MAC_RELEASE_INDICATION_CB)(uint16_t conHandle,
    MAC_RELEASE_INDICATION_REASON reason);

// *****************************************************************************
/* MAC release confirm

  Summary:
    Callback function pointer for the MAC release confirm.

  Description:
    This callback is used for the MAC release confirm.
    conHandle   - Unique identifier of the connection
    result      - Result of the connection release process

  Remarks:
    None.
*/
typedef void (*MAC_RELEASE_CONFIRM_CB)(uint16_t conHandle,
    MAC_RELEASE_CONFIRM_RESULT result);

// *****************************************************************************
/* MAC release response

  Summary:
    Function pointer to respond to a MAC release request.

  Description:
    This function pointer is used to respond to a MAC release request.
    conHandle   - Unique identifier of the connection
    answer:     - Action to be taken for this connection release procedure

  Remarks:
    None.
*/
typedef void (*MAC_RELEASE_RESPONSE)(uint16_t conHandle,
    MAC_RELEASE_RESPONSE_ANSWER answer);

// *****************************************************************************
/* MAC redirect response

  Summary:
    Function pointer to respond to a MAC direct connection establishment
    indication.

  Description:
    This function pointer is used to respond to a MAC direct connection
    establishment indication.
    conHandle   - Unique identifier of the connection
    eui48       - Pointer to the address of the node to which this connection
                  will be "redirected"
    data        - Data associated with the connection establishment procedure
    dataLen     - Length of the data in bytes

  Remarks:
    None.
*/
typedef void (*MAC_REDIRECT_RESPONSE)(uint16_t conHandle, uint8_t *eui48,
    uint8_t *data, uint16_t dataLen);

// *****************************************************************************
/* MAC join request

  Summary:
    Function pointer to request to join a MAC multicast connection.

  Description:
    This function pointer is used to request to join a MAC multicast connection.
    conMode     - Connection type: broadcast or multicast
    conHandle   - Unique identifier of the connection (only used in base node)
    eui48       - Pointer to the address of the node to which this join is
                  being requested (only used in base node)
    conType     - Connection type
    data        - Data associated with the join request procedure
    dataLen     - Length of the data in bytes
    ae          - Flag to indicate that authentication and encryption is
                  requested (v1.4)

  Remarks:
    None.
*/
typedef void (*MAC_JOIN_REQUEST)(MAC_JOIN_REQUEST_MODE conMode, uint16_t conHandle,
    uint8_t *eui48, MAC_CONNECTION_TYPE conType, uint8_t *data, uint16_t dataLen,
    uint8_t ae);

// *****************************************************************************
/* MAC join indication

  Summary:
    Callback function pointer for the MAC join indication.

  Description:
    This callback is used for the MAC join indication.
    conHandle   - Unique identifier of the connection
    eui48       - Pointer to the address of the node which wishes to join the
                  multicast group (only valid in base node)
    type        - Connection type
    data        - Data associated with the join request procedure
    dataLen     - Length of the data in bytes
    ae          - Flag to indicate that authentication and encryption is
                  requested (v1.4)

  Remarks:
    None.
*/
typedef void (*MAC_JOIN_INDICATION_CB)(uint16_t conHandle, uint8_t *eui48,
    uint8_t conType, uint8_t *data, uint16_t dataLen, uint8_t ae);

// *****************************************************************************
/* MAC join response

  Summary:
    Function pointer to respond to a MAC join indication.

  Description:
    This function pointer is used to respond to a MAC join indication.
    conHandle   - Unique identifier of the connection
    eui48       - Pointer to the address of the node which requested the multicast
                  group join (only used in base node)
    answer      - Action to be taken for this join request
    ae          - Flag to indicate that authentication and encryption is
                  requested (v1.4)

  Remarks:
    None.
*/
typedef void (*MAC_JOIN_RESPONSE)(uint16_t conHandle, uint8_t *eui48,
    MAC_JOIN_RESPONSE_ANSWER answer, uint8_t ae);

// *****************************************************************************
/* MAC join confirm

  Summary:
    Callback function pointer for the MAC join confirm.

  Description:
    This callback is used for the MAC join confirm.
    conHandle   - Unique identifier of the connection
    result      - Result of the join request process
    ae          - Flag to indicate that authentication and encryption is
                  requested (v1.4)

  Remarks:
    None.
*/
typedef void (*MAC_JOIN_CONFIRM_CB)(uint16_t conHandle,
    MAC_JOIN_CONFIRM_RESULT result, uint8_t ae);

// *****************************************************************************
/* MAC leave request

  Summary:
    Function pointer to request to leave a MAC multicast connection.

  Description:
    This function pointer is used to request to leave a MAC multicast connection.
    conHandle   - Unique identifier of the connection
    eui48       - Pointer to the address of the node to be removed from the
                  multicast group (only used in base node)

  Remarks:
    None.
*/
typedef void (*MAC_LEAVE_REQUEST)(uint16_t conHandle, uint8_t *eui48);

// *****************************************************************************
/* MAC leave confirm

  Summary:
    Callback function pointer for the MAC leave confirm.

  Description:
    This callback is used for the MAC leave confirm.
    conHandle   - Unique identifier of the connection
    result      - Result of the leave request process

  Remarks:
    None.
*/
typedef void (*MAC_LEAVE_CONFIRM_CB)(uint16_t conHandle,
    MAC_LEAVE_CONFIRM_RESULT uc_result);

// *****************************************************************************
/* MAC leave indication

  Summary:
    Callback function pointer for the MAC leave indication.

  Description:
    This callback is used for the MAC leave indication.
    conHandle   - Unique identifier of the connection
    eui48       - Pointer to the address of the node to remove from the multicast
                  group (only valid in base node)

  Remarks:
    None.
*/
typedef void (*MAC_LEAVE_INDICATION_CB)(uint16_t conHandle, uint8_t *eui48);

// *****************************************************************************
/* MAC data request

  Summary:
    Function pointer to request the transmission of data over a MAC connection.

  Description:
    This function pointer is used to request the transmission of data over a MAC
    connection.
    conHandle   - Unique identifier of the connection
    data        - Pointer to data to be transmitted through this connection
    dataLen     - Length of the data in bytes
    prio        - Priority of the data to be sent when using the CSMA access
                  scheme
    timeRef     - Time reference (in 10s of microseconds) (v1.4)

  Remarks:
    None.
*/
typedef void (*MAC_DATA_REQUEST)(uint16_t conHandle, uint8_t *data, uint16_t dataLen,
    uint8_t prio, uint32_t timeRef);

// *****************************************************************************
/* MAC data confirm

  Summary:
    Callback function pointer for the MAC data confirm.

  Description:
    This callback is used for the MAC data confirm.
    conHandle   - Unique identifier of the connection
    data        - Pointer to data to be transmitted through this connection
    result      - Result of transmission

  Remarks:
    None.
*/
typedef void (*MAC_DATA_CONFIRM_CB)(uint16_t conHandle, uint8_t *data,
    MAC_DATA_CONFIRM_RESULT result);

// *****************************************************************************
/* MAC data indication

  Summary:
    Callback function pointer for the MAC data indication.

  Description:
    This callback is used for the MAC data indication.
    conHandle   - Unique identifier of the connection
    data        - Pointer to data to be received through this connection
    dataLen     - Length of the data in bytes
    timeRef     - Time reference (in 10s of microseconds) (v1.4)

  Remarks:
    None.
*/
typedef void (*MAC_DATA_INDICATION_CB)(uint16_t conHandle, uint8_t *data,
    uint16_t dataLen, uint32_t timeRef);

// *****************************************************************************
/* PLME reset request

  Summary:
    Function pointer to request a reset of the functional state of the PHY layer.

  Description:
    This function pointer is used to request a reset of the functional state of
    the PHY layer.
    pch     - Physical channel (v1.4)

  Remarks:
    None.
*/
typedef void (*PLME_RESET_REQUEST)(uint16_t pch);

// *****************************************************************************
/* PLME reset confirm

  Summary:
    Callback function pointer for the PLME reset confirm.

  Description:
    This callback is used for the PLME reset confirm.
    result  - Result of the operation
    pch     - Physical channel (v1.4)

  Remarks:
    None.
*/
typedef void (*PLME_RESET_CONFIRM_CB)(PLME_RESULT result, uint16_t pch);

// *****************************************************************************
/* PLME sleep request

  Summary:
    Function pointer to request a suspension of all present activities of the
    PHY layer.

  Description:
    This function pointer is used to request a suspension of all present activities
    of the PHY layer.
    pch     - Physical channel (v1.4)

  Remarks:
    None.
*/
typedef void (*PLME_SLEEP_REQUEST)(uint16_t pch);

// *****************************************************************************
/* PLME sleep confirm

  Summary:
    Callback function pointer for the PLME sleep confirm.

  Description:
    This callback is used for the PLME sleep confirm.
    result  - Result of the operation
    pch     - Physical channel (v1.4)

  Remarks:
    None.
*/
typedef void (*PLME_SLEEP_CONFIRM_CB)(PLME_RESULT result, uint16_t pch);

// *****************************************************************************
/* PLME resume request

  Summary:
    Function pointer to request to resume all suspended actitivities of the PHY
    layer.

  Description:
    This function pointer is used to request to resume all suspended actitivities
    of the PHY layer.
    pch     - Physical channel (v1.4)

  Remarks:
    None.
*/
typedef void (*PLME_RESUME_REQUEST)(uint16_t pch);

// *****************************************************************************
/* PLME resume confirm

  Summary:
    Callback function pointer for the PLME resume confirm.

  Description:
    This callback is used for the PLME resume confirm.
    result  - Result of the operation
    pch     - Physical channel (v1.4)

  Remarks:
    None.
*/
typedef void (*PLME_RESUME_CONFIRM_CB)(PLME_RESULT result, uint16_t pch);

// *****************************************************************************
/* PLME testmode request

  Summary:
    Function pointer to request the PHY layer to enter the given test mode.

  Description:
    This function pointer is used to request the PHY layer to enter the given
    test mode.
    enable      - Start/Stop test mode
    mode        - Transmission mode
    modulation  - Transmission modulation
    pwrLevel    - Transmission power level
    pch         - Physical channel (v1.4)

  Remarks:
    None.
*/
typedef void (*PLME_TESTMODE_REQUEST)(uint8_t enable, uint8_t mode,
    uint8_t modulation, uint8_t pwrLevel, uint16_t pch);

// *****************************************************************************
/* PLME testmode confirm

  Summary:
    Callback function pointer for the PLME testmode confirm.

  Description:
    This callback is used for the PLME testmode confirm.
    result  - Result of the operation
    pch     - Physical channel (v1.4)

  Remarks:
    None.
*/
typedef void (*PLME_TESTMODE_CONFIRM_CB)(PLME_RESULT result, uint16_t pch);

// *****************************************************************************
/* PLME get request

  Summary:
    Function pointer to request information about a given PIB attribute of the
    PHY layer.

  Description:
    This function pointer is used to request information about a given PIB
    attribute of the PHY layer.
    pibAttribute    - PIB attribute
    pch             - Physical channel (v1.4)

  Remarks:
    None.
*/
typedef void (*PLME_GET_REQUEST)(uint16_t pibAttrib, uint16_t pch);

// *****************************************************************************
/* PLME get confirm

  Summary:
    Callback function pointer for the PLME get confirm.

  Description:
    This callback is used for the PLME get confirm.
    status          - Status of the operation
    pibAttribute    - PIB attribute
    pibValue        - PIB attribute value
    pibSize         - PIB attribute value size
    pch             - Physical channel (v1.4)

  Remarks:
    None.
*/
typedef void (*PLME_GET_CONFIRM_CB)(PLME_RESULT status, uint16_t pibAttrib,
    void *pibValue, uint8_t pibSize, uint16_t pch);

// *****************************************************************************
/* PLME set request

  Summary:
    Function pointer to request to set a new value for a given PIB attribute of
    the PHY layer.

  Description:
    This function pointer is used to request to set a new value for a given PIB
    attribute of the PHY layer.
    pibAttribute    - PIB attribute
    pibValue        - PIB attribute value
    pibSize         - PIB attribute value size
    pch             - Physical channel (v1.4)

  Remarks:
    None.
*/
typedef void (*PLME_SET_REQUEST)(uint16_t pibAttrib, void *pibValue, uint8_t pibSize,
    uint16_t pch);

// *****************************************************************************
/* PLME set confirm

  Summary:
    Callback function pointer for the PLME set confirm.

  Description:
    This callback is used for the PLME set confirm.
    result  - Result of the operation
    pch     - Physical channel (v1.4)

  Remarks:
    None.
*/
typedef void (*PLME_SET_CONFIRM_CB)(PLME_RESULT result, uint16_t pch);

// *****************************************************************************
/* MLME register request

  Summary:
    Function pointer to request to trigger the registration process to a
    subnetwork through a specific switch node.

  Description:
    This function pointer is used to request to trigger the registration process
    to a subnetwork through a specific switch node.
    sna     - Pointer to the subnetwork address
    sid     - Switch identifier

  Remarks:
    None.
*/
typedef void (*MLME_REGISTER_REQUEST)(uint8_t *sna, uint8_t sid);

// *****************************************************************************
/* MLME register confirm

  Summary:
    Callback function pointer for the MLME register confirm.

  Description:
    This callback is used for the MLME register confirm.
    result  - Result of the operation
    sna     - Pointer to the subnetwork address
    sid     - Switch identifier

  Remarks:
    None.
*/
typedef void (*MLME_REGISTER_CONFIRM_CB)(MLME_RESULT result, uint8_t *sna,
    uint8_t sid);

// *****************************************************************************
/* MLME register indication

  Summary:
    Callback function pointer for the MLME register indication.

  Description:
    This callback is used for the MLME register indication.
    sna     - Pointer to the subnetwork address
    sid     - Switch identifier

  Remarks:
    None.
*/
typedef void (*MLME_REGISTER_INDICATION_CB)(uint8_t *sna, uint8_t sid);

// *****************************************************************************
/* MLME unregister request

  Summary:
    Function pointer to request to trigger the unregistration process.

  Description:
    This function pointer is used to request to trigger the unregistration
    process.

  Remarks:
    None.
*/
typedef void (*MLME_UNREGISTER_REQUEST)(void);

// *****************************************************************************
/* MLME unregister confirm

  Summary:
    Callback function pointer for the MLME unregister confirm.

  Description:
    This callback is used for the MLME unregister confirm.
    result  - Result of the operation

  Remarks:
    None.
*/
typedef void (*MLME_UNREGISTER_CONFIRM_CB)(MLME_RESULT result);

// *****************************************************************************
/* MLME unregister indication

  Summary:
    Callback function pointer for the MLME unregister indication.

  Description:
    This callback is used for the MLME unregister indication.

  Remarks:
    None.
*/
typedef void (*MLME_UNREGISTER_INDICATION_CB)(void);

// *****************************************************************************
/* MLME promote request

  Summary:
    Function pointer to request to trigger the promotion process in a Service Node
    that is in a Terminal functional state.

  Description:
    This function pointer is used to request to trigger the promotion process in a
    Service Node that is in a Terminal functional state.
    eui48       - Pointer to the address of the node to be promoted (NULL in
                  Service Node)
    bcnMode     - Beacon PDU modulation scheme

  Remarks:
    None.
*/
typedef void (*MLME_PROMOTE_REQUEST)(uint8_t *eui48, uint8_t bcnMode);

// *****************************************************************************
/* MLME promote confirm

  Summary:
    Callback function pointer for the MLME promote confirm.

  Description:
    This callback is used for the MLME promote confirm.
    result  - Result of the operation

  Remarks:
    None.
*/
typedef void (*MLME_PROMOTE_CONFIRM_CB)(MLME_RESULT result);

// *****************************************************************************
/* MLME promote indication

  Summary:
    Callback function pointer for the MLME promote indication.

  Description:
    This callback is used for the MLME promote indication.

  Remarks:
    None.
*/
typedef void (*MLME_PROMOTE_INDICATION_CB)(void);

// *****************************************************************************
/* MLME MultiPHY promote request

  Summary:
    Function pointer to request to trigger the promotion process in a Service Node
    (Terminal or Switch) in a medium (PLC or RF) different from the one the node
    is connected to the network.

  Description:
    This function pointer is used to request to trigger the promotion process in
    a Service Node (Terminal or Switch) in a medium (PLC or RF) different from the
    one the node is connected to the network. This primitive only applies in PRIME
    v1.4.
    eui48   - Pointer to the address of the node to be promoted (NULL in Service
              Node)
    bcnMode - Beacon PDU modulation scheme
    pch     - Physical channel of promotion

  Remarks:
    None.
*/
typedef void (*MLME_MP_PROMOTE_REQUEST)(uint8_t *eui48, uint8_t bcnMode,
    uint16_t pch);

// *****************************************************************************
/* MLME MultiPHY promote confirm

  Summary:
    Callback function pointer for the MLME MultiPHY promote confirm.

  Description:
    This callback is used for the MLME MultiPHY promote confirm.
    result  - Result of the operation

  Remarks:
    None.
*/
typedef void (*MLME_MP_PROMOTE_CONFIRM_CB)(MLME_RESULT result);

// *****************************************************************************
/* MLME MultiPHY promoteindication

  Summary:
    Callback function pointer for the MLME MultiPHY promote indication.

  Description:
    This callback is used for the MLME MultiPHY promote indication.
    pch     - Physical channel of promotion

  Remarks:
    None.
*/
typedef void (*MLME_MP_PROMOTE_INDICATION_CB)(uint16_t pch);

// *****************************************************************************
/* MLME demote request

  Summary:
    Function pointer to request to trigger a demotion process in a Service Node
    that is in a Switch functional state.

  Description:
    This function pointer is used to request to trigger a demotion process in a
    Service Node that is in a Switch functional state.

  Remarks:
    None.
*/
typedef void (*MLME_DEMOTE_REQUEST)(void);

// *****************************************************************************
/* MLME demote confirm

  Summary:
    Callback function pointer for the MLME demote confirm.

  Description:
    This callback is used for the MLME demote confirm.
    result  - Result of the operation

  Remarks:
    None.
*/
typedef void (*MLME_DEMOTE_CONFIRM_CB)(MLME_RESULT result);

// *****************************************************************************
/* MLME demote indication

  Summary:
    Callback function pointer for the MLME demote indication.

  Description:
    This callback is used for the MLME demote indication.

  Remarks:
    None.
*/
typedef void (*MLME_DEMOTE_INDICATION_CB)(void);

// *****************************************************************************
/* MLME MultiPHY demote request

  Summary:
    Function pointer to request to trigger a demotion process in a Service Node
    that is in a Switch functional state and supports MultiPHY promotion.

  Description:
    This function pointer is used to request to trigger a demotion process in a
    Service Node that is in a Switch functional state and supports MultiPHY
    promotion. This primitive only applies in PRIME v1.4.
    lsid    - Local switch identifier

  Remarks:
    None.
*/
typedef void (*MLME_MP_DEMOTE_REQUEST)(uint8_t lsid);

// *****************************************************************************
/* MLME MultiPHY demote confirm

  Summary:
    Callback function pointer for the MLME MultiPHY demote confirm.

  Description:
    This callback is used for the MLME MultiPHY demote confirm.
    result  - Result of the operation

  Remarks:
    None.
*/
typedef void (*MLME_MP_DEMOTE_CONFIRM_CB)(MLME_RESULT result);

// *****************************************************************************
/* MLME MultiPHY demote indication

  Summary:
    Callback function pointer for the MLME MultiPHY demote indication.

  Description:
    This callback is used for the MLME MultiPHY demote indication.
    lsid    - Local switch identifier

  Remarks:
    None.
*/
typedef void (*MLME_MP_DEMOTE_INDICATION_CB)(uint8_t lsid);

// *****************************************************************************
/* MLME reset request

  Summary:
    Function pointer to request the flushing of all transmit and receive buffers
    and the resetting of all state variables.

  Description:
    This function pointer is used to request the flushing of all transmit and
    receive buffers and the resetting of all state variables. As a result, a
    Service Node will transit from its present functional state to the
    Disconnected functional state.

  Remarks:
    None.
*/
typedef void (*MLME_RESET_REQUEST)(void);

// *****************************************************************************
/* MLME reset confirm

  Summary:
    Callback function pointer for the MLME reset confirm.

  Description:
    This callback is used for the MLME reset confirm.
    result  - Result of the operation

  Remarks:
    None.
*/
typedef void (*MLME_RESET_CONFIRM_CB)(MLME_RESULT result);

// *****************************************************************************
/* MLME get request

  Summary:
    Function pointer to request information about a given PIB attribute of the
    MAC layer.

  Description:
    This function pointer is used to request information about a given PIB
    attribute of the MAC layer.
    pibAttribute    - PIB attribute

  Remarks:
    None.
*/
typedef void (*MLME_GET_REQUEST)(uint16_t pibAttrib);

// *****************************************************************************
/* MLME get confirm

  Summary:
    Callback function pointer for the MLME get confirm.

  Description:
    This callback is used for the MLME get confirm.
    status          - Status of the operation
    pibAttribute    - PIB attribute
    pibValue        - PIB attribute value
    pibSize         - PIB attribute value size

  Remarks:
    None.
*/
typedef void (*MLME_GET_CONFIRM_CB)(MLME_RESULT status, uint16_t pibAttrib,
    void *pibValue, uint8_t pibSize);

// *****************************************************************************
/* MLME list get request

  Summary:
    Function pointer to request information about a given PIB list attribute of
    the MAC layer.

  Description:
    This function pointer is used to request information about a given PIB list
    attribute of the MAC layer.
    pibAttribute    - PIB attribute

  Remarks:
    None.
*/
typedef void (*MLME_LIST_GET_REQUEST)(uint16_t pibAttrib);

// *****************************************************************************
/* MLME list get confirm

  Summary:
    Callback function pointer for the MLME list get confirm.

  Description:
    This callback is used for the MLME list get confirm.
    status          - Status of the operation
    pibAttribute    - PIB attribute
    pibBuff         - Buffer with PIB attribute values
    pibLen          - Buffer length


  Remarks:
    The buffer contains an array of records according to the PRIME specification.
    Values are stored starting with the MSB.
*/
typedef void (*MLME_LIST_GET_CONFIRM_CB)(MLME_RESULT status, uint16_t pibAttrib,
    uint8_t *pibBuff, uint16_t pibLen);

// *****************************************************************************
/* MLME set request

  Summary:
    Function pointer to request to set a new value for a given PIB attribute of
    the MAC layer.

  Description:
    This function pointer is used to request to set a new value for a given PIB
    attribute of the MAC layer.
    pibAttribute    - PIB attribute
    pibValue        - PIB attribute value
    pibSize         - PIB attribute value size

  Remarks:
    None.
*/
typedef void (*MLME_SET_REQUEST)(uint16_t pibAttrib, void *pibValue,
    uint8_t pibSize);

// *****************************************************************************
/* MLME set confirm

  Summary:
    Callback function pointer for the MLME set confirm.

  Description:
    This callback is used for the MLME set confirm.
    result  - Result of the operation

  Remarks:
    None.
*/
typedef void (*MLME_SET_CONFIRM_CB)(MLME_RESULT result);

// ****************************************************************************
/* MAC callback configuration

  Summary:
    Defines the callbacks to handle the MAC layer.

  Description:
    This structure defines the callbacks to handle the MAC layer.

  Remarks:
    None.
*/
typedef struct {
	MAC_ESTABLISH_INDICATION_CB mac_establish_ind;
	MAC_ESTABLISH_CONFIRM_CB mac_establish_cfm;
	MAC_RELEASE_INDICATION_CB mac_release_ind;
	MAC_RELEASE_CONFIRM_CB mac_release_cfm;
	MAC_JOIN_INDICATION_CB mac_join_ind;
	MAC_JOIN_CONFIRM_CB mac_join_cfm;
	MAC_LEAVE_INDICATION_CB mac_leave_ind;
	MAC_LEAVE_CONFIRM_CB mac_leave_cfm;
	MAC_DATA_INDICATION_CB mac_data_ind;
	MAC_DATA_CONFIRM_CB mac_data_cfm;
	PLME_RESET_CONFIRM_CB plme_reset_cfm;
	PLME_SLEEP_CONFIRM_CB plme_sleep_cfm;
	PLME_RESUME_CONFIRM_CB plme_resume_cfm;
	PLME_TESTMODE_CONFIRM_CB plme_testmode_cfm;
	PLME_GET_CONFIRM_CB plme_get_cfm;
	PLME_SET_CONFIRM_CB plme_set_cfm;
	MLME_REGISTER_INDICATION_CB mlme_register_ind;
	MLME_REGISTER_CONFIRM_CB mlme_register_cfm;
	MLME_UNREGISTER_INDICATION_CB mlme_unregister_ind;
	MLME_UNREGISTER_CONFIRM_CB mlme_unregister_cfm;
	MLME_PROMOTE_INDICATION_CB mlme_promote_ind;
	MLME_PROMOTE_CONFIRM_CB mlme_promote_cfm;
	MLME_DEMOTE_INDICATION_CB mlme_demote_ind;
	MLME_DEMOTE_CONFIRM_CB mlme_demote_cfm;
	MLME_RESET_CONFIRM_CB mlme_reset_cfm;
	MLME_GET_CONFIRM_CB mlme_get_cfm;
	MLME_LIST_GET_CONFIRM_CB mlme_list_get_cfm;
	MLME_SET_CONFIRM_CB mlme_set_cfm;
	MLME_MP_PROMOTE_INDICATION_CB mlme_mp_promote_ind;
	MLME_MP_PROMOTE_CONFIRM_CB mlme_mp_promote_cfm;
	MLME_MP_DEMOTE_INDICATION_CB mlme_mp_demote_ind;
	MLME_MP_DEMOTE_CONFIRM_CB mlme_mp_demote_cfm;
} MAC_CALLBACKS;

// ****************************************************************************
/* MAC callback function pointer

  Summary:
    Defines the funtion pointer to set the callbacks to handle the MAC layer.

  Description:
    This data type defines the function pointer to set the callbacks to
    handle the MAC layer.

  Remarks:
    None.
*/
typedef void (*MAC_SET_CALLBACKS)(MAC_CALLBACKS *primeMacCbs);

//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif /* MAC_DEFS_H_INCLUDE */

/*******************************************************************************
 End of File
*/
