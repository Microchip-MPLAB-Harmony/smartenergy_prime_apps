/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    crypto_sym_cipher.c

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
#include "crypto/common_crypto/crypto_common.h"
#include "crypto/common_crypto/crypto_sym_cipher.h"
#include "crypto/wolfcrypt/crypto_sym_wc_wrapper.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

#define CRYPTO_SYM_SESSION_MAX (1) 

// *****************************************************************************
// *****************************************************************************
// Section: Function Definitions
// *****************************************************************************
// *****************************************************************************

/* MISRA C-2012 deviation block start */
/* MISRA C-2012 Rule 14.3 deviated: 1. Deviation record ID - H3_MISRAC_2012_R_14_3_DR_1 */

crypto_Sym_Status_E Crypto_Sym_Aes_Init(st_Crypto_Sym_BlockCtx *ptr_aesCtx_st, crypto_HandlerType_E handlerType_en, crypto_CipherOper_E cipherOpType_en, 
                                                crypto_Sym_OpModes_E opMode_en, uint8_t *ptr_key, uint32_t keyLen, uint8_t *ptr_initVect, uint32_t sessionID)
{
    crypto_Sym_Status_E ret_aesStatus_en = CRYPTO_SYM_ERROR_CIPNOTSUPPTD;
    
    if(ptr_aesCtx_st == NULL)
    {
        ret_aesStatus_en = CRYPTO_SYM_ERROR_CTX;
    }
    else if( (cipherOpType_en <= CRYPTO_CIOP_INVALID) || (cipherOpType_en >= CRYPTO_CIOP_MAX) )
    {
       ret_aesStatus_en =  CRYPTO_SYM_ERROR_CIPOPER; 
    }
    else if( (opMode_en <= CRYPTO_SYM_OPMODE_INVALID) || (opMode_en >= CRYPTO_SYM_OPMODE_MAX) )
    {
        ret_aesStatus_en = CRYPTO_SYM_ERROR_OPMODE;
    }         
    else if(
            ( (ptr_key == NULL) || (keyLen < (uint32_t)CRYPTO_AESKEYSIZE_128) || (keyLen > (uint32_t)CRYPTO_AESKEYSIZE_256) )  ) //key length check other than XTS mode 
    {
       ret_aesStatus_en =  CRYPTO_SYM_ERROR_KEY;
    }
    else if( (sessionID == 0u) || (sessionID > (uint32_t)CRYPTO_SYM_SESSION_MAX) )
    {
       ret_aesStatus_en =  CRYPTO_SYM_ERROR_SID; 
    }
    else if( ptr_initVect == NULL
            && (opMode_en != CRYPTO_SYM_OPMODE_ECB)
            )
    {
        ret_aesStatus_en = CRYPTO_SYM_ERROR_IV;
    }
    else
    {
        ptr_aesCtx_st->cryptoSessionID =  sessionID;
        ptr_aesCtx_st->symHandlerType_en = handlerType_en;
        ptr_aesCtx_st->ptr_initVect = ptr_initVect;
        ptr_aesCtx_st->ptr_key = ptr_key;
        ptr_aesCtx_st->symAlgoMode_en = opMode_en;
        ptr_aesCtx_st->symKeySize = keyLen;
        ptr_aesCtx_st->symCipherOper_en = cipherOpType_en;
        
        switch(ptr_aesCtx_st->symHandlerType_en)
        {
            case CRYPTO_HANDLER_SW_WOLFCRYPT:
                {
                    ret_aesStatus_en = Crypto_Sym_Wc_Aes_Init((void*)ptr_aesCtx_st->arr_symDataCtx,ptr_aesCtx_st->symCipherOper_en, 
                                                  ptr_aesCtx_st->ptr_key, ptr_aesCtx_st->symKeySize, ptr_aesCtx_st->ptr_initVect);

				}
                break;                
            default:
                ret_aesStatus_en = CRYPTO_SYM_ERROR_HDLR;
                break;
        }
        
    }
    return ret_aesStatus_en;
}
	
