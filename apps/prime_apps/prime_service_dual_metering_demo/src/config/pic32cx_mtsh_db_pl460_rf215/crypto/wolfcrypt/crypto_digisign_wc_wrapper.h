/*******************************************************************************
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    crypto_digisign_wc_wrapper.h

  Summary:
    This header file provides prototypes and definitions for the application.

  Description:
    This header file provides function prototypes and data type definitions for
    the application.  Some of these are required by the system (such as the
    "APP_Initialize" and "APP_Tasks" prototypes) and some of them are only used
    internally by the application (such as the "APP_STATES" definition).  Both
    are defined here for convenience.
*******************************************************************************/

/*******************************************************************************
* Copyright (C) 2026 Microchip Technology Inc. and its subsidiaries.
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

#ifndef CRYPTO_DIGISIGN_WC_WRAPPER_H
#define CRYPTO_DIGISIGN_WC_WRAPPER_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
crypto_DigiSign_Status_E Crypto_DigiSign_Wc_Ecdsa_SignHash(uint8_t *ptr_wcInputHash, uint32_t wcHashLen, uint8_t *ptr_wcSig, uint32_t wcSigLen, uint8_t *ptr_wcPrivKey, 
                                                       uint32_t wcPrivKeyLen, crypto_EccCurveType_E wcEccCurveType_en);

crypto_DigiSign_Status_E Crypto_DigiSign_Wc_Ecdsa_VerifyHash(uint8_t *ptr_wcInputHash, uint32_t wcHashLen, uint8_t *ptr_wcInputSig, uint32_t wcSigLen, uint8_t *ptr_wcPubKey, uint32_t wcPubKeyLen, 
                                                        int8_t *ptr_wcHashVerifyStat, crypto_EccCurveType_E wcEccCurveType_en);

crypto_DigiSign_Status_E Crypto_DigiSign_Wc_Ecdsa_SignData(uint8_t *ptr_wcInputData, uint32_t wcDataLen, uint8_t *ptr_wcSig, uint32_t wcSigLen, 
                                                            uint8_t *ptr_wcPrivKey, uint32_t wcPrivKeyLen, crypto_Hash_Algo_E hashType_en,                                                            
                                                            crypto_EccCurveType_E wcEccCurveType_en);

crypto_DigiSign_Status_E Crypto_DigiSign_Wc_Ecdsa_VerifyData(uint8_t *ptr_wcInputData, uint32_t wcDataLen, uint8_t *ptr_wcSig, uint32_t wcSigLen, 
                                                            uint8_t *ptr_wcPubKey, uint32_t wcPubKeyLen, crypto_Hash_Algo_E hashType_en,
                                                            int8_t *ptr_wcHashVerifyStat, crypto_EccCurveType_E wcEccCurveType_en);




#endif /* CRYPTO_DIGISIGN_WC_WRAPPER_H */
