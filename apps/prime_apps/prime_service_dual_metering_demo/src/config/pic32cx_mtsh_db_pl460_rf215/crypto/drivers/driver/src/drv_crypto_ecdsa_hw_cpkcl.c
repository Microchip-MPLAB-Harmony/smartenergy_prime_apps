/**************************************************************************
  Crypto Framework Library Header

  Company:
    Microchip Technology Inc.

  File Name:
    drv_crypto_ecdsa_hw_cpkcl.c
  
  Summary:
    Crypto Framework Library source for the CPKCC ECDSA functions.

  Description:
    This source contains the function code for the CPKCC ECDSA.
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

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <string.h>
#include "crypto/drivers/driver/drv_crypto_ecc_hw_cpkcl.h"
#include "crypto/drivers/driver/drv_crypto_ecdsa_hw_cpkcl.h"
#include "crypto/drivers/cpkcl_lib/cryptolib_typedef_pb.h"
#include "crypto/drivers/cpkcl_lib/cryptolib_typedef_pb.h"
#include "crypto/drivers/cpkcl_lib/cryptolib_mapping_pb.h"
#include "crypto/drivers/cpkcl_lib/cryptolib_headers_pb.h"
#include "crypto/drivers/cpkcl_lib/cryptolib_jumptable_addr_pb.h"

// *****************************************************************************
// *****************************************************************************
// Section: File Scope Variables
// *****************************************************************************
// *****************************************************************************

// All buffers maximum size + 4
static u1 pubKeyX[P521_PUBLIC_KEY_COORDINATE_SIZE + 4];    
static u1 pubKeyY[P521_PUBLIC_KEY_COORDINATE_SIZE + 4];  
static u1 localHash[P521_PUBLIC_KEY_COORDINATE_SIZE + 4];    
static u1 privateKey[P521_PUBLIC_KEY_COORDINATE_SIZE + 4]; 
static u1 signX[P521_PUBLIC_KEY_COORDINATE_SIZE + 4];
static u1 signY[P521_PUBLIC_KEY_COORDINATE_SIZE + 4];
    
// *****************************************************************************
// *****************************************************************************
// Section: CPKCL ECDSA Common Interface Implementation
// *****************************************************************************
// *****************************************************************************

CRYPTO_ECDSA_RESULT DRV_CRYPTO_ECDSA_InitEccParamsSign(CPKCL_ECC_DATA *pEccData, 
    pfu1 hash, u4 hashLen, pfu1 privKey, u4 privKeyLen, 
    CRYPTO_CPKCL_CURVE eccCurveType)
{
    CRYPTO_CPKCL_RESULT result;
    
    /* Initialize CPKCL */
    result = DRV_CRYPTO_ECC_InitCpkcl();
    if (result != CRYPTO_CPKCL_RESULT_INIT_SUCCESS) 
    {
        return CRYPTO_ECDSA_RESULT_INIT_FAIL;
    }
    
    /* Fill curve parameters */
    result = DRV_CRYPTO_ECC_InitCurveParams(pEccData, eccCurveType);
    if (result != CRYPTO_CPKCL_RESULT_CURVE_SUCCESS) 
    {
        return CRYPTO_ECDSA_RESULT_ERROR_CURVE;
    }
    
    /* Clean out local buffers */
    (void) memset(localHash, 0, sizeof(localHash));
    (void) memset(privateKey, 0, sizeof(privateKey));
    
    /* Copy leaving first 4 bytes empty */
    (void) memcpy(&localHash[4], hash, hashLen);
    (void) memcpy(&privateKey[4], privKey, privKeyLen);
    
    /* Store in context */
    pEccData->pfu1HashValue = (pfu1) localHash;
    pEccData->pfu1PrivateKey = (pfu1) privateKey;
    
    return CRYPTO_ECDSA_RESULT_SUCCESS;
}

