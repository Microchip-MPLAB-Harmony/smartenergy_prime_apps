/**************************************************************************
  Crypto Framework Library Source

  Company:
    Microchip Technology Inc.

  File Name:
    drv_crypto_sha_hw_6156.c

  Summary:
    Crypto Framework Library interface file for hardware SHA.

  Description:
    This source file contains the interface that make up the SHA hardware 
    driver for the following families of Microchip microcontrollers:
    PIC32CXMTxx, SAMA5D2, SAM9X60, SAMA7D65.
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

#include "definitions.h"
#include "device.h"
#include "../drv_crypto_sha_hw_6156.h"

// *****************************************************************************
// *****************************************************************************
// Section: File scope functions
// *****************************************************************************
// *****************************************************************************

static void lDRV_CRYPTO_SHA_Configure(CRYPTO_SHA_ALGO shaAlgo)
{
    CRYPTO_SHA_MR shaMr = {0};
    CRYPTO_SHA_CR shaCr = {0};
    
    shaMr.s.SMOD = CRYPTO_SHA_AUTO_START;
    shaMr.s.PROCDLY = 0;
    shaMr.s.UIHV = 0; 
    shaMr.s.UIEHV = 0; 
    shaMr.s.ALGO = shaAlgo;
    shaMr.s.DUALBUFF = 0;
    /* MISRA C-2012 deviation block start */
    /* MISRA C-2012 Rule 10.3 deviated: 1. Deviation record ID - H3_MISRAC_2012_R_10_3_DR_1 */
    shaMr.s.CHECK = 0;
    /* MISRA C-2012 deviation block end */
    shaMr.s.CHKCNT = 0; 
    
    shaMr.s.AOE = 0;
    shaMr.s.BPE = 0; 
    shaMr.s.TMPLCK = 0;
  
    SHA_REGS->SHA_MR = shaMr.v;
  
    /* No automatic padding */
    SHA_REGS->SHA_MSR = 0;
    SHA_REGS->SHA_BCR = 0;
    
    /* First message */
    shaCr.s.FIRST = 1; 
    SHA_REGS->SHA_CR = shaCr.v;
}

static void lDRV_CRYPTO_SHA_WriteInputData(uint32_t *inputDataBuffer, 
                                           uint8_t blockSize)
{
    uint8_t i;
    uint8_t len1 = 0;
    uint8_t len2 = 0;
    
    if (blockSize <= (uint8_t)CRYPTO_SHA_BLOCK_SIZE_WORDS_16) 
    {
        len1 = blockSize;
    } 
    else 
    {
        len1 = (uint8_t)CRYPTO_SHA_BLOCK_SIZE_WORDS_16;
        len2 = (uint8_t)CRYPTO_SHA_BLOCK_SIZE_WORDS_16;
    }
	
    for (i = 0; i < len1; i++) 
    {
        SHA_REGS->SHA_IDATAR[i] = *inputDataBuffer;
        inputDataBuffer++;
    }

    if (len2 != 0U) 
    {
        for (i = 0; i < len2; i++) 
        {
            SHA_REGS->SHA_IODATAR[i] = *inputDataBuffer;
            inputDataBuffer++;
        }
    }
}

static void lDRV_CRYPTO_SHA_ReadOutputData(uint32_t *outputDataBuffer, 
    uint8_t bufferLen)
{   
    uint8_t i;
    
    for (i = 0; i < bufferLen; i++) 
    {
        *outputDataBuffer = SHA_REGS->SHA_IODATAR[i];
        outputDataBuffer++;
    }    
}

// *****************************************************************************
// *****************************************************************************
// Section: SHA256 Common Interface Implementation
// *****************************************************************************
// *****************************************************************************

void DRV_CRYPTO_SHA_Init(CRYPTO_SHA_ALGO shaAlgo)
{
    /* Software reset */
    SHA_REGS->SHA_CR = SHA_CR_SWRST_Msk;
    
    /* Set all the fields needed to set-up the SHA engine */
    lDRV_CRYPTO_SHA_Configure(shaAlgo);
}

void DRV_CRYPTO_SHA_Update(uint32_t *data, CRYPTO_SHA_BLOCK_SIZE dataBlockSize)
{
    /* Write the data to be hashed to the input data registers */
    lDRV_CRYPTO_SHA_WriteInputData(data, (uint8_t)dataBlockSize);
    
    /* Block until processing is done */
    while ((SHA_REGS->SHA_ISR & SHA_ISR_DATRDY_Msk) == 0U)
    {
        ;
    }
}

void DRV_CRYPTO_SHA_GetOutputData(uint32_t *digest, 
    CRYPTO_SHA_DIGEST_SIZE digestLen)
{
    /* Read the output (clear interruption) */
    lDRV_CRYPTO_SHA_ReadOutputData(digest, (uint8_t)digestLen);
}
