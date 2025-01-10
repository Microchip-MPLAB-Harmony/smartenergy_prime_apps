/*******************************************************************************
  PRIME Hardware Abstraction Layer Wrapper API Source

  Company:
    Microchip Technology Inc.

  File Name:
    prime_hal_wrapper.c

  Summary:
    PRIME Hardware Abstraction Layer Wrapper API Source File

  Description:
    This module contains the implementation of the API wrapper to be used by the
    PRIME stack when accessing the services connected to the hardware.
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

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdio.h>
#include "prime_hal_wrapper.h"

// *****************************************************************************
// *****************************************************************************
// Section: File Scope Variables
// *****************************************************************************
// *****************************************************************************

static const HAL_API *pPrimeHalApi;

// *****************************************************************************
// *****************************************************************************
// Section: PRIME HAL Wrapper Interface Routines
// *****************************************************************************
// *****************************************************************************

void PRIME_HAL_WRP_Configure(const HAL_API *pHalApi)
{
    pPrimeHalApi = pHalApi;
}

void PRIME_HAL_WRP_RestartSystem(SRV_RESET_HANDLER_RESET_CAUSE resetType)
{
    pPrimeHalApi->restart_system(resetType);
}

uint32_t PRIME_HAL_WRAPPER_PcrcCalculate(uint8_t *pData, size_t length,
    PCRC_HEADER_TYPE hdrType, PCRC_CRC_TYPE crcType, uint32_t initValue)
{
    return pPrimeHalApi->pcrc_calc(pData, length, hdrType, crcType, initValue);
}

void PRIME_HAL_WRP_PcrcConfigureSNA(uint8_t *sna)
{
    pPrimeHalApi->pcrc_config_sna(sna);
}

bool PRIME_HAL_WRP_GetConfigInfo(SRV_STORAGE_TYPE infoType, uint8_t size,
    void* pData)
{
    return pPrimeHalApi->get_config_info(infoType, size, pData);
}

bool PRIME_HAL_WRP_SetConfigInfo(SRV_STORAGE_TYPE infoType, uint8_t size,
    void* pData)
{
    return pPrimeHalApi->set_config_info(infoType, size, pData);
}

SRV_USI_HANDLE PRIME_HAL_WRP_UsiOpen(const SYS_MODULE_INDEX index)
{
    return pPrimeHalApi->usi_open(index);
}

void PRIME_HAL_WRP_UsiSetCallback(SRV_USI_HANDLE handle,
    SRV_USI_PROTOCOL_ID protocol, SRV_USI_CALLBACK callback)
{
    pPrimeHalApi->usi_set_callback(handle, protocol, callback);
}

void PRIME_HAL_WRP_UsiSend(SRV_USI_HANDLE handle,
    SRV_USI_PROTOCOL_ID protocol, uint8_t *data, size_t length)
{
    (void)pPrimeHalApi->usi_send(handle, protocol, data, length);
}

void PRIME_HAL_WRP_DebugReport(SRV_LOG_REPORT_LEVEL logLevel,
    SRV_LOG_REPORT_CODE code, const char *info, ...)
{
    pPrimeHalApi->debug_report(logLevel, code, info);
}

void PRIME_HAL_WRP_PibGetRequest(uint16_t pibAttrib)
{
    pPrimeHalApi->pib_get_request(pibAttrib);
}

void PRIME_HAL_WRP_PibGetRequestSetCallback(
    SRV_USER_PIB_GET_REQUEST_CALLBACK callback)
{
    pPrimeHalApi->pib_get_request_set_callback(callback);
}

void PRIME_HAL_WRP_PibSetRequest(uint16_t pibAttrib, void *pibValue,
    uint8_t pibSize)
{
    pPrimeHalApi->pib_set_request(pibAttrib, pibValue, pibSize);
}

void PRIME_HAL_WRP_PibSetRequestSetCallback(
    SRV_USER_PIB_SET_REQUEST_CALLBACK callback)
{
    pPrimeHalApi->pib_set_request_set_callback(callback);
}

uint32_t PRIME_HAL_WRP_RngGet(void)
{
    return pPrimeHalApi->rng_get();
}

int32_t PRIME_HAL_WRP_AesCmacDirect(uint8_t *input, uint32_t inputLen,
    uint8_t *outputMac, uint8_t *key)
{
    return pPrimeHalApi->aes_cmac_direct(input, inputLen, outputMac, key);
}

int32_t PRIME_HAL_WRP_AesCcmSetkey(uint8_t *key)
{
    return pPrimeHalApi->aes_ccm_set_key(key);
}

int32_t PRIME_HAL_WRP_AesCcmEncryptAndTag(uint8_t *data, uint32_t dataLen,
    uint8_t *iv, uint32_t ivLen, uint8_t *aad, uint32_t aadLen, uint8_t *tag,
    uint32_t tagLen)
{
    return pPrimeHalApi->aes_ccm_encrypt_tag(data, dataLen, iv, ivLen, aad,
        aadLen, tag, tagLen);
}

int32_t PRIME_HAL_WRP_AesCcmAuthDecrypt(uint8_t *data, uint32_t dataLen,
    uint8_t *iv, uint32_t ivLen, uint8_t *aad, uint32_t aadLen,
    uint8_t *tag, uint32_t tagLen)
{
    return pPrimeHalApi->aes_ccm_auth_decrypt(data, dataLen, iv, ivLen, aad,
        aadLen,  tag, tagLen);
}

void PRIME_HAL_WRP_AesWrapKey(uint8_t *key, uint32_t keyLen, uint8_t *in,
    uint32_t inLen, uint8_t *out)
{
    pPrimeHalApi->aes_wrap_key(key, keyLen, in, inLen, out);
}

bool PRIME_HAL_WRP_AesUnwrapKey(uint8_t *key, uint32_t keyLen, uint8_t *in,
    uint32_t inLen, uint8_t *out)
{
    return pPrimeHalApi->aes_unwrap_key(key, keyLen, in, inLen, out);
}

void PRIME_HAL_WRP_QueueInit(SRV_QUEUE *queue, uint16_t capacity,
    SRV_QUEUE_TYPE type)
{
    pPrimeHalApi->queue_init(queue, capacity, type);
}

void PRIME_HAL_WRP_QueueAppend(SRV_QUEUE *queue, SRV_QUEUE_ELEMENT *element)
{
    pPrimeHalApi->queue_append(queue, element);
}

void PRIME_HAL_WRP_QueueAppend_With_Priority(SRV_QUEUE *queue,
    uint32_t priority, SRV_QUEUE_ELEMENT *element)
{
    pPrimeHalApi->queue_append_with_priority(queue, priority, element);
}

void PRIME_HAL_WRP_QueueInsert_Before(SRV_QUEUE *queue,
    SRV_QUEUE_ELEMENT *currentElement, SRV_QUEUE_ELEMENT *element)
{
    pPrimeHalApi->queue_insert_before(queue, currentElement, element);
}

void PRIME_HAL_WRP_QueueInsert_After(SRV_QUEUE *queue,
    SRV_QUEUE_ELEMENT *currentElement, SRV_QUEUE_ELEMENT *element)
{
    pPrimeHalApi->queue_insert_after(queue, currentElement, element);
}

SRV_QUEUE_ELEMENT *PRIME_HAL_WRP_QueueRead_Or_Remove(SRV_QUEUE *queue,
    SRV_QUEUE_MODE accessMode, SRV_QUEUE_POSITION position)
{
    return pPrimeHalApi->queue_read_or_remove(queue, accessMode, position);
}

SRV_QUEUE_ELEMENT *PRIME_HAL_WRP_QueueRead_Element(SRV_QUEUE *queue,
    uint16_t elementIndex)
{
    return pPrimeHalApi->queue_read_element(queue, elementIndex);
}

void PRIME_HAL_WRP_QueueRemove_Element(SRV_QUEUE *queue,
    SRV_QUEUE_ELEMENT *element)
{
    pPrimeHalApi->queue_remove_element(queue, element);
}

void PRIME_HAL_WRP_QueueFlush(SRV_QUEUE *queue)
{
    pPrimeHalApi->queue_flush(queue);
}

void PRIME_HAL_WRP_QueueSet_Capacity(SRV_QUEUE *queue, uint16_t capacity)
{
    pPrimeHalApi->queue_set_capacity(queue, capacity);
}

void PRIME_HAL_WRP_FuStart(SRV_FU_INFO *fuInfo)
{
    pPrimeHalApi->fu_start(fuInfo);
}

void PRIME_HAL_WRP_FuEnd(SRV_FU_RESULT fuResult)
{
    pPrimeHalApi->fu_end(fuResult);
}

void PRIME_HAL_WRP_FuCfgRead(void *dst, uint16_t size)
{
    pPrimeHalApi->fu_cfg_read(dst, size);
}

void PRIME_HAL_WRP_FuCfgWrite(void *src, uint16_t size)
{
    pPrimeHalApi->fu_cfg_write(src, size);
}

void PRIME_HAL_WRP_FuRegisterCbMemTransfer(SRV_FU_MEM_TRANSFER_CB callback)
{
    pPrimeHalApi->fu_register_callback_mem_transfer(callback);
}

void PRIME_HAL_WRP_FuDataRead(uint32_t addr, uint8_t *buf, uint16_t size)
{
    pPrimeHalApi->fu_data_read(addr, buf, size);
}

void PRIME_HAL_WRP_FuDataWrite(uint32_t addr, uint8_t *buf, uint16_t size)
{
    pPrimeHalApi->fu_data_write(addr, buf, size);
}

void PRIME_HAL_WRP_FuRegisterCbCrc(SRV_FU_CRC_CB callback)
{
    pPrimeHalApi->fu_register_callback_crc(callback);
}

void PRIME_HAL_WRP_FuCalculateCrc(void)
{
    pPrimeHalApi->fu_calculate_crc();
}

void PRIME_HAL_WRP_FuRegisterCbVerify(SRV_FU_IMAGE_VERIFY_CB callback)
{
    pPrimeHalApi->fu_register_callback_verify(callback);
}

void PRIME_HAL_WRP_FuVerifyImage(void)
{
    pPrimeHalApi->fu_verify_image();
}

uint16_t PRIME_HAL_WRP_FuGetBitmap(uint8_t *bitmap, uint32_t *numRxPages)
{
    return pPrimeHalApi->fu_get_bitmap(bitmap, numRxPages);
}

void PRIME_HAL_WRP_FuRequestSwap(SRV_FU_TRAFFIC_VERSION trafficVersion)
{
    pPrimeHalApi->fu_request_swap(trafficVersion);
}

SYS_MODULE_OBJ PRIME_HAL_WRP_PAL_Initialize(const SYS_MODULE_INDEX index)
{
    return pPrimeHalApi->hal_pal_initialize(index);
}

void PRIME_HAL_WRP_PAL_Tasks(SYS_MODULE_OBJ object)
{
    return pPrimeHalApi->hal_pal_tasks(object);
}

SYS_STATUS PRIME_HAL_WRP_PAL_Status(SYS_MODULE_OBJ object)
{
    return pPrimeHalApi->hal_pal_status(object);
}

void PRIME_HAL_WRP_PAL_CallbackRegister(PAL_CALLBACKS *pCallbacks)
{
    return pPrimeHalApi->hal_pal_callback_register(pCallbacks);
}

uint8_t PRIME_HAL_WRP_PAL_DataRequest(PAL_MSG_REQUEST_DATA *pData)
{
    return pPrimeHalApi->hal_pal_data_request(pData);
}

uint8_t PRIME_HAL_WRP_PAL_GetSNR(uint16_t pch, uint8_t *snr, uint8_t qt)
{
    return pPrimeHalApi->hal_pal_get_snr(pch, snr, qt);
}

uint8_t PRIME_HAL_WRP_PAL_GetZCT(uint16_t pch, uint32_t *zct)
{
    return pPrimeHalApi->hal_pal_get_zct(pch, zct);
}

uint8_t PRIME_HAL_WRP_PAL_GetTimer(uint16_t pch, uint32_t *timer)
{
    return pPrimeHalApi->hal_pal_get_timer(pch, timer);
}

uint8_t PRIME_HAL_WRP_PAL_GetTimerExtended(uint16_t pch, uint64_t *timer)
{
    return pPrimeHalApi->hal_pal_get_timer_extended(pch, timer);
}

uint8_t PRIME_HAL_WRP_PAL_GetCD(uint16_t pch, uint8_t *cd, uint8_t *rssi,
    uint32_t *timePrime, uint8_t *header)
{
    return pPrimeHalApi->hal_pal_get_cd(pch, cd, rssi, timePrime, header);
}

uint8_t PRIME_HAL_WRP_PAL_GetNL(uint16_t pch, uint8_t *noise)
{
    return pPrimeHalApi->hal_pal_get_nl(pch, noise);
}

uint8_t PRIME_HAL_WRP_PAL_GetAGC(uint16_t pch, uint8_t *mode, uint8_t *gain)
{
    return pPrimeHalApi->hal_pal_get_agc(pch, mode, gain);
}

uint8_t PRIME_HAL_WRP_PAL_SetAGC(uint16_t pch, uint8_t mode, uint8_t gain)
{
    return pPrimeHalApi->hal_pal_set_agc(pch, mode, gain);
}

uint8_t PRIME_HAL_WRP_PAL_GetCCA(uint16_t pch, uint8_t *pState)
{
    return pPrimeHalApi->hal_pal_get_cca(pch, pState);
}

uint8_t PRIME_HAL_WRP_PAL_GetChannel(uint16_t *pPch, uint16_t channelReference)
{
    return pPrimeHalApi->hal_pal_get_channel(pPch, channelReference);
}

uint8_t PRIME_HAL_WRP_PAL_SetChannel(uint16_t pch)
{
    return pPrimeHalApi->hal_pal_set_channel(pch);
}

void PRIME_HAL_WRP_PAL_ProgramChannelSwitch(uint16_t pch, uint32_t timeSync,
    uint8_t timeMode)
{
    return pPrimeHalApi->hal_pal_program_channel_switch(pch, timeSync, timeMode);
}

uint8_t PRIME_HAL_WRP_PAL_GetConfiguration(uint16_t pch, uint16_t id, void *val,
    uint16_t length)
{
    return pPrimeHalApi->hal_pal_get_configuration(pch, id, val, length);
}

uint8_t PRIME_HAL_WRP_PAL_SetConfiguration(uint16_t pch, uint16_t id, void *val,
    uint16_t length)
{
    return pPrimeHalApi->hal_pal_set_configuration(pch, id, val, length);
}

uint16_t PRIME_HAL_WRP_PAL_GetSignalCapture(uint16_t pch, uint8_t *noiseCapture,
    PAL_FRAME frameType, uint32_t timeStart, uint32_t duration)
{
    return pPrimeHalApi->hal_pal_get_signal_capture(pch, noiseCapture, frameType,
        timeStart, duration);
}

uint8_t PRIME_HAL_WRP_PAL_GetMsgDuration(uint16_t pch, uint16_t length,
    PAL_SCHEME scheme, PAL_FRAME frameType, uint32_t *duration)
{
    return pPrimeHalApi->hal_pal_get_msg_duration(pch, length, scheme, frameType,
        duration);
}

bool PRIME_HAL_WRP_PAL_CheckMinimumQuality(uint16_t pch, uint8_t reference,
    uint8_t modulation)
{
    return pPrimeHalApi->hal_pal_check_minimum_quality(pch, reference, modulation);
}

uint8_t PRIME_HAL_WRP_PAL_GetLessRobustModulation(uint16_t pch, uint8_t mod1,
    uint8_t mod2)
{
    return pPrimeHalApi->hal_pal_get_less_robust_modulation(pch, mod1, mod2);
}

/* @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/* @endcond */
