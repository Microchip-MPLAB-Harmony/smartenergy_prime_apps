/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    crypto_sym_wc_wrapper.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It
    implements the logic of the application's state machine and it may call
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
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

#include "crypto/common_crypto/crypto_sym_cipher.h"
#include "crypto/wolfcrypt/crypto_sym_wc_wrapper.h"
#include "wolfssl/wolfcrypt/aes.h"
#include "wolfssl/wolfcrypt/error-crypt.h"

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// ***************************************************************************** 
static crypto_Sym_Status_E Crypto_Sym_Wc_Aes_Init(void *ptr_aesCtx, crypto_CipherOper_E symCipherOper_en, crypto_Sym_OpModes_E symAlgoMode_en, uint8_t *ptr_key, uint32_t keySize, uint8_t *ptr_initVect)
{
    crypto_Sym_Status_E ret_aesStatus_en = CRYPTO_SYM_ERROR_CIPNOTSUPPTD;
    int wcAesStatus = BAD_FUNC_ARG;
    int dir = -1;
    bool isKeystreamMode = false;
    if(ptr_aesCtx != NULL)
    {
        /* Keystream-style modes (CTR, OFB, CFB1/8/128) apply the AES forward
         * primitive to the IV/feedback/counter block in BOTH directions, so
         * wc_AesSetKey must always be called with AES_ENCRYPTION regardless
         * of the caller-supplied direction. Otherwise wolfCrypt expands the
         * inverse key schedule and decryption produces garbage from block 2
         * onwards (single-block CFB happens to work because only the IV
         * encryption applies). */
        if(    false
          )
        {
            isKeystreamMode = true;
        }

        if(isKeystreamMode)
        {
            dir = AES_ENCRYPTION;
        }
        else if(symCipherOper_en == CRYPTO_CIOP_ENCRYPT)
        {
            dir = AES_ENCRYPTION;
        }
        else if(symCipherOper_en == CRYPTO_CIOP_DECRYPT)
        {
            dir = AES_DECRYPTION;
        }
        else
        {
            ret_aesStatus_en = CRYPTO_SYM_ERROR_CIPOPER;
        }
        if(ret_aesStatus_en != CRYPTO_SYM_ERROR_CIPOPER)
        {
            wcAesStatus = wc_AesSetKey( (Aes*)ptr_aesCtx, (const byte*)ptr_key, (word32)keySize, ptr_initVect, dir);

            if(wcAesStatus == 0)
            {
                ret_aesStatus_en = CRYPTO_SYM_CIPHER_SUCCESS;
            }
            else if (wcAesStatus == WC_KEY_SIZE_E)
            {
                ret_aesStatus_en = CRYPTO_SYM_ERROR_KEY;
            }
            else if(wcAesStatus == BAD_FUNC_ARG)
            {
                ret_aesStatus_en = CRYPTO_SYM_ERROR_ARG;
            }
            else
            {
                ret_aesStatus_en  = CRYPTO_SYM_ERROR_CIPFAIL;
            }
        }
    }
    else
    {
        ret_aesStatus_en = CRYPTO_SYM_ERROR_CTX;
    }
    return ret_aesStatus_en;
}

//AES-KW
crypto_Sym_Status_E Crypto_Sym_Wc_AesKeyWrap_Init(void *ptr_aesCtx, crypto_CipherOper_E symCipherOper_en, uint8_t *ptr_key, uint32_t keySize, uint8_t *ptr_initVect)
{
    crypto_Sym_Status_E ret_aesKwStat_en = CRYPTO_SYM_ERROR_CIPNOTSUPPTD;
    /* KW is not part of the symAlgoMode_en enum; pass INVALID so the
     * wrapper falls through to the direction-based path (KW uses the
     * normal forward/inverse schedule depending on caller direction). */
    ret_aesKwStat_en = Crypto_Sym_Wc_Aes_Init( (Aes*)ptr_aesCtx, symCipherOper_en, CRYPTO_SYM_OPMODE_INVALID, ptr_key, keySize, ptr_initVect);
    return ret_aesKwStat_en;
}

crypto_Sym_Status_E Crypto_Sym_Wc_AesKeyWrap(void *ptr_aesCtx, uint8_t *ptr_inputData, uint32_t inputLen, uint8_t *ptr_outData, uint32_t outputLen, uint8_t *ptr_initVect)
{
    crypto_Sym_Status_E ret_aesKwStat_en = CRYPTO_SYM_ERROR_CIPNOTSUPPTD;
    int wcAesKwStat = -1;
    if(ptr_aesCtx == NULL)
    {
        ret_aesKwStat_en = CRYPTO_SYM_ERROR_CTX;
    }
    else if( (ptr_inputData == NULL) || (ptr_outData == NULL) || (inputLen == 0u) || (outputLen == 0u) )
    {
        ret_aesKwStat_en = CRYPTO_SYM_ERROR_ARG;
    }
    else
    {
        wcAesKwStat = wc_AesKeyWrap_ex( (Aes*)ptr_aesCtx, (const byte*)ptr_inputData, (word32)inputLen, (byte*)ptr_outData, (word32)outputLen, (const byte*)ptr_initVect);
   
        if(wcAesKwStat == (int)((int)inputLen + (int)KEYWRAP_BLOCK_SIZE) )
        {
            ret_aesKwStat_en = CRYPTO_SYM_CIPHER_SUCCESS;
        }
        else if(wcAesKwStat == BAD_FUNC_ARG)
        {
            ret_aesKwStat_en = CRYPTO_SYM_ERROR_ARG;
        }
        else
        {
            ret_aesKwStat_en = CRYPTO_SYM_ERROR_CIPFAIL;
        }
    }
    
    return ret_aesKwStat_en;
}

