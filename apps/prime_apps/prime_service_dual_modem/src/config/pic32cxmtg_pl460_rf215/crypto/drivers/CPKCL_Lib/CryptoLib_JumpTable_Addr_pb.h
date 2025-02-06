/**************************************************************************
  Crypto Framework Library Source

  Company:
    Microchip Technology Inc.

  File Name:
    CryptoLib_JumpTable_Addr_pb.h

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

#ifndef CRYPTOLIB_JUMPTABLE_ADDR_PB_INCLUDED_
#define CRYPTOLIB_JUMPTABLE_ADDR_PB_INCLUDED_

#include "CryptoLib_typedef_pb.h"
#include "CryptoLib_mapping_pb.h"
#include "CryptoLib_Headers_pb.h"

/* MISRA C-2012 deviation block start */
/* MISRA C-2012 Rule 8.2 deviated: 1. Deviation record ID - H3_MISRAC_2012_R_8_2_DR_1 */
typedef void (*PPKCL_FUNC) (PCPKCL_PARAM);
/* MISRAC 2012 deviation block end */

// JumpTable address + 1 as it is thumb code
#define vCPKCLCsJumpTableStart                0x02020001
#define vCPKCLCsNOP							      ((PPKCL_FUNC)(vCPKCLCsJumpTableStart + 0x00 ))
#define vCPKCLCsSelfTest						  ((PPKCL_FUNC)(vCPKCLCsJumpTableStart + 0x04 ))
#define vCPKCLCsFill          				((PPKCL_FUNC)(vCPKCLCsJumpTableStart + 0x08 ))
#define vCPKCLCsSwap          				((PPKCL_FUNC)(vCPKCLCsJumpTableStart + 0x0c ))
#define vCPKCLCsFastCopy      				((PPKCL_FUNC)(vCPKCLCsJumpTableStart + 0x10 ))
#define vCPKCLCsCondCopy      				((PPKCL_FUNC)(vCPKCLCsJumpTableStart + 0x14 ))
#define vCPKCLCsClearFlags    				((PPKCL_FUNC)(vCPKCLCsJumpTableStart + 0x18 ))
#define vCPKCLCsComp          				((PPKCL_FUNC)(vCPKCLCsJumpTableStart + 0x1c ))
#define vCPKCLCsFmult         				((PPKCL_FUNC)(vCPKCLCsJumpTableStart + 0x20 ))
#define vCPKCLCsSmult         				((PPKCL_FUNC)(vCPKCLCsJumpTableStart + 0x24 ))
#define vCPKCLCsSquare        				((PPKCL_FUNC)(vCPKCLCsJumpTableStart + 0x28 ))
#define vCPKCLCsDiv           				((PPKCL_FUNC)(vCPKCLCsJumpTableStart + 0x2c ))
#define vCPKCLCsRedMod        				((PPKCL_FUNC)(vCPKCLCsJumpTableStart + 0x30 ))
#define vCPKCLCsExpMod        				((PPKCL_FUNC)(vCPKCLCsJumpTableStart + 0x34 ))
#define vCPKCLCsCRT           				((PPKCL_FUNC)(vCPKCLCsJumpTableStart + 0x38 ))
#define vCPKCLCsGCD           				((PPKCL_FUNC)(vCPKCLCsJumpTableStart + 0x3c ))
#define vCPKCLCsPrimeGen      				((PPKCL_FUNC)(vCPKCLCsJumpTableStart + 0x40 ))
#define vCPKCLCsZpEcPointIsOnCurve			((PPKCL_FUNC)(vCPKCLCsJumpTableStart + 0x44 ))
#define vCPKCLCsZpEcRandomiseCoordinate       ((PPKCL_FUNC)(vCPKCLCsJumpTableStart + 0x48 ))
#define vCPKCLCsZpEcConvAffineToProjective    ((PPKCL_FUNC)(vCPKCLCsJumpTableStart + 0x4c ))
#define vCPKCLCsZpEccAddFast                  ((PPKCL_FUNC)(vCPKCLCsJumpTableStart + 0x50 ))
#define vCPKCLCsZpEccDblFast                  ((PPKCL_FUNC)(vCPKCLCsJumpTableStart + 0x54 ))
#define vCPKCLCsZpEccMulFast                  ((PPKCL_FUNC)(vCPKCLCsJumpTableStart + 0x58 ))
#define vCPKCLCsZpEcDsaGenerateFast           ((PPKCL_FUNC)(vCPKCLCsJumpTableStart + 0x5c ))
#define vCPKCLCsZpEcDsaVerifyFast             ((PPKCL_FUNC)(vCPKCLCsJumpTableStart + 0x60 ))
#define vCPKCLCsZpEcConvProjToAffine			((PPKCL_FUNC)(vCPKCLCsJumpTableStart + 0x64 ))
#define vCPKCLCsGF2NEcRandomiseCoordinate	  	((PPKCL_FUNC)(vCPKCLCsJumpTableStart + 0x68 ))
#define vCPKCLCsGF2NEcConvProjToAffine       	((PPKCL_FUNC)(vCPKCLCsJumpTableStart + 0x6c ))
#define vCPKCLCsGF2NEcConvAffineToProjective 	((PPKCL_FUNC)(vCPKCLCsJumpTableStart + 0x70 ))
#define vCPKCLCsGF2NEcPointIsOnCurve         	((PPKCL_FUNC)(vCPKCLCsJumpTableStart + 0x74 ))
#define vCPKCLCsGF2NEccAddFast               	((PPKCL_FUNC)(vCPKCLCsJumpTableStart + 0x78 ))
#define vCPKCLCsGF2NEccDblFast               	((PPKCL_FUNC)(vCPKCLCsJumpTableStart + 0x7C ))
#define vCPKCLCsGF2NEccMulFast               	((PPKCL_FUNC)(vCPKCLCsJumpTableStart + 0x80 ))
#define vCPKCLCsGF2NEcDsaGenerateFast        	((PPKCL_FUNC)(vCPKCLCsJumpTableStart + 0x84 ))
#define vCPKCLCsGF2NEcDsaVerifyFast          	((PPKCL_FUNC)(vCPKCLCsJumpTableStart + 0x88 ))
#define vCPKCLCsRng         					((PPKCL_FUNC)(vCPKCLCsJumpTableStart + 0x8C ))


#endif // CRYPTOLIB_JUMPTABLE_ADDR_PB_INCLUDED_
