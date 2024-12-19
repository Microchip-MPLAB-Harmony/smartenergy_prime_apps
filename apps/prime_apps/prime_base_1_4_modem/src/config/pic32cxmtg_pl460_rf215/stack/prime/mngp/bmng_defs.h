/*******************************************************************************
  PRIME Base Management Definitions Header

  Company:
    Microchip Technology Inc.

  File Name:
    bmng_defs.h

  Summary:
    PRIME MAC Definitions Header File

  Description:
    This file contains definitions of the PRIME Base Management primitives to be
    used by the PRIME application in the Base Node.
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

#ifndef BMNG_DEFS_H_
#define BMNG_DEFS_H_

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    extern "C" {

#endif
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Data Types
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Page sizes for Firmware Upgrade Protocol

 Summary:
    Defines the page sizes for Firmware Upgrade Protocol.

 Description:
    This enumeration defines the page sizes used in the Firmware Upgrade Protocol.

 Remarks:
    None.
*/
typedef enum {
	BMNG_FUP_PAGE_SIZE_AUTO = 0,
	BMNG_FUP_PAGE_SIZE_32,
	BMNG_FUP_PAGE_SIZE_64,
	BMNG_FUP_PAGE_SIZE_128,
	BMNG_FUP_PAGE_SIZE_192
} BMNG_FUP_PAGE_SIZE;

// *****************************************************************************
/* Acknowledgement codes for Firmware Upgrade Protocol

 Summary:
    Defines the acknowledgement codes for Firmware Upgrade Protocol.

 Description:
    This enumeration defines the acknowledgement codes used in the Firmware
    Upgrade Protocol.

 Remarks:
    None.
*/
typedef enum {
	BMNG_FUP_ACK_OK = 0,
	BMNG_FUP_ACK_ERROR,
	BMNG_FUP_ACK_ERROR_MAC,
	BMNG_FUP_ACK_ERROR_MODEL,
	BMNG_FUP_ACK_ERROR_CRC,
	BMNG_FUP_ACK_ERROR_DATA,
	BMNG_FUP_ACK_ERROR_CRC_FILE,
	BMNG_FUP_ACK_CRC_ONGOING,
	BMNG_FUP_ACK_FU_ONGOING
} BMNG_FUP_ACK;

// *****************************************************************************
/* Error codes for Firmware Upgrade Protocol

 Summary:
    Defines the error codes for Firmware Upgrade Protocol.

 Description:
    This enumeration defines the error codes used in the Firmware Upgrade
    Protocol.

 Remarks:
    None.
*/
typedef enum {
	BMNG_FUP_ERROR_NODE_NO_ERROR = 0,
	BMNG_FUP_ERROR_NODE_EXEC_WRONGSTATE,
	BMNG_FUP_ERROR_NODE_UPG_WRONGSTATE,
	BMNG_FUP_ERROR_NODE_WRONGSTATE,
	BMNG_FUP_ERROR_NODE_RCV_WRONGSTATE,
	BMNG_FUP_ERROR_NODE_CNTDWN_WRONGSTATE,
	BMNG_FUP_ERROR_NODE_FW_NOTMATCH,
	BMNG_FUP_ERROR_NODE_REVERT_ERRORINIT,
	BMNG_FUP_ERROR_NODE_REVERT_ERR7,
	BMNG_FUP_ERROR_NODE_RETRY_KILL,
	BMNG_FUP_ERROR_NODE_UNICAST_TIMEOUT,
	BMNG_FUP_ERROR_NODE_CONFIRM_IND,
	BMNG_FUP_ERROR_NODE_REVERT_CRCNOK,
	BMNG_FUP_ERROR_NODE_RESTART,
	BMNG_FUP_ERROR_NODE_MISS_BITMAP_WRONG,
	BMNG_FUP_ERROR_NODE_MISS_LIST_WRONG,
	BMNG_FUP_ERROR_NODE_VENDOR_INVALID,
	BMNG_FUP_ERROR_NODE_MODEL_NOTMATCH,
	BMNG_FUP_WARNING_NODE_ALREADY_UPDATED,
	BMNG_FUP_WARNING_NODE_ALREADY_EXECUTED,
	BMNG_FUP_WARNING_NODE_LINK_QUALITY
} BMNG_FUP_ERROR;

