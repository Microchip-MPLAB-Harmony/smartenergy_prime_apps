/*******************************************************************************
  PRIME Hardware Abstraction Layer API Header

  Company:
    Microchip Technology Inc.

  File Name:
    hal_api.h

  Summary:
    PRIME Hardware Abstraction Layer API Header File

  Description:
    This module contains configuration and utils for the interface between the
    services connected to the hardware and the PRIME stack.
*******************************************************************************/

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

#ifndef HAL_API_H_INCLUDE
#define HAL_API_H_INCLUDE

// *****************************************************************************
// *****************************************************************************
// Section: Include Files
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include "service/storage/srv_storage.h"
#include "service/user_pib/srv_user_pib.h"
#include "service/reset_handler/srv_reset_handler.h"
#include "service/firmware_upgrade/srv_firmware_upgrade.h"
#include "service/pcrc/srv_pcrc.h"
#include "service/random/srv_random.h"
#include "service/log_report/srv_log_report.h"
#include "service/usi/srv_usi.h"
#include "service/security/aes_wrapper.h"
#include "service/security/cipher_wrapper.h"
#include "service/queue/srv_queue.h"
#include "stack/pal/pal.h"
#include "stack/pal/pal_types.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    extern "C" {

#endif
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Data Types
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Restart system

  Summary:
    Function pointer to request a system restart.

  Description:
    This function pointer is used to request a system restart.

  Remarks:
    Related to Reset Handler service.
*/
typedef void (*HAL_RESTART_SYSTEM)(SRV_RESET_HANDLER_RESET_CAUSE resetType);

// *****************************************************************************
/* Calculate CRC

  Summary:
    Function pointer to request the calculation of a CRC.

  Description:
    This function pointer is used to request the calculation of a CRC.

  Remarks:
    Related to PCRC service.
*/
typedef uint32_t (*HAL_PCRC_CALCULATE)(uint8_t *pData, size_t length,
    PCRC_HEADER_TYPE hdrType, PCRC_CRC_TYPE crcType, uint32_t initValue);

// *****************************************************************************
/* Configure subnetwork address for CRC calculation

  Summary:
    Function pointer to set the subnetwork address for the calculation of a CRC.

  Description:
    This function pointer is used to set the subnetwork address for the
    calculation of a CRC.

  Remarks:
    Related to PCRC service.
*/
typedef void (*HAL_PCRC_CONFIGURE_SNA)(uint8_t *sna);

// *****************************************************************************
/* Get configuration information

  Summary:
    Function pointer to get configuration information stored externally.

  Description:
    This function pointer is used to get configuration information stored
    externally.

  Remarks:
    Related to PRIME Storage service.
*/
typedef bool (*HAL_GET_CONFIG_INFO)(SRV_STORAGE_TYPE infoType, uint8_t size,
    void *pData);

// *****************************************************************************
/* Set configuration information

  Summary:
    Function pointer to set configuration information stored externally.

  Description:
    This function pointer is used to set configuration information stored
    externally.

  Remarks:
    Related to PRIME Storage service.
*/
typedef bool (*HAL_SET_CONFIG_INFO)(SRV_STORAGE_TYPE infoType, uint8_t size,
    void *pData);

// *****************************************************************************
/* Open an USI instance

  Summary:
    Function pointer to request to open an USI instance.

  Description:
    This function pointer is used to request to open an USI instance.

  Remarks:
    Related to USI service.
*/
typedef SRV_USI_HANDLE (*HAL_USI_OPEN)(const SYS_MODULE_INDEX index);

// *****************************************************************************
/* Register USI callback function

  Summary:
    Function pointer to request the registration of a USI callback function.

  Description:
    This function pointer is used to request the registration of a USI callback
    function.

  Remarks:
    Related to USI service.
*/
typedef void (*HAL_USI_SET_CALLBACK)(SRV_USI_HANDLE handle,
    SRV_USI_PROTOCOL_ID protocol, SRV_USI_CALLBACK callback);

