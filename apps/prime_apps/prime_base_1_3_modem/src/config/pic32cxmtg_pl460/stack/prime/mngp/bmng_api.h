/*******************************************************************************
  PRIME Base Management API Header

  Company:
    Microchip Technology Inc.

  File Name:
    bmng_api.h

  Summary:
    PRIME Base Management API Header File

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

#ifndef BMNG_API_H_
#define BMNG_API_H_

// *****************************************************************************
// *****************************************************************************
// Section: File includes
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include "bmng_defs.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    extern "C" {

#endif
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: PRIME Base Management Interface Primitives
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Function:
    void BMNG_SetCallbacks
    (
        BMNG_CALLBACKS *bmngCallbacks
    )

  Summary:
    Sets the callbacks to the PRIME Base Management.

  Description:
    This routine sets the callbacks to the PRIME Base Management.
    
  Precondition:
    None.

  Parameters:
    bmngCallbacks    - Pointer to the Base Management callback structure

  Returns:
    None.

  Example:
    <code>
    BMNG_CALLBACKS bmngCallbacks;
    
    memset(bmngCallbacks, NULL, sizeof(bmngCallbacks);

	bmngCallbacks.network_event_ind = netEventInd;
	bmngCallbacks.pprof_ack = pprofACk;
	bmngCallbacks.pprof_get_response = pprofGetResp;

	BMNG_SetCallbacks(&bmngCallbacks);
    </code>

  Remarks:
    Unused callbacks must be set to NULL.
*/
void BMNG_SetCallbacks(BMNG_CALLBACKS *bmngCallbacks);

// *****************************************************************************
/* Function:
    void BMNG_FUP_StartFuRequest
    (
        uint8_t cmd,
        uint8_t enable
    )

  Summary:
    Requests to start a firmware upgrade process.

  Description:
    This routine requests the start of a firmware upgrade process.
    
  Precondition:
    None.

  Parameters:
    cmd     - Command to acknowledge
    enable  - Enable (1) or disable (0) FU 

  Returns:
    None.

  Example:
    <code>
    BMNG_FUP_StartFuRequest(FUP_START_FU_REQUEST, 1);
    </code>

  Remarks:
    The command is acknowledged with the FUP ACK callback.
*/
void BMNG_FUP_StartFuRequest(uint8_t cmd, uint8_t enable);

// *****************************************************************************
/* Function:
    void BMNG_FUP_ClearTargetListRequest
    (
        uint8_t cmd
    )

  Summary:
    Requests to clear the target list in a firmware upgrade process.

  Description:
    This routine requests to clear the target list in a firmware upgrade process.
    
  Precondition:
    The firmware upgrade process must have been started before. 

  Parameters:
    cmd     - Command to acknowledge

  Returns:
    None.

  Example:
    <code>
    BMNG_FUP_ClearTargetListRequest(FUP_CLEAR_TARGET_LIST_REQUEST);
    </code>

  Remarks:
    The command is acknowledged with the FUP ACK callback.
*/
void BMNG_FUP_ClearTargetListRequest(uint8_t cmd);

// *****************************************************************************
/* Function:
    void BMNG_FUP_AddTargetRequest
    (
        uint8_t cmd
        uint8_t *eui48
    )

  Summary:
    Requests to add a target node for a firmware upgrade process.

  Description:
    This routine requests to add a target node for a firmware upgrade process.
    
  Precondition:
    None. 

  Parameters:
    cmd     - Command to acknowledge
    eui48   - Pointer to the address of the node to be added

  Returns:
    None.

  Example:
    <code>
    uint8_t eui48[6];
    memset(eui48, 0x12, 6);
    
    BMNG_FUP_AddTargetRequest(FUP_CLEAR_ADD_TARGET_REQUEST, eui48);
    </code>

  Remarks:
    The command is acknowledged with the FUP ACK callback.
*/
void BMNG_FUP_AddTargetRequest(uint8_t cmd, uint8_t *eui48);

