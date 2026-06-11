/**************************************************************************
  Crypto Framework Library Source

  Company:
    Microchip Technology Inc.

  File Name:
    CryptoLib_Headers_pb.h

  Summary:
    Crypto Framework Library interface file for hardware Cryptography.

  Description:
    This file provides an example for interfacing with the CPKCC module.
**************************************************************************/

/*******************************************************************************
* Copyright (C) 2026 Microchip Technology Inc. and its subsidiaries.
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

#ifndef CRYPTOLIB_HEADERS_PB_INCLUDED
#define CRYPTOLIB_HEADERS_PB_INCLUDED

// Include the services headers ...
#include "cryptolib_rc_pb.h"
#include "cryptolib_clearflags_pb.h"
#include "cryptolib_comp_pb.h"
#include "cryptolib_condcopy_pb.h"
#include "cryptolib_crt_pb.h"
#include "cryptolib_div_pb.h"
#include "cryptolib_expmod_pb.h"
#include "cryptolib_fastcopy_pb.h"
#include "cryptolib_fill_pb.h"
#include "cryptolib_fmult_pb.h"
#include "cryptolib_gcd_pb.h"
#include "cryptolib_nop_pb.h"
#include "cryptolib_primegen_pb.h"
#include "cryptolib_redmod_pb.h"
#include "cryptolib_rng_pb.h"
#include "cryptolib_selftest_pb.h"
#include "cryptolib_hash_pb.h"
#include "cryptolib_smult_pb.h"
#include "cryptolib_square_pb.h"
#include "cryptolib_swap_pb.h"

// ECC
#include "cryptolib_zpeccadd_pb.h"
#include "cryptolib_zpeccdbl_pb.h"
#include "cryptolib_zpeccmul_pb.h"
#include "cryptolib_zpeccconv_pb.h"
#include "cryptolib_zpecdsa_pb.h"

#include "cryptolib_gf2neccadd_pb.h"
#include "cryptolib_gf2neccdbl_pb.h"
#include "cryptolib_gf2neccmul_pb.h"
#include "cryptolib_gf2neccconv_pb.h"
#include "cryptolib_gf2necdsa_pb.h"
#include "cryptolib_jumptable_pb.h"
#include "cryptolib_services_pb.h"

/* MISRA C-2012 deviation block start */
/* MISRA C-2012 Rule 6.1 deviated: 6. Deviation record ID - H3_MISRAC_2012_R_6_1_DR_1 */
typedef struct struct_CPKCL_status {
               u4             CarryIn        : 1;
               u4             CarryOut       : 1;
               u4             Zero           : 1;
               u4             Gf2n           : 1;
               u4             Violation      : 1;
               u4            RFU            : (32-5);
               } CPKCL_STATUS,  * PCPKCL_STATUS, * PFCPKCL_STATUS;
/* MISRAC 2012 deviation block end */

typedef struct struct_CPKCL_header {
               u1             u1Service;
               u1             u1SubService;
               u2             u2Option;
               CPKCL_STATUS   Specific;
               u2             u2Status;
               u2             padding0;
               u4             padding1;
               } CPKCL_HEADER,  * PCPKCL_HEADER,  *  PFCPKCL_HEADER;

