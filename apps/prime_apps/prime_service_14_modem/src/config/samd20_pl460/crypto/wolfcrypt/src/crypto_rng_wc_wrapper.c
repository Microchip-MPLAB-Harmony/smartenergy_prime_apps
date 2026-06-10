/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    crypto_rng_wc_wrapper.c

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
 
// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
#include "crypto/common_crypto/crypto_common.h"
#include "crypto/common_crypto/crypto_rng.h"
#include "crypto/wolfcrypt/crypto_rng_wc_wrapper.h"
#include "wolfssl/wolfcrypt/random.h"
#include "wolfssl/wolfcrypt/error-crypt.h"

crypto_Rng_Status_E Crypto_Rng_Wc_Prng_GenerateBlock(uint8_t* ptr_rngData, uint32_t rngLen, uint8_t* ptr_nonce, uint32_t nonceLen)
{
    crypto_Rng_Status_E ret_rngStat_en = CRYPTO_RNG_ERROR_FAIL;
    int wcStat = -1, wcFreeStat = -1;
    WC_RNG rng_st = {0};
        
    wcStat = wc_InitRngNonce(&rng_st, ptr_nonce, nonceLen);
    
    if (wcStat == 0)
    {
        wcStat = wc_RNG_GenerateBlock(&rng_st, ptr_rngData, rngLen);
    }

    wcFreeStat = wc_FreeRng(&rng_st);
    
    if (wcStat == 0 && wcFreeStat == 0)
    {
        ret_rngStat_en = CRYPTO_RNG_SUCCESS;
    }
    else if (wcStat == BAD_FUNC_ARG)
    {
        ret_rngStat_en = CRYPTO_RNG_ERROR_ARG;
    }
    else
    {
        ret_rngStat_en = CRYPTO_RNG_ERROR_FAIL;
    }
    
    return ret_rngStat_en;
}
