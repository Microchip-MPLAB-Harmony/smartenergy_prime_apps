/*******************************************************************************
  PRIME Stack Header File

  Company:
    Microchip Technology Inc.

  File Name:
    prime_stack.h

  Summary:
    PRIME Stack Header File

  Description:
    This header file provides a simple interface to manage the PRIME Stack from
    the application.
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

#ifndef PRIME_STACK_H
#define PRIME_STACK_H

// *****************************************************************************
// *****************************************************************************
// Section: File includes
// *****************************************************************************
// *****************************************************************************

#include "system/system.h"
#include "prime_stack_local.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    extern "C" {

#endif
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: PRIME Stack Common Interface Routines
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Function:
    SYS_MODULE_OBJ PRIME_Initialize
    (
      const SYS_MODULE_INDEX index,
      const SYS_MODULE_INIT * init
    )

  Summary:
    Initializes the PRIME Stack for the specified index.

  Description:
    This routine initializes the PRIME Stack making it ready for clients to use.

  Precondition:
    None.

  Parameters:
    index - Identifier for the instance to be initialized (single instance allowed)

    init  - Pointer to the init data structure containing any data necessary to
            initialize the module.

  Returns:
    If successful, returns a valid module instance object.
    Otherwise, returns SYS_MODULE_OBJ_INVALID.

  Example:
    <code>
    PRIME_STACK_INIT initData;
    SYS_MODULE_OBJ sysObjPrime;

    sysObjPrime = PRIME_Initialize(PRIME_INDEX_0, (SYS_MODULE_INIT *)&initData);
    if (sysObjPrime == SYS_MODULE_OBJ_INVALID)
    {

    }
    </code>

  Remarks:
    This routine must be called before any other PRIME routine is called.
*/
SYS_MODULE_OBJ PRIME_Initialize(const SYS_MODULE_INDEX index,
    const SYS_MODULE_INIT * init);

// *****************************************************************************
/* Function:
    void PRIME_Tasks
    (
      SYS_MODULE_OBJ object
    )

  Summary:
    Maintains the PRIME Stack State Machine.

  Description:
    This routine maintains the PRIME Stack State Machine.

  Precondition:
    PRIME_Initialize routine must have been called before, and its returned
    object used when calling this function.

  Parameters:
    object - Identifier for the object instance

  Returns:
    None.

  Example:
    <code>
    PRIME_STACK_INIT initData;
    SYS_MODULE_OBJ sysObjPrime;

    sysObjPrime = PRIME_Initialize(PRIME_INDEX_0, (SYS_MODULE_INIT *)&initData);

    while (true)
    {
        PRIME_Tasks(sysObjPrime);
    }
    </code>

  Remarks:
    This routine must be called periodically.
*/
void PRIME_Tasks(SYS_MODULE_OBJ object);

// *****************************************************************************
/* Function:
    void PRIME_Restart
    (
      uint32_t *primePtr,
      uint8_t version
    )

  Summary:
    Restarts the PRIME Stack from a different location.

  Description:
    This routine restarts the PRIME Stack from a different location.

  Precondition:
    PRIME_Initialize routine must have been called before.

  Parameters:
    primePtr   - Pointer to the PRIME API
    version    - PRIME version to run

  Returns:
    None.

  Example:
    <code>
    PRIME_STACK_INIT initData;

    PRIME_Initialize(PRIME_INDEX_0, (SYS_MODULE_INIT *)&initData);

    const PRIME_API *primeApiPtr = PRIME_SN_FWSTACK13_ADDRESS;
    PRIME_Restart((uint32_t *)primeApiPtr, PRIME_VERSION_1_3);
    </code>

  Remarks:
    None.
*/
void PRIME_Restart(uint32_t *primePtr, uint8_t version);

// *****************************************************************************
/* Function:
    SYS_STATUS PRIME_Status(void)

  Summary:
    Gets the current status of the PRIME Stack.

  Description:
    This routine provides the current status of the PRIME Stack.

  Precondition:
    PRIME_Initialize routine must have been called before.

  Parameters:
    None.

  Returns:
    SYS_STATUS_READY: Indicates that the PRIME Stack is ready and accepts
    requests for new operations.

    SYS_STATUS_UNINITIALIZED: Indicates the PRIME Stack is not initialized.

    SYS_STATUS_ERROR: Indicates the PRIME Stack is not initialized correctly.

    SYS_STATUS_BUSY: Indicates the PRIME Stack is initializing.

  Example:
    <code>
    PRIME_STACK_INIT initData;

    PRIME_Initialize(PRIME_INDEX_0, (SYS_MODULE_INIT *)&initData);

    if (PRIME_Status() == SYS_STATUS_READY)
    {

    }
    </code>

  Remarks:
    None.
  */

SYS_STATUS PRIME_Status(void);

//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif // #ifndef PRIME_STACK_H