// *****************************************************************************
/* Node states for Firmware Upgrade Protocol

 Summary:
    Defines the node states for Firmware Upgrade Protocol.

 Description:
    This enumeration defines the node states used in the Firmware Upgrade
    Protocol.

 Remarks:
    None.
*/
typedef enum {
	BMNG_FUP_NODE_STATE_IDLE        = 0,
	BMNG_FUP_NODE_STATE_RECEIVING   = 1,
	BMNG_FUP_NODE_STATE_COMPLETE    = 2,
	BMNG_FUP_NODE_STATE_COUNTDOWN   = 3,
	BMNG_FUP_NODE_STATE_UPGRADE     = 4,
	BMNG_FUP_NODE_STATE_EXCEPTION   = 5,   /* Only 1.4 */
	BMNG_FUP_NODE_STATE_UNKNOWN     = 0x7F
} BMNG_FUP_NODE_STATE;

// *****************************************************************************
/* Network events

 Summary:
    Defines the network events.

 Description:
    This enumeration defines the network events.

 Remarks:
    None.
*/
typedef enum {
	BMNG_NET_EVENT_REGISTER,
	BMNG_NET_EVENT_UNREGISTER,
	BMNG_NET_EVENT_PROMOTE,
	BMNG_NET_EVENT_DEMOTE,
	BMNG_NET_EVENT_ALIVE,
	BMNG_NET_EVENT_REBOOT,
	BMNG_NET_EVENT_NO_DUK,   /* Only 1.4 */
	BMNG_NET_EVENT_UNKNOWN_NODE
} BMNG_NET_EVENT;

// *****************************************************************************
/* Network Event information

 Summary:
    Defines the Network Event information.

 Description:
    This structure defines the Network Event information.

 Remarks:
    None.
*/
typedef struct {
	BMNG_NET_EVENT netEvent;
	uint8_t eui48[6];
	uint8_t sid;
	uint16_t lnid;
	uint8_t lsid;
	uint8_t alvRxCnt;
	uint8_t alvTxCnt;
	uint8_t alvTime;
    uint16_t pch;
	uint16_t pchLsid;
} BMNG_NET_EVENT_INFO;

// *****************************************************************************
/* Acknowledgement codes for PRIME Profile

 Summary:
    Defines the acknowledgement codes for PRIME Profile.

 Description:
    This enumeration defines the acknowledgement codes to be used in the PRIME
    Profile.

 Remarks:
    None.
*/
typedef enum {
	BMNG_PPROF_ACK_OK = 0,
	BMNG_PPROF_ACK_ERROR
} BMNG_PPROF_ACK;

// *****************************************************************************
/* Acknowledgement codes for the Whitelist

 Summary:
    Defines the acknowledgement codes for the Whitelist.

 Description:
    This enumeration defines the acknowledgement codes to be used when handling
    the Whitelist.

 Remarks:
    None.
*/
typedef enum {
	BMNG_WHITELIST_ACK_OK = 0,
	BMNG_WHITELIST_ACK_ERROR
} BMNG_WHITELIST_ACK;

// *****************************************************************************
/* Start a firmware upgrade process

  Summary:
    Function pointer to request to start a firmware upgrade process.

  Description:
    This data type contains the definition of the function pointer to start a
    firmware upgrade process.
    - cmd: Command to acknowledge
    - enable: Enable (1) or disable (0) FU

  Remarks:
    The command is acknowledged with the FUP ACK callback..
*/
typedef void (*BMNG_FUP_START_FU_REQUEST)(uint8_t cmd, uint8_t enable);

// *****************************************************************************
/* Clear the target list in a firmware upgrade process

  Summary:
    Function pointer to request to clear the target list in a firmware upgrade
    process.

  Description:
    This data type contains the definition of the function pointer to clear the
    target list in a firmware upgrade process.
    - cmd: Command to acknowledge

  Remarks:
    The command is acknowledged with the FUP ACK callback.
*/
typedef void (*BMNG_FUP_CLEAR_TARGET_LIST_REQUEST)(uint8_t cmd);

