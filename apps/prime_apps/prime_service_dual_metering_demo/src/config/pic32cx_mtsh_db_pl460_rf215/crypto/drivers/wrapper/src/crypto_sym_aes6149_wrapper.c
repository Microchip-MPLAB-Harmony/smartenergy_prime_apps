/**************************************************************************
  Crypto Framework Library Source

  Company:
    Microchip Technology Inc.

  File Name:
    crypto_sym_aes6149_wrapper.c

  Summary:
    Crypto Framework Library wrapper file for hardware AES.

  Description:
    This source file contains the wrapper interface to access the symmetric 
    AES algorithms in the AES hardware driver for Microchip microcontrollers.
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
#include "crypto/drivers/wrapper/crypto_sym_aes6149_wrapper.h"
#include "crypto/drivers/driver/drv_crypto_aes_hw_6149.h"

// *****************************************************************************
// *****************************************************************************
// Section: File scope functions
// *****************************************************************************
// *****************************************************************************

static crypto_Sym_Status_E lCrypto_Sym_Hw_Aes_GetOperationMode
    (crypto_Sym_OpModes_E opMode, CRYPTO_AES_OPERATION_MODE* aesMode, 
     CRYPTO_AES_CFB_SIZE* cfbSize)
{
    *cfbSize = CRYPTO_AES_CFB_SIZE_128BIT; // Default if not used
    crypto_Sym_Status_E retStat = CRYPTO_SYM_CIPHER_SUCCESS;
    switch (opMode) 
    {
        case CRYPTO_SYM_OPMODE_ECB:
            *aesMode = CRYPTO_AES_MODE_ECB;
            break;
        case CRYPTO_SYM_OPMODE_INVALID:
            retStat = CRYPTO_SYM_ERROR_CIPNOTSUPPTD;
            break;        
        default:
            retStat = CRYPTO_SYM_ERROR_CIPNOTSUPPTD;
            break;
    }
    
    return retStat;
}
    
// *****************************************************************************
// *****************************************************************************
// Section: Symmetric Common Interface Implementation
// *****************************************************************************
// *****************************************************************************

crypto_Sym_Status_E Crypto_Sym_Hw_Aes_Init(crypto_CipherOper_E cipherOpType_en, 
    crypto_Sym_OpModes_E opMode_en, uint8_t *key, uint32_t keyLen, 
    uint8_t *initVect)
{ 
    CRYPTO_AES_CONFIG aesCfg;
    CRYPTO_AES_OPERATION_MODE opMode = CRYPTO_AES_MODE_ECB;
    CRYPTO_AES_CFB_SIZE cfbSize;
    crypto_Sym_Status_E result;
        
    /* Get operation mode for driver */
    result = lCrypto_Sym_Hw_Aes_GetOperationMode(opMode_en, &opMode, &cfbSize);
    if (result != CRYPTO_SYM_CIPHER_SUCCESS)
    {
        return result;
    }
    
    /* Get the default configuration of the driver */
    DRV_CRYPTO_AES_GetConfigDefault(&aesCfg);
    
    /* Initialize the driver */
    DRV_CRYPTO_AES_Init();
    
    /* Set the configuration for the driver */
    aesCfg.keySize = DRV_CRYPTO_AES_GetKeySize(keyLen / 4UL);
    aesCfg.startMode = CRYPTO_AES_AUTO_START;
    aesCfg.opMode = opMode;
    aesCfg.cfbSize = cfbSize;
    if (cipherOpType_en == CRYPTO_CIOP_ENCRYPT)
    {
        aesCfg.encryptMode = CRYPTO_AES_ENCRYPTION;
    }
    else 
    {
        aesCfg.encryptMode = CRYPTO_AES_DECRYPTION;
    }
    
    DRV_CRYPTO_AES_SetConfig(&aesCfg);
    
    /* MISRA C-2012 deviation block start */
    /* MISRA C-2012 Rule 11.3 deviated: 1. Deviation record ID - H3_MISRAC_2012_R_11_3_DR_1 */
    /* Write the key */
    DRV_CRYPTO_AES_WriteKey((uint32_t *)key);
    /* MISRA C-2012 deviation block end */
    
    /* Write the initialization vector */
    if (initVect != NULL)
    {
        /* MISRA C-2012 deviation block start */
        /* MISRA C-2012 Rule 11.3 deviated: 1. Deviation record ID - H3_MISRAC_2012_R_11_3_DR_1 */
        DRV_CRYPTO_AES_WriteInitVector((uint32_t *) initVect);
        /* MISRA C-2012 deviation block end */
    }
    
    return CRYPTO_SYM_CIPHER_SUCCESS;
}
    