typedef struct struct_CPKCL_param {
               CPKCL_HEADER    CPKCL_Header;
               union
                    {
                    CPKCL_CLEARFLAGS_STRUCT      CPKCL_ClearFlags_s;
                    CPKCL_COMP_STRUCT            CPKCL_Comp_s;
                    CPKCL_CONDCOPY_STRUCT        CPKCL_CondCopy_s;
                    CPKCL_CRT_STRUCT             CPKCL_CRT_s;
                    CPKCL_DIV_STRUCT             CPKCL_Div_s;
                    CPKCL_EXPMOD_STRUCT          CPKCL_ExpMod_s;
                    CPKCL_FASTCOPY_STRUCT        CPKCL_FastCopy_s;
                    CPKCL_FILL_STRUCT            CPKCL_Fill_s;
                    CPKCL_FMULT_STRUCT           CPKCL_Fmult_s;
                    CPKCL_GCD_STRUCT             CPKCL_GCD_s;
                    CPKCL_NOP_STRUCT             CPKCL_Nop_s;
                    CPKCL_PRIMEGEN_STRUCT        CPKCL_PrimeGen_s;
                    CPKCL_REDMOD_STRUCT          CPKCL_RedMod_s;
                    CPKCL_RNG_STRUCT             CPKCL_Rng_s;
                    CPKCL_SELFTEST_STRUCT        CPKCL_SelfTest_s;
                    CPKCL_HASH_STRUCT            CPKCL_Hash_s;
                    CPKCL_SMULT_STRUCT           CPKCL_Smult_s;
                    CPKCL_SQUARE_STRUCT          CPKCL_Square_s;
                    CPKCL_SWAP_STRUCT            CPKCL_Swap_s;
                    
                    // ECC
                    CPKCL_ZPECCADD_STRUCT               CPKCL_ZpEccAdd_s;
                    CPKCL_ZPECCDBL_STRUCT               CPKCL_ZpEccDbl_s;
                    CPKCL_ZPECCMUL_STRUCT               CPKCL_ZpEccMul_s;
                    CPKCL_ZPECDSAGENERATE_STRUCT        CPKCL_ZpEcDsaGenerate_s;
                    CPKCL_ZPECDSAVERIFY_STRUCT          CPKCL_ZpEcDsaVerify_s;
                    CPKCL_ZPECCONVPROJTOAFFINE_STRUCT         CPKCL_ZpEcConvProjToAffine_s;
                    CPKCL_ZPECCONVAFFINETOPROJECTIVE_STRUCT   CPKCL_ZpEcConvAffineToProj_s;
                    CPKCL_ZPECRANDOMIZECOORDINATE_STRUCT      CPKCL_ZpEcRandomiseCoordinate_s;
                    CPKCL_ZPECPOINTISONCURVE_STRUCT           CPKCL_ZpEcPointIsOnCurve_s;

                    // ECC
					CPKCL_GF2NECCADD_STRUCT                     CPKCL_GF2NEccAdd_s;
					CPKCL_GF2NECCDBL_STRUCT                     CPKCL_GF2NEccDbl_s;
					CPKCL_GF2NECCMUL_STRUCT                     CPKCL_GF2NEccMul_s;
					CPKCL_GF2NECDSAGENERATE_STRUCT              CPKCL_GF2NEcDsaGenerate_s;
					CPKCL_GF2NECDSAVERIFY_STRUCT                CPKCL_GF2NEcDsaVerify_s;
					CPKCL_GF2NECCONVPROJTOAFFINE_STRUCT         CPKCL_GF2NEcConvProjToAffine_s;
					CPKCL_GF2NECCONVAFFINETOPROJECTIVE_STRUCT   CPKCL_GF2NEcConvAffineToProj_s;
					CPKCL_GF2NECRANDOMIZECOORDINATE_STRUCT	    CPKCL_GF2NEcRandomiseCoord_s;
					CPKCL_GF2NECPOINTISONCURVE_STRUCT           CPKCL_GF2NEcPointIsOnCurve_s;

                    } P;
               //u4   __Padding0;
               } CPKCL_PARAM, * PCPKCL_PARAM, * PFCPKCL_PARAM;

// CPKCL helpers
#define DEF_PARAM                          pvoid pvCPKCLParam
#define GET_PARAM()
#define USE_PARAM                          (PCPKCL_PARAM)pvCPKCLParam

#define CPKCL(a)                           (USE_PARAM)->CPKCL_Header.a

#define CPKCL_ClearFlags(a)                (USE_PARAM)->P.CPKCL_ClearFlags_s.a
#define CPKCL_Comp(a)                      (USE_PARAM)->P.CPKCL_Comp_s.a
#define CPKCL_CondCopy(a)                  (USE_PARAM)->P.CPKCL_CondCopy_s.a
#define CPKCL_CRT(a)                       (USE_PARAM)->P.CPKCL_CRT_s.a
#define CPKCL_Div(a)                       (USE_PARAM)->P.CPKCL_Div_s.a
#define CPKCL_ExpMod(a)                    (USE_PARAM)->P.CPKCL_ExpMod_s.a
#define CPKCL_FastCopy(a)                  (USE_PARAM)->P.CPKCL_FastCopy_s.a
#define CPKCL_Fill(a)                      (USE_PARAM)->P.CPKCL_Fill_s.a
#define CPKCL_Fmult(a)                     (USE_PARAM)->P.CPKCL_Fmult_s.a
#define CPKCL_GCD(a)                       (USE_PARAM)->P.CPKCL_GCD_s.a
#define CPKCL_NOP(a)                       (USE_PARAM)->P.CPKCL_NOP_s.a
#define CPKCL_PrimeGen(a)                  (USE_PARAM)->P.CPKCL_PrimeGen_s.a
#define CPKCL_RedMod(a)                    (USE_PARAM)->P.CPKCL_RedMod_s.a
#define CPKCL_Rng(a)                       (USE_PARAM)->P.CPKCL_Rng_s.a
#define CPKCL_SelfTest(a)                  (USE_PARAM)->P.CPKCL_SelfTest_s.a
#define CPKCL_Hash(a)                      (USE_PARAM)->P.CPKCL_Hash_s.a
#define CPKCL_Smult(a)                     (USE_PARAM)->P.CPKCL_Smult_s.a
#define CPKCL_Square(a)                    (USE_PARAM)->P.CPKCL_Square_s.a
#define CPKCL_Swap(a)                      (USE_PARAM)->P.CPKCL_Swap_s.a

