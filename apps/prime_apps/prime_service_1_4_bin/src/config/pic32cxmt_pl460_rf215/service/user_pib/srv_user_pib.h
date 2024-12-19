/*******************************************************************************
  PRIME User PIBs Service Interface Header File

  Company:
    Microchip Technology Inc.

  File Name:
    srv_user_pib.h

  Summary:
    PRIME User PIBs Service Interface Header File.

  Description:
    The User PIBs service provides a simple interface to handle a parameter
    interface base defined by the user from the PRIME stack. This file provides
    the interface definition for this service.
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

#ifndef SRV_USER_PIB_H    // Guards against multiple inclusion
#define SRV_USER_PIB_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include <stdbool.h>

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

// *****************************************************************************
/* Default Parameter Interface Base Attributes

 Summary:
    Default user PIB attributes.

 Description:
    The default user PIB attributes include attributes to handle reset and register
    contents for debug.

 Remarks:
    None.
*/

/* Reset information: number of resets in the two higher bytes, reset type in two lower bytes */
#define PIB_USER_RESET_INFO                     0xF000U
/* Program counter */
#define PIB_USER_PC                             0xF001U
/* Link register */
#define PIB_USER_LR                             0xF002U
/* Program status register */
#define PIB_USER_PSR                            0xF003U
/* Hard fault status register */
#define PIB_USER_HFSR                           0xF004U
/* Configurable full status register */
#define PIB_USER_CFSR                           0xF005U
/* Core register set */
#define PIB_USER_R0                             0xF006U
#define PIB_USER_R1                             0xF007U
#define PIB_USER_R2                             0xF008U
#define PIB_USER_R3                             0xF009U
#define PIB_USER_R12                            0xF00AU

// *****************************************************************************
// *****************************************************************************
// Section: Data Types
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* User PIB Service Get Request Handler Function Pointer

   Summary
    Pointer to a User PIB Get Request callback function.

   Description
    This data type defines a pointer to a User PIB Get Request callback function,
    thus defining the function signature. Clients of the User PIB service can
    register a callback function to get the requested user PIB with
    SRV_USER_PIB_GetRequestCbRegister function. That function will be called
    back when the PRIME stack returns the requested user PIB.

  Parameters:
    getResult     - Result of the operation
    pibAttrib     - PIB attribute identifier
    pibValue      - PIB attribute value
    pibSize       - PIB attribute value size

  Returns:
    None.

  Example:
    <code>
    static void APP_GetRequestHandler(uint8_t getResult, uint16_t pibAttrib,
                                void *pibValue, uint8_t pibSize)
    {
        if (getResult == true)
        {
            ...
        }
    }
    </code>

  Remarks:
    None.
*/

typedef void (*SRV_USER_PIB_GET_REQUEST_CALLBACK)(uint8_t getResult, uint16_t pibAttrib,
                                                  void *pibValue, uint8_t pibSize);

// *****************************************************************************
/* User PIB Service Set Request Handler Function Pointer

   Summary
    Pointer to a User PIB Set Request callback function.

   Description
    This data type defines a pointer to a User PIB Set Request callback function,
    thus defining the function signature. Clients of the User PIB service can
    register a callback function to get the result of settting user PIB with
    SRV_USER_PIB_SetRequestCbRegister function. That function will be called
    back when the PRIME stack returns the results of setting a user PIB.

  Parameters:
    setResult        - Result of the operation

  Returns:
    None.

  Example:
    <code>
    static void APP_SetRequestHandler(uint8_t setResult)
    {
        if (setResult == true)
        {
            ...
        }
    }
    </code>

  Remarks:
    None.
*/

typedef void (*SRV_USER_PIB_SET_REQUEST_CALLBACK)(uint8_t setResult);

// *****************************************************************************
// *****************************************************************************
// Section: User PIBs Service Interface Definition
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Function:
    void SRV_USER_PIB_Initialize(void)

  Summary:
    Initializes the User PIBs service.

  Description:
    This routine initializes the PRIME User PIBs service.

  Precondition:
    None.

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    int main()
    {
        SRV_USER_PIB_Initialize();
    }
    </code>

  Remarks:
    This routine must be called before any other PRIME User PIBs service routine.
    This function is normally not called directly by an application. It is called
    by the system's initialize routine (SYS_Initialize).
*/

void SRV_USER_PIB_Initialize(void);

