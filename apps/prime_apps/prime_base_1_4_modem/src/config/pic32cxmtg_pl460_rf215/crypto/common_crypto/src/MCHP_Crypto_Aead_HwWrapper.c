/**************************************************************************
  Crypto Framework Library Source

  Company:
    Microchip Technology Inc.

  File Name:
    MCHP_Crypto_Aead_HwWrapper.c

  Summary:
    Crypto Framework Library wrapper file for hardware AES.

  Description:
    This source file contains the wrapper interface to access the AEAD
    algorithms in the AES hardware driver for Microchip microcontrollers.
**************************************************************************/

//DOM-IGNORE-BEGIN
/*
Copyright (C) 2024, Microchip Technology Inc., and its subsidiaries. All rights reserved.

The software and documentation is provided by microchip and its contributors
"as is" and any express, implied or statutory warranties, including, but not
limited to, the implied warranties of merchantability, fitness for a particular
purpose and non-infringement of third party intellectual property rights are
disclaimed to the fullest extent permitted by law. In no event shall microchip
or its contributors be liable for any direct, indirect, incidental, special,
exemplary, or consequential damages (including, but not limited to, procurement
of substitute goods or services; loss of use, data, or profits; or business
interruption) however caused and on any theory of liability, whether in contract,
strict liability, or tort (including negligence or otherwise) arising in any way
out of the use of the software and documentation, even if advised of the
possibility of such damage.

Except as expressly permitted hereunder and subject to the applicable license terms
for any third-party software incorporated in the software and any applicable open
source software license terms, no license or other rights, whether express or
implied, are granted under any patent or other intellectual property rights of
Microchip or any third party.
*/
//DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include <string.h>
#include "crypto/common_crypto/MCHP_Crypto_Aead_HwWrapper.h"
#include "crypto/drivers/drv_crypto_aes_hw_6149.h"

// *****************************************************************************
// *****************************************************************************
// Section: File Scope Variables
// *****************************************************************************
// *****************************************************************************

static CRYPTO_AES_CONFIG aesGcmCfg;

// *****************************************************************************
// *****************************************************************************
// Section: File scope functions
// *****************************************************************************
// *****************************************************************************

static void lCrypto_Aead_Hw_Gcm_WriteKey(uint32_t *gcmKey)
{
    DRV_CRYPTO_AES_WriteKey(gcmKey);
   
    /* Wait for the GCMH to generate */
    while (!DRV_CRYPTO_AES_CipherIsReady())
    {
        ;
	} 
}    
    
static void lCrypto_Aead_Hw_Gcm_WriteGeneratedIv(CRYPTO_GCM_HW_CONTEXT *gcmCtx)
{
    uint32_t ivBuffer[4];
    uint8_t x;
    
    for (x = 0; x < 3UL; x++)
    {
        ivBuffer[x] = gcmCtx->calculatedIv[x];        
    }    

    ivBuffer[3] = gcmCtx->invokeCtr[0];
    
    DRV_CRYPTO_AES_WriteInitVector(ivBuffer);
}

