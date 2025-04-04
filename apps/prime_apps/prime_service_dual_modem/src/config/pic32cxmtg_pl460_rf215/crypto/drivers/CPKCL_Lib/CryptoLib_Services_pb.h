/**************************************************************************
  Crypto Framework Library Source

  Company:
    Microchip Technology Inc.

  File Name:
    CryptoLib_Services_pb.h

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

#ifndef CRYPTOLIB_SERVICES_PB_INCLUDED
#define CRYPTOLIB_SERVICES_PB_INCLUDED

// Services definition
#define CPKCL_SERVICE_SelfTest                    		0xA0
#define CPKCL_SERVICE_Rng                         		0xA1
#define CPKCL_SERVICE_Smult                       		0xA2
#define CPKCL_SERVICE_Comp                        		0xA3
#define CPKCL_SERVICE_Fmult                       		0xA4
#define CPKCL_SERVICE_Square                      		0xA5
#define CPKCL_SERVICE_Swap                        		0xA6
#define CPKCL_SERVICE_FastCopy                    		0xA7
#define CPKCL_SERVICE_CondCopy                    		0xA8
#define CPKCL_SERVICE_ClearFlags                  		0xA9
#define CPKCL_SERVICE_Fill                        		0xAA
#define CPKCL_SERVICE_NOP                         		0xAB
//=======================================================
#define CPKCL_SERVICE_Hash                        		0xB0
//=======================================================

#define CPKCL_SERVICE_Div                         		0xC0
#define CPKCL_SERVICE_CRT                         		0xC1
#define CPKCL_SERVICE_PrimeGen                    		0xC2
#define CPKCL_SERVICE_ExpMod                      		0xC3
#define CPKCL_SERVICE_GCD                         		0xC4
#define CPKCL_SERVICE_RedMod                      		0xC5
//=======================================================
#define CPKCL_SERVICE_ZpEcRandomiseCoordinate     		0xE0
#define CPKCL_SERVICE_ZpEcDsaVerifyFast           		0xE2
#define CPKCL_SERVICE_ZpEccDblFast                		0xE3
#define CPKCL_SERVICE_ZpEccMulFast                		0xE8
#define CPKCL_SERVICE_ZpEcConvAffineToProjective  		0xE9
#define CPKCL_SERVICE_ZpEcDsaGenerateFast         		0xEA
#define CPKCL_SERVICE_ZpEccAddFast                		0xEB
#define CPKCL_SERVICE_ZpEcPointIsOnCurve          		0xEC
#define CPKCL_SERVICE_ZpEcConvProjToAffine        		0xED
//=======================================================
#define CPKCL_SERVICE_GF2NEcRandomiseCoordinate     	0x90
#define CPKCL_SERVICE_GF2NEcDsaVerifyFast           	0x92
#define CPKCL_SERVICE_GF2NEccDblFast                	0x93
#define CPKCL_SERVICE_GF2NEccMulFast                	0x98
#define CPKCL_SERVICE_GF2NEcConvAffineToProjective  	0x99
#define CPKCL_SERVICE_GF2NEcDsaGenerateFast         	0x9A
#define CPKCL_SERVICE_GF2NEccAddFast                	0x9B
#define CPKCL_SERVICE_GF2NEcPointIsOnCurve          	0x9C
#define CPKCL_SERVICE_GF2NEcConvProjToAffine        	0x9D


#endif // CRYPTOLIB_SERVICES_PB_INCLUDED

