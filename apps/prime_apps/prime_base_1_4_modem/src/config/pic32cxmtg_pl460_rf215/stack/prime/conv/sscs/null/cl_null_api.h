/*******************************************************************************
  PRIME NULL Convergence Sublayer API Header

  Company:
    Microchip Technology Inc.

  File Name:
    cl_null_api.h

  Summary:
    PRIME NULL Convergence Sublayer API Header File

  Description:
    This file contains definitions of the PRIME Null Convergence Sublayer
    primitives to be used by the PRIME application.
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

#ifndef CL_NULL_API_H
#define CL_NULL_API_H

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
#include "stack/prime/mac/mac_defs.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    extern "C" {

#endif
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: PRIME NULL Convergence Sublayer Interface Primitives
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Function:
    void CL_NULL_SetCallbacks
    (
        MAC_CALLBACKS *macCallbacks
    )

  Summary:
    Sets the callbacks to the PRIME Null Convergence Sublayer.

  Description:
    This routine sets the callbacks to the PRIME Null Convergence Sublayer.

  Precondition:
    The CL_NULL_Initialize routine must have been called before.

  Parameters:
    macCallbacks    - Pointer to the MAC callback structure

  Returns:
    None.

  Example:
    <code>
    MAC_CALLBACKS macCallbacks;

    memset(macCallbacks, NULL, sizeof(macCallbacks);

	macCallbacks.macDataConfirm = appDataConfirm;
	macCallbacks.macDataIndication = appDataIndication;
	macCallbacks.macEstablishConfirm = appEstablishConfirm;
	macCallbacks.macEstablishIndication = appEstablishIndication;
	macCallbacks.mlmeGetCofirm = appGetConfirm;
	macCallbacks.mlmeListGetConfirm = appListGetCofirm;
	macCallbacks.mlmeSetConfirm = appSetConfirm;

	CL_NULL_SetCallbacks(&macCallbacks);
    </code>

  Remarks:
    Unused callbacks must be set to NULL.
*/
void CL_NULL_SetCallbacks(MAC_CALLBACKS *macCallbacks);

// *****************************************************************************
/* Function:
    void CL_NULL_EstablishRequest
    (
        uint8_t *eui48,
        uint8_t type,
        uint8_t *data,
        uint16_t dataLen,
        uint8_t arq,
        uint8_t cfBytes,
        uint8_t ae
    )

  Summary:
    Request a connection establishment.

  Description:
    This routine is used to request a connection establishment.

  Precondition:
    The CL_NULL_Initialize routine must have been called before.

  Parameters:
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

  Returns:
    None.

  Example:
    <code>
    uint8_t eui48[6];
    memset(eui48, 0x12, 6);

	CL_NULL_EstablishRequest(eui48, 9, NULL, 0, 1, 0, 0);
    </code>

  Remarks:
    The result of the request is returned in the confirm callback.
*/
void CL_NULL_EstablishRequest(uint8_t *eui48, uint8_t type, uint8_t *data,
    uint16_t dataLen, uint8_t arq, uint8_t cfBytes, uint8_t ae);

// *****************************************************************************
/* Function:
    void CL_NULL_EstablishResponse
    (
        uint16_t conHandle,
        MAC_ESTABLISH_RESPONSE_ANSWER answer,
        uint8_t *data,
        uint16_t dataLen,
        uint8_t uc_ae
    )

  Summary:
    Response to a connection establishment indication.

  Description:
    This routine is used to respond to a connection establishment indication.

  Precondition:
    The CL_NULL_Initialize routine must have been called before.

  Parameters:
    conHandle   - Unique identifier of the connection
    answer      - Action to be taken for this connection establishment
    data        - Data associated with the connection establishment procedure
    dataLen     - Length of the data in bytes
    ae          - Flag to indicate that authentication and encryption is
                  requested (v1.4)

  Returns:
    None.

  Example:
    <code>
	CL_NULL_EstablishResponse(3, MAC_ESTABLISH_RESPONSE_ANSWER_ACCEPT, NULL, 0, 0);
    </code>

  Remarks:
    None.
*/
void CL_NULL_EstablishResponse(uint16_t conHandle,
    MAC_ESTABLISH_RESPONSE_ANSWER answer, uint8_t *data, uint16_t dataLen,
    uint8_t uc_ae);

