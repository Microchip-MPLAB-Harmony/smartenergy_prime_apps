/*******************************************************************************
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    crypto_mac_wc_wrapper.h

  Summary:
    This header file provides prototypes and definitions for the application.

  Description:
    This header file provides function prototypes and data type definitions for
    the application.  Some of these are required by the system (such as the
    "APP_Initialize" and "APP_Tasks" prototypes) and some of them are only used
    internally by the application (such as the "APP_STATES" definition).  Both
    are defined here for convenience.
*******************************************************************************/

#ifndef CRYPTO_MAC_WC_WRAPPER_H
#define CRYPTO_MAC_WC_WRAPPER_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************


// *****************************************************************************
// *****************************************************************************
// Section: Type Definitions
// *****************************************************************************
crypto_Mac_Status_E Crypto_Mac_Wc_AesCmac_Init(void *ptr_aesCmacCtx, uint8_t *ptr_key, uint32_t keySize);
crypto_Mac_Status_E Crypto_Mac_Wc_AesCmac_Cipher(void *ptr_aesCmacCtx, uint8_t *ptr_inputData, uint32_t dataLen);
crypto_Mac_Status_E Crypto_Mac_Wc_AesCmac_Final(void *ptr_aesCmacCtx, uint8_t *ptr_outMac, uint32_t macLen);
crypto_Mac_Status_E Crypto_Mac_Wc_AesCmac_Direct(uint8_t *ptr_inputData, uint32_t inuptLen, uint8_t *ptr_outMac, uint32_t macLen, uint8_t *ptr_key, uint32_t keyLen);
        
#endif //CRYPTO_MAC_WC_WRAPPER_H