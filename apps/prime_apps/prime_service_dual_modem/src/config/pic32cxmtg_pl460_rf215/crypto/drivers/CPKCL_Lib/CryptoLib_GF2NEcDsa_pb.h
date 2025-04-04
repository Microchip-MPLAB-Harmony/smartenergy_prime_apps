/**************************************************************************
  Crypto Framework Library Source

  Company:
    Microchip Technology Inc.

  File Name:
    CryptoLib_GF2NEcDsa_pb.h

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

#ifndef CRYPTOLIBGF2NECDSA_INCLUDED
#define CRYPTOLIBGF2NECDSA_INCLUDED

// Structure definition
typedef struct struct_CPKCL_GF2NEcDsaGenerate {
               nu1       nu1ModBase;
               nu1       nu1CnsBase;
               u2        u2ModLength;

               nu1       nu1PointABase;
               nu1       nu1PrivateKey;
               nu1       nu1ScalarNumber;
               nu1       nu1OrderPointBase;
               nu1       nu1ABase;
               nu1       nu1Workspace;
               nu1       nu1HashBase;
               u2        u2ScalarLength;
               u2        padding0;
               } CPKCL_GF2NECDSAGENERATE_STRUCT, *P_CPKCL_GF2NECDSAGENERATE_STRUCT;

typedef struct struct_CPKCL_GF2NEcDsaVerify {
               nu1       nu1ModBase;
               nu1       nu1CnsBase;
               u2        u2ModLength;

               nu1       nu1PointABase;
               nu1       nu1PointPublicKeyGen;
               nu1       nu1PointSignature;
               nu1       nu1OrderPointBase;
               nu1       nu1ABase;
               nu1       nu1Workspace;
               nu1       nu1HashBase;
               u2        u2ScalarLength;
               u2        padding0;
               } CPKCL_GF2NECDSAVERIFY_STRUCT, *P_CPKCL_GF2NECDSAVERIFY_STRUCT;



#endif // CRYPTOLIBGF2NEcDSA_INCLUDED
