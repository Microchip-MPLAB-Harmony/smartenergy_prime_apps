/*******************************************************************************
  PRIME 4-32 Convergence Sublayer Definitions Header

  Company:
    Microchip Technology Inc.

  File Name:
    cl_null_api.h

  Summary:
    PRIME 4-32 Convergence Sublayer Definitions Header File

  Description:
    This file contains definitions of the PRIME 4-32 Convergence Sublayer to be
    used by the PRIME application.
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

#ifndef CL_432_DEFS_H_INCLUDE
#define CL_432_DEFS_H_INCLUDE

// *****************************************************************************
// *****************************************************************************
// Section: File includes
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>

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

/* Length of the LPDU (Link Protocol Data Unit) */
#define CL_432_LPDU_HEADER                3

/* Maxixum length of the LSDU (Link Service Data Unit) */
/* (It must be smaller than PRIME_MACSAP_DATA_SIZE) */
#define CL_432_MAX_LENGTH_DATA            (1024 - CL_432_LPDU_HEADER)

/* Addresses defined in 4-32 layer */
#define CL_432_INVALID_ADDRESS            (0xFFFFU)
#define CL_432_BROADCAST_ADDRESS          (0x0FFFU)

// *****************************************************************************
/* Connection status macros

 Summary:
    4-32 connection status macros.

 Description:
    Macrod for the 4-32 connection status.

 Remarks:
    None.
*/
#define CL_432_CON_CLOSE              (0)
#define CL_432_CON_CONNECTING         (1)
#define CL_432_CON_DISCONNECTING      (2)
#define CL_432_CON_OPEN               (3)

// *****************************************************************************
// *****************************************************************************
// Section: Data Types
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Result values enumeration

 Summary:
    Enumeration of the result values for the 4-32 Convergence Layer primitives.

 Description:
    This enumeration contains the result values for the 4-32 Convergence Layer
    primitives.

 Remarks:
    None.
*/
typedef enum {
	DL_432_RESULT_SUCCESS = 0,
	DL_432_RESULT_REJECT = 1,
	DL_432_RESULT_TIMEOUT = 2,
	DL_432_RESULT_NOT_REGISTERED = 6
} DL_432_RESULT;

// *****************************************************************************
/* Transmission errors enumeration

 Summary:
    Enumeration of the transmission errors defined in the 4-32 Convergence Layer.

 Description:
    This enumeration contains the transmission errors defined in the 4-32
    Convergence Layer.

 Remarks:
    None.
*/
typedef enum {
	/* Standard errors */
	CL_432_TX_STATUS_SUCCESS                           = 0,
	CL_432_TX_STATUS_TIMEOUT                           = 2,
	/* Errors from the MAC layer */
	CL_432_TX_STATUS_ERROR_MAC_SENDING                 = 0x80,
	CL_432_TX_STATUS_ERROR_MAC_BUSY                    = 0x81,
	CL_432_TX_STATUS_ERROR_MAC_NO_FREE_BUFFERS         = 0x82,
	CL_432_TX_STATUS_ERROR_MAC_CON_CLOSED              = 0x83,
	CL_432_TX_STATUS_ERROR_MAC_RECEIVING_DATA          = 0x84,
	/* Errors from the CL 4-32 */
	CL_432_TX_STATUS_ERROR_BAD_ADDRESS                 = 0xC0,
	CL_432_TX_STATUS_ERROR_BAD_HANLDER                 = 0xC1,
	CL_432_TX_STATUS_ERROR_BUSY                        = 0xC2,
	CL_432_TX_STATUS_ERROR_BAD_DST_ADDRESS             = 0xC3
} DL_432_TX_STATUS;

// *****************************************************************************
/* Buffer for transmission / reception

 Summary:
    Data structure of the buffer for transmission and reception.

 Description:
    This structure contains the definition of the buffer used for tranmission and
    reception of messages in the 4-32 Convergence layer.

 Remarks:
    None.
*/
typedef union {
	uint8_t lpdu[CL_432_MAX_LENGTH_DATA + CL_432_LPDU_HEADER];

	struct {
		uint8_t control;
		uint8_t dsap;
		uint8_t lsap;
		uint8_t buff[CL_432_MAX_LENGTH_DATA];
	} dl;
} DL_432_BUFFER;

// *****************************************************************************
/* CL 4-32 Establish Request function pointer

 Summary:
    Function pointer for the CL 4-32 Establish Request primitive.

 Description:
    This data type contains the definition of the function pointer for the
    CL 4-32 Establish Request primitive.
    - deviceId: Pointer to the device identifier data
    - deviceIdLen: Length of the device identifier
    - ae (v1.4): Flag to indicate that authentication and encryption is requested

 Remarks:
    None.
*/
typedef void (*CL_432_ESTABLISH_REQUEST)(uint8_t *deviceId, uint8_t deviceIdLen,
    uint8_t ae);