CRYPTO_ECDSA_RESULT DRV_CRYPTO_ECDSA_Sign(CPKCL_ECC_DATA *pEccData, 
    pfu1 pfulSignature, u4 signatureLen)
{
    /* Clean out local buffers */
    (void) memset(signX, 0, sizeof(signX));
    (void) memset(signY, 0, sizeof(signY));
    
    /* Set sizes */
    u2 u2ModuloPSize = pEccData->u2ModuloPSize;
    u2 u2OrderSize = pEccData->u2OrderSize;
    
    /* MISRA C-2012 deviation block start */
    /* MISRA C-2012 Rule 10.1, 10.4, 10.8, 20.7 deviated below. Deviation record ID - 
       H3_MISRAC_2012_R_10_1_DR_1 & H3_MISRAC_2012_R_10_4_DR_1 & H3_MISRAC_2012_R_10_8_DR_1 & H3_MISRAC_2012_R_20_7_DR_1 */
    /* Generate scalar number */
    CPKCL_Rng(nu1RBase) = (nu1) BASE_CONV_RANDOM(u2ModuloPSize);
    /* MISRA C-2012 deviation block end */
    CPKCL_Rng(u2RLength) = u2OrderSize;
    CPKCL(u2Option) = CPKCL_RNG_GET;
    /* MISRA C-2012 deviation block start */
    /* MISRA C-2012 Rule 10.1, 11.1, 20.7 deviated below. Deviation record ID - 
       H3_MISRAC_2012_R_10_1_DR_1 & H3_MISRAC_2012_R_11_1_DR_1 & H3_MISRAC_2012_R_20_7_DR_1 */
    vCPKCL_Process(Rng, pvCPKCLParam);
    /* MISRA C-2012 deviation block end */
    if (CPKCL(u2Status) != (unsigned)CPKCL_OK)
    {
        return CRYPTO_ECDSA_RESULT_ERROR_RNG;
    }
    
    u1 au1ScalarNumber[72];   // u2OrderSize - maximum size
    /* MISRA C-2012 deviation block start */
    /* MISRA C-2012 Rule 10.1, 10.4, 10.8, 20.7 deviated below. Deviation record ID - 
       H3_MISRAC_2012_R_10_1_DR_1 & H3_MISRAC_2012_R_10_4_DR_1 & H3_MISRAC_2012_R_10_8_DR_1 & H3_MISRAC_2012_R_20_7_DR_1 */
    DRV_CRYPTO_ECC_SecureCopy(au1ScalarNumber,
	(pu1) ((BASE_CONV_RANDOM(u2ModuloPSize))), u2OrderSize + 4U);

    /* Copy parameters for ECDSA signature generation in memory areas */
    DRV_CRYPTO_ECC_SecureCopy(
        (pu1) ((BASE_ECDSA_MODULO(u2ModuloPSize, u2OrderSize))),
        pEccData->pfu1ModuloP, u2ModuloPSize + 4U);
    DRV_CRYPTO_ECC_SecureCopy(
        (pu1) ((BASE_ECDSA_POINT_A_X(u2ModuloPSize, u2OrderSize))),
        pEccData->pfu1APointX, u2ModuloPSize + 4U);
    DRV_CRYPTO_ECC_SecureCopy(
        (pu1) ((BASE_ECDSA_POINT_A_Y(u2ModuloPSize, u2OrderSize))),
        pEccData->pfu1APointY, u2ModuloPSize + 4U);
    DRV_CRYPTO_ECC_SecureCopy(
        (pu1) ((BASE_ECDSA_POINT_A_Z(u2ModuloPSize, u2OrderSize))),
        pEccData->pfu1APointZ, u2ModuloPSize + 4U);
    DRV_CRYPTO_ECC_SecureCopy(
        (pu1) ((BASE_ECDSA_A(u2ModuloPSize, u2OrderSize))),
        pEccData->pfu1ACurve, u2ModuloPSize + 4U);
    DRV_CRYPTO_ECC_SecureCopy(
        (pu1) ((BASE_ECDSA_ORDER(u2ModuloPSize, u2OrderSize))),
        pEccData->pfu1APointOrder, u2OrderSize + 4U);
    DRV_CRYPTO_ECC_SecureCopy(
        (pu1) ((BASE_PRIVATE_KEY(u2ModuloPSize, u2OrderSize))),
        pEccData->pfu1PrivateKey, u2OrderSize + 4U);
    DRV_CRYPTO_ECC_SecureCopy(
        (pu1) ((BASE_ECDSA_HASH(u2ModuloPSize, u2OrderSize))),
        pEccData->pfu1HashValue, u2ModuloPSize + 4U);
    DRV_CRYPTO_ECC_SecureCopy(
        (pu1) ((BASE_ECDSA_SCALAR(u2ModuloPSize, u2OrderSize))),
        (pfu1) au1ScalarNumber, u2OrderSize + 4U);

    /* ECC signature */
    /* Ask for a signature generation */
    CPKCL_ZpEcDsaGenerate(nu1ModBase) = (nu1) BASE_ECDSA_MODULO(u2ModuloPSize,
        u2OrderSize);
    CPKCL_ZpEcDsaGenerate(nu1CnsBase) = (nu1) BASE_ECDSA_CNS(u2ModuloPSize,
        u2OrderSize);
    CPKCL_ZpEcDsaGenerate(nu1PointABase) = (nu1) BASE_ECDSA_POINT_A(
        u2ModuloPSize, u2OrderSize);
    CPKCL_ZpEcDsaGenerate(nu1PrivateKey) = (nu1) BASE_PRIVATE_KEY(u2ModuloPSize,
        u2OrderSize);
    CPKCL_ZpEcDsaGenerate(nu1ScalarNumber) = (nu1) BASE_ECDSA_SCALAR(
        u2ModuloPSize, u2OrderSize);
    CPKCL_ZpEcDsaGenerate(nu1OrderPointBase) = (nu1) BASE_ECDSA_ORDER(
        u2ModuloPSize, u2OrderSize);
    CPKCL_ZpEcDsaGenerate(nu1ABase) = (nu1) BASE_ECDSA_A(u2ModuloPSize,
        u2OrderSize);
    CPKCL_ZpEcDsaGenerate(nu1Workspace) = (nu1) BASE_ECDSA_WORKSPACE(
        u2ModuloPSize, u2OrderSize);
    CPKCL_ZpEcDsaGenerate(nu1HashBase) = (nu1) BASE_ECDSA_HASH(u2ModuloPSize,
        u2OrderSize);
    /* MISRA C-2012 deviation block end */
    CPKCL_ZpEcDsaGenerate(u2ModLength) = u2ModuloPSize;
    CPKCL_ZpEcDsaGenerate(u2ScalarLength) = u2OrderSize;

    /* MISRA C-2012 deviation block start */
    /* MISRA C-2012 Rule 10.1, 11.1, 20.7 deviated below. Deviation record ID - 
       H3_MISRAC_2012_R_10_1_DR_1 & H3_MISRAC_2012_R_11_1_DR_1 & H3_MISRAC_2012_R_20_7_DR_1 */
    /* Launch the signature generation */
    /* See CPKCL_Rc_pb.h for possible u2Status Values */
    vCPKCL_Process(ZpEcDsaGenerateFast, pvCPKCLParam);
    /* MISRA C-2012 deviation block end */
    if (CPKCL(u2Status) != (unsigned)CPKCL_OK)
    {
        return CRYPTO_ECDSA_RESULT_ERROR_FAIL;
    }

    /* MISRA C-2012 deviation block start */
    /* MISRA C-2012 Rule 10.1, 10.4, 20.7 deviated below. Deviation record ID - 
       H3_MISRAC_2012_R_10_1_DR_1 & H3_MISRAC_2012_R_10_4_DR_1 & H3_MISRAC_2012_R_20_7_DR_1 */
    /* Copy the result */
    DRV_CRYPTO_ECC_SecureCopy(signX,
        (pu1) ((BASE_ECDSA_POINT_A(u2ModuloPSize, u2OrderSize))),
                u2OrderSize + 4U);
    DRV_CRYPTO_ECC_SecureCopy(signY,
        (pu1) ((BASE_ECDSA_POINT_A(u2ModuloPSize, u2OrderSize)))
                + u2OrderSize + 4U, u2OrderSize + 4U);
    /* MISRA C-2012 deviation block end */
    
    (void) memcpy(pfulSignature, &signX[4], u2OrderSize);
    (void) memcpy(&pfulSignature[u2OrderSize], &signY[4], u2OrderSize);
                    
    return CRYPTO_ECDSA_RESULT_SUCCESS;
}

 CRYPTO_ECDSA_RESULT DRV_CRYPTO_ECDSA_InitEccParamsVerify(CPKCL_ECC_DATA *pEccData, 
    pfu1 hash, u4 hashLen, pfu1 pubKey, CRYPTO_CPKCL_CURVE eccCurveType)
{
    CRYPTO_CPKCL_RESULT result;

   /* Initialize CPKCL */
    result = DRV_CRYPTO_ECC_InitCpkcl();
    if (result != CRYPTO_CPKCL_RESULT_INIT_SUCCESS) 
    {
        return CRYPTO_ECDSA_RESULT_INIT_FAIL;     
    }
    
    /* Fill curve parameters */
    result = DRV_CRYPTO_ECC_InitCurveParams(pEccData, eccCurveType);
    if (result != CRYPTO_CPKCL_RESULT_CURVE_SUCCESS) 
    {
        return CRYPTO_ECDSA_RESULT_ERROR_CURVE;
    }
    
    /* Get coordinates of public key */
    (void) memset(pubKeyX, 0, sizeof(pubKeyX));
    (void) memset(pubKeyY, 0, sizeof(pubKeyY));
    result = DRV_CRYPTO_ECC_SetPubKeyCoordinates(pEccData, pubKey, &pubKeyX[4], 
                                                 &pubKeyY[4], eccCurveType);
    if (result == CRYPTO_CPKCL_RESULT_CURVE_ERROR)
    {
        return CRYPTO_ECDSA_RESULT_ERROR_CURVE;
    }
    else if (result == CRYPTO_CPKCL_RESULT_COORD_COMPRESS_ERROR) 
    {
        return CRYPTO_ECDSA_ERROR_PUBKEYCOMPRESS;
    }
    else 
    {
        // Successful - continue
    }
    
    pEccData->pfu1PublicKeyX = (pfu1) pubKeyX;
    pEccData->pfu1PublicKeyY = (pfu1) pubKeyY;
    
    /* Store hash locally */
    (void) memset(localHash, 0, sizeof(localHash));
    (void) memcpy(&localHash[4], hash, hashLen);
    pEccData->pfu1HashValue = (pfu1) localHash;
    
    return CRYPTO_ECDSA_RESULT_SUCCESS;
}
    