// *****************************************************************************
/* Request to send a USI message

  Summary:
    Function pointer to request the sending of a USI message.

  Description:
    This function pointer is used to request the sending of a USI message.

  Remarks:
    Related to USI service.
*/
typedef size_t (*HAL_USI_SEND)(SRV_USI_HANDLE handle, SRV_USI_PROTOCOL_ID protocol,
    uint8_t *data, size_t length);

// *****************************************************************************
/* Report an error for the debug log

  Summary:
    Function pointer to request the reporting of an error for the debug log.

  Description:
    This function pointer is used to request the reporting of an error for the
    debug log.

  Remarks:
    Related to Log Report service.
*/
typedef void (*HAL_DEBUG_REPORT)(SRV_LOG_REPORT_LEVEL logLevel,
    SRV_LOG_REPORT_CODE code, const char *info, ...);

// *****************************************************************************
/* Get a user PIB attribute

  Summary:
    Function pointer to request a user PIB attribute.

  Description:
    This function pointer is used to request a user PIB attribute.

  Remarks:
    Related to PRIME User PIB service.
*/
typedef void (*HAL_PIB_GET_REQUEST)(uint16_t pibAttrib);

// *****************************************************************************
/* Register a callback function to get a user PIB attribute

  Summary:
    Function pointer to request the registration of the callback function to get
    a user PIB attribute.

  Description:
    This function pointer is used to request the registration of the callback
    function to get a user PIB attribute.

  Remarks:
    Related to PRIME User PIB service.
*/
typedef void (*HAL_PIB_GET_REQUEST_SET_CALLBACK)(SRV_USER_PIB_GET_REQUEST_CALLBACK callback);

// *****************************************************************************
/* Set a user PIB attribute

  Summary:
    Function pointer to set a user PIB attribute.

  Description:
    This function pointer is used to set a user PIB attribute.

  Remarks:
    Related to PRIME User PIB service.
*/
typedef void (*HAL_PIB_SET_REQUEST)(uint16_t pibAttrib, void *pibValue, uint8_t pibSize);

// *****************************************************************************
/* Register a callback function to set a user PIB attribute

  Summary:
    Function pointer to request the registration of the callback function to set
    a user PIB attribute.

  Description:
    This function pointer is used to request the registration of the callback
    function to set a user PIB attribute.

  Remarks:
    Related to PRIME User PIB service.
*/
typedef void (*HAL_PIB_SET_REQUEST_SET_CALLBACK)(SRV_USER_PIB_SET_REQUEST_CALLBACK callback);

// *****************************************************************************
/* Get a random number

  Summary:
    Function pointer to request a random number.

  Description:
    This function pointer is used to request a random number.

  Remarks:
    Related to Random service.
*/
typedef uint32_t (*HAL_RNG_GET)(void);

// *****************************************************************************
/* Perform AES-CMAC

  Summary:
    Function pointer to perform AES-CMAC to generate the MAC in single step
    without initialization.

  Description:
    This function pointer is used to perform AES-CMAC to generate the MAC in
    single step without initialization.

  Remarks:
    Related to Security service.
*/
typedef int32_t (*HAL_AES_CMAC_DIRECT)(uint8_t *input, uint32_t inputLen,
    uint8_t *outputMac, uint8_t *key);

// *****************************************************************************
/* Set the encryption key for AES-CCM

  Summary:
    Function pointer to initialize the AES-CCM context and set the encryption
    key.

  Description:
    This function pointer is used to initialize the AES-CCM context and set the
    16-byte encryption key.

  Remarks:
    Related to Security service.
*/
typedef int32_t (*HAL_AES_CCM_SET_KEY)(uint8_t *key);

// *****************************************************************************
/* Perform AES-CCM authenticated encryption of a buffer

  Summary:
    Function pointer to perform AES-CCM authenticated encryption of a buffer.

  Description:
    This function pointer is used to perform AES-CCM authenticated encryption
    of a buffer.

  Remarks:
    Related to Security service.
*/
typedef int32_t (*HAL_AES_CCM_ENCRYPT_TAG)(uint8_t *data, uint32_t dataLen,
    uint8_t *iv, uint32_t ivLen, uint8_t *aad, uint32_t aadLen, uint8_t *tag,
    uint32_t tagLen);

