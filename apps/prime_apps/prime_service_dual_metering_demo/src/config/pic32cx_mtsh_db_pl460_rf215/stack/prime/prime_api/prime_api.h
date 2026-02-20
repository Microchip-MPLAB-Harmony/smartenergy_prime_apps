/*******************************************************************************
  PRIME API Header

  Company:
    Microchip Technology Inc.

  File Name:
    prime_api.h

  Summary:
    PRIME API Header File

  Description:
    This module converts the PRIME Library interface into a global
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
    PRIME_API_STATE_WAITING_OPEN,
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
    void PRIME_API_GetPrime13API(const PRIME_API **pPrimeApi)

  Summary:
    Gets the PRIME API location for PRIME Library v1.3.

  Description:
    This routine gets the PRIME API location for PRIME Library v1.3 in a dual
    application.

  Precondition:
    None.

  Parameters:
    pPrimeApi  - Pointer to PRIME Library v1.3

  Returns:
    None.

  Example:
    <code>
    const PRIME_API *gPrimeApi;
    
    SRV_STORAGE_PRIME_MODE_INFO_CONFIG boardInfo;
    SRV_STORAGE_GetConfigInfo(SRV_STORAGE_TYPE_MODE_PRIME, (uint8_t)sizeof(boardInfo),
                              (void *)&boardInfo);

    switch (boardInfo.primeVersion)
    {
        case PRIME_VERSION_1_3:
            PRIME_API_GetPrime13API(&gPrimeApi);
            break;

        case PRIME_VERSION_1_4:
        default:
            PRIME_API_GetPrime14API(&gPrimeApi);
            break;
    }
    
    if (gPrimeApi->Status() == SYS_STATUS_READY)
    {
        ...
    }
    </code>

  Remarks:
    None.
*/
void PRIME_API_GetPrime13API(const PRIME_API **pPrimeApi);

// *****************************************************************************
/* Function:
    void PRIME_API_GetPrime14API(const PRIME_API **pPrimeApi)

  Summary:
    Gets the PRIME API location for PRIME Library v1.4.

  Description:
    This routine gets the PRIME API location for PRIME Library v1.4 in a dual
    application.

  Precondition:
    None.

  Parameters:
    pPrimeApi  - Pointer to PRIME Library v1.4

  Returns:
    None.

  Example:
    <code>
    const PRIME_API *gPrimeApi;
    
    SRV_STORAGE_PRIME_MODE_INFO_CONFIG boardInfo;
    SRV_STORAGE_GetConfigInfo(SRV_STORAGE_TYPE_MODE_PRIME, (uint8_t)sizeof(boardInfo),
                              (void *)&boardInfo);

    switch (boardInfo.primeVersion)
    {
        case PRIME_VERSION_1_3:
            PRIME_API_GetPrime13API(&gPrimeApi);
            break;

        case PRIME_VERSION_1_4:
        default:
            PRIME_API_GetPrime14API(&gPrimeApi);
            break;
    }
    
    if (gPrimeApi->Status() == SYS_STATUS_READY)
    {
        ...
    }
    </code>

  Remarks:
    None.
*/
void PRIME_API_GetPrime14API(const PRIME_API **pPrimeApi);


//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif /* PRIME_API_H_INCLUDE */

/*******************************************************************************
 End of File
*/
