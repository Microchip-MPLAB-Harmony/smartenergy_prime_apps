/*******************************************************************************
  PRIME User PIBs Service Implementation.

  Company:
    Microchip Technology Inc.

  File Name:
    srv_user_pib.c

  Summary:
    Source code for the PRIME User PIBs service implementation.

  Description:
    The User PIBs service provides a simple interface to handle a parameter
    interface base defined by the user from the PRIME stack. This file contains
    the source code for the implementation of this service.
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

#include <string.h>

#include "definitions.h"
#include "srv_user_pib.h"
#include "device.h"
#include "service/firmware_upgrade/srv_firmware_upgrade.h"
#include "service/storage/srv_storage.h"

// *****************************************************************************
// *****************************************************************************
// Section: File Scope Data
// *****************************************************************************
// *****************************************************************************

/* Callback function pointers */
static SRV_USER_PIB_GET_REQUEST_CALLBACK SRV_USER_PIB_GetRequestCb;
static SRV_USER_PIB_SET_REQUEST_CALLBACK SRV_USER_PIB_SetRequestCb;

/* GPBR slot that holds reset_info (slots 5..15 cover RESET_INFO + the
 * full HardFault dump; see srv_reset_handler.c). User PIBs 0xF000..0xF00A
 * map to those slots via the convention slot = base + (pibAttrib & 0xF). */
#define SRV_USER_PIB_GPBR_BASE_SLOT     5U

/* Deferred-reboot state machine driven by writes to
 * PIB_USER_BOOTLOADER_UART_MODE. The Set handler arms the trigger and
 * SRV_USER_PIB_Tasks finishes the work asynchronously so the SET
 * response is on the wire before the modem actually reboots. */
typedef enum
{
    SRV_USER_PIB_UART_MODE_IDLE       = 0,
    SRV_USER_PIB_UART_MODE_DELAY      = 1,
    SRV_USER_PIB_UART_MODE_KICK_FU    = 2,
    SRV_USER_PIB_UART_MODE_WAIT_FU    = 3,
} SRV_USER_PIB_UART_MODE_STATE;

/* ~70 ms grace period between the SET response and the actual reboot.
 * Driven by SRV_USER_PIB_Tasks counter; assumes Tasks is called from
 * the same loop as the rest of system tasks (~µs per iteration), so
 * the count is in iterations rather than milliseconds. The exact
 * wall-clock value is not critical -- the only requirement is that
 * the BS receive the response before the modem reboots. */
#define SRV_USER_PIB_UART_MODE_DELAY_TICKS      (50000U)

static volatile SRV_USER_PIB_UART_MODE_STATE gUartModeState
        = SRV_USER_PIB_UART_MODE_IDLE;
static volatile uint32_t gUartModeDelay = 0U;

// *****************************************************************************
// *****************************************************************************
// Section: User PIBs Service Interface Implementation
// *****************************************************************************
// *****************************************************************************

void SRV_USER_PIB_GetRequest(uint16_t pibAttrib)
{
    uint32_t pibValue;
    uint8_t  getResult;

    getResult = 0; /* false: PIB not handled */
    pibValue  = 0U;

    if ((pibAttrib >= PIB_USER_RESET_INFO) && (pibAttrib <= PIB_USER_R12))
    {
        /* Reset info + HardFault dump live in emulated GPBR (SRV_STORAGE
         * EEPROM cache, slots 5..15). Read directly each time the BS asks
         * -- no RAM cache. The dump persists in EEPROM until the next
         * fault overwrites it; the BS can correlate against
         * PIB_USER_RESET_INFO (high16 = reset counter, low16 = cause)
         * to tell whether a fresh fault happened since the last read. */
        uint8_t slot = (uint8_t)(SRV_USER_PIB_GPBR_BASE_SLOT
                               + (pibAttrib & 0x000FU));
        pibValue  = SRV_STORAGE_ReadNonVolatileData(slot);
        getResult = 1;
    }
    else if (pibAttrib == PIB_USER_BOOTLOADER_UART_MODE)
    {
        /* Write-only PIB; reads always return 0 so the BS can poll
         * without harm. uint32_t/4 B response keeps the wire format
         * consistent with every other user PIB in this project. */
        getResult = 1;
        pibValue  = 0U;
    }
    else
    {
        /* Unhandled user PIB: respond with getResult=0 so the library
         * generates MLME_RESULT_FAILED to the BS. */
    }

    if (SRV_USER_PIB_GetRequestCb != NULL)
    {
        SRV_USER_PIB_GetRequestCb(getResult, pibAttrib, &pibValue,
                                   (uint8_t) sizeof(pibValue));
    }
}