crypto_Sym_Status_E Crypto_Sym_Hw_Aes_Cipher(uint8_t *inputData, 
    uint32_t dataLen, uint8_t *outData)
{
    DRV_CRYPTO_AES_WritePCTextLen(dataLen);
    
    /* MISRA C-2012 deviation block start */
    /* MISRA C-2012 Rule 11.3 deviated: 2. Deviation record ID - H3_MISRAC_2012_R_11_3_DR_1 */
    uint32_t *iData = (uint32_t *)inputData;
    uint32_t *oData = (uint32_t *)outData;
    /* MISRA C-2012 deviation block end */
    uint32_t fullBlocks = dataLen / 16UL;
    uint32_t remainder  = dataLen % 16UL;
    uint32_t block;
    
    /* Process all complete 16-byte blocks */
    for (block = 0; block < fullBlocks; block++)
    {
        /* Write the data to be ciphered to the input data registers */
        DRV_CRYPTO_AES_WriteInputData(iData);
        iData += 4;

        /* Wait for the cipher process to end */
        while (!DRV_CRYPTO_AES_CipherIsReady())
        {
            ;
        }   

        /* Cipher complete - read out the data */
        DRV_CRYPTO_AES_ReadOutputData(oData);
        oData += 4;
    }
    
    /* Process trailing partial block for stream modes (CTR, OFB, CFB-128).
     * Copy remaining bytes into a zero-padded temp buffer, process one full
     * block through hardware, then copy only the valid output bytes back. */
    if (remainder != 0UL)
    {
        uint8_t tempIn[16]  = {0};
        uint8_t tempOut[16] = {0};
        
        (void) memcpy(tempIn, (const uint8_t *)iData, remainder);
        
        /* MISRA C-2012 deviation block start */
        /* MISRA C-2012 Rule 11.3 deviated: 1. Deviation record ID - H3_MISRAC_2012_R_11_3_DR_1 */
        DRV_CRYPTO_AES_WriteInputData((uint32_t *)tempIn);
        
        while (!DRV_CRYPTO_AES_CipherIsReady())
        {
            ;
        }
        
        DRV_CRYPTO_AES_ReadOutputData((uint32_t *)tempOut);
        /* MISRA C-2012 deviation block end */
        
        (void) memcpy((uint8_t *)oData, tempOut, remainder);
    }
    
    return CRYPTO_SYM_CIPHER_SUCCESS;
}

crypto_Sym_Status_E Crypto_Sym_Hw_Aes_EncryptDirect(crypto_Sym_OpModes_E opMode_en, 
    uint8_t *inputData, uint32_t dataLen, uint8_t *outData, 
    uint8_t *key, uint32_t keyLen, uint8_t *initVect)
{
    crypto_Sym_Status_E result = CRYPTO_SYM_CIPHER_SUCCESS;
    
    result = Crypto_Sym_Hw_Aes_Init(CRYPTO_CIOP_ENCRYPT, opMode_en, key, 
                                    keyLen, initVect);
                
    if (result != CRYPTO_SYM_CIPHER_SUCCESS)
    {
        return result;
    }
    
    return Crypto_Sym_Hw_Aes_Cipher(inputData, dataLen, outData);
}

crypto_Sym_Status_E Crypto_Sym_Hw_Aes_DecryptDirect(crypto_Sym_OpModes_E opMode_en, 
    uint8_t *inputData, uint32_t dataLen, uint8_t *outData, 
    uint8_t *key, uint32_t keyLen, uint8_t *initVect)
{
    crypto_Sym_Status_E result = CRYPTO_SYM_CIPHER_SUCCESS;
    
    result = Crypto_Sym_Hw_Aes_Init(CRYPTO_CIOP_DECRYPT, opMode_en, key, 
                                    keyLen, initVect);
                
    if (result != CRYPTO_SYM_CIPHER_SUCCESS)
    {
        return result;
    }
    
    return Crypto_Sym_Hw_Aes_Cipher(inputData, dataLen, outData);
}