// *****************************************************************************
/* Add a target node for a firmware upgrade process

  Summary:
    Function pointer to request to add a target node for a firmware upgrade
    process.

  Description:
    This data type contains the definition of the function pointer to add a
    target node for a firmware upgrade process.
    - cmd: Command to acknowledge
    - eui48: Pointer to the address of the node to be added

  Remarks:
    The command is acknowledged with the FUP ACK callback.
*/
typedef void (*BMNG_FUP_ADD_TARGET_REQUEST)(uint8_t cmd, uint8_t *eui48);

// *****************************************************************************
/* Set the firmware data for a firmware upgrade process

  Summary:
    Function pointer to request to set the firmware data for a firmware upgrade
    process.

  Description:
    This data type contains the definition of the function pointer to set the
    firmware data for a firmware upgrade process.
    - cmd: Command to acknowledge
    - vendorLen: Vendor length
    - vendor: Pointer to the vendor identification
    - modelLen: Model length
    - model: Pointer to the model identification
    - versionLen: Version length
    - version: Pointer to the version identification

  Remarks:
    The command is acknowledged with the FUP ACK callback.
*/
typedef void (*BMNG_FUP_SET_FW_DATA_REQUEST)(uint8_t cmd, uint8_t vendorLen,
    char *vendor, uint8_t modelLen, char *model, uint8_t versionLen,
    char *version);

// *****************************************************************************
/* Set the upgrade options for a firmware upgrade process

  Summary:
    Function pointer to request to set the upgrade options for a firmware
    upgrade process.

  Description:
    This data type contains the definition of the function pointer to set the
    upgrade options for a firmware upgrade process.
    - cmd: Command to acknowledge
    - arqEn: Enable (1) or disable (0) ARQ protocol
    - pageSize: Page size
    - multicastEn: Enable (1) or disable (0) multicast
    - delayRestart: Delay restart time in seconds
    - safetyTimer: Safety timer in seconds

  Remarks:
    The command is acknowledged with the FUP ACK callback.
*/
typedef void (*BMNG_FUP_SET_UPGRADE_OPTIONS_REQUEST)(uint8_t cmd, uint8_t arqEn,
    BMNG_FUP_PAGE_SIZE pageSize, uint8_t multicastEn, uint32_t delayRestart,
    uint32_t safetyTimer);

// *****************************************************************************
/* Initialize the file transmission for a firmware upgrade process

  Summary:
    Function pointer to request to initialize the file transmission for a
    firmware upgrade process.

  Description:
    This data type contains the definition of the function pointer to initialize
    the file transmission for a firmware upgrade process.
    - cmd: Command to acknowledge
    - frameNumber: Frame number (0x0000)
    - fileSize: File size
    - frameSize: Frame size
    - crc: File CRC-32

  Remarks:
    The command is acknowledged with the FUP ACK callback.
*/
typedef void (*BMNG_FUP_INIT_FILE_TX_REQUEST)(uint8_t cmd, uint16_t frameNumber,
    uint32_t fileSize, uint16_t frameSize, uint32_t crc);

// *****************************************************************************
/* Receive a data frame for a firmware upgrade process

  Summary:
    Function pointer to request to receive a data frame during the file
    transmission for a firmware upgrade process.

  Description:
    This data type contains the definition of the function pointer to receive a
    data frame during the file transmission for a firmware upgrade process.
    - cmd: Command to acknowledge
    - frameNumber: Frame number
    - dataLen: Data length
    - data: Pointer to the data frame

  Remarks:
    The command is acknowledged with the FUP ACK callback.
*/
typedef void (*BMNG_FUP_DATA_FRAME_REQUEST)(uint8_t cmd, uint16_t frameNumber,
    uint16_t dataLen, uint8_t *data);

// *****************************************************************************
/* Check the CRC of the transmitted file for a firmware upgrade process

  Summary:
    Function pointer to request to check the CRC of the transmitted file for a
    firmware upgrade process.

  Description:
    This data type contains the definition of the function pointer to check the
    CRC of the transmitted file for a firmware upgrade process.
    - cmd: Command to acknowledge

  Remarks:
    The command is acknowledged with the FUP ACK callback.
*/
typedef void (*BMNG_FUP_CHECK_CRC_REQUEST)(uint8_t cmd);