static void lCrypto_Aead_Hw_Gcm_GenerateJ0(CRYPTO_GCM_HW_CONTEXT *gcmCtx, 
                                           uint8_t *iv, 
                                           uint32_t ivLen)
{
    uint8_t *ivSaved = (uint8_t*)gcmCtx->calculatedIv;
    
    /* Check if IV length is 96 bits */
    if (ivLen == 12UL)    
    {
        (void) memcpy(ivSaved, iv, ivLen);
        ivSaved[(sizeof(gcmCtx->calculatedIv) - 1UL)] = 0x1;
        return;
    }
    
    /* Write the key */
    lCrypto_Aead_Hw_Gcm_WriteKey(gcmCtx->key);

    /* Configure AADLEN with: len(IV || 0s+64 || [len(IV)]64) */
    uint32_t numFullBlocks = ivLen / 16UL;
    if (ivLen % 16UL > 0UL)
    {
        // This is questionable. The formula says to use the bit size.
        // But the register description is byte size.
        DRV_CRYPTO_AES_WriteAuthDataLen((numFullBlocks + 2UL) * 128UL);
    }
    else
    {   
        DRV_CRYPTO_AES_WriteAuthDataLen((numFullBlocks + 1UL) * 128UL);        
    }
    
    /* Configure CLEN to 0. This will allow running a GHASHH only. */
    DRV_CRYPTO_AES_WritePCTextLen(0);
    
    /* MISRA C-2012 deviation block start */
    /* MISRA C-2012 Rule 11.3 deviated: 1. Deviation record ID - H3_MISRAC_2012_R_11_3_DR_1 */
    /* Write message to process (IV || 0s+64 || [len(IV)]64) */
    uint32_t *inPtr = (uint32_t *)iv;
    /* MISRA C-2012 deviation block end */
    uint32_t block;   /* 4 32bit block size */
    for (block = 0; block < numFullBlocks; block++)
    {
        /* Write the data to be ciphered to the input data registers */
        DRV_CRYPTO_AES_WriteInputData(inPtr);
        inPtr += 4;

        /* Wait for the cipher process to end */
        while (!DRV_CRYPTO_AES_CipherIsReady())
        {
            ;
        }        
    }    
    
    uint32_t numPartialBytes = ivLen % 16UL;
    if (numPartialBytes > 0UL)
    {
        uint32_t partialPlusPad[4] = {0};
        (void) memcpy(partialPlusPad, inPtr, numPartialBytes);
        
        /* Write the data to be ciphered to the input data registers */
        DRV_CRYPTO_AES_WriteInputData(partialPlusPad);

        /* Wait for the cipher process to end */
        while (!DRV_CRYPTO_AES_CipherIsReady())
        {
            ;
        }              
    }
    
    uint8_t finalBlock[16] = {0};
    uint32_t bits = ivLen * 8UL;
    // This may be wrong, but we have to change it to big endian format.
    // Per NIST AES GCM is big endian.
    finalBlock[15] = (uint8_t)(bits & (uint32_t)0xFFUL);
    finalBlock[14] = (uint8_t)((bits >> 8) & (uint32_t)0xFFUL);
    finalBlock[13] = (uint8_t)((bits >> 16)& (uint32_t)0xFFUL);
    finalBlock[12] = (uint8_t)((bits >> 24)& (uint32_t)0xFFUL);
    
    /* The lines below are subject to a type-punning warning because 
    * the (uint8_t*) is cast to a (uint32_t*) which might typically suffer 
    * from a misalignment problem. The conditional breakpoint will
    * trigger the debugger if the byte-pointer is misaligned, but will
    * be eliminated if the compiler can prove correct alignment.
    * Such a warning is thrown only at higher optimization levels.
    */
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
#include <assert.h> // prove we have 4-byte alignment
    __conditional_software_breakpoint(0 == ((uint32_t)finalBlock) % 4);
#endif
    /* MISRA C-2012 deviation block start */
    /* MISRA C-2012 Rule 11.3 deviated: 1. Deviation record ID - H3_MISRAC_2012_R_11_3_DR_1 */
    /* Write the data to be ciphered to the input data registers */
    DRV_CRYPTO_AES_WriteInputData((uint32_t *)finalBlock);
    /* MISRA C-2012 deviation block end */
#if defined(__GNUC__)
#pragma GCC diagnostic push
#endif

    /* Wait for the cipher process to end */
    while (!DRV_CRYPTO_AES_CipherIsReady())
    {
        ;
    }     
    
    /* Read hash to obtain the J0 value */
    DRV_CRYPTO_AES_ReadGcmHash(gcmCtx->intermediateHash);
    DRV_CRYPTO_AES_ReadGcmH(gcmCtx->H);
    (void) memcpy(ivSaved, (uint8_t *)gcmCtx->intermediateHash, 16);
    
    uint32_t tmp = (ivSaved[15] & 0xFFUL) |
                   ((ivSaved[14] & 0xFFUL) << 8U) |
                   ((ivSaved[13] & 0xFFUL) << 16U) |
                   ((ivSaved[12] & 0xFFUL) << 24U);
    tmp++;
    gcmCtx->invokeCtr[0] = (uint32_t)((tmp & 0x000000FFUL) << 24U) |
                           ((tmp & 0x0000FF00UL) << 8U) |
                           ((tmp & 0x00FF0000UL) >> 8U) |
                           ((tmp & 0xFF000000UL) >> 24U);
}