// *****************************************************************************
/* Perform AES-CCM authenticated decryption of a buffer

  Summary:
    Function pointer to perform AES-CCM authenticated decryption of a buffer.

  Description:
    This function pointer is used to perform AES-CCM authenticated decryption
    of a buffer.

  Remarks:
    Related to Security service.
*/
typedef int32_t (*HAL_AES_CCM_AUTH_DECRYPT)(uint8_t *data, uint32_t dataLen,
    uint8_t *iv, uint32_t ivLen, uint8_t *aad, uint32_t aadLen, uint8_t *tag,
    uint32_t tagLen);

// *****************************************************************************
/* Wrap a key with AES Key Wrap Algorithm

  Summary:
    Function pointer to wrap a key with AES Key Wrap Algorithm.

  Description:
    This function pointer is used to wrap a key using AES Key Wrap Algorithm.

  Remarks:
    Related to Security service.
*/
typedef void (*HAL_AES_WRAP_KEY)(uint8_t *key, uint32_t keyLen, uint8_t *in,
    uint32_t inLen, uint8_t *out);

// *****************************************************************************
/* Unwrap a key with AES Key Wrap Algorithm

  Summary:
    Function pointer to unwrap a key with AES Key Wrap Algorithm.

  Description:
    This function pointer is used to unwrap a key using AES Key Wrap Algorithm.

  Remarks:
    Related to Security service.
*/
typedef bool (*HAL_AES_UNWRAP_KEY)(uint8_t *key, uint32_t keyLen, uint8_t *in,
    uint32_t inLen, uint8_t *out);

//***************************************************************************
/* Queue initialization

  Summary:
    Function pointer to initialize a queue.

  Description:
    This function pointer is used to initialize a queue.

  Remarks:
    Related to Queue service.
*/
typedef void (*HAL_QUEUE_INIT)(SRV_QUEUE *queue, uint16_t capacity,
    SRV_QUEUE_TYPE type);

//***************************************************************************
/* Queue append

  Summary:
    Function pointer to append an element into a queue.

  Description:
    This function pointer is used to append an element into a queue.

  Remarks:
    Related to Queue service.
*/
typedef void (*HAL_QUEUE_APPEND)(SRV_QUEUE *queue, SRV_QUEUE_ELEMENT *element);

//***************************************************************************
/* Queue append with priority

  Summary:
    Function pointer to append an element into a priority queue.

  Description:
    This function pointer is used to append an element into a priority queue.

  Remarks:
    Related to Queue service.
*/
typedef void (*HAL_QUEUE_APPEND_WITH_PRIORITY)(SRV_QUEUE *queue, uint32_t priority,
    SRV_QUEUE_ELEMENT *element);

//***************************************************************************
/* Insert before

  Summary:
    Function pointer to insert an element into a queue before a given element.

  Description:
    This function pointer is used to insert an element into a queue before a
    given element.

  Remarks:
    Related to Queue service.
*/
typedef void (*HAL_QUEUE_INSERT_BEFORE)(SRV_QUEUE *queue,
    SRV_QUEUE_ELEMENT *currentElement, SRV_QUEUE_ELEMENT *element);

//***************************************************************************
/* Insert after

  Summary:
    Function pointer to insert an element into a queue after a given element.

  Description:
    This function pointer is used to insert an element into a queue after a
    given element.

  Remarks:
    Related to Queue service.
*/
typedef void (*HAL_QUEUE_INSERT_AFTER)(SRV_QUEUE *queue,
    SRV_QUEUE_ELEMENT *currentElement, SRV_QUEUE_ELEMENT *element);

//***************************************************************************
/* Read or remove

  Summary:
    Function pointer to read or remove an element from a queue.

  Description:
    This function pointer is used to read or remove an element from a queue.

  Remarks:
    Related to Queue service.
*/
typedef SRV_QUEUE_ELEMENT *(*HAL_QUEUE_READ_OR_REMOVE)(SRV_QUEUE *queue,
    SRV_QUEUE_MODE accessMode, SRV_QUEUE_POSITION position);