// *****************************************************************************
/* Abort an ongoing firmware upgrade process

  Summary:
    Function pointer to request to abort an ongoing firmware upgrade process.

  Description:
    This data type contains the definition of the function pointer to abort an
    ongoing firmware upgrade process.
    - cmd: Command to acknowledge
    - eui48: Pointer to the address of the node to be aborted

  Remarks:
    If the MAC address is FF:FF:FF:FF:FF:FF, the FU is aborted for all nodes.
    The command is acknowledged with the FUP ACK callback.
*/
typedef void (*BMNG_FUP_ABORT_FU_REQUEST)(uint8_t cmd, uint8_t *eui48);

// *****************************************************************************
/* Set the matching rules for a firmware upgrade process

  Summary:
    Function pointer to request to set the matching rules for a firmware upgrade
    process.

  Description:
    This data type contains the definition of the function pointer to set the
    matching rules for a firmware upgrade process.
    - cmd: Command to acknowledge
    - rules: Match rules: 0000 0MV0

  Remarks:
    If M and/or V are set, only the nodes matching model and/or vendor will be
    upgraded.
    The command is acknowledged with the FUP ACK callback.
*/
typedef void (*BMNG_FUP_SET_MATCH_RULE_REQUEST)(uint8_t cmd, uint8_t rules);

// *****************************************************************************
/* Set the signature data for a firmware upgrade process

  Summary:
    Function pointer to request to set the signature data for a firmware
    upgrade process.

  Description:
    This data type contains the definition of the function pointer to set the
    signature data for a firmware upgrade process.
    - cmd: Command to acknowledge
    - algorithm: Used algorithm to check signature
    - length: Signature length in bytes

  Remarks:
    The command is acknowledged with the FUP ACK callback.
*/
typedef void (*BMNG_FUP_SET_SIGNATURE_DATA_REQUEST)(uint8_t cmd,
    uint8_t algorithm, uint16_t length);

// *****************************************************************************
/* Get the firmware version

  Summary:
    Function pointer to request to get the firmware version.

  Description:
    This data type contains the definition of the function pointer to get the
    firmware version.
    - cmd: Command to acknowledge
    - eui48: Pointer to the address of the node to be asked

  Remarks:
    If the MAC address is FF:FF:FF:FF:FF:FF, the version information is asked to
    all registered nodes.
    The command is acknowledged with the FUP ACK callback.
    The result of the request is returned in the FUP version indication callback.
*/
typedef void (*BMNG_FUP_GET_VERSION_REQUEST)(uint8_t cmd, uint8_t *eui48);

// *****************************************************************************
/* Get the firmware upgrade state

  Summary:
    Function pointer to request to get the firmware upgrade state.

  Description:
    This data type contains the definition of the function pointer to get the
    firmware upgrade state.
    - cmd: Command to acknowledge
    - eui48: Pointer to the address of the node to be asked

  Remarks:
    If the MAC address is FF:FF:FF:FF:FF:FF, the state information is asked to
    all registered nodes.
    The command is acknowledged with the FUP ACK callback.
    The result of the request is returned in the FUP state indication callback.
*/
typedef void (*BMNG_FUP_GET_STATE_REQUEST)(uint8_t cmd, uint8_t *eui48);

// *****************************************************************************
/* Firmware Upgrade Protocol acknowledgement

  Summary:
    Callback function pointer for the Firmware Upgrade Protocol acknowledgement.

  Description:
    This callback is used for the Firmware Upgrade Protocol acknowledgement.
    cmd         - Command being acknowledged
    ackCode     - ACK returned code
    extraInfo   - Extra information

  Remarks:
    None.
*/
typedef void (*BMNG_FUP_ACK_CB)(uint8_t cmd, BMNG_FUP_ACK ackCode,
    uint16_t extraInfo);