// *****************************************************************************
/* Function:
    void CL_NULL_ReleaseRequest
    (
        uint16_t conHandle
    )

  Summary:
    Request to release a connection.

  Description:
    This routine is used to request a connection release.

  Precondition:
    The CL_NULL_Initialize routine must have been called before.

  Parameters:
    conHandle   - Unique identifier of the connection

  Returns:
    None.

  Example:
    <code>
	CL_NULL_ReleaseRequest(3);
    </code>

  Remarks:
    The result of the request is returned in the confirm callback.
*/
void CL_NULL_ReleaseRequest(uint16_t conHandle);

// *****************************************************************************
/* Function:
    void CL_NULL_ReleaseResponse
    (
        uint16_t conHandle,
        MAC_RELEASE_RESPONSE_ANSWER answer
    )

  Summary:
    Response to a connection release indication.

  Description:
    This routine is used to respond to a connection release indication.

  Precondition:
    The CL_NULL_Initialize routine must have been called before.

  Parameters:
    conHandle   - Unique identifier of the connection
    answer      - Action to be taken for this connection release

  Returns:
    None.

  Example:
    <code>
	CL_NULL_ReleaseResponse(3, MAC_RELEASE_RESPONSE_ANSWER_ACCEPT);
    </code>

  Remarks:
    None.
*/
void CL_NULL_ReleaseResponse(uint16_t conHandle,
    MAC_RELEASE_RESPONSE_ANSWER answer);

// *****************************************************************************
/* Function:
    void CL_NULL_JoinRequest
    (
        MAC_JOIN_REQUEST_MODE connMode,
        uint16_t conHandle,
        uint8_t *eui48,
        MAC_CONNECTION_TYPE connType,
        uint8_t *data,
        uint16_t dataLen,
        uint8_t ae
    )

  Summary:
    Request to join a multicast connection.

  Description:
    This routine is used to request to join a multicast connection.

  Precondition:
    The CL_NULL_Initialize routine must have been called before.

  Parameters:
    conMode     - Connection type: broadcast or multicast
    conHandle   - Unique identifier of the connection (only used in base node)
    eui48       - Pointer to the address of the node to which this join is
                  being requested (only used in base node)
    conType     - Connection type
    data        - Data associated with the join request procedure
    dataLen     - Length of the data in bytes
    ae          - Flag to indicate that authentication and encryption is
                  requested (v1.4)

  Returns:
    None.

  Example:
    <code>
	CL_NULL_JoinRequest(JOIN_REQUEST_BROADCAST, 0, NULL, 9, NULL, 0, 0);
    </code>

  Remarks:
    The result of the request is returned in the confirm callback.
*/
void CL_NULL_JoinRequest(MAC_JOIN_REQUEST_MODE conMode, uint16_t conHandle,
    uint8_t *eui48, MAC_CONNECTION_TYPE conType, uint8_t *data, uint16_t dataLen,
    uint8_t ae);

// *****************************************************************************
/* Function:
    void CL_NULL_JoinResponse
    (
        uint16_t conHandle,
        uint8_t *eui48,
        MAC_JOIN_RESPONSE_ANSWER answer,
        uint8_t ae
    )

  Summary:
    Response to a connection join indication.

  Description:
    This routine is used to respond to a connection join indication.

  Precondition:
    The CL_NULL_Initialize routine must have been called before.

  Parameters:
    conHandle   - Unique identifier of the connection
    eui48       - Pointer to the address of the node which requested the multicast
                  group join(only used in base node)
    answer      - Action to be taken for this join request
    ae          - Flag to indicate that authentication and encryption is
                  requested (v1.4)

  Returns:
    None.

  Example:
    <code>
	CL_NULL_JoinResponse(0, NULL, JOIN_RESPONSE_ACCEPT, 0);
    </code>

  Remarks:
    None.
*/
void CL_NULL_JoinResponse(uint16_t conHandle, uint8_t *eui48,
    MAC_JOIN_RESPONSE_ANSWER answer, uint8_t ae);

// *****************************************************************************
/* Function:
    void CL_NULL_LeaveRequest
    (
        uint16_t conHandle,
        uint8_t *eui48
    )

  Summary:
    Request to leave a multicast connection.

  Description:
    This routine is used to request to leave a multicast connection.

  Precondition:
    The CL_NULL_Initialize routine must have been called before.

  Parameters:
    conHandle   - Unique identifier of the connection
    eui48       - Pointer to the address of the node to be removed from the
                  multicast group (only used in base node)

  Returns:
    None.

  Example:
    <code>
	CL_NULL_LeaveRequest(0, NULL);
    </code>

  Remarks:
    The result of the request is returned in the confirm callback.
*/
void CL_NULL_LeaveRequest(uint16_t conHandle, uint8_t *eui48);

