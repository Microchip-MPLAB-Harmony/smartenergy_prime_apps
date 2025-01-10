/*******************************************************************************
  PRIME API Definitions Header

  Company:
    Microchip Technology Inc.

  File Name:
    prime_api_defs.h

  Summary:
    PRIME API Definitions Header File

  Description:
    This file contains definitions of the PRIME API functions to be used by the
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

#ifndef PRIME_API_DEFS_H_INCLUDE
#define PRIME_API_DEFS_H_INCLUDE

// *****************************************************************************
// *****************************************************************************
// Section: File includes
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include "../hal_api/hal_api.h"

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
/* PRIME API Init Structure

   Summary:
    Initialization data for PRIME API to be provided on Initialize routine.

   Description:
    Defines application data required in the PRIME stack.

   Remarks:
    None.
*/
typedef struct
{
    /* HAL API pointer */
    const HAL_API *halApi;
    /* PAL index from configuration */
    uint8_t palIndex;
    /* USI port for Management Plane */
    uint8_t mngPlaneUsiPort;
} PRIME_API_INIT;

// *****************************************************************************
/* PRIME stack initialization

  Summary:
    Function pointer to initialize the PRIME stack.

  Description:
    This function pointer is used to initialize the PRIME stack.

  Remarks:
    None.
*/
typedef void (*PRIME_API_INITIALIZE)(PRIME_API_INIT *init);

// *****************************************************************************
/* PRIME stack state machine maintenance

  Summary:
    Function pointer to maintain the PRIME stack state machine.

  Description:
    This function pointer is used to maintain the PRIME stack internal state
    machine and generate callbacks.

  Remarks:
    None.
*/
typedef void (*PRIME_API_TASKS)(void);

// *****************************************************************************
/* PRIME stack status

  Summary:
    Function pointer to get the current status of the PRIME stack.

  Description:
    This function pointer is used to get the current status of the PRIME stack.

  Remarks:
    None.
*/
typedef SYS_STATUS (*PRIME_API_STATUS)(void);

//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif /* PRIME_API_DEFS_H_INCLUDE */

/*******************************************************************************
 End of File
*/
