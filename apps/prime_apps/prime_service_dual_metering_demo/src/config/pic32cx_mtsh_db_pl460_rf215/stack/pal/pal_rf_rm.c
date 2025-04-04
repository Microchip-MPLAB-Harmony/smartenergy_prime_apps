/*******************************************************************************
  Company:
    Microchip Technology Inc.

  File Name:
    pal_rf_rm.c

  Summary:
    Platform Abstraction Layer (PAL) RF Robust Management.

  Description:
    Platform Abstraction Layer (PAL) RF Robust Management source file.
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

/* System includes */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "pal_types.h"
#include "driver/rf215/drv_rf215.h"
#include "pal_rf_rm.h"

/* RF RM mode */
#define PAL_RF_RM_FORCED_OFF   1
#define PAL_RF_RM_FORCED_ON    2

/* RSSI Thresholds */
#define PAL_RF_RM_THRESHOLD_FSK_FEC_OFF     (-89)
#define PAL_RF_RM_THRESHOLD_FSK_FEC_ON      (-94)

static uint8_t palRfRmMode = PAL_RF_RM_FORCED_OFF;

/* Bandwidth of every modulation */
static const uint8_t palRfBandwidth[] = {
        0,   /* PAL_SCHEME_RF */
        50,  /* PAL_SCHEME_RF_FSK_FEC_OFF */
        25   /* PAL_SCHEME_RF_FSK_FEC_ON */
};

uint8_t PAL_RF_RM_GetLqi(int16_t rssi)
{
    if (rssi > 80)
    {
        return 0xFEU;
    }
    else
    {
        int16_t result = rssi + 174;
        if (result < 0) {
            result = 0;
        } else {
            /* Nothing to update */
        }
        return (uint8_t)result;
    }
}

uint8_t PAL_RF_RM_GetLessRobustModulation(PAL_SCHEME mod1, PAL_SCHEME mod2)
{
    uint8_t index1 = (uint8_t)(mod1) - (uint8_t)(PAL_SCHEME_RF);
    uint8_t index2 = (uint8_t)(mod2) - (uint8_t)(PAL_SCHEME_RF);

    if (palRfBandwidth[index1] > palRfBandwidth[index2])
    {
        return (uint8_t)(mod1);
    }
    else
    {
        return (uint8_t)(mod2);
    }
}

bool PAL_RF_RM_CheckMinimumQuality(PAL_SCHEME reference, PAL_SCHEME modulation)
{
    if (modulation == PAL_SCHEME_RF)
    {
        return false;
    }

    if ((reference == PAL_SCHEME_RF_FSK_FEC_OFF) &&
        (modulation == PAL_SCHEME_RF_FSK_FEC_ON))
    {
        return false;
    }
    else
    {
        return true;
    }
}

PAL_SCHEME PAL_RF_RM_GetScheme(void)
{
    PAL_SCHEME result;
    switch(palRfRmMode)
    {
        case PAL_RF_RM_FORCED_OFF:
            result = PAL_SCHEME_RF_FSK_FEC_OFF;
            break;

        case PAL_RF_RM_FORCED_ON:
            result = PAL_SCHEME_RF_FSK_FEC_ON;
            break;

        default:
            /* Handle unexpected values appropriately */
            result = PAL_SCHEME_RF_FSK_FEC_OFF;
            break;
    }
    return result;
}

void PAL_RF_RM_SetScheme(PAL_SCHEME scheme)
{
    if (scheme == PAL_SCHEME_RF_FSK_FEC_OFF)
    {
        palRfRmMode = PAL_RF_RM_FORCED_OFF;
    }
    else
    {
        palRfRmMode = PAL_RF_RM_FORCED_ON;
    }
}

void PAL_RF_RM_GetRobustModulation(void *indObj, uint16_t *pBitRate,
                                   PAL_SCHEME *pModulation, uint16_t pch)
{
    DRV_RF215_RX_INDICATION_OBJ *pIndObj;
    PAL_SCHEME bestScheme = PAL_SCHEME_RF;

    (void)pch;

    pIndObj = (DRV_RF215_RX_INDICATION_OBJ *)indObj;

    switch(palRfRmMode)
    {
        case PAL_RF_RM_FORCED_OFF:
            if (pIndObj->rssiDBm >= PAL_RF_RM_THRESHOLD_FSK_FEC_OFF)
            {
                bestScheme = PAL_SCHEME_RF_FSK_FEC_OFF;
            }
            break;

        case PAL_RF_RM_FORCED_ON:
            if (pIndObj->rssiDBm >= PAL_RF_RM_THRESHOLD_FSK_FEC_ON)
            {
                bestScheme = PAL_SCHEME_RF_FSK_FEC_ON;
            }
            break;

        default:
            if (pIndObj->rssiDBm >= PAL_RF_RM_THRESHOLD_FSK_FEC_OFF)
            {
                bestScheme = PAL_SCHEME_RF_FSK_FEC_OFF;
            }
            else if (pIndObj->rssiDBm >= PAL_RF_RM_THRESHOLD_FSK_FEC_ON)
            {
                bestScheme = PAL_SCHEME_RF_FSK_FEC_ON;
            }
            else{
                /* No change needed */
            }
            break;
    }

    *pModulation = bestScheme;

    if (bestScheme != PAL_SCHEME_RF)
    {
        *pBitRate = palRfBandwidth[(uint8_t)(bestScheme) - (uint8_t)(PAL_SCHEME_RF)];
    }
    else
    {
        *pBitRate = 0;
    }
}
