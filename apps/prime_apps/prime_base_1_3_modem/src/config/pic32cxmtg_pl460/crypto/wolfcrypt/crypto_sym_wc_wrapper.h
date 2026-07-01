/*******************************************************************************
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    crypto_sym_wc_wrapper.h

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

#ifndef CRYPTO_SYM_WC_WRAPPER_H
#define CRYPTO_SYM_WC_WRAPPER_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************


// *****************************************************************************
// *****************************************************************************
// Section: Type Definitions
// *****************************************************************************
crypto_Sym_Status_E Crypto_Sym_Wc_Aes_Init(void *ptr_aesCtx, crypto_CipherOper_E symCipherOper_en, crypto_Sym_OpModes_E symAlgoMode_en, uint8_t *ptr_key, uint32_t keySize, uint8_t *ptr_initVect);
    
crypto_Sym_Status_E Crypto_Sym_Wc_Aes_Encrypt(void *ptr_aesCtx, crypto_Sym_OpModes_E symAlgoMode_en, uint8_t *ptr_inputData, uint32_t dataLen, uint8_t *ptr_outData);
crypto_Sym_Status_E Crypto_Sym_Wc_Aes_Decrypt(void *ptr_aesCtx, crypto_Sym_OpModes_E symAlgoMode_en, uint8_t *ptr_inputData, uint32_t dataLen, uint8_t *ptr_outData);
    
crypto_Sym_Status_E Crypto_Sym_Wc_Aes_EncryptDirect(crypto_Sym_OpModes_E symAlgoMode_en, uint8_t *ptr_inputData, uint32_t dataLen, uint8_t *ptr_outData,
                                                        uint8_t *ptr_key, uint32_t keySize, uint8_t *ptr_initVect);
crypto_Sym_Status_E Crypto_Sym_Wc_Aes_DecryptDirect(crypto_Sym_OpModes_E symAlgoMode_en, uint8_t *ptr_inputData, uint32_t dataLen, uint8_t *ptr_outData,
                                                        uint8_t *ptr_key, uint32_t keySize, uint8_t *ptr_initVect);

//AES-KEYWRAP
crypto_Sym_Status_E Crypto_Sym_Wc_AesKeyWrap_Init(void *ptr_aesCtx, crypto_CipherOper_E symCipherOper_en, uint8_t *ptr_key, uint32_t keySize, uint8_t *ptr_initVect);
crypto_Sym_Status_E Crypto_Sym_Wc_AesKeyWrap(void *ptr_aesCtx, uint8_t *ptr_inputData, uint32_t inputLen, uint8_t *ptr_outData, uint32_t outputLen, uint8_t *ptr_initVect);
crypto_Sym_Status_E Crypto_Sym_Wc_AesKeyUnWrap(void *ptr_aesCtx, uint8_t *ptr_inputData, uint32_t inputLen, uint8_t *ptr_outData, uint32_t outputLen, uint8_t *ptr_initVect);
crypto_Sym_Status_E Crypto_Sym_Wc_AesKeyWrapDirect(uint8_t *ptr_inputData, uint32_t inputLen, uint8_t *ptr_outData, uint32_t outputLen,
                                                                        uint8_t *ptr_key, uint32_t keySize, uint8_t *ptr_initVect);
crypto_Sym_Status_E Crypto_Sym_Wc_AesKeyUnWrapDirect(uint8_t *ptr_inputData, uint32_t inputLen, uint8_t *ptr_outData, uint32_t outputLen,
                                                                        uint8_t *ptr_key, uint32_t keySize, uint8_t *ptr_initVect);

#endif //CRYPTO_SYM_WC_WRAPPER_H