// *****************************************************************************
/* Firmware Upgrade Protocol status indication

  Summary:
    Callback function pointer for the Firmware Upgrade Protocol status
    indication.

  Description:
    This callback is used for the Firmware Upgrade Protocol status indication.
    nodeState   - Node upgrade state
    pages       - Pages received by the SN according to the BN
    eui48       - Pointer to the MAC address of the node

  Remarks:
    None.
*/
typedef void (*BMNG_FUP_STATUS_INDICATION_CB)(BMNG_FUP_NODE_STATE nodeState,
    uint16_t pages, uint8_t *eui48);

// *****************************************************************************
/* Firmware Upgrade Protocol error indication

  Summary:
    Callback function pointer for the Firmware Upgrade Protocol error
    indication.

  Description:
    This callback is used for the Firmware Upgrade Protocol error indication.
    errorCode   - Error code
    eui48       - Pointer to the MAC address of the node

  Remarks:
    None.
*/
typedef void (*BMNG_FUP_ERROR_INDICATION_CB)(BMNG_FUP_ERROR errorCode,
    uint8_t *eui48);

// *****************************************************************************
/* Firmware Upgrade Protocol version indication

  Summary:
    Callback function pointer for the Firmware Upgrade Protocol version
    indication.

  Description:
    This callback is used for the Firmware Upgrade Protocol version indication.
    eui48       - Pointer to the MAC address of the node
    vendorLen   - Vendor length
    vendor      - Pointer to the vendor identification
    modelLen    - Model length
    model       - Pointer to the model identification
    versionLen  - Version length
    version     - Pointer to the version identification

  Remarks:
    None.
*/
typedef void (*BMNG_FUP_VERSION_INDICATION_CB)(uint8_t *eui48, uint8_t vendorLen,
    char *vendor, uint8_t modelLen, char *model, uint8_t versionLen, char *version);

// *****************************************************************************
/* Firmware Upgrade Protocol kill indication

  Summary:
    Callback function pointer for the Firmware Upgrade Protocol kill indication.

  Description:
    This callback is used for the Firmware Upgrade Protocol kill indication.
    eui48       - Pointer to the MAC address of the node

  Remarks:
    None.
*/
typedef void (*BMNG_FUP_KILL_INDICATION_CB)(uint8_t *eui48);

// *****************************************************************************
/* Network Event indication

  Summary:
    Callback function pointer for the Network Event indication.

  Description:
    This callback is used for the Network Event indication.
    netEvent    - Network event information

  Remarks:
    None.
*/
typedef void (*BMNG_NET_EVENT_INDICATION_CB)(BMNG_NET_EVENT_INFO *netEvent);

// *****************************************************************************
/* Get a PIB attribute using the PRIME Profile

  Summary:
    Function pointer to request to get a PIB attribute from a node using the
    PRIME Profile.

  Description:
    This data type contains the definition of the function pointer to get a PIB
    attribute from a node using the PRIME Profile.
    - cmd: Command to acknowledge
    - eui48: Pointer to the address of the node to be asked
    - dataLen: Data length
    - data: Pointer to the data

  Remarks:
    The command is acknowledged with the PPROF ACK callback.
    The result of the request is returned in the PPROF response indication
    callback.
*/
typedef void (*BMNG_PPROF_GET_REQUEST)(uint8_t cmd, uint8_t *eui48,
    uint16_t dataLen, uint8_t *data);

// *****************************************************************************
/* Set a PIB attribute using the PRIME Profile

  Summary:
    Function pointer to request to set a PIB attribute in a node using the
    PRIME Profile.

  Description:
    This data type contains the definition of the function pointer to set a PIB
    attribute in a node using the PRIME Profile.
    - cmd: Command to acknowledge
    - eui48: Pointer to the address of the node to be asked
    - dataLen: Data length
    - data: Pointer to the data

  Remarks:
    The command is acknowledged with the PPROF ACK callback.
*/
typedef void (*BMNG_PPROF_SET_REQUEST)(uint8_t cmd, uint8_t *eui48,
    uint16_t dataLen, uint8_t *data);