crypto_Sym_Status_E Crypto_Sym_Aes_Cipher(st_Crypto_Sym_BlockCtx *ptr_aesCtx_st, uint8_t *ptr_inputData, uint32_t dataLen, uint8_t *ptr_outData)
{
    crypto_Sym_Status_E ret_aesStatus_en = CRYPTO_SYM_ERROR_CIPNOTSUPPTD;
    
    if(ptr_aesCtx_st == NULL)
    {
        ret_aesStatus_en = CRYPTO_SYM_ERROR_CTX;
    }
    else if( (ptr_inputData == NULL) || (dataLen == 0u) )
    {
        ret_aesStatus_en = CRYPTO_SYM_ERROR_INPUTDATA; 
    }
    else if(ptr_outData == NULL)
    {
        ret_aesStatus_en = CRYPTO_SYM_ERROR_OUTPUTDATA;
    }
    else
    {
        switch(ptr_aesCtx_st->symHandlerType_en)
        {
            case CRYPTO_HANDLER_SW_WOLFCRYPT:
                if(ptr_aesCtx_st->symCipherOper_en == CRYPTO_CIOP_ENCRYPT)
                {
                    ret_aesStatus_en = Crypto_Sym_Wc_Aes_Encrypt(ptr_aesCtx_st->arr_symDataCtx, ptr_aesCtx_st->symAlgoMode_en, ptr_inputData, dataLen, ptr_outData);
                }
                else if(ptr_aesCtx_st->symCipherOper_en == CRYPTO_CIOP_DECRYPT)
                {
                    ret_aesStatus_en = Crypto_Sym_Wc_Aes_Decrypt(ptr_aesCtx_st->arr_symDataCtx, ptr_aesCtx_st->symAlgoMode_en, ptr_inputData, dataLen, ptr_outData);
                }
                else
                {
                    ret_aesStatus_en = CRYPTO_SYM_ERROR_CIPOPER;
                }
                break;
            default:
                ret_aesStatus_en = CRYPTO_SYM_ERROR_HDLR;
                break;
        }
    }
    return ret_aesStatus_en; 
}

crypto_Sym_Status_E Crypto_Sym_Aes_EncryptDirect(crypto_HandlerType_E handlerType_en, crypto_Sym_OpModes_E opMode_en, uint8_t *ptr_inputData, 
                                                        uint32_t dataLen, uint8_t *ptr_outData, uint8_t *ptr_key, uint32_t keyLen, uint8_t *ptr_initVect, uint32_t sessionID)
{
    crypto_Sym_Status_E ret_aesStatus_en = CRYPTO_SYM_ERROR_CIPNOTSUPPTD;
    
    if( (ptr_inputData == NULL) || (dataLen == 0u) )
    {
        ret_aesStatus_en = CRYPTO_SYM_ERROR_INPUTDATA;
    }
    else if(ptr_outData == NULL)
    {
        ret_aesStatus_en = CRYPTO_SYM_ERROR_OUTPUTDATA;
    }
    else if( (opMode_en <= CRYPTO_SYM_OPMODE_INVALID) || (opMode_en >= CRYPTO_SYM_OPMODE_MAX) )
    {
        ret_aesStatus_en = CRYPTO_SYM_ERROR_OPMODE;
    }   
    else if(ptr_key == NULL)
	{
		ret_aesStatus_en =  CRYPTO_SYM_ERROR_KEY;
	}	
	else if(
            ((keyLen < (uint32_t)CRYPTO_AESKEYSIZE_128) || (keyLen > (uint32_t)CRYPTO_AESKEYSIZE_256) ) ) //key length check other than XTS mode 
    {
       ret_aesStatus_en =  CRYPTO_SYM_ERROR_KEY;
    }
    else if( (sessionID == 0u ) || (sessionID > (uint32_t)CRYPTO_SYM_SESSION_MAX) )
    {
       ret_aesStatus_en =  CRYPTO_SYM_ERROR_SID; 
    }
    else if( (ptr_initVect == NULL)
            && (opMode_en != CRYPTO_SYM_OPMODE_ECB)
            )
    {
        ret_aesStatus_en = CRYPTO_SYM_ERROR_IV;
    }
    else
    {
        switch(handlerType_en)
        {
            case CRYPTO_HANDLER_SW_WOLFCRYPT:
                ret_aesStatus_en = Crypto_Sym_Wc_Aes_EncryptDirect(opMode_en, ptr_inputData, dataLen, ptr_outData, ptr_key, keyLen, ptr_initVect);
                break;
            default:
                ret_aesStatus_en = CRYPTO_SYM_ERROR_HDLR;
                break;
        }
    }
    return ret_aesStatus_en;    
}