// *****************************************************************************
/* Function:
    void BMNG_FUP_SetFwDataRequest
    (
        uint8_t cmd,
        uint8_t vendorLen, 
        char *vendor, 
        uint8_t modelLen, 
        char *model, 
        uint8_t versionLen, 
        char *version
    )

  Summary:
    Requests to set the firmware data for a firmware upgrade process.

  Description:
    This routine requests to set the firmware data for a firmware upgrade process.
    
  Precondition:
    The firmware upgrade process must have been started before. 

  Parameters:
    cmd         - Command to acknowledge
    vendorLen   - Vendor length
    vendor      - Pointer to the vendor identification
    modelLen    - Model length
    model       - Pointer to the model identification
    versionLen  - Version length
    version     - Pointer to the version identification

  Returns:
    None.

  Example:
    <code>
    char vendor[] = "MCHP";
    char model[] = "PIC32CXXPL460";
    char version = "HS14.01.01\0\0\0\0\0\0";
    
    BMNG_FUP_SetFwDataRequest(FUP_SET_FW_DATA_REQUEST, sizeof(vendor), &vendor, 
        sizeof(model), &model, sizeof(version), &version);
    </code>

  Remarks:
    The command is acknowledged with the FUP ACK callback.
*/
void BMNG_FUP_SetFwDataRequest(uint8_t cmd, uint8_t vendorLen, char *vendor, 
    uint8_t modelLen, char *model, uint8_t versionLen, char *version);

// *****************************************************************************
/* Function:
    void BMNG_FUP_SetUpgradeOptionsRequest
    (
        uint8_t cmd,
        uint8_t arqEn, 
        BMNG_FUP_PAGE_SIZE pageSize, 
        uint8_t multicastEn, 
        uint32_t delayRestart, 
        uint32_t safetyTimer
    )

  Summary:
    Requests to set the upgrade options for a firmware upgrade process.

  Description:
    This routine requests to set the upgrade options for a firmware upgrade 
    process.
    
  Precondition:
    The firmware upgrade process must have been started before. 

  Parameters:
    cmd             - Command to acknowledge
    arqEn           - Enable (1) or disable (0) ARQ protocol
    pageSize        - Page size
    multicastEn     - Enable (1) or disable (0) multicast
    delayRestart    - Delay restart time in seconds
    safetyTimer     - Safety timer in seconds

  Returns:
    None.

  Example:
    <code>
    BMNG_FUP_SetUpgradeOptionsRequest(FUP_SET_UPG_OPTIONS_REQUEST, 0, 
        BMNG_FUP_PAGE_SIZE_192, 1, 60, 200);
    </code>

  Remarks:
    The command is acknowledged with the FUP ACK callback.
*/
void BMNG_FUP_SetUpgradeOptionsRequest(uint8_t cmd, uint8_t arqEn, 
    BMNG_FUP_PAGE_SIZE pageSize, uint8_t multicastEn, uint32_t delayRestart, 
    uint32_t safetyTimer);

// *****************************************************************************
/* Function:
    void BMNG_FUP_InitFileTxRequest
    (
        uint8_t cmd,
        uint16_t frameNumber, 
        uint32_t fileSize, 
        uint16_t frameSize, 
        uint32_t crc
    )

  Summary:
    Requests to initialize the file transmission for a firmware upgrade process.

  Description:
    This routine requests to initialize the file transmission for a firmware 
    upgrade process.
    
  Precondition:
    The firmware upgrade process must have been started before. 

  Parameters:
    cmd             - Command to acknowledge
    frameNumber     - Frame number (0x0000)
    fileSize        - File size
    frameSize       - Frame size
    crc             - File CRC-32

  Returns:
    None.

  Example:
    <code>
    BMNG_FUP_InitFileTxRequest(FUP_INIT_FILE_TX_REQUEST, 0x0000, 0x4000, 0x200, 
        0x34567421AABB0066);
    </code>

  Remarks:
    The command is acknowledged with the FUP ACK callback.
*/
void BMNG_FUP_InitFileTxRequest(uint8_t cmd, uint16_t frameNumber, 
    uint32_t fileSize, uint16_t frameSize, uint32_t crc);