// *****************************************************************************
/* Reset all PIB statistics attributes using the PRIME Profile

  Summary:
    Function pointer to request to reset all PIB statistics attributes of a node
    using the PRIME Profile.

  Description:
    This data type contains the definition of the function pointer to reset all
    PIB statistics attributes of a node using the PRIME Profile.
    - cmd: Command to acknowledge
    - eui48: Pointer to the address of the node to be reset

  Remarks:
    The command is acknowledged with the PPROF ACK callback.
*/
typedef void (*BMNG_PPROF_RESET_REQUEST)(uint8_t cmd, uint8_t *eui48);

// *****************************************************************************
/* Reboot a node using the PRIME Profile

  Summary:
    Function pointer to request to reboot a node using the PRIME Profile.

  Description:
    This data type contains the definition of the function pointer to reboot a
    node using the PRIME Profile.

  Remarks:
    The command is acknowledged with the PPROF ACK callback.
*/
typedef void (*BMNG_PPROF_REBOOT_REQUEST)(uint8_t cmd, uint8_t *eui48);

// *****************************************************************************
/* Get a PIB attribute using the PRIME Profile

  Summary:
    Function pointer to request to get a PIB attribute from a node using the
    PRIME Profile.

  Description:
    This data type contains the definition of the function pointer to get a PIB
    attribute from a node using the PRIME Profile.
    - cmd: Command to acknowledge
    - eui48: Pointer to the address of the node to be asked
    - dataLen: Data length
    - data: Pointer to the data

  Remarks:
    The command is acknowledged with the PPROF ACK callback.
    The result of the request is returned in the PPROF enhanced response
    indication callback.
*/
typedef void (*BMNG_PPROF_GET_ENHANCED_REQUEST)(uint8_t cmd, uint8_t *eui48,
    uint16_t dataLen, uint8_t *data);

// *****************************************************************************
/* Get the zero cross difference between BN and SN using the PRIME Profile

  Summary:
    Function pointer to request to get the zero cross difference between BN and
    SN using the PRIME Profile.

  Description:
    This data type contains the definition of the function pointer to get the
    zero cross difference between BN and SN using the PRIME Profile.
    - cmd: Command to acknowledge
    - eui48: Pointer to the address of the node to be asked

  Remarks:
    The command is acknowledged with the PPROF ACK callback.
    The result of the request is returned in the PPROF zero cross difference
    response indication callback.
*/
typedef void (*BMNG_PPROF_GET_ZC_DIFF_REQUEST)(uint8_t cmd, uint8_t *eui48);

// *****************************************************************************
/* PRIME Profile acknowledgement

  Summary:
    Callback function pointer for the PRIME Profile acknowledgement.

  Description:
    This callback is used for the PRIME Profile acknowledgement.
    cmd         - Command being acknowledged
    ackCode     - ACK returned code

  Remarks:
    None.
*/
typedef void (*BMNG_PPROF_ACK_CB)(uint8_t cmd, BMNG_PPROF_ACK ackCode);

// *****************************************************************************
/* PRIME Profile get response

  Summary:
    Callback function pointer for the PRIME Profile get response.

  Description:
    This callback is used for the PRIME Profile get response.
    eui48       - Pointer to the MAC address of the node
    dataLen     - Data length
    data        - Pointer to the data

  Remarks:
    None.
*/
typedef void (*BMNG_PPROF_GET_RESPONSE_CB)(uint8_t *eui48, uint16_t dataLen,
    uint8_t *data);

// *****************************************************************************
/* PRIME Profile get enhanced response

  Summary:
    Callback function pointer for the PRIME Profile get enhanced response.

  Description:
    This callback is used for the PRIME Profile get enhanced response.
    eui48       - Pointer to the MAC address of the node
    dataLen     - Data length
    data        - Pointer to the data

  Remarks:
    None.
*/
typedef void (*BMNG_PPROF_GET_ENHANCED_RESPONSE_CB)(uint8_t *eui48,
    uint16_t dataLen, uint8_t *data);

// *****************************************************************************
/* PRIME Profile get zero crossing response

  Summary:
    Callback function pointer for the PRIME Profile get zero crossing response.

  Description:
    This callback is used for the PRIME Profile get zero crossing response.
    eui48       - Pointer to the MAC address of the node
    zcStatus    - Zero cross status
    zcTime      - Zero crossing time (in microseconds)

  Remarks:
    None.
*/
typedef void (*BMNG_PPROF_GET_ZC_RESPONSE_CB)(uint8_t *eui48, uint8_t zcStatus,
    uint32_t zcTime);