// *****************************************************************************
/* Function:
    void CL_NULL_RedirectResponse
    (
        uint16_t conHandle,
        uint8_t *eui48,
        uint8_t *data,
        uint16_t dataLen
    )

  Summary:
    Response to a direct connection establishment indication.

  Description:
    This routine is used to respond to a direct connection establishment
    indication.

  Precondition:
    The CL_NULL_Initialize routine must have been called before.

  Parameters:
    conHandle   - Unique identifier of the connection
    eui48       - Pointer to the address of the node to which this connection
                  will be "redirected"
    data        - Data associated with the connection establishment procedure
    dataLen     - Length of the data in bytes

  Returns:
    None.

  Example:
    <code>
    uint8_t eui48[6];
    memset(eui48, 0x12, 6);

	CL_NULL_RedirectResponse(8, eui48, NULL, 0);
    </code>

  Remarks:
    The result of the request is returned in the confirm callback.
*/
void CL_NULL_RedirectResponse(uint16_t conHandle, uint8_t *eui48, uint8_t *data,
    uint16_t dataLen);

// *****************************************************************************
/* Function:
    void CL_NULL_DataRequest
    (
        uint16_t conHandle,
        uint8_t *data,
        uint16_t dataLen,
        uint8_t prio,
        uint32_t timeRef
    )

  Summary:
    Request the transmission of data over a connection.

  Description:
    This routine is used to request the transmission of data over a connection.

  Precondition:
    The CL_NULL_Initialize routine must have been called before.

  Parameters:
    conHandle   - Unique identifier of the connection
    data        - Pointer to data to be transmitted through this connection
    dataLen     - Length of the data in bytes
    prio        - Priority of the data to be sent when using the CSMA access
                  scheme
    timeRef     - Time reference (in 10s of microseconds) (v1.4)

  Returns:
    None.

  Example:
    <code>
    uint16_t msgLen = 20;
    uint8_t msg[msgLen] = {0};

	CL_NULL_DataRequest(9, msg, msgLen, 1, 1000);
    </code>

  Remarks:
    The result of the request is returned in the confirm callback.
*/
void CL_NULL_DataRequest(uint16_t conHandle, uint8_t *data, uint16_t dataLen,
    uint8_t prio, uint32_t timeRef);

// *****************************************************************************
/* Function:
    void CL_NULL_PlmeResetRequest
    (
        uint16_t pch
    )

  Summary:
    Request a reset of the functional state of the PHY layer.

  Description:
    This routine is used to request a reset of the functional state of the PHY
    layer.

  Precondition:
    The CL_NULL_Initialize routine must have been called before.

  Parameters:
    pch     - Physical channel (v1.4)

  Returns:
    None.

  Example:
    <code>
	CL_NULL_PlmeResetRequest(16);
    </code>

  Remarks:
    The result of the request is returned in the confirm callback.
*/
void CL_NULL_PlmeResetRequest(uint16_t pch);

// *****************************************************************************
/* Function:
    void CL_NULL_PlmeSleepRequest
    (
        uint16_t pch
    )

  Summary:
    Request a suspension of all present activities of the PHY layer.

  Description:
    This routine is used to request a suspension of all present activities of
    the PHY layer.

  Precondition:
    The CL_NULL_Initialize routine must have been called before.

  Parameters:
    pch     - Physical channel (v1.4)

  Returns:
    None.

  Example:
    <code>
	CL_NULL_PlmeSleepRequest(16);
    </code>

  Remarks:
    The result of the request is returned in the confirm callback.
*/
void CL_NULL_PlmeSleepRequest(uint16_t pch);

// *****************************************************************************
/* Function:
    void CL_NULL_PlmeResumeRequest
    (
        uint16_t pch
    )

  Summary:
    Request to resume all suspended actitivities of the PHY layer.

  Description:
    This routine is used to request to resume all suspended activities of the
    PHY layer.

  Precondition:
    The CL_NULL_Initialize routine must have been called before.

  Parameters:
    pch     - Physical channel (v1.4)

  Returns:
    None.

  Example:
    <code>
	CL_NULL_PlmeResumeRequest(16);
    </code>

  Remarks:
    The result of the request is returned in the confirm callback.
*/
void CL_NULL_PlmeResumeRequest(uint16_t us_pch);

