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

#include "stack/prime/prime_api/prime_api_types.h"

// *****************************************************************************
// *****************************************************************************
// Section: Data Type Definitions
// *****************************************************************************
// *****************************************************************************

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
    /* Pointer to the PRIME API */
    PRIME_API *primeApi;

} PRIME_OBJ;

#endif //#ifndef PRIME_STACK_LOCAL_H
