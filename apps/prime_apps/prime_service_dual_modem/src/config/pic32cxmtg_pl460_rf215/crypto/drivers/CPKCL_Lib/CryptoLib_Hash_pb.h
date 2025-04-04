/**************************************************************************
  Crypto Framework Library Source

  Company:
    Microchip Technology Inc.

  File Name:
    CryptoLib_Hash_pb.h

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

#ifndef CRYPTOLIB_HASH_INCLUDED
#define CRYPTOLIB_HASH_INCLUDED

// Structure definition
// POUR MD2 :	Le CheckSum se fait dans le Working Space;
// 				Le Buffer X est assimile a une valeur de hashage donc place dans pu1HBase
typedef struct struct_CPKCL_hash {
               pu1       pu1WorkSpace;
               pu1       pu1MBase;
               pu1       pu1HBase;
               pu1       pu1WorkSpace2;
               u2        padding1;
               u2        padding2;
               } CPKCL_HASH_STRUCT, *PPKCL_HASH_STRUCT;

// bits of the option:
// (b1,b0) :
//        0x0001 : Init
//        0x0002 : Current
// (b9,b8) :
//        0x0000 : SHA1
//        0x0100 : SHA256
//        0x0200 : SHA384
//        0x0300 : SHA512
// (b2) :
//        0x0000 : MSB First
//        0x0004 : LSB First

// Service definition
// SubServices definition
#define CPKCL_HASH_VOID			   0x00
#define CPKCL_HASH_SHA1			   0x01
#define CPKCL_HASH_MD2			   0x05
#define CPKCL_HASH_MD5			   0x06

// Options definition
// Choose function
#define CPKCL_HASH_INITIALISE       0x0001
#define CPKCL_HASH_UPDATE           0x0002
//#define CPKCL_HASH_???            0x0003
// Choose MSB/LSB
#define CPKCL_HASH_LSBFIRST         0x0008
#define CPKCL_HASH_MSBFIRST         0x0000
#define CPKCL_HASH_MSBLSBFIRST_MASK 0x0008




// SubServices definition
#define CPKCL_HASH_VOID			   0x00
#define CPKCL_HASH_SHA1			   0x01

// Options definition
// Choose function
#define CPKCL_HASH_INITIALISE       0x0001
#define CPKCL_HASH_UPDATE           0x0002
//#define CPKCL_HASH_???            0x0003
// Choose MSB/LSB
#define CPKCL_HASH_LSBFIRST         0x0008
#define CPKCL_HASH_MSBFIRST         0x0000
#define CPKCL_HASH_MSBLSBFIRST_MASK 0x0008
#endif // CRYPTOLIB_HASH_INCLUDED