// *****************************************************************************
/* Function:
    void CL_NULL_PlmeTestModeRequest
    (
        uint8_t enable,
        uint8_t mode,
        uint8_t modulation,
        uint8_t pwrLevel,
        uint16_t pch
    )

  Summary:
    Request the PHY layer to enter the given test mode.

  Description:
    This routine is used to request the PHY layer to enter the given test mode.

  Precondition:
    The CL_NULL_Initialize routine must have been called before.

  Parameters:
    enable      - Start/Stop test mode
    mode        - Transmission mode
    modulation  - Transmission modulation
    pwrLevel    - Transmission power level
    pch         - Physical channel (v1.4)

  Returns:
    None.

  Example:
    <code>
	CL_NULL_PlmeTestModeRequest(0, PAL_MODE_TYPE_B, PAL_PLC_DBPSK_R, 10, 16);
    </code>

  Remarks:
    The result of the request is returned in the confirm callback.
*/
void CL_NULL_PlmeTestModeRequest(uint8_t enable, uint8_t mode, uint8_t modulation,
    uint8_t pwrLevel, uint16_t pch);

// *****************************************************************************
/* Function:
    void CL_NULL_PlmeGetRequest
    (
        uint16_t pibAttrib,
        uint16_t pch
    )

  Summary:
    Request information about a given PIB attribute of the PHY layer.

  Description:
    This routine is used to request information about a given PIB attribute of
    the PHY layer.

  Precondition:
    The CL_NULL_Initialize routine must have been called before.

  Parameters:
    pibAttribute    - PIB attribute
    pch             - Physical channel (v1.4)

  Returns:
    None.

  Example:
    <code>
	CL_NULL_PlmeGetRequest(PIB_PHY_STATS_RX_TOTAL_COUNT, 16);
    </code>

  Remarks:
    The result of the request is returned in the confirm callback.
*/
void CL_NULL_PlmeGetRequest(uint16_t pibAttrib, uint16_t pch);

// *****************************************************************************
/* Function:
    void CL_NULL_PlmeSetRequest
    (
        uint16_t pibAttrib,
        void *pibValue,
        uint8_t pibSize,
        uint16_t pch
    )

  Summary:
    Set a new value for a given PIB attribute of the PHY layer.

  Description:
    This routine is used to set a new value for a given PIB attribute of
    the PHY layer.

  Precondition:
    The CL_NULL_Initialize routine must have been called before.

  Parameters:
    pibAttribute    - PIB attribute
    pibValue        - PIB attribute value
    pibSize         - PIB attribute value size
    pch             - Physical channel (v1.4)

  Returns:
    None.

  Example:
    <code>
    uint8_t snifferEn = 1;
	CL_NULL_PlmeSetRequest(PIB_PHY_SNIFFER_ENABLED, &snifferEn, sizeof(snifferEn), 16);
    </code>

  Remarks:
    The result of the request is returned in the confirm callback.
*/
void CL_NULL_PlmeSetRequest(uint16_t pibAttrib, void *pibValue, uint8_t pibSize,
    uint16_t pch);


// *****************************************************************************
/* Function:
    void CL_NULL_MlmePromoteRequest
    (
        uint8_t *eui48,
        uint8_t bcnMode
    )

  Summary:
    Request to trigger the promotion process in a Service Node that is in a
    Terminal functional state.

  Description:
    This routine is used to request to trigger the promotion process in a
    Service Node that is in a Terminal functional state.

  Precondition:
    The CL_NULL_Initialize routine must have been called before.

  Parameters:
    eui48           - Pointer to the address of the node to be promoted (NULL in
                      Service Node)
    bcnMode         - Beacon PDU modulation scheme

  Returns:
    None.

  Example:
    <code>
    uint8_t eui48[6];
    memset(eui48, 0x12, 6);

	CL_NULL_MlmePromoteRequest(eui48, 0);
    </code>

  Remarks:
    The result of the request is returned in the confirm callback.
*/
void CL_NULL_MlmePromoteRequest(uint8_t *eui48, uint8_t bcnMode);