crypto_Sym_Status_E Crypto_Sym_Wc_AesKeyUnWrap(void *ptr_aesCtx, uint8_t *ptr_inputData, uint32_t inputLen, uint8_t *ptr_outData, uint32_t outputLen, uint8_t *ptr_initVect)
{  
    crypto_Sym_Status_E ret_aesKwStat_en = CRYPTO_SYM_ERROR_CIPNOTSUPPTD;
    int wcAesKwStat = -1;
    if(ptr_aesCtx == NULL)
    {
        ret_aesKwStat_en = CRYPTO_SYM_ERROR_CTX;
    }
    else if( (ptr_inputData == NULL) || (ptr_outData == NULL) || (inputLen == 0u) || (outputLen == 0u) || (outputLen < inputLen) )
    {
        ret_aesKwStat_en = CRYPTO_SYM_ERROR_ARG;
    }
    else
    {
        wcAesKwStat = wc_AesKeyUnWrap_ex( (Aes*)ptr_aesCtx, (const byte*)ptr_inputData, (word32)inputLen, (byte*)ptr_outData, (word32)outputLen, (const byte*)ptr_initVect);
   
        if(wcAesKwStat == (int)((int)inputLen - (int)KEYWRAP_BLOCK_SIZE) )
        {
            ret_aesKwStat_en = CRYPTO_SYM_CIPHER_SUCCESS;
        }
        else if(wcAesKwStat == BAD_FUNC_ARG)
        {
            ret_aesKwStat_en = CRYPTO_SYM_ERROR_ARG;
        }
        else
        {
            ret_aesKwStat_en = CRYPTO_SYM_ERROR_CIPFAIL;
        }
    }
    return ret_aesKwStat_en;
}

crypto_Sym_Status_E Crypto_Sym_Wc_AesKeyWrapDirect(uint8_t *ptr_inputData, uint32_t inputLen, uint8_t *ptr_outData, uint32_t outputLen,
                                                                        uint8_t *ptr_key, uint32_t keySize, uint8_t *ptr_initVect)
{
    crypto_Sym_Status_E ret_aesKwStat_en = CRYPTO_SYM_ERROR_CIPNOTSUPPTD;    
    int wcAesKwStat = -1;
    if( (ptr_inputData == NULL) || (inputLen < (uint32_t)((8Lu)*(2Lu))) || (ptr_outData == NULL) || (outputLen == 0u))
    {
        ret_aesKwStat_en = CRYPTO_SYM_ERROR_ARG;
    }
    else if( (ptr_key == NULL) && (keySize > 0u) )
    {
        ret_aesKwStat_en = CRYPTO_SYM_ERROR_KEY;
    }
    else
    {
        wcAesKwStat = wc_AesKeyWrap( (const byte*)ptr_key, (word32)keySize, (const byte*)ptr_inputData, (word32)inputLen, (byte*)ptr_outData, (word32)outputLen, (const byte*)ptr_initVect);
    
        if(wcAesKwStat == (int)((int)inputLen + (int)KEYWRAP_BLOCK_SIZE) )
        {
            ret_aesKwStat_en = CRYPTO_SYM_CIPHER_SUCCESS;
        }
        else if(wcAesKwStat == BAD_FUNC_ARG)
        {
            ret_aesKwStat_en = CRYPTO_SYM_ERROR_ARG;
        }
        else
        {
            ret_aesKwStat_en = CRYPTO_SYM_ERROR_CIPFAIL;
        }     
    }
    return ret_aesKwStat_en;
}

crypto_Sym_Status_E Crypto_Sym_Wc_AesKeyUnWrapDirect(uint8_t *ptr_inputData, uint32_t inputLen, uint8_t *ptr_outData, uint32_t outputLen,
                                                                        uint8_t *ptr_key, uint32_t keySize, uint8_t *ptr_initVect)
{
    crypto_Sym_Status_E ret_aesKwStat_en = CRYPTO_SYM_ERROR_CIPNOTSUPPTD;
    
    int wcAesKwStat = -1;
    if( (ptr_inputData == NULL) || (inputLen < (uint32_t)((8Lu)*(2Lu))) || (ptr_outData == NULL) || (outputLen == 0u))
    {
        ret_aesKwStat_en = CRYPTO_SYM_ERROR_ARG;
    }
    else if( (ptr_key == NULL) && (keySize > 0u) )
    {
        ret_aesKwStat_en = CRYPTO_SYM_ERROR_KEY;
    }
    else
    {
        wcAesKwStat = wc_AesKeyUnWrap( (const byte*)ptr_key, (word32)keySize, (const byte*)ptr_inputData, (word32)inputLen, (byte*)ptr_outData, (word32)outputLen, (const byte*)ptr_initVect);
    
        if(wcAesKwStat == (int)((int)inputLen - (int)KEYWRAP_BLOCK_SIZE) )
        {
            ret_aesKwStat_en = CRYPTO_SYM_CIPHER_SUCCESS;
        }
        else if(wcAesKwStat == BAD_FUNC_ARG)
        {
            ret_aesKwStat_en = CRYPTO_SYM_ERROR_ARG;
        }
        else
        {
            ret_aesKwStat_en = CRYPTO_SYM_ERROR_CIPFAIL;
        }
    }
    return ret_aesKwStat_en;
}