// *****************************************************************************
/* CL 4-32 Establish Confirm callback function pointer

 Summary:
    Callback function pointer for the CL 4-32 Establish Confirm primitive.

 Description:
    This data type contains the definition of the callback function for the
    CL 4-32 Establish Confirm primitive.
    - deviceId: Pointer to the device identifier data
    - deviceIdLen: Length of the device identifier
    - dstAddress: Destination 4.32 address
    - baseAddress: Base 4-32 address
    - ae (v1.4): Flag to indicate that authentication and encryption is requested

 Remarks:
    None.
*/
typedef void (*CL_432_ESTABLISH_CONFIRM_CB)(uint8_t *deviceId, uint8_t deviceIdLen,
    uint16_t dstAddress, uint16_t baseAddress, uint8_t ae);

// *****************************************************************************
/* CL 4-32 Release Request function pointer

 Summary:
    Function pointer for the CL 4-32 Release Request primitive.

 Description:
    This data type contains the definition of the function pointer for the
    CL 4-32 Release Request primitive.
    - dstAddress: Address to disconnect

 Remarks:
    None.
*/
typedef void (*CL_432_RELEASE_REQUEST)(uint16_t dstAddress);

// *****************************************************************************
/* CL 4-32 Data Request function pointer

 Summary:
    Function pointer for the CL 4-32 Data Request primitive.

 Description:
    This data type contains the definition of the function pointer for the
    CL 4-32 Data Request primitive.
    - dstLsap: Destination LSAP
    - srcLsap: Source LSAP
    - dstAddress: Destination 4-32 address
    - buff: Pointer to the data buffer
    - lsduLen: Length of the data
    - linkClass: Link class (not used)

 Remarks:
    None.
*/
typedef void (*CL_432_DL_DATA_REQUEST)(uint8_t dstLsap, uint8_t srcLsap,
    uint16_t dstAddress, DL_432_BUFFER *buff, uint16_t lsduLen, uint8_t linkClass);

// *****************************************************************************
/* CL 4-32 Data Indication callback function pointer

 Summary:
    Callback function pointer for the CL 4-32 Data Indication primitive.

 Description:
    This data type contains the definition of the callback function for the
    CL 4-32 Data Indication primitive.
    - dstLsap: Destination LSAP
    - srcLsap: Source LSAP
    - dstAddress: Destination 4-32 address
    - srcAddress: Source 4-32 address
    - data: Pointer to received data
    - lsduLen: Length of the data
    - linkClass: Link class (not used)

 Remarks:
    None.
*/
typedef void (*CL_432_DL_DATA_INDICATION_CB)(uint8_t dstLsap, uint8_t srcLsap,
    uint16_t dstAddress, uint16_t srcAddress, uint8_t *data, uint16_t lsduLen,
    uint8_t link_Class);

// *****************************************************************************
/* CL 4-32 Data Confirm callback function pointer

 Summary:
    Callback function pointer for the CL 4-32 Data Confirm primitive.

 Description:
    This data type contains the definition of the callback function for the
    CL 4-32 Data Confirm primitive.
    - dstLsap: Destination LSAP
    - srcLsap: Source LSAP
    - dstAddress: Destination 4-32 address
    - txStatus: Transmission status

 Remarks:
    None.
*/
typedef void (*CL_432_DL_DATA_CONFIRM_CB)(uint8_t dstLsap, uint8_t srcLsap,
    uint16_t dstAddress, DL_432_TX_STATUS txStatus);


// *****************************************************************************
/* CL 4-32 Release Confirm callback function pointer

 Summary:
    Callback function pointer for the CL 4-32 Release Confirm primitive.

 Description:
    This data type contains the definition of the callback function for the
    CL 4-32 Release Confirm primitive.
    - dstAddress: Destination 4.32 address
    - result: Confirmation result

 Remarks:
    None.
*/
typedef void (*CL_432_RELEASE_CONFIRM_CB)(uint16_t dstAddress, DL_432_RESULT result);

// ****************************************************************************
/* CL-432 callback configuration

  Summary:
    Defines the callbacks to handle the CL-432 Convergence layer.

  Description:
    This structure defines the callbacks to handle the CL-432 Convergence layer.

  Remarks:
    None.
*/
typedef struct {
	CL_432_DL_DATA_INDICATION_CB cl_432_dl_data_ind;
	CL_432_DL_DATA_CONFIRM_CB cl_432_dl_data_cfm;
    CL_432_ESTABLISH_CONFIRM_CB cl_432_establish_cfm;
	CL_432_RELEASE_CONFIRM_CB cl_432_release_cfm;
} CL_432_CALLBACKS;

// ****************************************************************************
/* CL-432 callback function pointer

  Summary:
    Defines the funtion pointer to set the callbacks to handle the CL-432
    Convergence layer.

  Description:
    This data type defines the function pointer to set the callbacks to
    handle the CL-432 Convergence layer.

  Remarks:
    None.
*/
typedef void (*CL_432_SET_CALLBACKS)(CL_432_CALLBACKS *cl432cbs);

//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif /* CL_432_DEFS_H_INCLUDE */

/*******************************************************************************
 End of File
*/
