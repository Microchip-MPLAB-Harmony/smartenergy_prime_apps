/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    crypto_hash_wc_wrapper.c

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
#include "crypto/common_crypto/crypto_hash.h"
#include "crypto/wolfcrypt/crypto_hash_wc_wrapper.h"
#include "wolfssl/wolfcrypt/error-crypt.h"
#include "wolfssl/wolfcrypt/sha256.h"
// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************


// *****************************************************************************
	
crypto_Hash_Status_E Crypto_Hash_Wc_ShaDigest(uint8_t *ptr_data, uint32_t dataLen, uint8_t *ptr_digest, crypto_Hash_Algo_E hashAlgo_en)
{
    crypto_Hash_Status_E ret_shaStat_en = CRYPTO_HASH_ERROR_NOTSUPPTED;
    
    //As due to VLA misra Issue maximum Size is allocated
    uint8_t arr_shaDataCtx[CRYPTO_HASH_SHA512CTX_SIZE];
    
    if( (ptr_data != NULL) && (dataLen > 0u) && (ptr_digest != NULL) )
    {
        ret_shaStat_en = Crypto_Hash_Wc_ShaInit(arr_shaDataCtx, hashAlgo_en);
        if(ret_shaStat_en == CRYPTO_HASH_SUCCESS)
        {
            ret_shaStat_en = Crypto_Hash_Wc_ShaUpdate(arr_shaDataCtx, ptr_data, dataLen, hashAlgo_en);
            if(ret_shaStat_en == CRYPTO_HASH_SUCCESS)
            {
                ret_shaStat_en = Crypto_Hash_Wc_ShaFinal(arr_shaDataCtx, ptr_digest, hashAlgo_en);
            }
        }
    }
    else
    {
        ret_shaStat_en = CRYPTO_HASH_ERROR_ARG;
    }
    return ret_shaStat_en;
}

crypto_Hash_Status_E Crypto_Hash_Wc_ShaInit(void *ptr_shaCtx_st, crypto_Hash_Algo_E hashAlgo_en)
{
    crypto_Hash_Status_E ret_shaStat_en = CRYPTO_HASH_ERROR_NOTSUPPTED;
	int wcShaStatus = BAD_FUNC_ARG;
	
    if(ptr_shaCtx_st != NULL)
    {
        switch(hashAlgo_en)
        {    
            case CRYPTO_HASH_SHA2_256:
                wcShaStatus = wc_InitSha256((wc_Sha256*)ptr_shaCtx_st);
                break;
            default:
                ret_shaStat_en = CRYPTO_HASH_ERROR_ALGO;
                break;
        }

        if(ret_shaStat_en == CRYPTO_HASH_ERROR_ALGO)
        {
            //do nothing
        }
        else if(wcShaStatus == 0)
        {
            ret_shaStat_en = CRYPTO_HASH_SUCCESS;
        }
        else if (wcShaStatus == BAD_FUNC_ARG)
        {
            ret_shaStat_en = CRYPTO_HASH_ERROR_ARG;
        }
        else
        {
            ret_shaStat_en = CRYPTO_HASH_ERROR_FAIL;
        }
    }
    else
    {
        ret_shaStat_en = CRYPTO_HASH_ERROR_CTX;
    }
    return ret_shaStat_en;
}

crypto_Hash_Status_E Crypto_Hash_Wc_ShaUpdate(void *ptr_shaCtx_st, uint8_t *ptr_data, uint32_t dataLen, crypto_Hash_Algo_E hashAlgo_en)
{
    crypto_Hash_Status_E ret_shaStat_en = CRYPTO_HASH_ERROR_NOTSUPPTED;
    int wcShaStatus = BAD_FUNC_ARG;
	
	switch(hashAlgo_en)
	{
		case CRYPTO_HASH_SHA2_256:
			wcShaStatus = wc_Sha256Update((wc_Sha256*)ptr_shaCtx_st, (const byte*)ptr_data, (word32)dataLen);
            break;
        default:
            ret_shaStat_en = CRYPTO_HASH_ERROR_ALGO;
            break;
	}

	if(wcShaStatus == 0)
	{
		ret_shaStat_en = CRYPTO_HASH_SUCCESS;
	}
    else if(ret_shaStat_en == CRYPTO_HASH_ERROR_ALGO)
    {
        //do nothing
    }
    else
    {
        ret_shaStat_en = CRYPTO_HASH_ERROR_FAIL;
    }
    
	return ret_shaStat_en;  
}

crypto_Hash_Status_E Crypto_Hash_Wc_ShaFinal(void *ptr_shaCtx_st, uint8_t *ptr_digest, crypto_Hash_Algo_E hashAlgo_en)
{
    crypto_Hash_Status_E ret_shaStat_en = CRYPTO_HASH_ERROR_NOTSUPPTED;
    int wcShaStatus = BAD_FUNC_ARG;
	
	switch(hashAlgo_en)
	{
		case CRYPTO_HASH_SHA2_256:
			wcShaStatus = wc_Sha256Final((wc_Sha256*)ptr_shaCtx_st, (byte*)ptr_digest);
            break;
        default:
            ret_shaStat_en = CRYPTO_HASH_ERROR_ALGO;
            break;
	}

	if(wcShaStatus == 0)
	{
		ret_shaStat_en = CRYPTO_HASH_SUCCESS;
	}
    else if(ret_shaStat_en == CRYPTO_HASH_ERROR_ALGO)
    {
        //do nothing
    }
    else
    {
        ret_shaStat_en = CRYPTO_HASH_ERROR_FAIL;
    }
    
	return ret_shaStat_en;  
}