//***************************************************************************
/* Read with index

  Summary:
    Function pointer to read an element from a queue at the given index.

  Description:
    This function pointer is used to read an element from a queue at the
    given index.

  Remarks:
    Related to Queue service.
*/
typedef SRV_QUEUE_ELEMENT *(*HAL_QUEUE_READ_ELEMENT)(SRV_QUEUE *queue,
    uint16_t elementIndex);

//***************************************************************************
/* Remove element

  Summary:
    Function pointer to remove the given element from a queue.

  Description:
    This function pointer is used to remove a given element from a queue.

  Remarks:
    Related to Queue service.
*/
typedef void (*HAL_QUEUE_REMOVE_ELEMENT)(SRV_QUEUE *queue,
    SRV_QUEUE_ELEMENT *element);

//***************************************************************************
/* Queue flush

  Summary:
    Function pointer to flush the given queue.

  Description:
    This function pointer is used to flush the given queue.

  Remarks:
    Related to Queue service.
*/
typedef void (*HAL_QUEUE_FLUSH)(SRV_QUEUE *queue);

//***************************************************************************
/* Set queue capacity

  Summary:
    Function pointer to modify the total capacity of a queue.

  Description:
    This function pointer is used to modify the total capacity of a queue.

  Remarks:
    Related to Queue service.
*/
typedef void (*HAL_QUEUE_SET_CAPACITY)(SRV_QUEUE *queue, uint16_t capacity);

// ****************************************************************************
/* Start FU

  Summary:
    Function pointer to start the firmware upgrade process.

  Description:
    This function pointer is used to start the firmware upgrade process by
    initializing and unlocking the memory.

  Remarks:
    Related to Firmware Upgrade service.
*/
typedef void (*HAL_FU_START)(SRV_FU_INFO *fuInfo);

// ****************************************************************************
/* End FU

  Summary:
    Function pointer to end the firmware upgrade process.

  Description:
    This function pointer is used to finish the firmare upgrade process and to
    trigger the execution of the new firmware.

  Remarks:
    Related to Firmware Upgrade service.
*/
typedef void (*HAL_FU_END)(SRV_FU_RESULT fuResult);

// ****************************************************************************
/* Read FU information

  Summary:
    Function pointer to read the firmware upgrade information.

  Description:
    This function pointer is used to read the firmare upgrade information, which
    is stored out of the PRIME stack.

  Remarks:
    Related to Firmware Upgrade service.
*/
typedef void (*HAL_FU_CFG_READ)(void *dst, uint16_t size);

// ****************************************************************************
/* Write FU information

  Summary:
    Function pointer to write the firmware upgrade information.

  Description:
    This function pointer is used to write the firmare upgrade information, which
    is stored out of the PRIME stack.

  Remarks:
    Related to Firmware Upgrade service.
*/
typedef void (*HAL_FU_CFG_WRITE)(void *src, uint16_t size);

// ****************************************************************************
/* Callback for Memory transfer

  Summary:
    Function pointer to register a function to be called back when a memory
    transaction finishes

  Description:
    This function pointer allows the PRIME stack to register a function to be
    called back when a memory transaction finishes.

  Remarks:
    Related to Firmware Upgrade service.
*/
typedef void (*HAL_FU_REGISTER_MEM_TRANSFER_CB)(SRV_FU_MEM_TRANSFER_CB callback);

// ****************************************************************************
/* Read image

  Summary:
    Function pointer to read image from memory.

  Description:
    This function pointer is used to read the image from memory.

  Remarks:
    Related to Firmware Upgrade service.
*/
typedef void (*HAL_FU_DATA_READ)(uint32_t addr, uint8_t *buf, uint16_t size);

// ****************************************************************************
/* Write image

  Summary:
    Function pointer to write image in memory.

  Description:
    This function pointer is used to write the image in memory.

  Remarks:
    Related to Firmware Upgrade service.
*/
typedef void (*HAL_FU_DATA_WRITE)(uint32_t addr, uint8_t *buf, uint16_t size);

// ****************************************************************************
/* Callback for CRC

  Summary:
    Function pointer to register a function to be called back when the CRC of
    the received image has been calculated.

  Description:
    This function pointer allows the PRIME stack to register a function to be
    called back when the CRC of the received image has been calculated.

  Remarks:
    Related to Firmware Upgrade service.
*/
typedef void (*HAL_FU_REGISTER_CRC_CB)(SRV_FU_CRC_CB callback);

