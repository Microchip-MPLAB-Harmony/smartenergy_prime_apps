/*******************************************************************************
  PRIME 4.32 Convergence Sublayer Control Interface Header

  Company:
    Microchip Technology Inc.

  File Name:
    cl_432.h

  Summary:
    PRIME 4.32 Convergence Sublayer Control Interface Header File

  Description:
    This file contains definitions of the control functions to be used by the 
    PRIME stack when managing the PRIME 4.32 Convergence Sublayer.
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

#ifndef CL_432_H_INCLUDE
#define CL_432_H_INCLUDE

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    extern "C" {

#endif
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: PRIME 4.32 Convergence Sublayer Control Interface Routines
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Function:
    void CL_432_Initialize(void)

  Summary:
    Initializes the PRIME 4.32 Convergence Sublayer.

  Description:
    This routine initializes the PRIME 4.32 Convergence Sublayer.

  Precondition:
    None.

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    CL_432_Initialize();
    </code>

  Remarks:
    This routine is normally not called directly by an application. It is 
    called by the PRIME stack initalization routine.
*/
void CL_432_Initialize(void);

//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif /* CL_432_H_INCLUDE */

/*******************************************************************************
 End of File
*/
