/**************************************************************************
  Crypto Framework Library Source

  Company:
    Microchip Technology Inc.

  File Name:
    crypto_hash_sha6156_wrapper.h

  Summary:
    Crypto Framework Library wrapper file for hardware SHA.

  Description:
    This header file contains the wrapper interface to access the SHA 
    hardware driver for Microchip microcontrollers.
**************************************************************************/

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

#ifndef CRYPTO_HASH_SHA6156_WRAPPER_H
#define CRYPTO_HASH_SHA6156_WRAPPER_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include "crypto/common_crypto/crypto_common.h"
#include "crypto/common_crypto/crypto_hash.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    extern "C" {

#endif
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Data Types
// *****************************************************************************
// *****************************************************************************

typedef struct 
{
    uint64_t totalLen;   /* Number of bytes to be processed  */
    crypto_Hash_Algo_E algo;
    uint8_t buffer[128]; /* Maximum size for all */
} CRYPTO_HASH_HW_CONTEXT;

// *****************************************************************************
// *****************************************************************************
// Section: Hash Algorithms Common Interface 
// *****************************************************************************
// *****************************************************************************

crypto_Hash_Status_E Crypto_Hash_Hw_Sha_Digest(uint8_t *data, uint32_t dataLen, 
    uint8_t *digest, crypto_Hash_Algo_E shaAlgorithm_en);
    
crypto_Hash_Status_E Crypto_Hash_Hw_Sha_Init(void *shaInitCtx, 
    crypto_Hash_Algo_E shaAlgorithm_en);

crypto_Hash_Status_E Crypto_Hash_Hw_Sha_Update(void *shaUpdateCtx, 
    uint8_t *data, uint32_t dataLen);

crypto_Hash_Status_E Crypto_Hash_Hw_Sha_Final(void *shaFinalCtx, 
    uint8_t *digest);

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    }

#endif
// DOM-IGNORE-END

#endif /* CRYPTO_HASH_SHA6156_WRAPPER_H */