// *****************************************************************************
/* Function:
    void BMNG_FUP_DataFrameRequest
    (
        uint8_t cmd,
        uint16_t frameNumber, 
        uint16_t dataLen, 
        uint8_t *data
    )

  Summary:
    Requests to receive a data frame during the file transmission for a firmware 
    upgrade process.

  Description:
    This routine requests to receive a data frame during the file transmission 
    for a firmware upgrade process.
    
  Precondition:
    The firmware upgrade process must have been started before. 

  Parameters:
    cmd             - Command to acknowledge
    frameNumber     - Frame number
    dataLen         - Data length
    data            - Pointer to the data frame

  Returns:
    None.

  Example:
    <code>
    uint8_t file[0x4000];
    uint8_t frame[0x200];
    memcpy(frame, &file[0x600], 0x200);
    
    BMNG_FUP_DataFrameRequest(FUP_DATA_FRAME_REQUEST, 0x600, 0x200, &frame);
    </code>

  Remarks:
    The command is acknowledged with the FUP ACK callback.
*/
void BMNG_FUP_DataFrameRequest(uint8_t cmd, uint16_t frameNumber, 
    uint16_t dataLen, uint8_t *data);

// *****************************************************************************
/* Function:
    void BMNG_FUP_CheckCrcRequest
    (
        uint8_t cmd
    )

  Summary:
    Requests to check the CRC of the transmitted file for a firmware upgrade 
    process.

  Description:
    This routine requests to check the CRC of the transmitted file for a 
    firmware upgrade process.
    
  Precondition:
    The firmware upgrade process must have been started before. 

  Parameters:
    cmd             - Command to acknowledge

  Returns:
    None.

  Example:
    <code>
    uint8_t file[0x4000];
    uint8_t frame[0x200];
    memcpy(frame, &file[0x600], 0x200);
    
    BMNG_FUP_CheckCrcRequest(FUP_CHECK_CRC_REQUEST);
    </code>

  Remarks:
    The command is acknowledged with the FUP ACK callback.
*/
void BMNG_FUP_CheckCrcRequest(uint8_t cmd);

// *****************************************************************************
/* Function:
    void BMNG_FUP_AbortFuRequest
    (
        uint8_t cmd,
        uint8_t *eui48
    )

  Summary:
    Requests to abort an ongoing firmware upgrade process.

  Description:
    This routine requests to abort an ongoing firmware upgrade process.
    
  Precondition:
    The firmware upgrade process must have been started before. 

  Parameters:
    cmd     - Command to acknowledge
    eui48   - Pointer to the address of the node to be aborted

  Returns:
    None.

  Example:
    <code>
    uint8_t eui48[6];
    memset(eui48, 0x12, 6);
    
    BMNG_FUP_AbortFuRequest(FUP_CLEAR_ADD_TARGET_REQUEST, eui48);
    </code>

  Remarks:
    If the MAC address is FF:FF:FF:FF:FF:FF, the FU is aborted for all nodes.
    The command is acknowledged with the FUP ACK callback.
*/
void BMNG_FUP_AbortFuRequest(uint8_t cmd, uint8_t *eui48);

// *****************************************************************************
/* Function:
    void BMNG_FUP_SetMatchRuleRequest
    (
        uint8_t cmd,
        uint8_t rules
    )

  Summary:
    Requests to set the matching rules for a firmware upgrade process.

  Description:
    This routine requests to set the matching rules for a firmware upgrade 
    process.
    
  Precondition:
    The firmware upgrade process must have been started before. 

  Parameters:
    cmd     - Command to acknowledge
    rules   - Match rules: 0000 0MV0

  Returns:
    None.

  Example:
    <code>
    BMNG_FUP_SetMatchRuleRequest(FUP_SET_MACTH_RULE_REQUEST, 6);
    </code>

  Remarks:
    If M and/or V are set, only the nodes matching model and/or vendor will be 
    upgraded.
    The command is acknowledged with the FUP ACK callback.
*/
void BMNG_FUP_SetMatchRuleRequest(uint8_t cmd, uint8_t rules);

