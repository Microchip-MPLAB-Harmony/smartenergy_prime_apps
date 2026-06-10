/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    crypto_rng.c

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

//DOM-IGNORE-BEGIN
/*
Copyright (C) 2026 Microchip Technology Inc., and its subsidiaries. All rights reserved.

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

#include "crypto/common_crypto/crypto_common.h"
#include "crypto/common_crypto/crypto_rng.h"
#include "crypto/wolfcrypt/crypto_rng_wc_wrapper.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

#define CRYPTO_RNG_SESSION_MAX (1)

// *****************************************************************************
// *****************************************************************************
// Section: Function Definitions
// *****************************************************************************
// *****************************************************************************

crypto_Rng_Status_E Crypto_Rng_Generate(crypto_HandlerType_E rngHandlerType_en, 
    uint8_t* ptr_rngData, uint32_t rngLen, uint8_t* ptr_nonce, uint32_t nonceLen, 
    uint32_t sessionID)
{
    crypto_Rng_Status_E ret_rngStat_en = CRYPTO_RNG_ERROR_NOTSUPPTED;
    
    if((ptr_rngData == NULL) || (rngLen == 0u))
    {
        ret_rngStat_en = CRYPTO_RNG_ERROR_ARG;
    }
    else if( ((ptr_nonce == NULL) && (nonceLen > 0u))
             || ((ptr_nonce != NULL) && (nonceLen == 0u)) )
    {
        ret_rngStat_en = CRYPTO_RNG_ERROR_NONCE;
    }
    else if( (sessionID == 0u) || (sessionID > (uint32_t)CRYPTO_RNG_SESSION_MAX))
    {
        ret_rngStat_en = CRYPTO_RNG_ERROR_SID;
    }
    else
    {
        switch(rngHandlerType_en)
        {
            case CRYPTO_HANDLER_SW_WOLFCRYPT:
                ret_rngStat_en = Crypto_Rng_Wc_Prng_GenerateBlock(
                    ptr_rngData, rngLen, ptr_nonce, nonceLen);
				break; 
            default:
                ret_rngStat_en = CRYPTO_RNG_ERROR_HDLR;
				break;
        }
    }

    return ret_rngStat_en;
}
