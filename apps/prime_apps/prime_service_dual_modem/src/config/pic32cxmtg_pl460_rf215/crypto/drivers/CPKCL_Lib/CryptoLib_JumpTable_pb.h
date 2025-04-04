/**************************************************************************
  Crypto Framework Library Source

  Company:
    Microchip Technology Inc.

  File Name:
    CryptoLib_JumpTable_pb.h

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

#ifndef CRYPTOLIB_JUMPTABLE_PB_INCLUDED_
#define CRYPTOLIB_JUMPTABLE_PB_INCLUDED_

extern ServiceFctType vCPKCLCsClearFlags_f;
extern ServiceFctType vCPKCLCsComp_f;
extern ServiceFctType vCPKCLCsCondCopy_f;
extern ServiceFctType vCPKCLCsFastCopy_f;
extern ServiceFctType vCPKCLCsFill_f;
extern ServiceFctType vCPKCLCsFmult_f;
extern ServiceFctType vCPKCLCsNOP_f;
extern ServiceFctType vCPKCLCsRng_f;
extern ServiceFctType vCPKCLCsSelfTest_f;
extern ServiceFctType vCPKCLCsSmult_f;
extern ServiceFctType vCPKCLCsSquare_f;
extern ServiceFctType vCPKCLCsSwap_f;
extern ServiceFctType vCPKCLCsZpEccAddFast_f;
extern ServiceFctType vCPKCLCsZpEcConvProjToAffine_f;
extern ServiceFctType vCPKCLCsZpEcPointIsOnCurve_f;
extern ServiceFctType vCPKCLCsZpEcConvAffineToProj_f;
extern ServiceFctType vCPKCLCsZpEcRandomiseCoord_f;
extern ServiceFctType vCPKCLCsZpEccDblFast_f;
extern ServiceFctType vCPKCLCsZpEccMulFast_f;
extern ServiceFctType vCPKCLCsZpEcDsaGenerateFast_f;
extern ServiceFctType vCPKCLCsZpEcDsaVerifyFast_f;
extern ServiceFctType vCPKCLCsGF2NEccAddFast_f;
extern ServiceFctType vCPKCLCsGF2NEcConvProjToAffine_f;
extern ServiceFctType vCPKCLCsGF2NEcPointIsOnCurve_f;
extern ServiceFctType vCPKCLCsGF2NEcRandomiseCoord_f;
extern ServiceFctType vCPKCLCsGF2NEcConvAffineToProj_f;
extern ServiceFctType vCPKCLCsGF2NEccDblFast_f;
extern ServiceFctType vCPKCLCsGF2NEccMulFast_f;
extern ServiceFctType vCPKCLCsGF2NEcDsaGenerateFast_f;
extern ServiceFctType vCPKCLCsGF2NEcDsaVerifyFast_f;
extern ServiceFctType vCPKCLCsCRT_f;
extern ServiceFctType vCPKCLCsDiv_f;
extern ServiceFctType vCPKCLCsExpMod_f;
extern ServiceFctType vCPKCLCsGCD_f;
extern ServiceFctType vCPKCLCsPrimeGen_f;
extern ServiceFctType vCPKCLCsRedMod_f;

#endif  // CRYPTOLIB_JUMPTABLE_PB_INCLUDED_