void SRV_USER_PIB_SetRequest(uint16_t pibAttrib, void *pibValue, uint8_t pibSize)
{
    bool setResult;

    setResult = false;

    if (pibAttrib == PIB_USER_BOOTLOADER_UART_MODE)
    {
        /* Every user PIB in this project is uint32_t; reject anything
         * else so a malformed SET on the wire cannot quietly trigger a
         * reboot. */
        if ((pibValue != NULL) && (pibSize == (uint8_t) sizeof(uint32_t)))
        {
            /* PRIME wire format is big-endian for multi-byte PIB values
             * (see mngp_sn.c _mngp_add_pib_value for the GET symmetric
             * serialization). The library hands us the raw BE bytes
             * from the SET request without converting them, so we have
             * to decode MSB-first ourselves. A naive memcpy into a
             * host-LE uint32_t would interpret 0x00000001 BE as
             * 0x01000000, breaking the value comparison. */
            const uint8_t *bytes = (const uint8_t *) pibValue;
            uint32_t requested = ((uint32_t) bytes[0] << 24)
                               | ((uint32_t) bytes[1] << 16)
                               | ((uint32_t) bytes[2] <<  8)
                               |  (uint32_t) bytes[3];

            if (requested == 1U)
            {
                /* Arm the deferred reboot. The actual SST26 write +
                 * NVIC_SystemReset happens in SRV_USER_PIB_Tasks once
                 * the SET response has had a chance to leave the line.
                 * Re-arming while already armed is a no-op (idempotent
                 * with respect to repeated 1 writes). */
                if (gUartModeState == SRV_USER_PIB_UART_MODE_IDLE)
                {
                    gUartModeDelay = SRV_USER_PIB_UART_MODE_DELAY_TICKS;
                    gUartModeState = SRV_USER_PIB_UART_MODE_DELAY;
                }
                setResult = true;
            }
            else if (requested == 0U)
            {
                /* Idempotent ack; the PIB has no persistent storage. */
                setResult = true;
            }
            else
            {
                /* Out-of-range value: reject so the BS sees an error
                 * instead of silently doing nothing. */
                setResult = false;
            }
        }
    }

    if (SRV_USER_PIB_SetRequestCb != NULL)
    {
        SRV_USER_PIB_SetRequestCb(setResult);
    }
}

void SRV_USER_PIB_Tasks(void)
{
    SRV_FU_EXT_MEM_BOOT_MODE_STATUS fuStatus;

    switch (gUartModeState)
    {
        case SRV_USER_PIB_UART_MODE_IDLE:
            /* Nothing pending; cheap fall-through every iteration. */
            break;

        case SRV_USER_PIB_UART_MODE_DELAY:
            if (gUartModeDelay > 0U)
            {
                gUartModeDelay--;
            }
            else
            {
                gUartModeState = SRV_USER_PIB_UART_MODE_KICK_FU;
            }
            break;

        case SRV_USER_PIB_UART_MODE_KICK_FU:
            if (SRV_FU_ExtMemBootModeSet(
                    SRV_FU_EXT_MEM_BOOT_MODE_UART_PENDING,
                    0U, 0U) == true)
            {
                gUartModeState = SRV_USER_PIB_UART_MODE_WAIT_FU;
            }
            /* else: FU service busy this iteration -- retry next tick. */
            break;

        case SRV_USER_PIB_UART_MODE_WAIT_FU:
            fuStatus = SRV_FU_ExtMemBootModeStatus();
            if (fuStatus == SRV_FU_EXT_MEM_BOOT_MODE_STATUS_OK)
            {
                /* BOOT_FLAG persisted; reboot lands in UART recovery. */
                NVIC_SystemReset();
                /* unreachable */
            }
            else if (fuStatus == SRV_FU_EXT_MEM_BOOT_MODE_STATUS_ERROR)
            {
                /* Page program failed. Drop the request and let the BS
                 * retry; better than rebooting into an inconsistent
                 * state. */
                gUartModeState = SRV_USER_PIB_UART_MODE_IDLE;
            }
            /* IDLE / BUSY: keep waiting. */
            break;

        default:
            gUartModeState = SRV_USER_PIB_UART_MODE_IDLE;
            break;
    }
}

void SRV_USER_PIB_GetRequestCbRegister(SRV_USER_PIB_GET_REQUEST_CALLBACK callback)
{
    SRV_USER_PIB_GetRequestCb = callback;
}

void SRV_USER_PIB_SetRequestCbRegister(SRV_USER_PIB_SET_REQUEST_CALLBACK callback)
{
    SRV_USER_PIB_SetRequestCb = callback;
}

void SRV_USER_PIB_Initialize(void)
{
    SRV_USER_PIB_GetRequestCb = NULL;
    SRV_USER_PIB_SetRequestCb = NULL;

    gUartModeState = SRV_USER_PIB_UART_MODE_IDLE;
    gUartModeDelay = 0U;
}