// *****************************************************************************
/* Function:
    void SRV_USER_PIB_GetRequestCbRegister(
        SRV_USER_PIB_GET_REQUEST_CALLBACK callback)

  Summary:
    Register a function to be called to get the requested user PIB.

  Description:
    This routine allows a client to register an event handling function to be
    called when a requested user PIB is returned.

  Precondition:
    The SRV_USER_PIB_Initialize routine must have been called before.

  Parameters:
    callback      - Pointer to the callback function

  Returns:
    None.

  Example:
    <code>
    static void APP_GetRequestHandler(uint8_t getResult, uint16_t pibAttrib,
                                      void *pibValue, uint8_t pibSize)
    {
        if (getResult == true)
        {
            ...
        }
    }

    int main(void)
    {
        SRV_USER_PIB_Initialize;

        SRV_USER_PIB_GetRequestCbRegister(APP_GetRequestHandler);
    }
    </code>

  Remarks:
    None.
*/

void SRV_USER_PIB_GetRequestCbRegister(SRV_USER_PIB_GET_REQUEST_CALLBACK callback);

// *****************************************************************************
/* Function:
    void SRV_USER_PIB_GetRequest(uint16_t pibAttrib)

  Summary:
    Get a user PIB.

  Description:
    This routine is used to get a user PIB.

  Precondition:
    The SRV_USER_PIB_Initialize and the SRV_USER_PIB_GetRequestCbRegister
    routines must have been called before.

  Parameters:
    pibAttrib     - PIB attribute identifier

  Returns:
    None.

  Example:
    <code>
    static void APP_GetRequestHandler(uint8_t result, uint16_t pibAttrib,
                                      void *pibValue, uint8_t pibSize)
    {
        if (getResult == true)
        {
            ...
        }
    }

    int main(void)
    {
        SRV_USER_PIB_Initialize;

        SRV_USER_PIB_GetRequestCbRegister(APP_GetRequestHandler);

        SRV_USER_PIB_GetRequest(PIB_USER_RESET_INFO);
    }
    </code>

  Remarks:
    The requester user PIB is returned in a callback function.
*/

void SRV_USER_PIB_GetRequest(uint16_t pibAttrib);

// *****************************************************************************
/* Function:
    void SRV_USER_PIB_SetRequestCbRegister(
        SRV_USER_PIB_GET_REQUEST_CALLBACK callback)

  Summary:
    Register a function to be called to get the result of setting a user PIB.

  Description:
    This routine allows a client to register an event handling function to be
    called with the result of setting a user PIB.

  Precondition:
    The SRV_USER_PIB_Initialize routine must have been called before.

  Parameters:
    callback      - Pointer to the callback function

  Returns:
    None.

  Example:
    <code>
    static void APP_SetRequestHandler(uint8_t setResult)
    {
        if (setResult == true)
        {
            ...
        }
    }

    int main(void)
    {
        SRV_USER_PIB_Initialize;

        SRV_USER_PIB_SetRequestCbRegister(APP_SetRequestHandler);
    }
    </code>

  Remarks:
    None.
*/

void SRV_USER_PIB_SetRequestCbRegister(SRV_USER_PIB_SET_REQUEST_CALLBACK callback);

// *****************************************************************************
/* Function:
    void SRV_USER_PIB_SetRequest(
        uint16_t pibAttrib,
        void *pibValue,
        uint8_t pibSize)

  Summary:
    Set a user PIB.

  Description:
    This routine is used to set a user PIB.

  Precondition:
    The SRV_USER_PIB_Initialize and the SRV_USER_PIB_SetRequestCbRegister
    routines must have been called before.

  Parameters:
    pibAttrib     - PIB attribute identifier
    pibValue      - PIB attribute value
    pibSize       - PIB attribute value size

  Returns:
    None.

  Example:
    <code>
    static void APP_SetRequestHandler(uint8_t setResult)
    {
        if (setResult == true)
        {
            ...
        }
    }


    int main(void)
    {
        uint32_t resetValue = 0;

        SRV_USER_PIB_Initialize;

        SRV_USER_PIB_SetRequestCbRegister(APP_SetRequestHandler);

        SRV_USER_PIB_SetRequest(PIB_USER_RESET_INFO, &resetValue, sizeof(resetValue));
    }
    </code>

  Remarks:
    The result of setting the user PIB is returned in a callback function.
*/

void SRV_USER_PIB_SetRequest(uint16_t pibAttrib, void *pibValue, uint8_t pibSize);

#endif //SRV_USER_PIB_H
