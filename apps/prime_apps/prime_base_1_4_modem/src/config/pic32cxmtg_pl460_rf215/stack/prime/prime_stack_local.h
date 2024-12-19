/*******************************************************************************
  PRIME Stack Local Data Structures

  Company:
    Microchip Technology Inc.

  File Name:
    prime_stack_local.h

  Summary:
    PRIME Stack Local Data Structures

  Description:
    PRIME Stack Local Data Structures
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

#ifndef PRIME_STACK_LOCAL_H
#define PRIME_STACK_LOCAL_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "system/system.h"
#include "stack/prime/prime_api/prime_api_types.h"

// *****************************************************************************
// *****************************************************************************
// Section: Data Type Definitions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* PRIME Initialization Data

   Summary:
    Defines the data required to initialize the PRIME stack.

  Description:
    This data type defines the data required to initialize the PRIME stack and
    all of its components.

   Remarks:
    None.
*/
typedef struct
{
    /* PAL index from configuration */
    uint8_t palIndex;
    /* USI port for Management Plane */
    uint8_t mngPlaneUsiPort;
} PRIME_STACK_INIT;

/* PRIME Status

  Summary:
    Identifies the current status/state of the PRIME stack.

  Description:
    This enumeration identifies the current status/state of the PRIME stack.

  Remarks:
    This enumeration is the return type for the PRIME_Status routine. The
    upper layer must ensure that PRIME_Status returns PRIME_STATUS_READY
    before performing PRIME operations.
*/
typedef enum {
    PRIME_STATUS_UNINITIALIZED = SYS_STATUS_UNINITIALIZED,
    PRIME_STATUS_INITIALIZING = SYS_STATUS_BUSY,
    PRIME_STATUS_RUNNING = SYS_STATUS_READY,
    PRIME_STATUS_ERROR = SYS_STATUS_ERROR,
    PRIME_STATUS_POINTER_READY = SYS_STATUS_ERROR_EXTENDED - 1,
} PRIME_STATUS;

// *****************************************************************************
/* PRIME Stack Instance Object

  Summary:
    Object used to keep any data required to handle the PRIME stack.

  Description:
    Object used to keep any data required to handle the PRIME stack.

  Remarks:
    None.
*/

typedef struct
{
    /* State of this instance */
    PRIME_STATUS status;

    /* Pointer to the PRIME API */
    const PRIME_API *primeApi;

} PRIME_OBJ;

#endif //#ifndef PRIME_STACK_LOCAL_H