// *****************************************************************************
/* Function:
    void BMNG_FUP_SetSignatureDataRequest
    (
        uint8_t cmd,
        uint8_t algorithm, 
        uint16_t length
    )

  Summary:
    Requests to set the signature data for a firmware upgrade process.

  Description:
    This routine requests to set the signature data for a firmware upgrade
    process.
    
  Precondition:
    The firmware upgrade process must have been started before. 

  Parameters:
    cmd         - Command to acknowledge
    algorithm   - Used algorithm to check signature
    length      - Signature length in bytes

  Returns:
    None.

  Example:
    <code>
    BMNG_FUP_SetSignatureDataRequest(FUP_SET_SIGNATURE_DATA_REQUEST, 2, 12);
    </code>

  Remarks:
    The command is acknowledged with the FUP ACK callback.
*/
void BMNG_FUP_SetSignatureDataRequest(uint8_t cmd, uint8_t algorithm, 
    uint16_t length);

// *****************************************************************************
/* Function:
    void BMNG_FUP_GetVersionRequest
    (
        uint8_t cmd,
        uint8_t *eui48
    )

  Summary:
    Requests to get the firmware version.

  Description:
    This routine requests to get the firmware version.
    
  Precondition:
    None. 

  Parameters:
    cmd     - Command to acknowledge
    eui48   - Pointer to the address of the node to be asked

  Returns:
    None.

  Example:
    <code>
    uint8_t eui48[6];
    memset(eui48, 0x12, 6);
    
    BMNG_FUP_GetVersionRequest(FUP_GET_VERSION_REQUEST, eui48);
    </code>

  Remarks:
    If the MAC address is FF:FF:FF:FF:FF:FF, the version information is asked to 
    all registered nodes.
    The command is acknowledged with the FUP ACK callback.
    The result of the request is returned in the FUP version indication callback.
*/
void BMNG_FUP_GetVersionRequest(uint8_t cmd, uint8_t *eui48);

// *****************************************************************************
/* Function:
    void BMNG_FUP_GetStateRequest
    (
        uint8_t cmd,
        uint8_t *eui48
    )

  Summary:
    Requests to get the firmware upgrade state.

  Description:
    This routine requests to get the firmware upgrade state.
    
  Precondition:
    None. 

  Parameters:
    cmd     - Command to acknowledge
    eui48   - Pointer to the address of the node to be asked

  Returns:
    None.

  Example:
    <code>
    uint8_t eui48[6];
    memset(eui48, 0x12, 6);
    
    BMNG_FUP_GetStateRequest(FUP_GET_STATE_REQUEST, eui48);
    </code>

  Remarks:
    If the MAC address is FF:FF:FF:FF:FF:FF, the state information is asked to 
    all registered nodes.
    The command is acknowledged with the FUP ACK callback.
    The result of the request is returned in the FUP state indication callback.
*/
void BMNG_FUP_GetStateRequest(uint8_t cmd, uint8_t *eui48);

// *****************************************************************************
/* Function:
    void BMNG_PPROF_GetRequest
    (
        uint8_t cmd,
        uint8_t *eui48,
        uint16_t dataLen, 
        uint8_t *data
    )

  Summary:
    Requests to get a PIB attribute from a node using the PRIME Profile.

  Description:
    This routine requests to get a PIB attribute from a node using the PRIME 
    Profile.
    
  Precondition:
    None. 

  Parameters:
    cmd     - Command to acknowledge
    eui48   - Pointer to the address of the node to be asked
    dataLen - Data length
    data    - Pointer to the data

  Returns:
    None.

  Example:
    <code>
    uint8_t data[3] = {0x00, 0x20, 0};
    uint8_t eui48[6];
    memset(eui48, 0x12, 6);
    
    BMNG_PPROF_GetRequest(PPROF_GET_REQUEST, eui48, sizeof(data), &data);
    </code>

  Remarks:
    The command is acknowledged with the PPROF ACK callback.
    The result of the request is returned in the PPROF response indication 
    callback.
*/
void BMNG_PPROF_GetRequest(uint8_t cmd, uint8_t *eui48, uint16_t dataLen, 
    uint8_t *data);

