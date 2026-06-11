/**************************************************************************
  Crypto Framework Library Source

  Company:
    Microchip Technology Inc.

  File Name:
    crypto_hash_sha6156_wrapper.c

  Summary:
    Crypto Framework Library wrapper file for hardware SHA.

  Description:
    This source file contains the wrapper interface to access the SHA 
    hardware driver for Microchip microcontrollers.
**************************************************************************/

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

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include <string.h>
#include "device.h"
#include "crypto/drivers/wrapper/crypto_hash_sha6156_wrapper.h"
#include "crypto/drivers/driver/drv_crypto_sha_hw_6156.h"

// *****************************************************************************
// *****************************************************************************
// Section: Macro definitions
// *****************************************************************************
// *****************************************************************************

#define HASH_SHORT_PAD_SIZE_BYTES     (56U)
#define HASH_LONG_PAD_SIZE_BYTES      (112U)

// *****************************************************************************
// *****************************************************************************
// Section: File scope data
// *****************************************************************************
// *****************************************************************************
       
static uint8_t hashPaddingMsg[CRYPTO_SHA_BLOCK_SIZE_WORDS_32 << 2] = {
    0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

// *****************************************************************************
// *****************************************************************************
// Section: File scope functions
// *****************************************************************************
// *****************************************************************************

static crypto_Hash_Status_E lCrypto_Hash_Hw_Sha_GetAlgorithm
    (crypto_Hash_Algo_E shaAlgorithm, CRYPTO_SHA_ALGO *shaAlgo)
{
    crypto_Hash_Status_E ret_status = CRYPTO_HASH_ERROR_FAIL;
    switch(shaAlgorithm)
    {  
        case CRYPTO_HASH_SHA2_256:
            *shaAlgo = CRYPTO_SHA_ALGO_SHA256;
            ret_status = CRYPTO_HASH_SUCCESS;
            break;
        default:
            ret_status = CRYPTO_HASH_ERROR_ALGO;
            break;
    }    
   return ret_status;
}

static uint32_t lCrypto_Hash_Hw_Sha_GetBlockSizeBytes(crypto_Hash_Algo_E shaAlgorithm)
{
    CRYPTO_SHA_BLOCK_SIZE blockSize = CRYPTO_SHA_BLOCK_SIZE_WORDS_32;
    uint32_t blockLen = ((uint32_t)blockSize) << 2UL; // 128 bytes
    crypto_Hash_Status_E retVal = CRYPTO_HASH_ERROR_FAIL;
    
    CRYPTO_SHA_ALGO shaAlgo = CRYPTO_SHA_ALGO_SHA256;
    retVal = lCrypto_Hash_Hw_Sha_GetAlgorithm(shaAlgorithm, &shaAlgo);
    
    if(retVal == CRYPTO_HASH_SUCCESS)
    {
        if ((shaAlgo == CRYPTO_SHA_ALGO_SHA1) || 
            (shaAlgo == CRYPTO_SHA_ALGO_SHA224) ||
            (shaAlgo == CRYPTO_SHA_ALGO_SHA256))
        {
            blockSize =  CRYPTO_SHA_BLOCK_SIZE_WORDS_16;
            blockLen = ((uint32_t)blockSize) << 2UL; // 64 bytes   
        }
    }
    else
    {
        //do nothing
    }
    
    return blockLen;
}
 
static uint8_t lCrypto_Hash_Hw_Sha_GetPaddingSizeBytes(crypto_Hash_Algo_E shaAlgorithm)
{
    uint8_t paddingLen = HASH_LONG_PAD_SIZE_BYTES;
    crypto_Hash_Status_E retVal = CRYPTO_HASH_ERROR_FAIL;
    
    CRYPTO_SHA_ALGO shaAlgo = CRYPTO_SHA_ALGO_SHA256;
    
    retVal = lCrypto_Hash_Hw_Sha_GetAlgorithm(shaAlgorithm, &shaAlgo);
    if(retVal == CRYPTO_HASH_SUCCESS)
    {
        if ((shaAlgo == CRYPTO_SHA_ALGO_SHA1) || 
            (shaAlgo == CRYPTO_SHA_ALGO_SHA224) ||
            (shaAlgo == CRYPTO_SHA_ALGO_SHA256))
        {
            paddingLen = HASH_SHORT_PAD_SIZE_BYTES;    
        }
    }
    else
    {
        //do nothing
    }
    
    return paddingLen;
}

static CRYPTO_SHA_DIGEST_SIZE lCrypto_Hash_Hw_Sha_GetDigestLen
    (crypto_Hash_Algo_E shaAlgorithm)
{
    CRYPTO_SHA_DIGEST_SIZE digestSize = CRYPTO_SHA_DIGEST_SIZE_INVALID;
    crypto_Hash_Status_E retVal = CRYPTO_HASH_ERROR_FAIL;
    CRYPTO_SHA_ALGO shaAlgo = CRYPTO_SHA_ALGO_SHA256;
    retVal = lCrypto_Hash_Hw_Sha_GetAlgorithm(shaAlgorithm, &shaAlgo);
    
    if(retVal == CRYPTO_HASH_SUCCESS)
    {
        switch(shaAlgo)
        {
            case CRYPTO_SHA_ALGO_SHA1:
                digestSize = CRYPTO_SHA_DISGEST_SIZE_SHA1;
                break;

            case CRYPTO_SHA_ALGO_SHA224:
            case CRYPTO_SHA_ALGO_SHA512_224:
                digestSize = CRYPTO_SHA_DIGEST_SIZE_SHA224;
                break;        

            case CRYPTO_SHA_ALGO_SHA256:
            case CRYPTO_SHA_ALGO_SHA512_256:
                digestSize = CRYPTO_SHA_DIGEST_SIZE_SHA256;
                break;

            case CRYPTO_SHA_ALGO_SHA384:
                digestSize = CRYPTO_SHA_DIGEST_SIZE_SHA384;
                break;

            case CRYPTO_SHA_ALGO_SHA512:
                digestSize = CRYPTO_SHA_DIGEST_SIZE_SHA512;
                break;

            default:
                digestSize = CRYPTO_SHA_DIGEST_SIZE_INVALID;
                break;
        }
    }
    
   return digestSize;
}
 
// *****************************************************************************
// *****************************************************************************
// Section: Hash Algorithms Common Interface Implementation
// *****************************************************************************
// *****************************************************************************
    
crypto_Hash_Status_E Crypto_Hash_Hw_Sha_Init(void *shaInitCtx, 
    crypto_Hash_Algo_E shaAlgorithm_en)
{
    CRYPTO_SHA_ALGO shaAlgo;
    crypto_Hash_Status_E result;
    uint8_t *retAdr = NULL;
    CRYPTO_HASH_HW_CONTEXT *shaCtx = (CRYPTO_HASH_HW_CONTEXT*)shaInitCtx;
            
    /* Set algorithm for driver */
    result = lCrypto_Hash_Hw_Sha_GetAlgorithm(shaAlgorithm_en, &shaAlgo);
    if (result != CRYPTO_HASH_SUCCESS)
    {
        return result;
    }
    
    /* Initialize context */
    shaCtx->algo = shaAlgorithm_en;
    shaCtx->totalLen = 0;
    retAdr = memset(shaCtx->buffer, 0, sizeof(shaCtx->buffer));

    if(retAdr == NULL)
    {
        return CRYPTO_HASH_ERROR_FAIL;
    }
    
    /* Configure the driver */
    DRV_CRYPTO_SHA_Init(shaAlgo);
    
    return CRYPTO_HASH_SUCCESS;
}

crypto_Hash_Status_E Crypto_Hash_Hw_Sha_Update(void *shaUpdateCtx,
    uint8_t *data, uint32_t dataLen)
{
    uint32_t *localBuffer = NULL;
    uint32_t fill;
    uint32_t left;
    uint32_t blockSizeBytes;
    uint32_t tempWords;
    CRYPTO_SHA_BLOCK_SIZE blockSizeWords;
    CRYPTO_HASH_HW_CONTEXT *shaCtx = (CRYPTO_HASH_HW_CONTEXT*)shaUpdateCtx;
    
    blockSizeBytes = lCrypto_Hash_Hw_Sha_GetBlockSizeBytes(shaCtx->algo);
    tempWords = (blockSizeBytes >> 2UL);
    blockSizeWords = (CRYPTO_SHA_BLOCK_SIZE)tempWords;
    
    left = ((uint32_t)(shaCtx->totalLen)) & ((uint32_t)(blockSizeBytes - 1UL));
    fill = blockSizeBytes - left;
    shaCtx->totalLen += dataLen;

    if ((left != 0U) && (dataLen >= fill))
    {
        (void) memcpy((shaCtx->buffer + left), data, fill);
        
        /* MISRA C-2012 deviation block start */
        /* MISRA C-2012 Rule 11.3 deviated: 1. Deviation record ID - H3_MISRAC_2012_R_11_3_DR_1 */
        /* Load message */
        localBuffer = (uint32_t *)shaCtx->buffer;
        /* MISRA C-2012 deviation block end */
        
        DRV_CRYPTO_SHA_Update(localBuffer, blockSizeWords);
        data += fill;
        dataLen -= fill;
        left = 0;
    }

    while (dataLen >= blockSizeBytes)
    {
        /* MISRA C-2012 deviation block start */
        /* MISRA C-2012 Rule 11.3 deviated: 1. Deviation record ID - H3_MISRAC_2012_R_11_3_DR_1 */
        /* Load message */
        localBuffer = (uint32_t *)data;
        /* MISRA C-2012 deviation block end */
        DRV_CRYPTO_SHA_Update(localBuffer, blockSizeWords);
        data += blockSizeBytes;
        dataLen -= blockSizeBytes; 
    }

    if (dataLen > 0U)
    {
        (void) memcpy((shaCtx->buffer + left), data, dataLen);
    }

    return CRYPTO_HASH_SUCCESS;
}

crypto_Hash_Status_E Crypto_Hash_Hw_Sha_Final(void *shaFinalCtx, 
    uint8_t *digest)
{
    uint32_t blockSizeBytes;
    uint32_t last;
    uint8_t lenMsg[16] = {0};
    uint8_t paddingSizeBytes;
    uint8_t paddingLen;
    /* MISRA C-2012 deviation block start */
    /* MISRA C-2012 Rule 11.3 deviated: 1. Deviation record ID - H3_MISRAC_2012_R_11_3_DR_1 */
    uint32_t *ptr_digest = (uint32_t*)digest;
    /* MISRA C-2012 deviation block end */
    CRYPTO_SHA_DIGEST_SIZE digestLen;
    crypto_Hash_Status_E retVal = CRYPTO_HASH_ERROR_FAIL;
    CRYPTO_HASH_HW_CONTEXT *shaCtx = (CRYPTO_HASH_HW_CONTEXT*)shaFinalCtx;
   
    blockSizeBytes = lCrypto_Hash_Hw_Sha_GetBlockSizeBytes(shaCtx->algo);
    paddingSizeBytes = lCrypto_Hash_Hw_Sha_GetPaddingSizeBytes(shaCtx->algo);
   
    /* Get the number of bits before padding */    
    uint64_t totalBits = shaCtx->totalLen << 3;
    
    /* Add padding */
    last = ((uint32_t)(shaCtx->totalLen)) & ((uint32_t)(blockSizeBytes - 1UL));
    if (last < paddingSizeBytes)
    { 
        paddingLen = (uint8_t) (paddingSizeBytes - (uint8_t)last);
    }
    else 
    {
        paddingLen = (uint8_t)blockSizeBytes + (uint8_t)(paddingSizeBytes - last);
    }

    retVal = Crypto_Hash_Hw_Sha_Update(shaCtx, (uint8_t*)&hashPaddingMsg[0], paddingLen);
    
    /* Create the message bit length block */
    if (paddingSizeBytes == HASH_SHORT_PAD_SIZE_BYTES)
    {
        lenMsg[0] = (uint8_t)(totalBits >> 56U);
        lenMsg[1] = (uint8_t)(totalBits >> 48U);
        lenMsg[2] = (uint8_t)(totalBits >> 40U);
        lenMsg[3] = (uint8_t)(totalBits >> 32U);
        lenMsg[4] = (uint8_t)(totalBits >> 24U);
        lenMsg[5] = (uint8_t)(totalBits >> 16U);
        lenMsg[6] = (uint8_t)(totalBits >>  8U);
        lenMsg[7] = (uint8_t)(totalBits);   
    
        retVal = Crypto_Hash_Hw_Sha_Update(shaCtx, lenMsg, 8);    
    }
    else 
    {
        lenMsg[8] = (uint8_t)(totalBits >> 56U);
        lenMsg[9] = (uint8_t)(totalBits >> 48U);
        lenMsg[10] = (uint8_t)(totalBits >> 40U);
        lenMsg[11] = (uint8_t)(totalBits >> 32U);
        lenMsg[12] = (uint8_t)(totalBits >> 24U);
        lenMsg[13] = (uint8_t)(totalBits >> 16U);
        lenMsg[14] = (uint8_t)(totalBits >>  8U);
        lenMsg[15] = (uint8_t)(totalBits);
        
        retVal = Crypto_Hash_Hw_Sha_Update(shaCtx, lenMsg, 16);   
    }

    digestLen = lCrypto_Hash_Hw_Sha_GetDigestLen(shaCtx->algo);
    DRV_CRYPTO_SHA_GetOutputData(ptr_digest, digestLen);

    return retVal;
}

crypto_Hash_Status_E Crypto_Hash_Hw_Sha_Digest(uint8_t *data, uint32_t dataLen, 
    uint8_t *digest, crypto_Hash_Algo_E shaAlgorithm_en)
{
    CRYPTO_HASH_HW_CONTEXT shaCtx;
    crypto_Hash_Status_E result = CRYPTO_HASH_SUCCESS;

    result = Crypto_Hash_Hw_Sha_Init(&shaCtx, shaAlgorithm_en);
    if (result != CRYPTO_HASH_SUCCESS)
    {
        return result;
    }
    
    result = Crypto_Hash_Hw_Sha_Update(&shaCtx, data, dataLen);
    if (result != CRYPTO_HASH_SUCCESS)
    {
        return result;
    }    
    return Crypto_Hash_Hw_Sha_Final(&shaCtx, digest);
}
