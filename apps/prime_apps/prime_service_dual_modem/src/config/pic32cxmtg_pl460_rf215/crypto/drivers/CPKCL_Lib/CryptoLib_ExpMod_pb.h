/**************************************************************************
  Crypto Framework Library Source

  Company:
    Microchip Technology Inc.

  File Name:
    CryptoLib_ExpMod_pb.h

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

#ifndef CRYPTOLIB_EXPMOD_PB_INCLUDED
#define CRYPTOLIB_EXPMOD_PB_INCLUDED

// Structure definition
typedef struct struct_CPKCL_expmod {
               nu1       nu1ModBase;
               nu1       nu1CnsBase;
               u2        u2ModLength;

               nu1       nu1XBase;           // (3*u2NLength + 6) words LSW is always zero
               nu1       nu1PrecompBase;     // xxx words LSW is always zero
               u2        u2ExpLength;
               pfu1      pfu1ExpBase;        // u2ExpLength words
               u1        u1Blinding;         // Exponent blinding using a 32-bits Xor
               u1        padding0;
               u2        padding1;
               } CPKCL_EXPMOD_STRUCT, *PPKCL_EXPMOD_STRUCT;

// Options definition
#define CPKCL_EXPMOD_REGULARRSA      0x01
#define CPKCL_EXPMOD_EXPINPKCCRAM    0x02
#define CPKCL_EXPMOD_FASTRSA         0x04
#define CPKCL_EXPMOD_OPERATIONMASK   0x07
#define CPKCL_EXPMOD_MODEMASK        0x05     // For faults protection

#define CPKCL_EXPMOD_WINDOWSIZE_MASK 0x18
#define CPKCL_EXPMOD_WINDOWSIZE_1    0x00
#define CPKCL_EXPMOD_WINDOWSIZE_2    0x08
#define CPKCL_EXPMOD_WINDOWSIZE_3    0x10
#define CPKCL_EXPMOD_WINDOWSIZE_4    0x18
#define CPKCL_EXPMOD_WINDOWSIZE_BIT(a)   (u2)((a) & CPKCL_EXPMOD_WINDOWSIZE_MASK) >> 3


#endif // CRYPTOLIB_EXPMOD_PB_INCLUDED