// ****************************************************************************
/* Calculate CRC

  Summary:
    Function pointer to calculate the CRC of the received image.

  Description:
    This function pointer is used to calculate the CRC of the received image.

  Remarks:
    Related to Firmware Upgrade service.
*/
typedef void (*HAL_FU_CALCULATE_CRC)(void);

// ****************************************************************************
/* Callback for verification

  Summary:
    Function pointer to register a function to be called back when the received
    image has been verified.

  Description:
    This function pointer allows the PRIME stack to register a function to be
    called back when the received image has been verified.

  Remarks:
    Related to Firmware Upgrade service.
*/
typedef void (*HAL_FU_REGISTER_VERIFY_CB)(SRV_FU_IMAGE_VERIFY_CB callback);

// ****************************************************************************
/* Verify image

  Summary:
    Function pointer to verify the received image.

  Description:
    This function pointer is used to verify the received image. Metadata and
    signature, if available, are checked.

  Remarks:
    Related to Firmware Upgrade service.
*/
typedef void (*HAL_FU_VERIFY_IMAGE)(void);

// ****************************************************************************
/* Get bitmap

  Summary:
    Function pointer to get the bitmap with the information about the status of
    each page of the image.

  Description:
    This function pointer is used to gets the bitmap with the information about
    the status of each page of the image.

  Remarks:
    Related to Firmware Upgrade service.
*/
typedef uint16_t (*HAL_FU_GET_BITMAP)(uint8_t *bitmap, uint32_t *numRxPages);

// ****************************************************************************
/* Request Swap stack

  Summary:
    Function pointer to request to swap the PRIME stack version.

  Description:
    This function pointer is used to request to swap the PRIME stack version.

  Remarks:
    Related to Firmware Upgrade service.
*/
typedef void (*HAL_FU_REQUEST_SWAP)(SRV_FU_TRAFFIC_VERSION trafficVersion);

// *****************************************************************************
/*  Initializes PAL

  Summary:
    Function pointer to initialize the PRIME PAL module.

  Description:
    This function pointer is used to initialize the PRIME PAL module.

  Remarks:
    Related to PRIME PAL.
*/
typedef SYS_MODULE_OBJ (*HAL_PAL_INITIALIZE)(const SYS_MODULE_INDEX index);

// *****************************************************************************
/* Maintains the PAL state machine

  Summary:
    Function pointer to maintain the PRIME PAL module.

  Description:
    This function pointer is used to maintain the PAL internal state machine and
    generate callbacks.

  Remarks:
    Related to PRIME PAL.
*/
typedef void (*HAL_PAL_TASKS)(SYS_MODULE_OBJ object);

// *************************************************************************
/* Get PAL status

  Summary:
    Function pointer to get the current status of the PRIME PAL module.

  Description:
    This function pointer is used to get the current status of the PRIME
    PAL module.

  Remarks:
    Related to PRIME PAL.
*/
typedef SYS_STATUS (*HAL_PAL_STATUS)(SYS_MODULE_OBJ object);

// ****************************************************************************
/* Set PAL callback functions

  Summary:
    Function pointer to set PRIME PAL layer callback functions.

  Description:
    This function pointer is used to link callback functions between upper
    layer and PHY layer.

  Remarks:
    Related to PRIME PAL.
*/
typedef void (*HAL_PAL_CALLBACK_REGISTER)(PAL_CALLBACKS *pCallbacks);

// ****************************************************************************
/* Transmit message

  Summary:
    Function pointer to request to transmit a message.

  Description:
    This functions pointer is used to initiate the transmission process of a
    PPDU (PHY Protocol Data Unit) to the medium indicated in the transmission
    information structure.

  Remarks:
    Related to PRIME PAL.
*/
typedef uint8_t (*HAL_PAL_DATA_REQUEST)(PAL_MSG_REQUEST_DATA *pData);

