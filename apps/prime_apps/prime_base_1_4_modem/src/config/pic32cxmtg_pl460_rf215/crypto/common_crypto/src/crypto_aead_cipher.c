/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    crypto_aead_cipher.c

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

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// ***************************************************************************** 

#define CRYPTO_AEAD_SESSION_MAX (1)

// *****************************************************************************
// *****************************************************************************
// Section: Function Definitions
// *****************************************************************************
// *****************************************************************************

crypto_Aead_Status_E Crypto_Aead_AesCcm_Init(st_Crypto_Aead_AesCcm_ctx *ptr_aesCcmCtx_st, crypto_HandlerType_E handlerType_en, 
                                              uint8_t *ptr_key, uint32_t keyLen, uint32_t sessionID)
{
    crypto_Aead_Status_E ret_aesCcmStat_en = CRYPTO_AEAD_ERROR_CIPNOTSUPPTD;
    
    if(ptr_aesCcmCtx_st == NULL)
    {
        ret_aesCcmStat_en = CRYPTO_AEAD_ERROR_CTX;
    }
    else if( (ptr_key == NULL) || 
                ( (keyLen != (uint32_t)CRYPTO_AESKEYSIZE_128)
                    && (keyLen != (uint32_t)CRYPTO_AESKEYSIZE_192)
                    && (keyLen != (uint32_t)CRYPTO_AESKEYSIZE_256) ) ) 
    {
       ret_aesCcmStat_en =  CRYPTO_AEAD_ERROR_KEY;
    }
    else if( (sessionID <= 0u) || (sessionID > (uint32_t)CRYPTO_AEAD_SESSION_MAX) )
    {
       ret_aesCcmStat_en =  CRYPTO_AEAD_ERROR_SID; 
    }
    else
    {
        ptr_aesCcmCtx_st->cryptoSessionID =  sessionID;
        ptr_aesCcmCtx_st->aeadHandlerType_en = handlerType_en;
        ptr_aesCcmCtx_st->ptr_key = ptr_key;
        ptr_aesCcmCtx_st->aeadKeySize = keyLen;
        
        switch(ptr_aesCcmCtx_st->aeadHandlerType_en)
        {
            case CRYPTO_HANDLER_SW_WOLFCRYPT:
                    ret_aesCcmStat_en = Crypto_Aead_Wc_AesCcm_Init((void*)ptr_aesCcmCtx_st->arr_aeadDataCtx, ptr_aesCcmCtx_st->ptr_key, ptr_aesCcmCtx_st->aeadKeySize);     
                break;
               
            case CRYPTO_HANDLER_HW_INTERNAL:
                
                break;
				
            default:
                ret_aesCcmStat_en = CRYPTO_AEAD_ERROR_HDLR;
                break;
        }
        
    }
    return ret_aesCcmStat_en;
}

crypto_Aead_Status_E Crypto_Aead_AesCcm_Cipher(st_Crypto_Aead_AesCcm_ctx *ptr_aesCcmCtx_st, crypto_CipherOper_E cipherOper_en, uint8_t *ptr_inputData, uint32_t dataLen, 
                                                    uint8_t *ptr_outData, uint8_t *ptr_nonce, uint32_t nonceLen, uint8_t *ptr_authTag,
                                                    uint32_t authTagLen, uint8_t *ptr_aad, uint32_t aadLen)
{
    crypto_Aead_Status_E ret_aesCcmStat_en = CRYPTO_AEAD_ERROR_CIPNOTSUPPTD;
    
    if(ptr_aesCcmCtx_st == NULL)
    {
        ret_aesCcmStat_en = CRYPTO_AEAD_ERROR_CTX;
    }
    else if( ((ptr_inputData == NULL)&& (dataLen != 0u))
                || ((ptr_inputData != NULL)&& (dataLen == 0u)) )
    {
        ret_aesCcmStat_en = CRYPTO_AEAD_ERROR_INPUTDATA;
    }
    else if( ((ptr_inputData != NULL) && (ptr_outData == NULL))
                || ((ptr_inputData == NULL) && (ptr_outData != NULL)) )
    {
        ret_aesCcmStat_en = CRYPTO_AEAD_ERROR_OUTPUTDATA;
    }
    else if( (ptr_nonce == NULL) || (nonceLen < 7u) || (nonceLen > 13u) )
    {
        ret_aesCcmStat_en = CRYPTO_AEAD_ERROR_NONCE;
    }
    else if( (ptr_authTag == NULL) || (authTagLen < 4u) || (authTagLen > 16u) )
    {
        ret_aesCcmStat_en = CRYPTO_AEAD_ERROR_AUTHTAG;
    }
    else if( ((ptr_aad == NULL) && (aadLen != 0u))
                || ((ptr_aad != NULL) && (aadLen == 0u)))
    {
        ret_aesCcmStat_en = CRYPTO_AEAD_ERROR_AAD;
    }
    else if((ptr_inputData == NULL) && (ptr_aad == NULL))
    {
        ret_aesCcmStat_en = CRYPTO_AEAD_ERROR_ARG;
    }
    else if((cipherOper_en != CRYPTO_CIOP_ENCRYPT) && (cipherOper_en != CRYPTO_CIOP_DECRYPT))
    {
        ret_aesCcmStat_en = CRYPTO_AEAD_ERROR_CIPOPER;
    }
    else
    {
        switch(ptr_aesCcmCtx_st->aeadHandlerType_en)
        {
            case CRYPTO_HANDLER_SW_WOLFCRYPT:
                ret_aesCcmStat_en = Crypto_Aead_Wc_AesCcm_Cipher(cipherOper_en, ptr_aesCcmCtx_st->arr_aeadDataCtx, ptr_inputData, dataLen, 
                                                        ptr_outData, ptr_nonce, nonceLen, ptr_authTag, authTagLen, ptr_aad, aadLen);
                break; 

            case CRYPTO_HANDLER_HW_INTERNAL:

                break;
				
            default:
                ret_aesCcmStat_en = CRYPTO_AEAD_ERROR_HDLR;
                break;
        }
    }
    return ret_aesCcmStat_en;
}
// *****************************************************************************