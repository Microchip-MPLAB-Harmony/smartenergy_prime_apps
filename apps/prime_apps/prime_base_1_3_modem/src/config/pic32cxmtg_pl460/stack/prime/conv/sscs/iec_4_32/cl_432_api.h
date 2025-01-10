/*******************************************************************************
  PRIME 4.32 Convergence Sublayer API Header

  Company:
    Microchip Technology Inc.

  File Name:
    cl_432_api.h

  Summary:
    PRIME 4-32 Convergence Sublayer API Header File

  Description:
    This file contains definitions of the PRIME 4-32 Convergence Sublayer
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

#ifndef CL_432_API_H_INCLUDE
#define CL_432_API_H_INCLUDE

// *****************************************************************************
// *****************************************************************************
// Section: File includes
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include "cl_432_defs.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    extern "C" {

#endif
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: PRIME 4-32 Convergence Sublayer Interface Primitives
// *****************************************************************************
// *****************************************************************************

// ****************************************************************************
/* Function:
    void CL_432_SetCallbacks(CL_432_CALLBACKS *cl432cbs)

  Summary:
    Sets 4-32 Convergence layer callback functions. 

  Description:
    This routine links callback functions between upper layer and the CL-432 
    Convergence layer.

  Precondition:
    The CL_432_Initialize function should have been called before calling this 
    function.

  Parameters:
    cl432cbs         Callbacks structure

  Returns:
    None

  Example:
    <code>
    static void _dl_data_ind((uint8_t dstLsap, uint8_t srcLsap, uint16_t dstAddress, 
        uint16_t srcAddress, uint8_t *data, uint16_t lsduLen, uint8_t link_Class)
    {
        ...
    }
    
    void main(void) 
    {
        CL_432_CALLBACKS cl432cbs;
    
        CL_432_Initialize();
    
        memset(cl432cbs, NULL, sizeof(cl432cbs));
        cl432cbs.cl_432_dl_data_ind = _dl_data_ind;

        CL_432_SetCallbacks(&cl432cbs);
    }
    </code>

  Remarks:
    None
*/
void CL_432_SetCallbacks(CL_432_CALLBACKS *cl432cbs);

// *****************************************************************************
/* Function:
    void CL_432_ReleaseRequest
    (
        uint16_t dstAddress
    )

  Summary:
    Request to release a CL-432 connection.

  Description:
    This routine is used to request a CL-432 connection release.
    
  Precondition:
    The CL_432_Initialize routine must have been called before.

  Parameters:
    dstAddress  - Address to disconnect

  Returns:
    None.

  Example:
    <code>
	CL_432_ReleaseRequest(300);
    </code>

  Remarks:
    The result of the request is returned in the confirm callback.
*/
void CL_432_ReleaseRequest(uint16_t dstAddress);

// *****************************************************************************
/* Function:
    void CL_432_DlDataRequest
    (
        uint8_t dstLsap, 
        uint8_t srcLsap, 
        uint16_t dstAddress, 
        DL_432_BUFFER *buff, 
        uint16_t lsduLen, 
        uint8_t linkClass
    )

  Summary:
    Request the transmission of data over a CL-432 connection.

  Description:
    This routine is used to request the transmission of data over a CL-432 connection.
    
  Precondition:
    The CL_432_Initialize routine must have been called before.

  Parameters:
    dstLsap     - Destination LSAP
    srcLsap     - Source LSAP
    dstAddress  - Destination 4-32 address
    buff        - Pointer to the data buffer
    lsduLen     - Length of the data
    linkClass   - Link class (not used)

  Returns:
    None.

  Example:
    <code>    
    uint16_t msgLen = 20;
    DL_432_BUFFER msg[msgLen] = {0};
    
    CL_432_DlDataRequest(2, 0, 300, msg, msgLen, 0);
    </code>

  Remarks:
    The result of the request is returned in the confirm callback.
*/
void CL_432_DlDataRequest(uint8_t dstLsap, uint8_t srcLsap, uint16_t dstAddress, 
    DL_432_BUFFER *buff, uint16_t lsduLen, uint8_t linkClass);


//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif /* CL_432_API_H_INCLUDE */

/*******************************************************************************
 End of File
*/