crypto_Sym_Status_E Crypto_Sym_Aes_DecryptDirect(crypto_HandlerType_E handlerType_en, crypto_Sym_OpModes_E opMode_en, uint8_t *ptr_inputData, 
                                                        uint32_t dataLen, uint8_t *ptr_outData, uint8_t *ptr_key, uint32_t keyLen, uint8_t *ptr_initVect, uint32_t sessionID)
{
    crypto_Sym_Status_E ret_aesStatus_en = CRYPTO_SYM_ERROR_CIPNOTSUPPTD;
    
    if( (ptr_inputData == NULL) || (dataLen == 0u) )
    {
        ret_aesStatus_en = CRYPTO_SYM_ERROR_INPUTDATA;
    }
    else if(ptr_outData == NULL)
    {
        ret_aesStatus_en = CRYPTO_SYM_ERROR_OUTPUTDATA;
    }
    else if( (opMode_en <= CRYPTO_SYM_OPMODE_INVALID) || (opMode_en >= CRYPTO_SYM_OPMODE_MAX) )
    {
        ret_aesStatus_en = CRYPTO_SYM_ERROR_OPMODE;
    }
    else if(
            ( (ptr_key == NULL) || (keyLen < (uint32_t)CRYPTO_AESKEYSIZE_128) || (keyLen > (uint32_t)CRYPTO_AESKEYSIZE_256) )  ) //key length check other than XTS mode 
    {
       ret_aesStatus_en =  CRYPTO_SYM_ERROR_KEY;
    }
    else if( (sessionID == 0u) || (sessionID > (uint32_t)CRYPTO_SYM_SESSION_MAX) )
    {
       ret_aesStatus_en =  CRYPTO_SYM_ERROR_SID; 
    }
    else if( (ptr_initVect == NULL)
            && (opMode_en != CRYPTO_SYM_OPMODE_ECB)
            )
    {
        ret_aesStatus_en = CRYPTO_SYM_ERROR_IV;
    }
    else
    {
        switch(handlerType_en)
        {
            case CRYPTO_HANDLER_SW_WOLFCRYPT:
                ret_aesStatus_en = Crypto_Sym_Wc_Aes_DecryptDirect(opMode_en, ptr_inputData, dataLen, ptr_outData, ptr_key, keyLen, ptr_initVect);
                break;
                
            default:
                ret_aesStatus_en = CRYPTO_SYM_ERROR_HDLR;
                break;
        }
    }
    return ret_aesStatus_en;    
}

