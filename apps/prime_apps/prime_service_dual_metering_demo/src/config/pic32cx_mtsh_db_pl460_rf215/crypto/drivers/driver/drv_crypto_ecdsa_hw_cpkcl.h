/**************************************************************************
  Crypto Framework Library Header

  Company:
    Microchip Technology Inc.

  File Name:
    drv_crypto_ecdsa_hw_cpkcl.h
  
  Summary:
    Crypto Framework Library header for the CPKCC ECDSA functions.

  Description:
    This header contains the function definitions for the CPKCC ECDSA functions.
**************************************************************************/

/*******************************************************************************
* Copyright (C) 2025 Microchip Technology Inc. and its subsidiaries.
*
* Subject to your compliance with these terms, you may use Microchip software
* and any derivatives exclusively with Microchip products. It is your
* responsibility to comply with third party license terms applicable to your
* use of third party software (including open source software) that may
* accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
* EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
* WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
* PARTICULAR PURPOSE.
*
* IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
* INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
* WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
* BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
* FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
* ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
*******************************************************************************/

#ifndef DRV_CRYPTO_ECDSA_HW_CPKCL_H
#define DRV_CRYPTO_ECDSA_HW_CPKCL_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include "crypto/drivers/cpkcl_lib/cryptolib_typedef_pb.h"

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

typedef enum
{
    CRYPTO_ECDSA_RESULT_SUCCESS, 
    CRYPTO_ECDSA_RESULT_INIT_FAIL,
    CRYPTO_ECDSA_ERROR_PUBKEYCOMPRESS,
    CRYPTO_ECDSA_RESULT_ERROR_CURVE, 
    CRYPTO_ECDSA_RESULT_ERROR_RNG,        
    CRYPTO_ECDSA_RESULT_ERROR_FAIL
} CRYPTO_ECDSA_RESULT;

// *****************************************************************************
// *****************************************************************************
// Section: CPKCC ECDSA Common Interface 
// *****************************************************************************
// *****************************************************************************

CRYPTO_ECDSA_RESULT DRV_CRYPTO_ECDSA_InitEccParamsSign(CPKCL_ECC_DATA *pEccData, 
    pfu1 hash, u4 hashLen, pfu1 privKey, u4 privKeyLen, 
    CRYPTO_CPKCL_CURVE eccCurveType);

/* The signature is in MSB mode with the following order: */
/* 4 "0" bytes - S - 4 "0" bytes - R */
CRYPTO_ECDSA_RESULT DRV_CRYPTO_ECDSA_Sign(CPKCL_ECC_DATA *pEccData, 
    pfu1 pfulSignature, u4 signatureLen);
                                          
CRYPTO_ECDSA_RESULT DRV_CRYPTO_ECDSA_InitEccParamsVerify(CPKCL_ECC_DATA *pEccData, 
    pfu1 hash, u4 hashLen, pfu1 pubKey, CRYPTO_CPKCL_CURVE eccCurveType);

CRYPTO_ECDSA_RESULT DRV_CRYPTO_ECDSA_Verify(CPKCL_ECC_DATA *pEccData, 
    pfu1 pfu1Signature);

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    }

#endif
// DOM-IGNORE-END

#endif /* DRV_CRYPTO_ECDSA_HW_CPKCL_H */