// *****************************************************************************
/* Function:
    void BMNG_PPROF_SetRequest
    (
        uint8_t cmd,
        uint8_t *eui48,
        uint16_t dataLen, 
        uint8_t *data
    )

  Summary:
    Requests to set a PIB attribute in a node using the PRIME Profile.

  Description:
    This routine requests to set a PIB attribute in a node using the PRIME 
    Profile.
    
  Precondition:
    None. 

  Parameters:
    cmd     - Command to acknowledge
    eui48   - Pointer to the address of the node to be asked
    dataLen - Data length
    data    - Pointer to the data

  Returns:
    None.

  Example:
    <code>
    uint8_t data[3] = {0x00, 0x1A, 25};
    uint8_t eui48[6];
    memset(eui48, 0x12, 6);
    
    BMNG_PPROF_SetRequest(PPROF_SET_REQUEST, eui48, sizeof(data), &data);
    </code>

  Remarks:
    The command is acknowledged with the PPROF ACK callback.
*/
void BMNG_PPROF_SetRequest(uint8_t cmd, uint8_t *eui48, uint16_t dataLen, 
    uint8_t *data);

// *****************************************************************************
/* Function:
    void BMNG_PPROF_ResetRequest
    (
        uint8_t cmd,
        uint8_t *eui48
    )

  Summary:
    Requests to reset all PIB statistics attributes of a node using the PRIME 
    Profile.

  Description:
    This routine requests to reset all PIB statistics attributes of a node 
    using the PRIME Profile.
    
  Precondition:
    None. 

  Parameters:
    cmd     - Command to acknowledge
    eui48   - Pointer to the address of the node to be reset

  Returns:
    None.

  Example:
    <code>
    uint8_t eui48[6];
    memset(eui48, 0x12, 6);
    
    BMNG_PPROF_ResetRequest(PPROF_RESET_REQUEST, eui48);
    </code>

  Remarks:
    The command is acknowledged with the PPROF ACK callback.
*/
void BMNG_PPROF_ResetRequest(uint8_t cmd, uint8_t *eui48);

// *****************************************************************************
/* Function:
    void BMNG_PPROF_RebootRequest
    (
        uint8_t cmd,
        uint8_t *eui48
    )

  Summary:
    Requests to reboot a node using the PRIME Profile.

  Description:
    This routine requests to reboot a node using the PRIME Profile.
    
  Precondition:
    None. 

  Parameters:
    cmd     - Command to acknowledge
    eui48   - Pointer to the address of the node to be rebooted

  Returns:
    None.

  Example:
    <code>
    uint8_t eui48[6];
    memset(eui48, 0x12, 6);
    
    BMNG_PPROF_RebootRequest(PPROF_REBOOT_REQUEST, eui48);
    </code>

  Remarks:
    The command is acknowledged with the PPROF ACK callback.
*/
void BMNG_PPROF_RebootRequest(uint8_t cmd, uint8_t *eui48);

