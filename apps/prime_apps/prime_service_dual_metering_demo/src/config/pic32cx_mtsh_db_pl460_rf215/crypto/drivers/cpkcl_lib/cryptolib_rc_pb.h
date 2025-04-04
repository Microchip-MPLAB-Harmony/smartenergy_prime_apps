/**************************************************************************
  Crypto Framework Library Source

  Company:
    Microchip Technology Inc.

  File Name:
    CryptoLib_Rc_pb.h

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

#ifndef CRYPTOLIB_RC_PB_INCLUDED
#define CRYPTOLIB_RC_PB_INCLUDED




// Standard Return and Severity Codes
#define CPKCL_SEVERE(a)                           (a | 0xC000)
#define CPKCL_WARNING(a)                          (a | 0x8000)
#define CPKCL_INFO(a)                             (a | 0x4000)
#define CPKCL_SEVERITY_MASK(a)                    (a | 0xC000)

// Generic Return Codes
#define CPKCL_OK                                   0x0000
#define CPKCL_COMPUTATION_NOT_STARTED              CPKCL_SEVERE(0x0001)
#define CPKCL_UNKNOWN_SERVICE                      CPKCL_SEVERE(0x0002)
#define CPKCL_UNEXPLOITABLE_OPTIONS                CPKCL_SEVERE(0x0003)
#define CPKCL_HARDWARE_ISSUE                       CPKCL_SEVERE(0x0004)
#define CPKCL_WRONG_HARDWARE                       CPKCL_SEVERE(0x0005)
#define CPKCL_LIBRARY_MALFORMED                    CPKCL_SEVERE(0x0006)
#define CPKCL_ERROR                                CPKCL_SEVERE(0x0007)
#define CPKCL_UNKNOWN_SUBSERVICE                   CPKCL_SEVERE(0x0008)

// Preliminary tests Return Codes (when not in release)
#define CPKCL_OVERLAP_NOT_ALLOWED                  CPKCL_SEVERE(0x0010)
#define CPKCL_PARAM_NOT_IN_CPKCCRAM                 CPKCL_SEVERE(0x0011)
#define CPKCL_PARAM_NOT_IN_RAM                     CPKCL_SEVERE(0x0012)
#define CPKCL_PARAM_NOT_IN_CPURAM                  CPKCL_SEVERE(0x0013)
#define CPKCL_PARAM_WRONG_LENGTH                   CPKCL_SEVERE(0x0014)
#define CPKCL_PARAM_BAD_ALIGNEMENT                 CPKCL_SEVERE(0x0015)
#define CPKCL_PARAM_X_BIGGER_THAN_Y                CPKCL_SEVERE(0x0016)
#define CPKCL_PARAM_LENGTH_TOO_SMALL               CPKCL_SEVERE(0x0017)

// Run time errors (even when in release)
#define CPKCL_DIVISION_BY_ZERO                     CPKCL_SEVERE(0x0101)
#define CPKCL_MALFORMED_MODULUS                    CPKCL_SEVERE(0x0102)
#define CPKCL_FAULT_DETECTED                       CPKCL_SEVERE(0x0103)
#define CPKCL_MALFORMED_KEY                        CPKCL_SEVERE(0x0104)
#define CPKCL_APRIORI_OK                           CPKCL_SEVERE(0x0105)
#define CPKCL_WRONG_SERVICE                        CPKCL_SEVERE(0x0106)

// Run time events (not obviously severe)
#define CPKCL_POINT_AT_INFINITY                    CPKCL_WARNING(0x0001)
#define CPKCL_WRONG_SIGNATURE                      CPKCL_WARNING(0x0002)
#define CPKCL_WRONG_SELECTNUMBER                   CPKCL_WARNING(0x0003)
#define CPKCL_POINT_IS_NOT_ON_CURVE                CPKCL_WARNING(0x0004)
// Run time informations (even when in release)
#define CPKCL_NUMBER_IS_NOT_PRIME                  CPKCL_INFO  (0x0001)
#define CPKCL_NUMBER_IS_PRIME                      CPKCL_INFO  (0x0002)

#endif // CRYPTOLIB_RC_PB_INCLUDED