#define CPKCL_ZpEccAdd(a)                  (USE_PARAM)->P.CPKCL_ZpEccAdd_s.a
#define CPKCL_ZpEccDbl(a)                  (USE_PARAM)->P.CPKCL_ZpEccDbl_s.a
#define CPKCL_ZpEccMul(a)                  (USE_PARAM)->P.CPKCL_ZpEccMul_s.a
#define CPKCL_ZpEcDsaGenerate(a)           (USE_PARAM)->P.CPKCL_ZpEcDsaGenerate_s.a
#define CPKCL_ZpEcDsaVerify(a)             (USE_PARAM)->P.CPKCL_ZpEcDsaVerify_s.a
#define CPKCL_ZpEcConvProjToAffine(a)      (USE_PARAM)->P.CPKCL_ZpEcConvProjToAffine_s.a
#define CPKCL_ZpEcConvAffineToProjective(a)(USE_PARAM)->P.CPKCL_ZpEcConvAffineToProj_s.a
#define CPKCL_ZpEcRandomiseCoordinate(a)   (USE_PARAM)->P.CPKCL_ZpEcRandomiseCoordinate_s.a
#define CPKCL_ZpEcPointIsOnCurve(a)        (USE_PARAM)->P.CPKCL_ZpEcPointIsOnCurve_s.a

#define CPKCL_GF2NEccAdd(a)                  (USE_PARAM)->P.CPKCL_GF2NEccAdd_s.a
#define CPKCL_GF2NEccDbl(a)                  (USE_PARAM)->P.CPKCL_GF2NEccDbl_s.a
#define CPKCL_GF2NEccMul(a)                  (USE_PARAM)->P.CPKCL_GF2NEccMul_s.a
#define CPKCL_GF2NEcDsaGenerate(a)           (USE_PARAM)->P.CPKCL_GF2NEcDsaGenerate_s.a
#define CPKCL_GF2NEcDsaVerify(a)             (USE_PARAM)->P.CPKCL_GF2NEcDsaVerify_s.a
#define CPKCL_GF2NEcConvProjToAffine(a)      (USE_PARAM)->P.CPKCL_GF2NEcConvProjToAffine_s.a
#define CPKCL_GF2NEcConvAffineToProjective(a)(USE_PARAM)->P.CPKCL_GF2NEcConvAffineToProj_s.a
#define CPKCL_GF2NEcRandomiseCoordinate(a)   (USE_PARAM)->P.CPKCL_GF2NEcRandomiseCoord_s.a
#define CPKCL_GF2NEcPointIsOnCurve(a)        (USE_PARAM)->P.CPKCL_GF2NEcPointIsOnCurve_s.a

// Services options helpers
#define MULTIPLIEROPTION_MASK      0x0003
#define CARRYOPTION_MASK           0x00fc
#define REDUCTIONOPTION_MASK       0xff00

// Common carry options to all services supporting arithmetic operations
// These two definitions are internal only
#define FORCE_CARRYIN              0x10
#define FORCE_NOCARRYIN            0x08

// These definitions are available for final user use
#define MISC_COMMAND               0x00
#define ADD_CARRY                  0x01
#define SUB_CARRY                  0x02
#define ADD_1_PLUS_CARRY           0x03
#define ADD_1_MINUS_CARRY          0x04
#define CARRY_NONE                 ADD_CARRY           | FORCE_NOCARRYIN
#define ADD_1                      ADD_CARRY           | FORCE_CARRYIN
#define SUB_1                      SUB_CARRY           | FORCE_CARRYIN
#define ADD_2                      ADD_1_PLUS_CARRY    | FORCE_CARRYIN

// Common multiplier options to all services supporting arithmetic operations
#define MULT_ONLY                  0x01
#define MULT_ADD                   0x02
#define MULT_SUB                   0x03

// Macro enabling to have access to the Carry Options
#define CARRYOPTION()              ((CPKCL(u2Option) & CARRYOPTION_MASK) >> 2)
#define SET_CARRYOPTION(a)         (u2)((a) << 2)

// Macro enabling to have access to the Multiplier Options
#define MULTIPLIEROPTION()         (CPKCL(u2Option) & MULTIPLIEROPTION_MASK)
#define SET_MULTIPLIEROPTION(a)    (u2)(a)

// Macro enabling to have access to the Multiplier Options
#define REDUCTIONOPTION()          ((CPKCL(u2Option) & REDUCTIONOPTION_MASK) >> 8)
#define SET_REDUCTIONOPTION(a)     (u2)((a) << 8)

/* MISRA C-2012 deviation block start */
/* MISRA C-2012 Rule 20.7 deviated below. Deviation record ID - H3_MISRAC_2012_R_20_7_DR_1 */
// Calling a cryptographic service
#define vCPKCL_Process(a,b)     \
          {\
          b->CPKCL_Header.u1Service = CPKCL_SERVICE_##a;\
          b->CPKCL_Header.u2Status  = CPKCL_COMPUTATION_NOT_STARTED;\
          vCPKCLCs##a(b);\
          }
/* MISRA C-2012 deviation block end */
#endif // CRYPTOLIB_HEADERS_PB_INCLUDED


