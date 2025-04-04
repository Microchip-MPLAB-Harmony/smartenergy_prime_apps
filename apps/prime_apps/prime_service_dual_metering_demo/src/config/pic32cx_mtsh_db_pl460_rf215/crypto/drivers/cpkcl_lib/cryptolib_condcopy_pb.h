/**************************************************************************
  Crypto Framework Library Source

  Company:
    Microchip Technology Inc.

  File Name:
    CryptoLib_CondCopy_pb.h

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

#ifndef CRYPTOLIB_CONDCOPY_PB_INCLUDED
#define CRYPTOLIB_CONDCOPY_PB_INCLUDED

// Structure definition
typedef struct struct_CPKCL_condcopy {
               nu1       padding0;
               nu1       padding1;
               u2        padding2;

               nu1       nu1XBase;
               u2        padding3;
               u2        padding4;
               nu1       nu1RBase;
               u2        u2RLength;
               u2        u2XLength;
               u2        padding5;
               } CPKCL_CONDCOPY_STRUCT, *PCPKCL_CONDCOPY_STRUCT;

// Options definition
#define CPKCL_CONDCOPY_ALWAYS        0x00
#define CPKCL_CONDCOPY_NEVER         0x01
#define CPKCL_CONDCOPY_IF_CARRY      0x02
#define CPKCL_CONDCOPY_IF_NOT_CARRY  0x03

#endif // CRYPTOLIB_CONDCOPY_PB_INCLUDED