static void lCrypto_Aead_Hw_Gcm_RunBlocks(uint32_t *in, uint32_t byteLen, 
                                          uint32_t* out)
{
    if (byteLen == 0UL)
    {
        return;
    }
    
    uint32_t blockLen = byteLen / 4UL;
    uint32_t block;   /* 4 32bit block size */
    for (block = 0; block < blockLen; block += 4UL)
    {
        /* Write the data to be ciphered to the input data registers. */
        DRV_CRYPTO_AES_WriteInputData(in);
        in += 4;

        /* Wait for the cipher process to end */
        while (!DRV_CRYPTO_AES_CipherIsReady())
        {
            ;
        }  

        if (out != NULL)
        {
            /* Cipher complete - read out the data */
            DRV_CRYPTO_AES_ReadOutputData(out);
            out += 4;
        }
    }
    
    uint32_t numBytes = byteLen % 16UL;
    if (numBytes > 0UL)
    {
        uint32_t partialPlusPad[4] = {0};
        (void) memcpy(partialPlusPad, in, numBytes);
        
        /* Write the data to be ciphered to the input data registers. */
        DRV_CRYPTO_AES_WriteInputData(partialPlusPad);
        
        /* Wait for the cipher process to end */
        while (!DRV_CRYPTO_AES_CipherIsReady())
        {
            ;
        }  

        if (out != NULL)
        {
            uint32_t completeOut[4] = {0};
            
            /* Cipher complete - read out the data */
            DRV_CRYPTO_AES_ReadOutputData(completeOut);
            
            if (numBytes >= 4UL)
            {
                *out++ = completeOut[0];
                if (numBytes >= 8UL)
                {
                    *out++ = completeOut[1];
                    if (numBytes >= 12UL)
                    {
                        *out++ = completeOut[2];
                        if (numBytes > 12UL)
                        {
                            uint32_t tmp = completeOut[3];
                            (void) memcpy(out, &tmp, (numBytes - 12UL));                                                
                        }
                    }
                    else
                    {
                        uint32_t tmp = completeOut[2];
                        (void) memcpy(out, &tmp, (numBytes - 8UL));                    
                    }
                }
                else
                {
                    uint32_t tmp = completeOut[1];
                    (void) memcpy(out, &tmp, (numBytes - 4UL));                    
                }
            }
            else
            {
                uint32_t tmp = completeOut[0];
                (void) memcpy(out, &tmp, numBytes);
            }
        }
    }
}

static void lCrypto_Aead_Hw_Gcm_CmpMsgWithTag(CRYPTO_GCM_HW_CONTEXT *gcmCtx,
    uint8_t *iv, uint32_t ivLen, uint8_t *inData, uint32_t dataLen, 
    uint8_t *outData, uint8_t *aad, uint32_t aadLen, uint8_t *tag, 
    uint32_t tagLen)
{
    /* Calculate the J0 value */
    gcmCtx->invokeCtr[0] = 0x02000000;
    lCrypto_Aead_Hw_Gcm_GenerateJ0(gcmCtx, iv, ivLen);
    
    /* Enable tag generation in driver */
    aesGcmCfg.gtagEn = 1;
    DRV_CRYPTO_AES_SetConfig(&aesGcmCfg);
    
    /* Write the key */
    lCrypto_Aead_Hw_Gcm_WriteKey(gcmCtx->key);
    
    /* Write IV with inc32(J0) (J0 + 1 on 32 bits) */
    lCrypto_Aead_Hw_Gcm_WriteGeneratedIv(gcmCtx);
    
    /* Write lengths */
    DRV_CRYPTO_AES_WriteAuthDataLen(aadLen);
    DRV_CRYPTO_AES_WritePCTextLen(dataLen);
    
    /* MISRA C-2012 deviation block start */
    /* MISRA C-2012 Rule 11.3 deviated: 3. Deviation record ID - H3_MISRAC_2012_R_11_3_DR_1 */
    lCrypto_Aead_Hw_Gcm_RunBlocks((uint32_t *)aad, aadLen, NULL);
    lCrypto_Aead_Hw_Gcm_RunBlocks((uint32_t *)inData, dataLen, 
                                  (uint32_t *)outData);
    /* MISRA C-2012 deviation block end */
    
    if ((aadLen != 0UL) || (dataLen != 0UL))
    {
        /* Wait for the tag to generate */
        while (!DRV_CRYPTO_AES_TagIsReady())
        {
            ;
        }   
    }  
    
    /* MISRA C-2012 deviation block start */
    /* MISRA C-2012 Rule 11.3 deviated: 1. Deviation record ID - H3_MISRAC_2012_R_11_3_DR_1 */
    /* Read the tag */
    DRV_CRYPTO_AES_ReadTag((uint32_t *)tag);
    /* MISRA C-2012 deviation block end */
    
    /* Read hash */
    DRV_CRYPTO_AES_ReadGcmHash(gcmCtx->intermediateHash);
    DRV_CRYPTO_AES_ReadGcmH(gcmCtx->H);
}

