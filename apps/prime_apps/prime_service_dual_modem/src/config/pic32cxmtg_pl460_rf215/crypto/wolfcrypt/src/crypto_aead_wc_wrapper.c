/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    crypto_aead_wc_wrapper.c

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

 
// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
#include "crypto/common_crypto/crypto_common.h"
#include "crypto/common_crypto/crypto_aead_cipher.h"
#include "crypto/wolfcrypt/crypto_aead_wc_wrapper.h"
#include "wolfssl/wolfcrypt/error-crypt.h"
#include "wolfssl/wolfcrypt/aes.h"
// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************
crypto_Aead_Status_E Crypto_Aead_Wc_AesCcm_Init(void *ptr_aesCcmCtx, uint8_t *ptr_key, uint32_t keySize)
{
    crypto_Aead_Status_E ret_aesCcmStat_en = CRYPTO_AEAD_ERROR_CIPNOTSUPPTD;
    int wcAesCcmStatus = BAD_FUNC_ARG;
    
    if(ptr_aesCcmCtx != NULL)
    {
        wcAesCcmStatus = wc_AesCcmSetKey( (Aes*)ptr_aesCcmCtx, (const byte*)ptr_key, (word32)keySize);

        if(wcAesCcmStatus == 0)
        {
            ret_aesCcmStat_en = CRYPTO_AEAD_CIPHER_SUCCESS;
        }
        else if (wcAesCcmStatus == WC_KEY_SIZE_E)
        {
            ret_aesCcmStat_en = CRYPTO_AEAD_ERROR_KEY;
        }
        else if(wcAesCcmStatus == BAD_FUNC_ARG)
        {
            ret_aesCcmStat_en = CRYPTO_AEAD_ERROR_ARG;
        }
        else
        {
            ret_aesCcmStat_en  = CRYPTO_AEAD_ERROR_CIPFAIL;
        }
    }
    else
    {
        ret_aesCcmStat_en = CRYPTO_AEAD_ERROR_CTX;
    }
    return ret_aesCcmStat_en;
}

crypto_Aead_Status_E Crypto_Aead_Wc_AesCcm_Cipher(crypto_CipherOper_E cipherOper_en, void *ptr_aesCcmCtx, uint8_t *ptr_inputData, uint32_t dataLen, 
                                                    uint8_t *ptr_outData, uint8_t *ptr_nonce, uint32_t nonceLen, uint8_t *ptr_authTag,
                                                    uint32_t authTagLen, uint8_t *ptr_aad, uint32_t aadLen)
{
    crypto_Aead_Status_E ret_aesCcmStat_en = CRYPTO_AEAD_ERROR_CIPNOTSUPPTD;
    int wcAesCcmStatus = BAD_FUNC_ARG;

    if(cipherOper_en == CRYPTO_CIOP_ENCRYPT)
    {
        wcAesCcmStatus = wc_AesCcmEncrypt(ptr_aesCcmCtx, ptr_outData, (const byte*)ptr_inputData, dataLen, 
                                            (const byte*)ptr_nonce, nonceLen, ptr_authTag, authTagLen, (const byte*)ptr_aad, aadLen);
    }
    else if(cipherOper_en == CRYPTO_CIOP_DECRYPT)
    {
        wcAesCcmStatus = wc_AesCcmDecrypt(ptr_aesCcmCtx, ptr_outData, (const byte*)ptr_inputData, dataLen, 
                                            (const byte*)ptr_nonce, nonceLen, (const byte*)ptr_authTag, authTagLen, (const byte*)ptr_aad, aadLen);
    }
    else
    {
        ret_aesCcmStat_en = CRYPTO_AEAD_ERROR_CIPOPER;
    }
    if(wcAesCcmStatus == 0)
    {
        ret_aesCcmStat_en = CRYPTO_AEAD_CIPHER_SUCCESS;
    }
    else if(ret_aesCcmStat_en == CRYPTO_AEAD_ERROR_CIPOPER)
    {
        //do nothing
    }
    else
    {
        if(wcAesCcmStatus == BAD_FUNC_ARG)
        {
            ret_aesCcmStat_en = CRYPTO_AEAD_ERROR_ARG;
        }
        else
        {
            ret_aesCcmStat_en  = CRYPTO_AEAD_ERROR_CIPFAIL;
        }
    }
    return ret_aesCcmStat_en;
}
// *****************************************************************************