crypto_Sym_Status_E Crypto_Sym_AesKeyWrap_Init(st_Crypto_Sym_BlockCtx *ptr_aesCtx_st, crypto_HandlerType_E handlerType_en, crypto_CipherOper_E cipherOpType_en, 
                                                                                      uint8_t *ptr_key, uint32_t keyLen, uint8_t *ptr_initVect, uint32_t sessionID)
{
    crypto_Sym_Status_E ret_aesKwStat_en = CRYPTO_SYM_ERROR_CIPNOTSUPPTD;
    
    if(ptr_aesCtx_st == NULL)
    {
        ret_aesKwStat_en = CRYPTO_SYM_ERROR_CTX;
    }
    else if( (ptr_key == NULL) || (keyLen < (uint32_t)CRYPTO_AESKEYSIZE_128) || (keyLen > (uint32_t)CRYPTO_AESKEYSIZE_256)  ) 
    {
       ret_aesKwStat_en =  CRYPTO_SYM_ERROR_KEY;
    }
    else if( (cipherOpType_en <= CRYPTO_CIOP_INVALID) || (cipherOpType_en >= CRYPTO_CIOP_MAX) )
    {
       ret_aesKwStat_en =  CRYPTO_SYM_ERROR_CIPOPER; 
    }
    else if( (sessionID == 0u) || (sessionID > (uint32_t)CRYPTO_SYM_SESSION_MAX) )
    {
       ret_aesKwStat_en =  CRYPTO_SYM_ERROR_SID; 
    }
    else
    {
        ptr_aesCtx_st->cryptoSessionID =  sessionID;
        ptr_aesCtx_st->symHandlerType_en = handlerType_en;
        ptr_aesCtx_st->ptr_initVect = ptr_initVect;
        ptr_aesCtx_st->ptr_key = ptr_key;
        ptr_aesCtx_st->symKeySize = keyLen;
        ptr_aesCtx_st->symCipherOper_en = cipherOpType_en;
        
        switch(ptr_aesCtx_st->symHandlerType_en)
        {
            case CRYPTO_HANDLER_SW_WOLFCRYPT:
                ret_aesKwStat_en = Crypto_Sym_Wc_AesKeyWrap_Init(ptr_aesCtx_st->arr_symDataCtx, ptr_aesCtx_st->symCipherOper_en, 
                                                                       ptr_aesCtx_st->ptr_key, ptr_aesCtx_st->symKeySize, ptr_aesCtx_st->ptr_initVect);
                break;
            case CRYPTO_HANDLER_HW_INTERNAL:
                
                break;
            default:
                ret_aesKwStat_en = CRYPTO_SYM_ERROR_HDLR;
                break;
        }
        
    }
    return ret_aesKwStat_en;
}

crypto_Sym_Status_E Crypto_Sym_AesKeyWrap_Cipher(st_Crypto_Sym_BlockCtx *ptr_aesCtx_st, uint8_t *ptr_inputData, uint32_t dataLen, uint8_t *ptr_outData)
{
    crypto_Sym_Status_E ret_aesKwStat_en = CRYPTO_SYM_ERROR_CIPNOTSUPPTD;
    
    if(ptr_aesCtx_st == NULL)
    {
        ret_aesKwStat_en = CRYPTO_SYM_ERROR_CTX;
    }
    else if( (ptr_inputData == NULL) || (dataLen == 0u) )
    {
        ret_aesKwStat_en = CRYPTO_SYM_ERROR_INPUTDATA;
    }
    else if(ptr_outData == NULL)
    {
        ret_aesKwStat_en = CRYPTO_SYM_ERROR_OUTPUTDATA;
    }
    else
    {
        switch(ptr_aesCtx_st->symHandlerType_en)
        {
            case CRYPTO_HANDLER_SW_WOLFCRYPT:
                if(ptr_aesCtx_st->symCipherOper_en == CRYPTO_CIOP_ENCRYPT)
                {
                    ret_aesKwStat_en = Crypto_Sym_Wc_AesKeyWrap(ptr_aesCtx_st->arr_symDataCtx, ptr_inputData, dataLen, ptr_outData, (dataLen+8u), ptr_aesCtx_st->ptr_initVect);
                }
                else if(ptr_aesCtx_st->symCipherOper_en == CRYPTO_CIOP_DECRYPT)
                {
                    ret_aesKwStat_en = Crypto_Sym_Wc_AesKeyUnWrap(ptr_aesCtx_st->arr_symDataCtx, ptr_inputData, dataLen, ptr_outData, (dataLen+8u), ptr_aesCtx_st->ptr_initVect);
                }
                else
                {
                    ret_aesKwStat_en = CRYPTO_SYM_ERROR_CIPOPER;
                }
                break;
            case CRYPTO_HANDLER_HW_INTERNAL:
                
                break;
            default:
                ret_aesKwStat_en = CRYPTO_SYM_ERROR_HDLR;
                break;
        }
    }
    return ret_aesKwStat_en; 
}