static void lCrypto_Aead_Hw_Gcm_1stMsgFrag(CRYPTO_GCM_HW_CONTEXT *gcmCtx,
    uint8_t *iv, uint32_t ivLen, uint8_t *inData, uint32_t dataLen, 
    uint8_t *outData, uint8_t *aad, uint32_t aadLen)
{
    /* Calculate the J0 value */
    gcmCtx->invokeCtr[0] = 0x02000000;
    lCrypto_Aead_Hw_Gcm_GenerateJ0(gcmCtx, iv, ivLen);
    
    /* Disable tag generation in driver */
    aesGcmCfg.gtagEn = 0;
    DRV_CRYPTO_AES_SetConfig(&aesGcmCfg);
    
    /* Write the key */
    lCrypto_Aead_Hw_Gcm_WriteKey(gcmCtx->key);
    
    /* Write IV with inc32(J0) (J0 + 1 on 32 bits) */
    lCrypto_Aead_Hw_Gcm_WriteGeneratedIv(gcmCtx);

    /* Write lengths */
    DRV_CRYPTO_AES_WriteAuthDataLen(aadLen);
    DRV_CRYPTO_AES_WritePCTextLen(dataLen);
    
    /* MISRA C-2012 deviation block start */
    /* MISRA C-2012 Rule 11.3 deviated: 1. Deviation record ID - H3_MISRAC_2012_R_11_3_DR_1 */
    lCrypto_Aead_Hw_Gcm_RunBlocks((uint32_t *)aad, aadLen, NULL);
    lCrypto_Aead_Hw_Gcm_RunBlocks((uint32_t *)inData, dataLen, 
                                  (uint32_t *)outData);
    /* MISRA C-2012 deviation block end */
   
    /* Read hash */
    DRV_CRYPTO_AES_ReadGcmHash(gcmCtx->intermediateHash);
    DRV_CRYPTO_AES_ReadGcmH(gcmCtx->H);
}

static void lCrypto_Aead_Hw_Gcm_MoreMsgFrag(CRYPTO_GCM_HW_CONTEXT *gcmCtx,
    uint8_t *inData, uint32_t dataLen, uint8_t *outData)
{
    /* Disable tag generation in driver */
    aesGcmCfg.gtagEn = 0;
    DRV_CRYPTO_AES_SetConfig(&aesGcmCfg);
    
    /* Write the key */
    lCrypto_Aead_Hw_Gcm_WriteKey(gcmCtx->key);
    
    /* Write IV with inc32(J0) (J0 + 1 on 32 bits) */
    lCrypto_Aead_Hw_Gcm_WriteGeneratedIv(gcmCtx);
    
    /* Write lengths */
    DRV_CRYPTO_AES_WriteAuthDataLen(0);
    DRV_CRYPTO_AES_WritePCTextLen(dataLen);

    /* Load hash */
    DRV_CRYPTO_AES_WriteGcmHash(gcmCtx->intermediateHash);
    
    /* MISRA C-2012 deviation block start */
    /* MISRA C-2012 Rule 11.3 deviated: 2. Deviation record ID - H3_MISRAC_2012_R_11_3_DR_1 */
    lCrypto_Aead_Hw_Gcm_RunBlocks((uint32_t *)inData, dataLen, 
                                  (uint32_t *)outData);
    /* MISRA C-2012 deviation block end */
   
    /* Read hash */
    DRV_CRYPTO_AES_ReadGcmHash(gcmCtx->intermediateHash);
    DRV_CRYPTO_AES_ReadGcmH(gcmCtx->H);
}

