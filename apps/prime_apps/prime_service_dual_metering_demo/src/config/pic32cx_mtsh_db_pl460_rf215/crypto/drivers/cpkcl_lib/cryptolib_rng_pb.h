/**************************************************************************
  Crypto Framework Library Source

  Company:
    Microchip Technology Inc.

  File Name:
    CryptoLib_Rng_pb.h

  Summary:
    Crypto Framework Library interface file for hardware Cryptography.

  Description:
    This file provides an example for interfacing with the CPKCC module.
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

#ifndef CRYPTOLIB_RNG_PB_INCLUDED
#define CRYPTOLIB_RNG_PB_INCLUDED

// Structure definition
typedef struct struct_PKCL_rng {
               nu1       nu1XKeyBase;        // Pointer to the Input and Output XKEY value of length u2XKeyLength bytes
               nu1       nu1WorkSpace;       // Pointer to the workspace of length 64 bytes
               u2        u2XKeyLength;       // Length in bytes multiple of 4 of XKEY, XSEED[0], XSEED[1]

               nu1       nu1XSeedBase;       // Pointer to the Input value of XSEED[0] and XSEED[1] of length (2*u1XKeyLength + 4)
               nu1       nu1WorkSpace2;		 // Pointer to the WorkSpace2 Of SHA (HICM)
               nu1       nu1QBase;           // Pointer to the Input prime number Q of length 160 bits = 20 bytes
               nu1       nu1RBase;           // (Significant length of 'N' without the padding word)
               u2        u2RLength;          // length of the resulting RNG
               u2        u2X9_31Rounds;
               u2        padding1;
               } CPKCL_RNG_STRUCT, *PCPKCL_RNG_STRUCT;

// Options definition
#define CPKCL_RNG_SEED              0x01
#define CPKCL_RNG_GET               0x02
#define CPKCL_RNG_GETSEED           0x03
#define CPKCL_RNG_X931_GET          0x04


#endif //CRYPTOLIB_RNG_PB_INCLUDED