// *****************************************************************************
/* Function:
    void BMNG_PPROF_GetEnhancedRequest
    (
        uint8_t cmd,
        uint8_t *eui48,
        uint16_t dataLen, 
        uint8_t *data
    )

  Summary:
    Requests to get a PIB attribute from a node using the PRIME Profile.

  Description:
    This routine requests to get a PIB attribute from a node using the PRIME 
    Profile.
    
  Precondition:
    None. 

  Parameters:
    cmd     - Command to acknowledge
    eui48   - Pointer to the address of the node to be asked
    dataLen - Data length
    data    - Pointer to the data

  Returns:
    None.

  Example:
    <code>
    uint8_t data[3] = {0x00, 0x20, 0};
    uint8_t eui48[6];
    memset(eui48, 0x12, 6);
    
    BMNG_PPROF_GetEnhancedRequest(PPROF_GET_ENHANCED_REQUEST, eui48, sizeof(data), 
        &data);
    </code>

  Remarks:
    The command is acknowledged with the PPROF ACK callback.
    The result of the request is returned in the PPROF enhanced response
    indication callback.
*/
void BMNG_PPROF_GetEnhancedRequest(uint8_t cmd, uint8_t *eui48, uint16_t dataLen, 
    uint8_t *data);

// *****************************************************************************
/* Function:
    void BMNG_PPROF_GetZcDiffRequest
    (
        uint8_t cmd,
        uint8_t *eui48
    )

  Summary:
    Requests to get the zero cross difference between BN and SN using the PRIME 
    Profile.

  Description:
    This routine requests to get the zero cross difference between BN and SN using 
    the PRIME Profile.
    
  Precondition:
    None. 

  Parameters:
    cmd     - Command to acknowledge
    eui48   - Pointer to the address of the node to be asked

  Returns:
    None.

  Example:
    <code>
    uint8_t data[3] = {0x00, 0x20, 0};
    uint8_t eui48[6];
    memset(eui48, 0x12, 6);
    
    BMNG_PPROF_GetZcDiffRequest(PPROF_GET_ZC_DIFF_REQUEST, eui48);
    </code>

  Remarks:
    The command is acknowledged with the PPROF ACK callback.
    The result of the request is returned in the PPROF zero cross difference 
    response indication callback.
*/
void BMNG_PPROF_GetZcDiffRequest(uint8_t cmd, uint8_t *eui48);

// *****************************************************************************
/* Function:
    void BMNG_WHITELIST_AddRequest
    (
        uint8_t cmd,
        uint8_t *eui48
    )

  Summary:
    Requests to add a node to the Whitelist.

  Description:
    This routine requests to add a node to the Whitelist.
    
  Precondition:
    None. 

  Parameters:
    cmd     - Command to acknowledge
    eui48   - Pointer to the address of the node to be added

  Returns:
    None.

  Example:
    <code>
    uint8_t data[3] = {0x00, 0x20, 0};
    uint8_t eui48[6];
    memset(eui48, 0x12, 6);
    
    BMNG_WHITELIST_AddRequest(PPROF_GET_ZC_DIFF_REQUEST, eui48);
    </code>

  Remarks:
    The command is acknowledged with the Whitelist ACK callback.
*/
void BMNG_WHITELIST_AddRequest(uint8_t cmd, uint8_t *eui48);

// *****************************************************************************
/* Function:
    void BMNG_WHITELIST_RemoveRequest
    (
        uint8_t cmd,
        uint8_t *eui48
    )

  Summary:
    Requests to remove a node from the Whitelist.

  Description:
    This routine requests to remove a node from the Whitelist.
    
  Precondition:
    None. 

  Parameters:
    cmd     - Command to acknowledge
    eui48   - Pointer to the address of the node to be removed

  Returns:
    None.

  Example:
    <code>
    uint8_t data[3] = {0x00, 0x20, 0};
    uint8_t eui48[6];
    memset(eui48, 0x12, 6);
    
    BMNG_WHITELIST_RemoveRequest(PPROF_GET_ZC_DIFF_REQUEST, eui48);
    </code>

  Remarks:
    The command is acknowledged with the Whitelist ACK callback.
*/
void BMNG_WHITELIST_RemoveRequest(uint8_t cmd, uint8_t *eui48);

//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif /* BMNG_API_H_ */

/*******************************************************************************
 End of File
*/
