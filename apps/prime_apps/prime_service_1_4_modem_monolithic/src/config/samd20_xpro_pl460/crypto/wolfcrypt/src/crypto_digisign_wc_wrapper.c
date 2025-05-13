/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    crypto_digisign_wc_wrapper.c

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
#include "crypto/common_crypto/crypto_digsign.h"
#include "crypto/common_crypto/crypto_common.h"
#include "crypto/wolfcrypt/crypto_digisign_wc_wrapper.h"
#include "crypto/wolfcrypt/crypto_common_wc_wrapper.h"
#include "wolfssl/wolfcrypt/error-crypt.h"
#include "wolfssl/wolfcrypt/random.h"
#include "wolfssl/wolfcrypt/ecc.h"
#include "crypto/common_crypto/crypto_hash.h"

crypto_DigiSign_Status_E Crypto_DigiSign_Wc_Ecdsa_SignHash(uint8_t *ptr_wcInputHash, uint32_t wcHashLen, uint8_t *ptr_wcSig, uint32_t wcSigLen, uint8_t *ptr_wcPrivKey, 
                                                       uint32_t wcPrivKeyLen, crypto_EccCurveType_E wcEccCurveType_en)
{
    crypto_DigiSign_Status_E ret_wcEcdsaStat_en;
    ecc_key wcEccPrivKey_st;
    WC_RNG wcRng_st;
    int wcEccCurveId = (int)ECC_CURVE_INVALID;
    int wcEcdsaStat = BAD_FUNC_ARG;
    mp_int r, s;
   
    wcEccCurveId = Crypto_Common_Wc_Ecc_GetWcCurveId(wcEccCurveType_en);
    
    if(wcEccCurveId == (int)ECC_CURVE_INVALID)
    {
        ret_wcEcdsaStat_en = CRYPTO_DIGISIGN_ERROR_CURVE;
    }
    else
    {
        // Setup the RNG
        wcEcdsaStat = wc_InitRng(&wcRng_st);

        if(wcEcdsaStat == 0)
        {
            /* Setup the ECC key */
            wcEcdsaStat = wc_ecc_init(&wcEccPrivKey_st);
            
            if(wcEcdsaStat == 0)
            {
                // Import private key, public part Pass as NULL
                wcEcdsaStat = wc_ecc_import_private_key_ex(ptr_wcPrivKey, wcPrivKeyLen,/* private key "d" */
                                                            NULL, 0,                    /* public (optional) */
                                                            &wcEccPrivKey_st,
                                                            wcEccCurveId                /* ECC Curve Id */
                                                            );
                if(wcEcdsaStat == 0)
                {
                    wcEcdsaStat = mp_init(&r);
                    
                    if(wcEcdsaStat == 0)
                    {        
                        wcEcdsaStat = mp_init(&s);           

                        if(wcEcdsaStat == 0)
                        {
                            wcEcdsaStat = wc_ecc_sign_hash_ex(ptr_wcInputHash, wcHashLen, &wcRng_st, &wcEccPrivKey_st, &r, &s);

                            //Import signature R and S
                            if(wcEcdsaStat == 0)
                            {
                                wcEcdsaStat = mp_to_unsigned_bin_len(&r, (byte*)ptr_wcSig, (int)wcPrivKeyLen);
                            }
                            if(wcEcdsaStat == 0)
                            {
                                wcEcdsaStat = mp_to_unsigned_bin_len(&s, (byte*)(ptr_wcSig + (uint32_t)wcPrivKeyLen), (int)wcPrivKeyLen);
                            }
                        }
                    }
                }
            }
            if(wcEcdsaStat == 0)
            {
                ret_wcEcdsaStat_en = CRYPTO_DIGISIGN_SUCCESS;
            }
            else if(wcEcdsaStat == ECC_CURVE_OID_E)
            {
                ret_wcEcdsaStat_en = CRYPTO_DIGISIGN_ERROR_CURVE;
            }
            else if( (wcEcdsaStat == BAD_FUNC_ARG) || (wcEcdsaStat == ECC_BAD_ARG_E) )
            {
                ret_wcEcdsaStat_en = CRYPTO_DIGISIGN_ERROR_ARG;
            }
            else
            {
                ret_wcEcdsaStat_en = CRYPTO_DIGISIGN_ERROR_FAIL;
            }
        }
        else
        {
            ret_wcEcdsaStat_en = CRYPTO_DIGISIGN_ERROR_RNG;
        }
    }
    
    return ret_wcEcdsaStat_en;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#pragma coverity compliance block deviate "MISRA C-2012 Rule 5.1" "H3_MISRAC_2012_R_5_1_DR_1" 
crypto_DigiSign_Status_E Crypto_DigiSign_Wc_Ecdsa_VerifyHash(uint8_t *ptr_wcInputHash, uint32_t wcHashLen, uint8_t *ptr_wcInputSig, uint32_t wcSigLen, uint8_t *ptr_wcPubKey, uint32_t wcPubKeyLen, 
                                                         int8_t *ptr_wcHashVerifyStat, crypto_EccCurveType_E wcEccCurveType_en)
{
    crypto_DigiSign_Status_E ret_wcEcdsaStat_en;
    ecc_key wcEccPubKey_st;
    int wcEccCurveId = (int)ECC_CURVE_INVALID;
    int wcEcdsaStat = BAD_FUNC_ARG;
    int ptr_verifyStat = 0;
    mp_int r, s;
    int curveLen = 0;
    
    wcEccCurveId = Crypto_Common_Wc_Ecc_GetWcCurveId(wcEccCurveType_en);
    
    if(wcEccCurveId == (int)ECC_CURVE_INVALID)
    {
        ret_wcEcdsaStat_en = CRYPTO_DIGISIGN_ERROR_CURVE;
    }
    else
    {
        /* Setup the ECC key */
        wcEcdsaStat = wc_ecc_init(&wcEccPubKey_st);
        if(wcEcdsaStat == 0)
        {
            wcEcdsaStat = mp_init(&r);
            if(wcEcdsaStat == 0)
            {        
                wcEcdsaStat = mp_init(&s);
            }
            if(wcEcdsaStat == 0)
            {   
                wcEcdsaStat = wc_ecc_import_x963_ex(ptr_wcPubKey, wcPubKeyLen, &wcEccPubKey_st, wcEccCurveId);

                if(wcEcdsaStat == 0)
                {
                    //Get Curve Length using Curve ID
                    curveLen = wc_ecc_get_curve_size_from_id(wcEccCurveId);

                    // Import signature r and s
                    wcEcdsaStat = mp_read_unsigned_bin(&r, ptr_wcInputSig, (word32)curveLen);
                    if (wcEcdsaStat == 0) 
                    {
                        wcEcdsaStat = mp_read_unsigned_bin(&s, (const byte*)(ptr_wcInputSig+(uint32_t)curveLen), (word32)curveLen);

                        //If Verify status value is 1 then verification is successfully
                        if (wcEcdsaStat == 0) 
                        {
                            wcEcdsaStat = wc_ecc_verify_hash_ex(&r, &s, (const byte*)ptr_wcInputHash, (word32)wcHashLen, &ptr_verifyStat, &wcEccPubKey_st);
                            *ptr_wcHashVerifyStat = (int8_t)ptr_verifyStat;
                        }
                    }
                }
            }
        }
        else
        {
           //do nothing 
        }
        if(wcEcdsaStat == 0)
        {
            ret_wcEcdsaStat_en = CRYPTO_DIGISIGN_SUCCESS;
        }
        else if(wcEcdsaStat == ECC_CURVE_OID_E)
        {
            ret_wcEcdsaStat_en = CRYPTO_DIGISIGN_ERROR_CURVE;
        }
        else if( (wcEcdsaStat == BAD_FUNC_ARG) || (wcEcdsaStat == ECC_BAD_ARG_E) )
        {
            ret_wcEcdsaStat_en = CRYPTO_DIGISIGN_ERROR_ARG;
        }
        else
        {
            ret_wcEcdsaStat_en = CRYPTO_DIGISIGN_ERROR_FAIL;
        }
    }
    
    return ret_wcEcdsaStat_en;
}
#pragma coverity compliance end_block "MISRA C-2012 Rule 5.1"
#pragma GCC diagnostic pop 
crypto_DigiSign_Status_E Crypto_DigiSign_Wc_Ecdsa_SignData(uint8_t *ptr_wcInputData, uint32_t wcDataLen, uint8_t *ptr_wcSig, uint32_t wcSigLen, 
                                                            uint8_t *ptr_wcPrivKey, uint32_t wcPrivKeyLen, crypto_Hash_Algo_E hashType_en,                                                            
                                                            crypto_EccCurveType_E wcEccCurveType_en)
{
    crypto_DigiSign_Status_E ret_wcEcdsaStat_en;
    uint8_t arr_hash[512];
    uint32_t wcHashLen = 0x00UL;
    
    //calculate the hash before signing
    wcHashLen = Crypto_Hash_GetHashAndHashSize(CRYPTO_HANDLER_SW_WOLFCRYPT, hashType_en, ptr_wcInputData,wcDataLen, arr_hash);
    
    if(wcHashLen == 0x00UL)
    {
        ret_wcEcdsaStat_en = CRYPTO_DIGISIGN_ERROR_HASHTYPE;
    }
    else
    {
        //Sign the Hash
        ret_wcEcdsaStat_en = Crypto_DigiSign_Wc_Ecdsa_SignHash(arr_hash, wcHashLen, ptr_wcSig, wcSigLen, ptr_wcPrivKey, 
                                                       wcPrivKeyLen, wcEccCurveType_en);
    }
    
    return ret_wcEcdsaStat_en;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#pragma coverity compliance block deviate "MISRA C-2012 Rule 5.1" "H3_MISRAC_2012_R_5_1_DR_1" 
crypto_DigiSign_Status_E Crypto_DigiSign_Wc_Ecdsa_VerifyData(uint8_t *ptr_wcInputData, uint32_t wcDataLen, uint8_t *ptr_wcSig, uint32_t wcSigLen, 
                                                            uint8_t *ptr_wcPubKey, uint32_t wcPubKeyLen, crypto_Hash_Algo_E hashType_en,
                                                            int8_t *ptr_wcHashVerifyStat, crypto_EccCurveType_E wcEccCurveType_en)
{
    crypto_DigiSign_Status_E ret_wcEcdsaStat_en;
    uint8_t arr_hash[512];
    uint32_t wcHashLen = 0x00UL;
    
    //calculate the hash before verify
    wcHashLen = Crypto_Hash_GetHashAndHashSize(CRYPTO_HANDLER_SW_WOLFCRYPT, hashType_en, ptr_wcInputData,wcDataLen, arr_hash);
    
    if(wcHashLen == 0x00UL)
    {
        ret_wcEcdsaStat_en = CRYPTO_DIGISIGN_ERROR_HASHTYPE;
    }
    else
    {
        //Verify the Hash
        ret_wcEcdsaStat_en = Crypto_DigiSign_Wc_Ecdsa_VerifyHash(arr_hash, wcHashLen, ptr_wcSig, wcSigLen, ptr_wcPubKey, 
                                                                wcPubKeyLen, ptr_wcHashVerifyStat, wcEccCurveType_en);
    }
    
    return ret_wcEcdsaStat_en;
}
#pragma coverity compliance end_block "MISRA C-2012 Rule 5.1"
#pragma GCC diagnostic pop 