// *****************************************************************************
/* Function:
    void CL_NULL_MlmeMpPromoteRequest
    (
        uint8_t *eui48,
        uint8_t bcnMode,
        uint16_t pch
    )

  Summary:
    Request to trigger the promotion process in a Service Node (Terminal or
    Switch) in a medium (PLC or RF) different from the one the node is connected
    to the network.

  Description:
    This routine is used to request to trigger the promotion process in a
    Service Node (Terminal or Switch) in a medium (PLC or RF) different from the
    one the node is connected to the network. This primitive only applies in
    PRIME v1.4.

  Precondition:
    The CL_NULL_Initialize routine must have been called before.

  Parameters:
    eui48           - Pointer to the address of the node to be promoted (NULL in
                      Service Node)
    bcnMode         - Beacon PDU modulation scheme
    pch             - Physical channel

  Returns:
    None.

  Example:
    <code>
    uint8_t eui48[6];
    memset(eui48, 0x12, 6);

	CL_NULL_MlmeMpPromoteRequest(eui48, 0, 16);
    </code>

  Remarks:
    The result of the request is returned in the confirm callback.
*/
void CL_NULL_MlmeMpPromoteRequest(uint8_t *eui48, uint8_t bcnMode, uint16_t pch);

// *****************************************************************************
/* Function:
    void CL_NULL_MlmeResetRequest
    (
        void
    )

  Summary:
    Request the flushing of all transmit and receive buffers and the resetting
    of all state variables.

  Description:
    This routine is used to request the flushing of all transmit and receive
    buffers and the resetting of all state variables. As a result, a Service
    Node will transit from its present functional state to the Disconnected
    functional state.

  Precondition:
    The CL_NULL_Initialize routine must have been called before.

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
	CL_NULL_MlmeResetRequest();
    </code>

  Remarks:
    The result of the request is returned in the confirm callback.
*/
void CL_NULL_MlmeResetRequest(void);

// *****************************************************************************
/* Function:
    void CL_NULL_MlmeGetRequest
    (
        uint16_t pibAttrib
    )

  Summary:
    Request information about a given PIB attribute of the MAC layer.

  Description:
    This routine is used to request information about a given PIB attribute of
    the MAC layer.

  Precondition:
    The CL_NULL_Initialize routine must have been called before.

  Parameters:
    pibAttribute    - PIB attribute

  Returns:
    None.

  Example:
    <code>
	CL_NULL_MlmeGetRequest(PIB_MAC_LNID);
    </code>

  Remarks:
    The result of the request is returned in the confirm callback.
*/
void CL_NULL_MlmeGetRequest(uint16_t pibAttrib);

// *****************************************************************************
/* Function:
    void CL_NULL_MlmeListGetRequest
    (
        uint16_t pibAttrib
    )

  Summary:
    Request information about a given PIB list attribute of the MAC layer.

  Description:
    This routine is used to request information about a given PIB list attribute
    of the MAC layer.

  Precondition:
    The CL_NULL_Initialize routine must have been called before.

  Parameters:
    pibAttribute    - PIB attribute

  Returns:
    None.

  Example:
    <code>
	CL_NULL_MlmeListGetRequest(PIB_MAC_LIST_AVAIL_SWITCHES);
    </code>

  Remarks:
    The result of the request is returned in the confirm callback.
*/
void CL_NULL_MlmeListGetRequest(uint16_t pibAttrib);

// *****************************************************************************
/* Function:
    void CL_NULL_MlmeSetRequest
    (
        uint16_t pibAttrib,
        void *pibValue,
        uint8_t pibSize
    )

  Summary:
    Set a new value for a given PIB attribute of the MAC layer.

  Description:
    This routine is used to set a new value for a given PIB attribute of
    the MAC layer.

  Precondition:
    The CL_NULL_Initialize routine must have been called before.

  Parameters:
    pibAttribute    - PIB attribute
    pibValue        - PIB attribute value
    pibSize         - PIB attribute value size

  Returns:
    None.

  Example:
    <code>
    uint8_t bandSearchTime = 25;
	CL_NULL_MlmeSetRequest(PIB_MAC_MIN_BAND_SEARCH_TIME, &bandSearchTime,
        sizeof(bandSearchTime));
    </code>

  Remarks:
    The result of the request is returned in the confirm callback.
*/
void CL_NULL_MlmeSetRequest(uint16_t pibAttrib, void *pibValue, uint8_t pibSize);

//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif /* CL_NULL_API_H */

/*******************************************************************************
 End of File
*/
