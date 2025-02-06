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

//DOM-IGNORE-BEGIN
/*
Copyright (C) 2024, Microchip Technology Inc., and its subsidiaries. All rights reserved.

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