// ****************************************************************************
/* Convert QT to SNR

  Summary:
    Function pointer to convert QT value to Signal Noise Ratio (SNR).

  Description:
    This function pointer is used to get the value of the Signal to Noise Ratio,
    defined as the ratio of measured received signal level to noise level of
    last received PPDU (PHY Protocol Data Unit).

  Remarks:
    Related to PRIME PAL.
*/
typedef uint8_t (*HAL_PAL_GET_SNR)(uint16_t pch, uint8_t *snr, uint8_t qt);

// ****************************************************************************
/* Get ZCT

  Summary:
    Function pointer to get the zero-cross time (ZCT).

  Description:
    This function pointer is used to get the value of the zero-cross time of
    the mains and the time between the last transmission or reception and the
    zero cross of the mains.

  Remarks:
    Related to PRIME PAL.
*/
typedef uint8_t (*HAL_PAL_GET_ZCT)(uint16_t pch, uint32_t *zct);

// ****************************************************************************
/* Get current PHY time

  Summary:
    Function pointer to get the current PHY time in us.

  Description:
    This function pointer is used to get current PHY time in microseconds.

  Remarks:
    Related to PRIME PAL.
*/
typedef uint8_t (*HAL_PAL_GET_TIMER)(uint16_t pch, uint32_t *timer);

// ****************************************************************************
/* Get extended PHY time

  Summary:
    Function pointer to get the extended PHY time in us.

  Description:
    This function pointer is used to get the extended PHY time in microseconds.

  Remarks:
    Related to PRIME PAL.
*/
typedef uint8_t (*HAL_PAL_GET_TIMER_EXTENDED)(uint16_t pch, uint64_t *timer);

// ****************************************************************************
/* Get carrier detect

  Summary:
    Function pointer to get the carrier detect signal.

  Description:
    This function pointer is used to get the value of carrier detect signal.

  Remarks:
    Related to PRIME PAL.
*/
typedef uint8_t (*HAL_PAL_GET_CD)(uint16_t pch, uint8_t *cd, uint8_t *rssi,
    uint32_t *time, uint8_t *header);

// ****************************************************************************
/* Get noise floor level

  Summary:
    Function pointer to get the noise floor level value.

  Description:
    This function pointer is used to know the noise level present in the
    powerline.

  Remarks:
    Related to PRIME PAL.
*/
typedef uint8_t (*HAL_PAL_GET_NL)(uint16_t pch, uint8_t *noise);

// ****************************************************************************
/* Get AGC

  Summary:
    Function pointer to get the automatic gain mode of the PHY PLC layer.

  Description:
    This fucntion pointer is used to get Automatic Gain Mode (AGC) of the PHY
    PLC layer.

  Remarks:
    Related to PRIME PAL.
*/
typedef uint8_t (*HAL_PAL_GET_AGC)(uint16_t pch, uint8_t *mode, uint8_t *gain);

// ****************************************************************************
/* Set AGC

  Summary:
    Function pointer to set the automatic gain mode of the PHY PLC layer.

  Description:
    This fucntion pointer is used to set Automatic Gain Mode (AGC) of the PHY
    PLC layer.

  Remarks:
    Related to PRIME PAL.
*/
typedef uint8_t (*HAL_PAL_SET_AGC)(uint16_t pch, uint8_t mode, uint8_t gain);

// ****************************************************************************
/* Get clear channel assessment mode

  Summary:
    Function pointer to get the clear pch assessment mode value.

  Description:
    This function pointer is used to get the clear pch assesment mode.
    The pch state helps to know whether or not the RF physical medium is
    free.

  Remarks:
    Related to PRIME PAL.
*/
typedef uint8_t (*HAL_PAL_GET_CCA)(uint16_t pch, uint8_t *pState);

// ****************************************************************************
/* Get physical channel

  Summary:
    Function pointer to get the band (PLC) or the physical channel (RF).

  Description:
    This function pointer is used to get the physical channel or band used for
    the communication.

  Remarks:
    Related to PRIME PAL.
*/
typedef uint8_t (*HAL_PAL_GET_CHANNEL)(uint16_t *pch, uint16_t channelReference);

// ****************************************************************************
/* Set physical channel

  Summary:
    Function pointer to set the band (PLC) or the physical channel (RF).

  Description:
    This function pointer is used to set the physical channel or band used for
    the communication.

  Remarks:
    Related to PRIME PAL.
*/
typedef uint8_t (*HAL_PAL_SET_CHANNEL)(uint16_t pch);

