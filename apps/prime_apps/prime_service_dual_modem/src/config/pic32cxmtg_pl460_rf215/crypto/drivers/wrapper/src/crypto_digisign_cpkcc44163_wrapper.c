/**************************************************************************
  Crypto Framework Library Source

  Company:
    Microchip Technology Inc.

  File Name:
    crypto_digisign_cpkcc44163_wrapper.c

  Summary:
    Crypto Framework Library wrapper file for the digital signature in the 
    hardware cryptographic library.

  Description:
    This source file contains the wrapper interface to access the hardware 
    cryptographic library in Microchip microcontrollers for digital signature.
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

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include "crypto/drivers/wrapper/crypto_digisign_cpkcc44163_wrapper.h"
#include "crypto/drivers/driver/drv_crypto_ecc_hw_cpkcl.h"
#include "crypto/drivers/driver/drv_crypto_ecdsa_hw_cpkcl.h"

// *****************************************************************************
// *****************************************************************************
// Section: File scope functions
// *****************************************************************************
// *****************************************************************************

static crypto_DigiSign_Status_E lCrypto_DigSign_Ecdsa_Hw_GetCurve(
    crypto_EccCurveType_E eccCurveType, CRYPTO_CPKCL_CURVE *hwEccCurve)
{
    crypto_DigiSign_Status_E digiSigntatus = CRYPTO_DIGISIGN_SUCCESS;
    
    switch (eccCurveType)
    {
        case CRYPTO_ECC_CURVE_P192:
            *hwEccCurve = CRYPTO_CPKCL_CURVE_P192;
            break;
        
        case CRYPTO_ECC_CURVE_P224:
            *hwEccCurve = CRYPTO_CPKCL_CURVE_P224;
            break;
        
        case CRYPTO_ECC_CURVE_P256:
            *hwEccCurve = CRYPTO_CPKCL_CURVE_P256;
            break;
        
        case CRYPTO_ECC_CURVE_P384:
            *hwEccCurve = CRYPTO_CPKCL_CURVE_P384;
            break;
        
        case CRYPTO_ECC_CURVE_P521:
            *hwEccCurve = CRYPTO_CPKCL_CURVE_P521;
            break;
            
        default:
            digiSigntatus = CRYPTO_DIGISIGN_ERROR_CURVE;
            break;
    }
    
    return digiSigntatus;
}    

static crypto_DigiSign_Status_E lCrypto_DigSign_Ecdsa_Hw_MapResult(
    CRYPTO_ECDSA_RESULT result)
{
    crypto_DigiSign_Status_E digiSigntatus;
    
    switch (result) 
    {
        case CRYPTO_ECDSA_RESULT_SUCCESS:
            digiSigntatus = CRYPTO_DIGISIGN_SUCCESS;
            break;
            
        case CRYPTO_ECDSA_ERROR_PUBKEYCOMPRESS:
            digiSigntatus = CRYPTO_DIGISIGN_ERROR_PUBKEYCOMPRESS;
            break;
            
        case CRYPTO_ECDSA_RESULT_ERROR_CURVE:
            digiSigntatus = CRYPTO_DIGISIGN_ERROR_CURVE;
            break;
            
        case CRYPTO_ECDSA_RESULT_ERROR_RNG:   
            digiSigntatus = CRYPTO_DIGISIGN_ERROR_RNG;
            break;
        
        case CRYPTO_ECDSA_RESULT_INIT_FAIL:
        case CRYPTO_ECDSA_RESULT_ERROR_FAIL:
            digiSigntatus = CRYPTO_DIGISIGN_ERROR_FAIL;
            break;
            
        default:
            digiSigntatus = CRYPTO_DIGISIGN_ERROR_FAIL;
            break;
    }
    
    return digiSigntatus;
}

// *****************************************************************************
// *****************************************************************************
// Section: DigSign Common Interface Implementation
// *****************************************************************************
// *****************************************************************************

crypto_DigiSign_Status_E Crypto_DigiSign_Ecdsa_Hw_Sign(uint8_t *inputHash, 
    uint32_t hashLen, uint8_t *outSig, uint32_t sigLen, uint8_t *privKey, 
    uint32_t privKeyLen, crypto_EccCurveType_E eccCurveType_En)
{
    crypto_DigiSign_Status_E result;
    CRYPTO_ECDSA_RESULT hwResult;
    CPKCL_ECC_DATA eccData;
    CRYPTO_CPKCL_CURVE hwEccCurve;
    
    /* Get curve */
    result = lCrypto_DigSign_Ecdsa_Hw_GetCurve(eccCurveType_En, &hwEccCurve);
    if (result != CRYPTO_DIGISIGN_SUCCESS)
    {
        return result;
    }
    
    /* Initialize the hardware library for ECDSA signature generation */
    hwResult = DRV_CRYPTO_ECDSA_InitEccParamsSign(&eccData, 
                                                  (pfu1)inputHash,
                                                  (u4) hashLen, 
                                                  (pfu1)privKey, 
                                                  (u4) privKeyLen, 
                                                  hwEccCurve);
    if (hwResult != CRYPTO_ECDSA_RESULT_SUCCESS) 
    {
        return lCrypto_DigSign_Ecdsa_Hw_MapResult(hwResult);
        
    }
    
    /* Sign the message */
    hwResult = DRV_CRYPTO_ECDSA_Sign(&eccData, (pfu1)outSig, (u4)sigLen);

    return lCrypto_DigSign_Ecdsa_Hw_MapResult(hwResult);
}

crypto_DigiSign_Status_E Crypto_DigiSign_Ecdsa_Hw_Verify(uint8_t *inputHash, 
    uint32_t hashLen, uint8_t *inputSig, uint32_t sigLen, uint8_t *pubKey, 
    uint32_t pubKeyLen, int8_t *hashVerifyStatus, 
    crypto_EccCurveType_E eccCurveType_En)
{
    crypto_DigiSign_Status_E result;
    CRYPTO_ECDSA_RESULT hwResult;
    CRYPTO_CPKCL_CURVE hwEccCurve;
    CPKCL_ECC_DATA eccData = {0};
    
    /* Get curve */
    result = lCrypto_DigSign_Ecdsa_Hw_GetCurve(eccCurveType_En, &hwEccCurve);
    if (result != CRYPTO_DIGISIGN_SUCCESS)
    {
        return result;
    }
    
    /* Initialize the hardware library for ECDSA signature verification */
    hwResult = DRV_CRYPTO_ECDSA_InitEccParamsVerify(&eccData, 
                                                    (pfu1)inputHash,
                                                    (u4) hashLen, 
                                                    (pfu1)pubKey, 
                                                    hwEccCurve);
    
    /* Verify the signature */
    if (hwResult != CRYPTO_ECDSA_RESULT_SUCCESS)
    {
        hwResult = DRV_CRYPTO_ECDSA_Verify(&eccData, (pfu1)inputSig);
    }
    
    /* Set verification status */
    if (hwResult == CRYPTO_ECDSA_RESULT_SUCCESS) 
    {
        *hashVerifyStatus = 1;
    }
    else 
    {
        *hashVerifyStatus = 0;
    }

    return lCrypto_DigSign_Ecdsa_Hw_MapResult(hwResult);
}
