/*******************************************************************************
  Secure Project System Configuration Header

  File Name:
    wolfcrypt_config.h

  Summary:
    Build-time configuration header for the TrustZone secure system defined by
    this project.

  Description:
    An MPLAB Project may have multiple configurations.  This file defines the
    build-time options for a single configuration.

  Remarks:
    This configuration header must not define any prototypes or data
    definitions (or include any files that do).  It only provides macro
    definitions for build-time configuration options

*******************************************************************************/

/*----------------------------------------------------------------------------
 Copyright (C) 2019-2024 Microchip Technology Inc. and its subsidiaries.

Microchip Technology Inc. and its subsidiaries.

Subject to your compliance with these terms, you may use Microchip software
and any derivatives exclusively with Microchip products. It is your
responsibility to comply with third party license terms applicable to your
use of third party software (including open source software) that may
accompany Microchip software.

THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A PARTICULAR
PURPOSE.

IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
----------------------------------------------------------------------------*/

#ifndef WOLFCRYPT_CONFIG_H
#define WOLFCRYPT_CONFIG_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

//#include "user.h"
#include "device.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

extern "C" {

#endif
// DOM-IGNORE-END

/*** Crypto Library Configuration ***/

//Crypto V4 Common Crypto API - WolfCrypt Library Support
#define CRYPTO_WOLFCRYPT_SUPPORT_ENABLE

//JK
#define CRYPTO_DIGISIGN_ALGO_EN
#define CRYPTO_KAS_ALGO_EN

/*** wolfCrypt Library Configuration ***/

// ---------- WOLFCRYPT FUNCTIONAL CONFIGURATION START ----------
//
// SAMD20-MINIMAL build. This Service Node only needs, from wolfCrypt:
//   - SHA-256          (FU image hash + Hash_DRBG RNG)
//   - ECDSA P-256      (FU signature verification)
//   - Hash_DRBG RNG    (SRV_RANDOM)
//   - AES-128          (PRIME security: CCM, CMAC, AES key wrap, ECB block)
// Everything else (other SHA/BLAKE/RIPEMD/MD, Camellia/IDEA/HC128/Rabbit/
// ChaCha/DES, AES-192/256 and GCM/XTS/EAX/CFB/OFB/CTR/CBC, ECC>256/SP-384,
// RSA/DH/DSA, X9.63-KDF, compressed keys, cert buffers, TLS/anon) has been
// removed to save flash and, crucially, to shrink the on-stack working set of
// the ECDSA verify (ECC sized for 256 bits instead of 521/4096). RAM on the
// SAMD20J18 is the binding constraint for the FU signature verify.

#define MICROCHIP_PIC32
#define MICROCHIP_MPLAB_HARMONY
#define MICROCHIP_MPLAB_HARMONY_3
#define HAVE_MCAPI

#define SIZEOF_LONG_LONG 8
#define WOLFSSL_USER_IO
#define NO_WRITEV
#define NO_FILESYSTEM
#define NO_INLINE

// Math: single-precision, P-256 only, on-stack (no malloc), small footprint.
#define WOLFSSL_SP_MATH_ALL
#define WOLFSSL_HAVE_SP_ECC
#define WOLFSSL_SP_NO_MALLOC
#define WOLFSSL_SP_SMALL
#define WOLFSSL_SP_NONBLOCK
#define WC_ECC_NONBLOCK

#define WOLFCRYPT_ONLY
#define SINGLE_THREADED
#define NO_ERROR_STRINGS
#define NO_WOLFSSL_MEMORY
#define WC_NO_HARDEN

// Hashes: keep SHA-256 only (on by default). Drop the rest.
#define NO_MD4

// AES-128 only: ECB/direct block, CCM (PRIME 1.4 security), CMAC, key wrap.
#define WOLFSSL_AES_128
#define WOLFSSL_AES_DIRECT
#define HAVE_AES_DECRYPT
#define HAVE_AES_ECB
#define HAVE_AESCCM
#define WOLFSSL_CMAC
#define HAVE_AES_KEYWRAP
#define WOLFSSL_AES_SMALL_TABLES

// ECC: P-256 only (ECDSA verify). Sizing the integers for 256 bits is the
// key stack reduction for wc_ecc_verify_hash_ex.
#define HAVE_ECC
#define ECC_USER_CURVES
#define HAVE_ECC256
#define ECC_SHAMIR
#define FP_MAX_BITS 256

// RNG: Hash_DRBG (SHA-256), seeded by the project entropy source below.
#define HAVE_HASHDRBG
#define NO_DEV_RANDOM

// Public-key / legacy algorithms not used by this node.
#define NO_RSA
#define NO_DH
#define NO_DSA
#define NO_PWDBASED

//*********************************************************
int Crypto_Rng_Wc_Prng_EntropySource(void); //User-modifiable entropy for PRNG
int Crypto_Rng_Wc_Prng_Srand(uint8_t* output, unsigned int sz);

//*********************************************************
#define CUSTOM_RAND_GENERATE_SEED Crypto_Rng_Wc_Prng_Srand
//**********************************************************

// ---------- WOLFCRYPT CONFIGURATION END ----------

//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif // WOLFCRYPT_CONFIG_H
/*******************************************************************************
 End of File
*/
