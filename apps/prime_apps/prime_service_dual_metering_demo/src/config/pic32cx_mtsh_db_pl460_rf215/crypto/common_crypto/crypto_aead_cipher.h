/*******************************************************************************
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    crypto_aead_cipher.h

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

#ifndef CRYPTO_AEAD_CIPHER_H
#define CRYPTO_AEAD_CIPHER_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
#include "crypto_common.h"

typedef enum
{
    CRYPTO_AEAD_ERROR_CIPNOTSUPPTD = -127,
    CRYPTO_AEAD_ERROR_CTX = -126,
    CRYPTO_AEAD_ERROR_KEY = -125,
    CRYPTO_AEAD_ERROR_HDLR = -124,
    CRYPTO_AEAD_ERROR_INPUTDATA = -123,
    CRYPTO_AEAD_ERROR_OUTPUTDATA = -122,
    CRYPTO_AEAD_ERROR_NONCE = -121,
    CRYPTO_AEAD_ERROR_AUTHTAG = -120,
    CRYPTO_AEAD_ERROR_AAD = -119,
    CRYPTO_AEAD_ERROR_CIPOPER = -118,
    CRYPTO_AEAD_ERROR_SID = -117,
    CRYPTO_AEAD_ERROR_ARG = -116,
    CRYPTO_AEAD_ERROR_CIPFAIL = -115,
    CRYPTO_AEAD_ERROR_AUTHFAIL = -114,
    CRYPTO_AEAD_CIPHER_SUCCESS = 0,
}crypto_Aead_Status_E;

// *****************************************************************************
// *****************************************************************************
// Section: Type Definitions
// *****************************************************************************
// *****************************************************************************
typedef struct
{
    uint32_t cryptoSessionID;
    crypto_HandlerType_E aeadHandlerType_en;
    uint8_t *ptr_key;
    uint32_t aeadKeySize;
    uint8_t arr_aeadDataCtx[512]__attribute__((aligned (4)));
}st_Crypto_Aead_AesCcm_ctx;
// *****************************************************************************

crypto_Aead_Status_E Crypto_Aead_AesCcm_Init(st_Crypto_Aead_AesCcm_ctx *ptr_aesCcmCtx_st, crypto_HandlerType_E handlerType_en, 
                                              uint8_t *ptr_key, uint32_t keyLen, uint32_t sessionID);

crypto_Aead_Status_E Crypto_Aead_AesCcm_Cipher(st_Crypto_Aead_AesCcm_ctx *ptr_aesCcmCtx_st, crypto_CipherOper_E cipherOper_en, uint8_t *ptr_inputData, uint32_t dataLen, 
                                                    uint8_t *ptr_outData, uint8_t *ptr_nonce, uint32_t nonceLen, uint8_t *ptr_authTag,
                                                    uint32_t authTagLen, uint8_t *ptr_aad, uint32_t aadLen);

#endif //CRYPTO_AEAD_CIPHER_H