// *****************************************************************************
/* PRIME Profile get zero crossing difference response

  Summary:
    Callback function pointer for the PRIME Profile get zero crossing difference
    response.

  Description:
    This callback is used for the PRIME Profile get the response to the zero
    crossing difference between BN and SN.
    eui48       - Pointer to the MAC address of the node
    timeFreq    - Frequency
    timeDiff    - Zero crossing difference (in 10s of microseconds)

  Remarks:
    None.
*/
typedef void (*BMNG_PPROF_GET_ZC_DIFF_RESPONSE_CB)(uint8_t *eui48,
    uint32_t timeFreq, uint32_t timeDiff);

// *****************************************************************************
/* Add a node to the Whitelist

  Summary:
    Function pointer to request to add a node to the Whitelist.

  Description:
    This data type contains the definition of the function pointer to add a node
    to the Whitelist.
    - cmd: Command to acknowledge
    - eui48: Pointer to the address of the node to be added

  Remarks:
    The command is acknowledged with the Whitelist ACK callback.
*/
typedef void (*BMNG_WHITELIST_ADD_REQUEST)(uint8_t cmd, uint8_t *eui48);

// *****************************************************************************
/* Remove a node from the Whitelist

  Summary:
    Function pointer to request to remove a node from the Whitelist.

  Description:
    This data type contains the definition of the function pointer to remove a
    node from the Whitelist.
    - cmd: Command to acknowledge
    - eui48: Pointer to the address of the node to be removed

  Remarks:
    The command is acknowledged with the Whitelist ACK callback.
*/
typedef void (*BMNG_WHITELIST_REMOVE_REQUEST)(uint8_t cmd, uint8_t *eui48);

// *****************************************************************************
/* Whitelist acknowledgement

  Summary:
    Callback function pointer for the Whitelist acknowledgement.

  Description:
    This callback is used for the Whitelist acknowledgement.
    cmd         - Command being acknowledged
    ackCode     - ACK returned code

  Remarks:
    None.
*/
typedef void (*BMNG_WHITELIST_ACK_CB)(uint8_t cmd, BMNG_WHITELIST_ACK ackCode);

// ****************************************************************************
/* Base Management callback configuration

  Summary:
    Defines the callbacks to handle the Base Management.

  Description:
    This structure defines the callbacks to handle the Base Management.

  Remarks:
    None.
*/
typedef struct {
	BMNG_FUP_ACK_CB fup_ack;
	BMNG_FUP_STATUS_INDICATION_CB fup_status_ind;
	BMNG_FUP_ERROR_INDICATION_CB fup_error_ind;
	BMNG_FUP_VERSION_INDICATION_CB fup_version_ind;
	BMNG_FUP_KILL_INDICATION_CB fup_kill_ind;
	BMNG_NET_EVENT_INDICATION_CB network_event_ind;
	BMNG_PPROF_ACK_CB pprof_ack;
	BMNG_PPROF_GET_RESPONSE_CB pprof_get_response;
	BMNG_PPROF_GET_ENHANCED_RESPONSE_CB pprof_get_enhanced_response;
	BMNG_PPROF_GET_ZC_RESPONSE_CB pprof_get_zc_response;
	BMNG_PPROF_GET_ZC_DIFF_RESPONSE_CB pprof_zc_diff_response;
	BMNG_WHITELIST_ACK_CB whitelist_ack;
} BMNG_CALLBACKS;

// *****************************************************************************
/* Base Management callback function pointer

  Summary:
    Defines the funtion pointer to set the callbacks to handle the PRIME Base
    Management.

  Description:
    This data type defines the function pointer to set the callbacks to
    handle the PRIME Base Management.

  Remarks:
    None.
*/
typedef void (*BMNG_SET_CALLBACKS)(BMNG_CALLBACKS *bmngCallbacks);

//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif /* BMNG_DEFS_H_ */

/*******************************************************************************
 End of File
*/