static void lCrypto_Aead_Hw_Gcm_GenerateTag(CRYPTO_GCM_HW_CONTEXT *gcmCtx,
    uint32_t dataLen, uint32_t aadLen, uint8_t *tag, uint32_t tagLen)
{
    /* Disable tag generation in driver */
    aesGcmCfg.gtagEn = 0;
    DRV_CRYPTO_AES_SetConfig(&aesGcmCfg);
    
    /* Write the key */
    lCrypto_Aead_Hw_Gcm_WriteKey(gcmCtx->key);
    
    /* Configure authentication data length to 0x10 (16 bytes) */
    /* And plain text length to 0 */
    DRV_CRYPTO_AES_WriteAuthDataLen(0x10);
    DRV_CRYPTO_AES_WritePCTextLen(0);
   
    /* Load hash */
    DRV_CRYPTO_AES_WriteGcmHash(gcmCtx->intermediateHash);
    
    /* Fill input data with lengths in bits */
    aadLen = aadLen * 8UL;
    dataLen = dataLen * 8UL;
    uint32_t lenIn[4] = {0};
    lenIn[1] = (((aadLen << 24U) & 0xFF000000UL) | 
                ((aadLen << 8U) & 0x00FF0000UL) |
                ((aadLen >> 8U) & 0x0000FF00UL) |
                ((aadLen >> 24U) & 0x000000FFUL));
    lenIn[3] = (((dataLen << 24U) & 0xFF000000UL) | 
                ((dataLen << 8U) & 0x00FF0000UL) |
                ((dataLen >> 8U) & 0x0000FF00UL) |
                ((dataLen >> 24U) & 0x000000FFUL));
    
    /* Write the data to be ciphered to the input data registers. */
    DRV_CRYPTO_AES_WriteInputData(lenIn);
        
    /* Wait for the cipher process to end */
    while (!DRV_CRYPTO_AES_CipherIsReady())
    { 
        ;
    }  
    
    /* Read hash */
    DRV_CRYPTO_AES_ReadGcmHash(gcmCtx->intermediateHash);
    DRV_CRYPTO_AES_ReadGcmH(gcmCtx->H);
    
    /* Reset the driver */
    DRV_CRYPTO_AES_Init();
    
    /* Processing T = GCTRK(J0, S) */
    
    /* Configure AES-CTR mode */
    aesGcmCfg.opMode = CRYPTO_AES_MODE_CTR;
    aesGcmCfg.encryptMode = CRYPTO_AES_ENCRYPTION;
    aesGcmCfg.gtagEn = 0;
    DRV_CRYPTO_AES_SetConfig(&aesGcmCfg);
    
    /* Write key */
    DRV_CRYPTO_AES_WriteKey(gcmCtx->key);
    
    /* Write initialization vector with J0 value */
    DRV_CRYPTO_AES_WriteInitVector(gcmCtx->calculatedIv);

    /* Write the data to be ciphered to the input data registers. */
    DRV_CRYPTO_AES_WriteInputData(gcmCtx->intermediateHash);

    /* Wait for the cipher process to end */
    while (!DRV_CRYPTO_AES_CipherIsReady())
    { 
        ;
    }  

    /* Cipher complete - read out the data */
    uint32_t gcmTag[4];
    DRV_CRYPTO_AES_ReadOutputData(gcmTag);
   
    (void) memcpy(tag, (uint8_t*)gcmTag, tagLen);
}

// *****************************************************************************
// *****************************************************************************
// Section: AEAD Algorithms Common Interface Implementation
// *****************************************************************************
// *****************************************************************************

crypto_Aead_Status_E Crypto_Aead_Hw_AesGcm_Init(void *gcmInitCtx,
    crypto_CipherOper_E cipherOper_en, uint8_t *key, uint32_t keyLen)
{
    CRYPTO_GCM_HW_CONTEXT *gcmCtx = (CRYPTO_GCM_HW_CONTEXT*)gcmInitCtx;
    
    /* Initialize the context */
    (void) memset(gcmCtx, 0, sizeof(CRYPTO_GCM_HW_CONTEXT));
    
    /* Get the default configuration from the driver */
    DRV_CRYPTO_AES_GetConfigDefault(&aesGcmCfg);
    
    /* Initialize the driver */
    DRV_CRYPTO_AES_Init();
    
    /* Set configuration in the driver */
    aesGcmCfg.keySize = DRV_CRYPTO_AES_GetKeySize(keyLen / 4UL);
    aesGcmCfg.startMode = CRYPTO_AES_AUTO_START;
    aesGcmCfg.opMode = CRYPTO_AES_MODE_GCM;
    aesGcmCfg.gtagEn = 0;
    if (cipherOper_en == CRYPTO_CIOP_ENCRYPT)
    {
        aesGcmCfg.encryptMode = CRYPTO_AES_ENCRYPTION;
    }
    else 
    {
        aesGcmCfg.encryptMode = CRYPTO_AES_DECRYPTION;
    }
    
    DRV_CRYPTO_AES_SetConfig(&aesGcmCfg);

    /* Store the key */
    uint32_t i;
    for (i = 0; i < (keyLen / 4UL); i++)
    {
        gcmCtx->key[i]  = ((uint32_t) *key++) << 24UL;
        gcmCtx->key[i] += ((uint32_t) *key++) << 16UL;
        gcmCtx->key[i] += ((uint32_t) *key++) << 8UL;
        gcmCtx->key[i] += ((uint32_t) *key++);
    }
    
    return CRYPTO_AEAD_CIPHER_SUCCESS;
}

