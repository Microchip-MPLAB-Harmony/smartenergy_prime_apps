/*******************************************************************************
  Company:
    Microchip Technology Inc.

  File Name:
    pal_plc_rm.h

  Summary:
    Platform Abstraction Layer (PAL) RF Robust Management header.

  Description:
    This module provides handling of the robust management of the
    RF physical layer.
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

#ifndef PAL_RF_RM_H
#define PAL_RF_RM_H

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
// Section: PAL RF Robust Management Interface Functions
// *****************************************************************************
// *****************************************************************************

uint8_t PAL_RF_RM_GetLqi(int16_t rssi);
uint8_t PAL_RF_RM_GetLessRobustModulation(PAL_SCHEME mod1, PAL_SCHEME mod2);
bool PAL_RF_RM_CheckMinimumQuality(PAL_SCHEME reference, PAL_SCHEME modulation);
PAL_SCHEME PAL_RF_RM_GetScheme(void);
void PAL_RF_RM_SetScheme(PAL_SCHEME scheme);
void PAL_RF_RM_GetRobustModulation(void *indObj, uint16_t *pBitRate, 
                                   PAL_SCHEME *pModulation, uint16_t pch);

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    }

#endif
// DOM-IGNORE-END

#endif //PAL_RF_RM_H