// ****************************************************************************
/* Program channel switch

  Summary:
    Function pointer to program a physical channel switch in the given time.

  Description:
    This function pointer is used to program a physical channel switch in the
    given time.

  Remarks:
    Related to PRIME PAL.
*/
typedef void (*HAL_PAL_PROGRAM_CHANNEL_SWITCH)(uint16_t pch, uint32_t timeSync,
    uint8_t timeMode);

// ****************************************************************************
/* Get PHY attribute

  Summary:
    Function pointer to get a PHY attribute.

  Description:
    This function pointer is used to get a PHY attribute from the selected
    medium.

  Remarks:
    Related to PRIME PAL.
*/
typedef uint8_t (*HAL_PAL_GET_CONFIGURATION)(uint16_t pch, uint16_t id,
    void *val, uint16_t length);

// ****************************************************************************
/* Set PHY attribute

  Summary:
    Function pointer to set a PHY attribute.

  Description:
    This function pointer is used to set a PHY attribute from the selected
    medium.

  Remarks:
    Related to PRIME PAL.
*/
typedef uint8_t (*HAL_PAL_SET_CONFIGURATION)(uint16_t pch, uint16_t id,
    void *val, uint16_t length);

// ****************************************************************************
/* Get capture noise data

  Summary:
    Function pointer to get the Capture Noise Data.

  Description:
    This function pointer is used to read noise data for PLC medium
    communication.

  Remarks:
    Related to PRIME PAL.
*/
typedef uint16_t (*HAL_PAL_GET_SIGNAL_CAPTURE)(uint16_t pch,
    uint8_t *noiseCapture, PAL_FRAME frameType, uint32_t timeStart, uint32_t duration);

// ****************************************************************************
/* Get message duration

  Summary:
    Function pinter to get the  duration.

  Description:
    This function pointer is used to calculate the message duration.

  Remarks:
    Related to PRIME PAL.
*/
typedef uint8_t (*HAL_PAL_GET_MSG_DURATION)(uint16_t pch, uint16_t length,
    PAL_SCHEME scheme, PAL_FRAME frameType, uint32_t *duration);

// ****************************************************************************
/* Check minimum quality

  Summary:
    Function pointer to check the minimum quality for a given modulation scheme.

  Description:
    This function pointer is used to check if the modulation is good enough for
    a low FER (Frame Error rate) for the given scheme.

  Remarks:
    Related to PRIME PAL.
*/
typedef bool (*HAL_PAL_CHECK_MINIMUM_QUALITY)(uint16_t pch, uint8_t reference,
    uint8_t modulation);

// ****************************************************************************
/* Get less robust modulation

  Summary:
    Function pointer to get the less robust modulation scheme.

  Description:
    This function pointer is used to get less robust modulation scheme for a
    selected pch.

  Remarks:
    Related to PRIME PAL.
*/
typedef uint8_t (*HAL_PAL_GET_LESS_ROBUST_MODULATION)(uint16_t pch, uint8_t mod1,
    uint8_t mod2);