crypto_Aead_Status_E Crypto_Aead_Hw_AesGcm_Cipher(void *gcmCipherCtx,
    uint8_t *initVect, uint32_t initVectLen, uint8_t *inputData,
    uint32_t dataLen, uint8_t *outData, uint8_t *aad, uint32_t aadLen,
    uint8_t *authTag, uint32_t authTagLen)
{
    CRYPTO_GCM_HW_CONTEXT *gcmCtx = (CRYPTO_GCM_HW_CONTEXT*)gcmCipherCtx;
    
    if (dataLen != 0U || aadLen != 0U)
    {
        if (gcmCtx->invokeCtr[0] == 0UL)
        {
            if (authTag != NULL)
            {
                lCrypto_Aead_Hw_Gcm_CmpMsgWithTag(gcmCtx, initVect, initVectLen, 
                    inputData, dataLen, outData, aad, aadLen, authTag, 
                    authTagLen);
                
                return CRYPTO_AEAD_CIPHER_SUCCESS;
            }
            else
            {
                lCrypto_Aead_Hw_Gcm_1stMsgFrag(gcmCtx, initVect, initVectLen,
                    inputData, dataLen, outData, aad, aadLen);
                
                return CRYPTO_AEAD_CIPHER_SUCCESS;
            }        
        }
        
        lCrypto_Aead_Hw_Gcm_MoreMsgFrag(gcmCtx, inputData, dataLen, outData);
    }
    
    if (authTag != NULL)
    {
        if (gcmCtx->invokeCtr[0] == 0UL)
        {
            /* Calculate the J0 value */
            lCrypto_Aead_Hw_Gcm_GenerateJ0(gcmCtx, initVect, initVectLen);
        }
        
        lCrypto_Aead_Hw_Gcm_GenerateTag(gcmCtx, dataLen, aadLen, authTag, 
                authTagLen);
    }
    
    return CRYPTO_AEAD_CIPHER_SUCCESS;
}
 
crypto_Aead_Status_E Crypto_Aead_Hw_AesGcm_EncryptAuthDirect(uint8_t *inputData, 
    uint32_t dataLen, uint8_t *outData, uint8_t *key, uint32_t keyLen, 
    uint8_t *initVect, uint32_t initVectLen, uint8_t *aad, uint32_t aadLen, 
    uint8_t *authTag, uint32_t authTagLen)
{
    CRYPTO_GCM_HW_CONTEXT gcmCtx;
    crypto_Aead_Status_E result;
    
    result = Crypto_Aead_Hw_AesGcm_Init(&gcmCtx, CRYPTO_CIOP_ENCRYPT, key, keyLen);
    if (result != CRYPTO_AEAD_CIPHER_SUCCESS)
    {
        return result;
    }
    
    return Crypto_Aead_Hw_AesGcm_Cipher(&gcmCtx, initVect, initVectLen, inputData, 
            dataLen, outData, aad, aadLen, authTag, authTagLen);
}
 
crypto_Aead_Status_E Crypto_Aead_Hw_AesGcm_DecryptAuthDirect(uint8_t *inputData, 
    uint32_t dataLen, uint8_t *outData, uint8_t *key, uint32_t keyLen, 
    uint8_t *initVect, uint32_t initVectLen, uint8_t *aad, uint32_t aadLen, 
    uint8_t *authTag, uint32_t authTagLen)
{
    CRYPTO_GCM_HW_CONTEXT gcmCtx;
    crypto_Aead_Status_E result;
    
    result = Crypto_Aead_Hw_AesGcm_Init(&gcmCtx, CRYPTO_CIOP_DECRYPT, key, keyLen);
    if (result != CRYPTO_AEAD_CIPHER_SUCCESS)
    {
        return result;
    }
    
    return Crypto_Aead_Hw_AesGcm_Cipher(&gcmCtx, initVect, initVectLen, inputData, 
            dataLen, outData, aad, aadLen, authTag, authTagLen);
}
