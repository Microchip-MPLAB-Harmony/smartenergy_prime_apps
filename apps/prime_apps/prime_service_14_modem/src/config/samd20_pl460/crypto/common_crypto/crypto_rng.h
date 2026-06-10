/*******************************************************************************
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    crypto_rng.h

  Summary:
    This header file provides prototypes and definitions for the application.

  Description:
    This header file provides function prototypes and data type definitions for
    the application.  Some of these are required by the system (such as the
    "APP_Initialize" and "APP_Tasks" prototypes) and some of them are only used
    internally by the application (such as the "APP_STATES" definition).  Both
    are defined here for convenience.
*******************************************************************************/

//DOM-IGNORE-BEGIN
/*
Copyright (C) 2026 Microchip Technology Inc., and its subsidiaries. All rights reserved.

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

#ifndef CRYPTO_RNG_H
#define CRYPTO_RNG_H

#include "crypto_common.h"

typedef enum
{
    CRYPTO_RNG_ERROR_NOTSUPPTED = -127,
    CRYPTO_RNG_ERROR_SEED       = -126,
    CRYPTO_RNG_ERROR_NONCE      = -125,
    CRYPTO_RNG_ERROR_ARG        = -124,
    CRYPTO_RNG_ERROR_SID        = -123,
    CRYPTO_RNG_ERROR_HDLR       = -122,
    CRYPTO_RNG_ERROR_FAIL       = -121,
    CRYPTO_RNG_SUCCESS = 0,   
}crypto_Rng_Status_E;

crypto_Rng_Status_E Crypto_Rng_Generate(crypto_HandlerType_E rngHandlerType_en, uint8_t* ptr_rngData, uint32_t rngLen, uint8_t* ptr_nonce, uint32_t nonceLen, uint32_t sessionID);

#endif /* CRYPTO_RNG_H */