CRYPTO_ECDSA_RESULT DRV_CRYPTO_ECDSA_Verify(CPKCL_ECC_DATA *pEccData, 
    pfu1 pfu1Signature)
{
    pu1 pu1Tmp;
    
    /* Set sizes */
    u2 u2ModuloPSize = pEccData->u2ModuloPSize;	
    u2 u2OrderSize = pEccData->u2OrderSize;
    
    /* Clean out local buffers */
    (void) memset(signX, 0, sizeof(signX));
    (void) memset(signY, 0, sizeof(signY));
    
    /* Copy signature leaving first 4 bytes empty */
    (void) memcpy(&signX[4], pfu1Signature, u2OrderSize);
    (void) memcpy(&signY[4], &pfu1Signature[u2OrderSize], u2OrderSize);

    /* MISRA C-2012 deviation block start */
    /* MISRA C-2012 Rule 10.1, 10.4, 20.7 deviated below. Deviation record ID - 
       H3_MISRAC_2012_R_10_1_DR_1 & H3_MISRAC_2012_R_10_4_DR_1 & H3_MISRAC_2012_R_20_7_DR_1 */
    /* Copy the signature into appropriate memory area */
    /* Take care of the input signature format */
    pu1Tmp = (pu1) ((BASE_ECDSAV_SIGNATURE(u2ModuloPSize, u2OrderSize)));
    /* MISRA C-2012 deviation block end */
    DRV_CRYPTO_ECC_SecureCopy(pu1Tmp, signX, u2OrderSize + 4U);
    DRV_CRYPTO_ECC_SecureCopy(pu1Tmp + u2OrderSize + 4U, signY, u2OrderSize + 4U);

    /* MISRA C-2012 deviation block start */
    /* MISRA C-2012 Rule 10.1, 10.4, 10.8, 20.7 deviated below. Deviation record ID - 
       H3_MISRAC_2012_R_10_1_DR_1 & H3_MISRAC_2012_R_10_4_DR_1 & H3_MISRAC_2012_R_10_8_DR_1 & H3_MISRAC_2012_R_20_7_DR_1 */
    /* Copy parameters for ECDSA signature verification in memory areas */
    DRV_CRYPTO_ECC_SecureCopy(
        (pu1) ((BASE_ECDSAV_MODULO(u2ModuloPSize, u2OrderSize))),
        pEccData->pfu1ModuloP, u2ModuloPSize + 4U);
    DRV_CRYPTO_ECC_SecureCopy(
        (pu1) ((BASE_ECDSAV_POINT_A_X(u2ModuloPSize, u2OrderSize))),
        pEccData->pfu1APointX, u2ModuloPSize + 4U);
    DRV_CRYPTO_ECC_SecureCopy(
        (pu1) ((BASE_ECDSAV_POINT_A_Y(u2ModuloPSize, u2OrderSize))),
        pEccData->pfu1APointY, u2ModuloPSize + 4U);
    DRV_CRYPTO_ECC_SecureCopy(
        (pu1) ((BASE_ECDSAV_POINT_A_Z(u2ModuloPSize, u2OrderSize))),
        pEccData->pfu1APointZ, u2ModuloPSize + 4U);
    DRV_CRYPTO_ECC_SecureCopy(
        (pu1) ((BASE_ECDSAV_A(u2ModuloPSize, u2OrderSize))),
        pEccData->pfu1ACurve, u2ModuloPSize + 4U);
    DRV_CRYPTO_ECC_SecureCopy(
        (pu1) ((BASE_ECDSAV_ORDER(u2ModuloPSize, u2OrderSize))),
        pEccData->pfu1APointOrder, u2OrderSize + 4U);
    DRV_CRYPTO_ECC_SecureCopy(
        (pu1) ((BASE_ECDSAV_PUBLIC_KEY_X(u2ModuloPSize, u2OrderSize))),
        pEccData->pfu1PublicKeyX, u2ModuloPSize + 4U);
    DRV_CRYPTO_ECC_SecureCopy(
        (pu1) ((BASE_ECDSAV_PUBLIC_KEY_Y(u2ModuloPSize, u2OrderSize))),
        pEccData->pfu1PublicKeyY, u2ModuloPSize + 4U);
    DRV_CRYPTO_ECC_SecureCopy(
        (pu1) ((BASE_ECDSAV_PUBLIC_KEY_Z(u2ModuloPSize, u2OrderSize))),
        pEccData->pfu1PublicKeyZ, u2ModuloPSize + 4U);
    DRV_CRYPTO_ECC_SecureCopy(
        (pu1) ((BASE_ECDSAV_HASH(u2ModuloPSize, u2OrderSize))),
        pEccData->pfu1HashValue, u2ModuloPSize + 4U);

    /* Ask for a verification generation */
    CPKCL_ZpEcDsaVerify(nu1ModBase) = (nu1) BASE_ECDSAV_MODULO(u2ModuloPSize,
        u2OrderSize);
    CPKCL_ZpEcDsaVerify(nu1CnsBase) = (nu1) BASE_ECDSAV_CNS(u2ModuloPSize,
        u2OrderSize);
    CPKCL_ZpEcDsaVerify(nu1PointABase) = (nu1) BASE_ECDSAV_POINT_A(
        u2ModuloPSize, u2OrderSize);
    CPKCL_ZpEcDsaVerify(nu1PointPublicKeyGen) = (nu1) BASE_ECDSAV_PUBLIC_KEY(
        u2ModuloPSize, u2OrderSize);
    CPKCL_ZpEcDsaVerify(nu1PointSignature) = (nu1) BASE_ECDSAV_SIGNATURE(
        u2ModuloPSize, u2OrderSize);
    CPKCL_ZpEcDsaVerify(nu1OrderPointBase) = (nu1) BASE_ECDSAV_ORDER(
        u2ModuloPSize, u2OrderSize);
    CPKCL_ZpEcDsaVerify(nu1ABase) = (nu1) BASE_ECDSAV_A(u2ModuloPSize,
        u2OrderSize);
    CPKCL_ZpEcDsaVerify(nu1Workspace) = (nu1) BASE_ECDSAV_WORKSPACE(
        u2ModuloPSize, u2OrderSize);
    CPKCL_ZpEcDsaVerify(nu1HashBase) = (nu1) BASE_ECDSAV_HASH(u2ModuloPSize,
        u2OrderSize);
    /* MISRA C-2012 deviation block end */
    CPKCL_ZpEcDsaVerify(u2ModLength) = u2ModuloPSize;
    CPKCL_ZpEcDsaVerify(u2ScalarLength) = u2OrderSize;

    /* MISRA C-2012 deviation block start */
    /* MISRA C-2012 Rule 10.1, 11.1, 20.7 deviated below. Deviation record ID - 
       H3_MISRAC_2012_R_10_1_DR_1 & H3_MISRAC_2012_R_11_1_DR_1 & H3_MISRAC_2012_R_20_7_DR_1 */
    /* Verify the signature */
    vCPKCL_Process(ZpEcDsaVerifyFast, pvCPKCLParam);
    /* MISRA C-2012 deviation block end */
    if (CPKCL(u2Status) != (unsigned)CPKCL_OK)
    {
        return CRYPTO_ECDSA_RESULT_ERROR_FAIL;
    }

    return CRYPTO_ECDSA_RESULT_SUCCESS;
}
