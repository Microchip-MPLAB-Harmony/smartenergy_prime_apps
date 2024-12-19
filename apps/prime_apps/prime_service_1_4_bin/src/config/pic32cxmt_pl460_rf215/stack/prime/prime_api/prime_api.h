/*******************************************************************************
  PRIME API Header

  Company:
    Microchip Technology Inc.

  File Name:
    prime_api.h

  Summary:
    PRIME API Header File

  Description:
    This module converts the PRIME stack library interface into a global
    interface to be used by the PRIME application.
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

#ifndef PRIME_API_H_INCLUDE
#define PRIME_API_H_INCLUDE

// *****************************************************************************
// *****************************************************************************
// Section: Include Files
// *****************************************************************************
// *****************************************************************************

#include "system/system.h"
#include "prime_api_defs.h"
#include "prime_api_types.h"

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

#define PRIME_FW_VENDOR          "MCHP"
#define PRIME_FW_MODEL           "PIC32CXXPL460"
#define PRIME_FW_VERSION         "HS14.01.01"
#define PRIME_PIB_VENDOR         0
#define PRIME_PIB_MODEL          0x3941


// *****************************************************************************
// *****************************************************************************
// Section: Data Types
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* PRIME API state

  Summary:
    List of possible values of PRIME API state.

  Description:
    This type defines the possible PRIME API states.

  Remarks:
    None.
*/

typedef enum
{
    PRIME_API_STATE_PAL_INITIALIZING,
    PRIME_API_STATE_PRIME_RUNNING
} PRIME_API_STATE;

// *****************************************************************************
// *****************************************************************************
// Section: PRIME API Control Interface Routines
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Function:
    void PRIME_API_Initialize(PRIME_API_INIT *init)

  Summary:
    Initializes the PRIME stack.

  Description:
    This routine initializes the PRIME stack.

  Precondition:
    None.

  Parameters:
    init  - Pointer to the init data structure containing any data necessary to
            initialize the module.

  Returns:
    None.

  Example:
    <code>
    PRIME_API_INIT init;

    init.palIndex = PRIME_PAL_INDEX;
    init.halApi = (HAL_API*)&halApi;
    init.mngPlaneUsiPort = PRIME_MNG_PLANE_USI_INDEX;

    PRIME_API_Initialize((&init);
    </code>

  Remarks:
    This routine is normally not called directly by an application. The
    PRIME application must use the function located in the header table.
*/
void PRIME_API_Initialize(PRIME_API_INIT *init);

// ****************************************************************************
/* Function:
    void PRIME_API_Tasks(void)

  Summary:
    Maintains the PRIME stack state machine.

  Description:
    This function is used to maintain the PRIME stack internal state machine and
    generate callbacks.

  Precondition:
    The PRIME_API_Initialize function should have been called before calling this
    function.

  Parameters:
    None

  Returns:
    None

  Example:
    <code>
    while (true)
    {
        PRIME_API_Tasks();
    }
    </code>

  Remarks:
    This routine is normally not called directly by an application. The
    PRIME application must use the function located in the header table
    and call it periodically.
*/
void PRIME_API_Tasks(void);

// *****************************************************************************
/* Function:
    SYS_STATUS PRIME_API_Status(void)

  Summary:
    Gets the status of the PRIME stack.

  Description:
    This routine gets the status of the PRIME stack.

  Precondition:
    The PRIME_API_Initialize function should have been called before calling this
    function.

  Parameters:
    None.

  Returns:
    SYS_STATUS_READY: Indicates that the PRIME stack is ready and accepts
    requests for new operations.

    SYS_STATUS_UNINITIALIZED: Indicates the PRIME stack is not initialized.

    SYS_STATUS_ERROR: Indicates the PRIME stack is not initialized correctly.

    SYS_STATUS_BUSY: Indicates the PRIME stack is initializing.

  Example:
    <code>
    PRIME_API_INIT init;

    init.palIndex = PRIME_PAL_INDEX;
    init.halApi = (HAL_API*)&halApi;
    init.mngPlaneUsiPort = PRIME_MNG_PLANE_USI_INDEX;

    PRIME_API_Initialize((&init);

    if (PRIME_API_Status() == SYS_STATUS_READY)
    {

    }
    </code>

  Remarks:
    This routine is normally not called directly by an application. The
    PRIME application must use the function located in the header table.
*/
SYS_STATUS PRIME_API_Status(void);


//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif /* PRIME_API_H_INCLUDE */

/*******************************************************************************
 End of File
*/