crypto_Sym_Status_E Crypto_Sym_AesKeyWrapDirect(crypto_HandlerType_E handlerType_en, uint8_t *ptr_inputData, uint32_t inputLen, 
                                                    uint8_t *ptr_outData, uint8_t *ptr_key, uint32_t keyLen, uint8_t *ptr_initVect, uint32_t sessionID)
{
    crypto_Sym_Status_E ret_aesKwStat_en = CRYPTO_SYM_ERROR_CIPNOTSUPPTD;
    
    if( (ptr_inputData == NULL) || (inputLen < (uint32_t)((8Lu)*(2Lu)) ) )
    {
        ret_aesKwStat_en = CRYPTO_SYM_ERROR_INPUTDATA;
    }
    else if(ptr_outData == NULL)
    {
        ret_aesKwStat_en = CRYPTO_SYM_ERROR_OUTPUTDATA;
    }
    else if( (ptr_key == NULL) || (keyLen < (uint32_t)CRYPTO_AESKEYSIZE_128) || (keyLen > (uint32_t)CRYPTO_AESKEYSIZE_256)  ) 
    {
       ret_aesKwStat_en =  CRYPTO_SYM_ERROR_KEY;
    }
    else if( (sessionID == 0u) || (sessionID > (uint32_t)CRYPTO_SYM_SESSION_MAX) )
    {
       ret_aesKwStat_en =  CRYPTO_SYM_ERROR_SID; 
    }
    else
    {
        switch(handlerType_en)
        {
            case CRYPTO_HANDLER_SW_WOLFCRYPT:
                ret_aesKwStat_en = Crypto_Sym_Wc_AesKeyWrapDirect(ptr_inputData, inputLen, ptr_outData, (inputLen + 8u), ptr_key, keyLen, ptr_initVect);
                break;
            case CRYPTO_HANDLER_HW_INTERNAL:

                break;
            default:
                ret_aesKwStat_en = CRYPTO_SYM_ERROR_HDLR;
                break;
        }
    }
    return ret_aesKwStat_en;    
}

crypto_Sym_Status_E Crypto_Sym_AesKeyUnWrapDirect(crypto_HandlerType_E handlerType_en, uint8_t *ptr_inputData, uint32_t inputLen, 
                                                    uint8_t *ptr_outData, uint8_t *ptr_key, uint32_t keyLen, uint8_t *ptr_initVect, uint32_t sessionID)
{
    crypto_Sym_Status_E ret_aesKwStat_en = CRYPTO_SYM_ERROR_CIPNOTSUPPTD;
    
    if( (ptr_inputData == NULL) || (inputLen < (uint32_t)((8Lu)*(2Lu))) )
    {
        ret_aesKwStat_en = CRYPTO_SYM_ERROR_INPUTDATA;
    }
    else if(ptr_outData == NULL)
    {
        ret_aesKwStat_en = CRYPTO_SYM_ERROR_OUTPUTDATA;
    }
    else if( (ptr_key == NULL) || (keyLen < (uint32_t)CRYPTO_AESKEYSIZE_128) || (keyLen > (uint32_t)CRYPTO_AESKEYSIZE_256) ) 
    {
       ret_aesKwStat_en =  CRYPTO_SYM_ERROR_KEY;
    }
    else if( (sessionID == 0u) || (sessionID > (uint32_t)CRYPTO_SYM_SESSION_MAX) )
    {
       ret_aesKwStat_en =  CRYPTO_SYM_ERROR_SID; 
    }
    else
    {
        switch(handlerType_en)
        {
            case CRYPTO_HANDLER_SW_WOLFCRYPT:
                ret_aesKwStat_en = Crypto_Sym_Wc_AesKeyUnWrapDirect(ptr_inputData, inputLen, ptr_outData, (inputLen + 8u), ptr_key, keyLen, ptr_initVect);
                break;
            case CRYPTO_HANDLER_HW_INTERNAL:

                break;
            default:
                ret_aesKwStat_en = CRYPTO_SYM_ERROR_HDLR;
                break;
        }
    }
    return ret_aesKwStat_en;    
}

/* MISRA C-2012 deviation block end */
