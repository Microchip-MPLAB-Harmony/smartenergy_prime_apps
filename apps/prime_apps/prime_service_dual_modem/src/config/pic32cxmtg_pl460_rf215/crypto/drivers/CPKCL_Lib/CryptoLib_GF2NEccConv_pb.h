/**************************************************************************
  Crypto Framework Library Source

  Company:
    Microchip Technology Inc.

  File Name:
    CryptoLib_GF2NEccConv_pb.h

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

#ifndef CRYPTOLIBGF2ECNCONV_INCLUDED
#define CRYPTOLIBGF2ECNCONV_INCLUDED

// Structure definition
typedef struct struct_CPKCL_GF2NEcConvProjToAffine {
               nu1       nu1ModBase;
               nu1       nu1CnsBase;
               u2        u2ModLength;

               nu1       nu1PointABase;
               nu1       padding0;
               nu1       nu1Workspace;
               } CPKCL_GF2NECCONVPROJTOAFFINE_STRUCT, *P_CPKCL_GF2NECCONVPROJTOAFFINE_STRUCT;


typedef struct struct_CPKCL_GF2NEcConvAffineToProjective {
               nu1       nu1ModBase;
               nu1       nu1CnsBase;
               u2        u2ModLength;

               nu1       nu1PointABase;
               nu1       padding0;
               nu1       nu1Workspace;
               nu1       padding1;
               nu1       padding2;
               nu1       padding3;
               nu1       padding4;
               } CPKCL_GF2NECCONVAFFINETOPROJECTIVE_STRUCT, *P_CPKCL_GF2NECCONVAFFINETOPROJECTIVE_STRUCT;

typedef struct struct_CPKCL_GF2NEcPointIsOnCurve {
               nu1       nu1ModBase;
               nu1       nu1CnsBase;
               u2        u2ModLength;

               nu1       nu1AParam;
               nu1       nu1BParam;
               nu1       nu1PointBase;
               nu1       nu1Workspace;
               u2        padding0;
               u2        padding1;               
               } CPKCL_GF2NECPOINTISONCURVE_STRUCT, *P_CPKCL_GF2NECPOINTISONCURVE_STRUCT;

typedef struct struct_CPKCL_GF2NEcRandomiseCoordinate {
               nu1       nu1ModBase;
               nu1       nu1CnsBase;
               u2        u2ModLength;

               nu1       nu1PointBase;
               nu1       nu1RandomBase;
               nu1       nu1Workspace;
               nu1       padding0;
               nu1       padding1;
               nu1       padding2;
               nu1       padding3;
               } CPKCL_GF2NECRANDOMIZECOORDINATE_STRUCT, *P_CPKCL_GF2NECRANDOMIZECOORDINATE_STRUCT;



#endif // CRYPTOLIBGF2NECCONV_INCLUDED