// *****************************************************************************
/* HAL API functions structure

  Summary:
    Structure with HAL functions for the API.

  Description:
    This structure defines the list of available functions in the HAL API.

  Remarks:
    None.
*/
typedef struct {
    HAL_RESTART_SYSTEM restart_system;

    HAL_PCRC_CALCULATE pcrc_calc;
    HAL_PCRC_CONFIGURE_SNA pcrc_config_sna;

    HAL_GET_CONFIG_INFO get_config_info;
    HAL_SET_CONFIG_INFO set_config_info;

    HAL_USI_OPEN usi_open;
    HAL_USI_SET_CALLBACK usi_set_callback;
    HAL_USI_SEND usi_send;

    HAL_DEBUG_REPORT debug_report;

    HAL_PIB_GET_REQUEST pib_get_request;
    HAL_PIB_GET_REQUEST_SET_CALLBACK pib_get_request_set_callback;
    HAL_PIB_SET_REQUEST pib_set_request;
    HAL_PIB_SET_REQUEST_SET_CALLBACK pib_set_request_set_callback;

    HAL_RNG_GET rng_get;

    HAL_AES_CMAC_DIRECT aes_cmac_direct;
    HAL_AES_CCM_SET_KEY aes_ccm_set_key;
    HAL_AES_CCM_ENCRYPT_TAG aes_ccm_encrypt_tag;
    HAL_AES_CCM_AUTH_DECRYPT aes_ccm_auth_decrypt;
    HAL_AES_WRAP_KEY aes_wrap_key;
    HAL_AES_UNWRAP_KEY aes_unwrap_key;

    HAL_QUEUE_INIT queue_init;
    HAL_QUEUE_APPEND queue_append;
    HAL_QUEUE_APPEND_WITH_PRIORITY queue_append_with_priority;
    HAL_QUEUE_INSERT_BEFORE queue_insert_before;
    HAL_QUEUE_INSERT_AFTER queue_insert_after;
    HAL_QUEUE_READ_OR_REMOVE queue_read_or_remove;
    HAL_QUEUE_READ_ELEMENT queue_read_element;
    HAL_QUEUE_REMOVE_ELEMENT queue_remove_element;
    HAL_QUEUE_FLUSH queue_flush;
    HAL_QUEUE_SET_CAPACITY queue_set_capacity;

    HAL_FU_START fu_start;
    HAL_FU_END fu_end;
    HAL_FU_CFG_READ fu_cfg_read;
    HAL_FU_CFG_WRITE fu_cfg_write;
    HAL_FU_REGISTER_MEM_TRANSFER_CB fu_register_callback_mem_transfer;
    HAL_FU_DATA_READ fu_data_read;
    HAL_FU_DATA_WRITE fu_data_write;
    HAL_FU_REGISTER_CRC_CB fu_register_callback_crc;
    HAL_FU_CALCULATE_CRC fu_calculate_crc;
    HAL_FU_REGISTER_VERIFY_CB fu_register_callback_verify;
    HAL_FU_VERIFY_IMAGE fu_verify_image;
    HAL_FU_GET_BITMAP fu_get_bitmap;
    HAL_FU_REQUEST_SWAP fu_request_swap;

    HAL_PAL_INITIALIZE hal_pal_initialize;
    HAL_PAL_TASKS hal_pal_tasks;
    HAL_PAL_STATUS hal_pal_status;
    HAL_PAL_CALLBACK_REGISTER hal_pal_callback_register;
    HAL_PAL_DATA_REQUEST hal_pal_data_request;
    HAL_PAL_GET_SNR hal_pal_get_snr;
    HAL_PAL_GET_ZCT hal_pal_get_zct;
    HAL_PAL_GET_TIMER hal_pal_get_timer;
    HAL_PAL_GET_TIMER_EXTENDED hal_pal_get_timer_extended;
    HAL_PAL_GET_CD hal_pal_get_cd;
    HAL_PAL_GET_NL hal_pal_get_nl;
    HAL_PAL_GET_AGC hal_pal_get_agc;
    HAL_PAL_SET_AGC hal_pal_set_agc;
    HAL_PAL_GET_CCA hal_pal_get_cca;
    HAL_PAL_GET_CHANNEL hal_pal_get_channel;
    HAL_PAL_SET_CHANNEL hal_pal_set_channel;
    HAL_PAL_PROGRAM_CHANNEL_SWITCH hal_pal_program_channel_switch;
    HAL_PAL_GET_CONFIGURATION hal_pal_get_configuration;
    HAL_PAL_SET_CONFIGURATION hal_pal_set_configuration;
    HAL_PAL_GET_SIGNAL_CAPTURE hal_pal_get_signal_capture;
    HAL_PAL_GET_MSG_DURATION hal_pal_get_msg_duration;
    HAL_PAL_CHECK_MINIMUM_QUALITY hal_pal_check_minimum_quality;
    HAL_PAL_GET_LESS_ROBUST_MODULATION hal_pal_get_less_robust_modulation;

    /* New functions must be added at the end */

} HAL_API;

extern const HAL_API primeHalAPI;

//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif /* HAL_API_H_INCLUDE */

/*******************************************************************************
 End of File
*/
