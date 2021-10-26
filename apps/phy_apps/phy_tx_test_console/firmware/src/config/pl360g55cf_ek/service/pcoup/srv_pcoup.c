/*******************************************************************************
  PLC Phy Coupling Service Implementation.

  Company:
    Microchip Technology Inc.

  File Name:
    srv_pcoup.c

  Summary:
    PLC PHY Coupling service implementation.

  Description:
    This file contains the source code for the implementation of the
    PLC PHY Cpupling service.
*******************************************************************************/

//DOM-IGNORE-BEGIN
/*******************************************************************************
* Copyright (C) 2021 Microchip Technology Inc. and its subsidiaries.
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
//DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "configuration.h"
#include "srv_pcoup.h"
#include "driver/driver_common.h"
#include "service/pcoup/srv_pcoup.h"
#include "driver/plc/phy/drv_plc_phy_comm.h"

/* Specific gain for each carrier to equalize transmission and compensate HW filter frequency response. */
static const uint16_t srvPlcCoupPredistCoefChn1High[SRV_PCOUP_EQU_NUM_COEF_CHN] = SRV_PCOUP_PRED_CHN1_HIGH_TBL;
static const uint16_t srvPlcCoupPredistCoefChn1Low[SRV_PCOUP_EQU_NUM_COEF_CHN] = SRV_PCOUP_PRED_CHN1_VLOW_TBL;

static const uint16_t srvPlcCoupPredistCoefDummy[SRV_PCOUP_EQU_NUM_COEF_CHN] = SRV_PCOUP_PRED_NOT_USED;

/* Configuration values of internal DACC peripheral */
static const uint32_t srvPlcCoupDaccTableCenA[17] = SRV_PCOUP_DACC_CENA_TBL;

static const uint32_t srvPlcCoupDaccTableFcc[17] = SRV_PCOUP_DACC_FCC_TBL;


// *****************************************************************************
/* PLC Coupling configuration data

  Summary:
    Holds PLC configuration data

  Description:
    This structure holds the PLC coupling configuration data.

  Remarks:
    Parameters are defined in srv_pcoup.h file
 */

static const SRV_PLC_PCOUP_CHANNEL_DATA srvPlcCoupChn1Data = {
  SRV_PCOUP_CHN1_RMS_HIGH_TBL, SRV_PCOUP_CHN1_RMS_VLOW_TBL,
  SRV_PCOUP_CHN1_THRS_HIGH_TBL, SRV_PCOUP_CHN1_THRS_VLOW_TBL,
  SRV_PCOUP_CHN1_GAIN_HIGH_TBL, SRV_PCOUP_CHN1_GAIN_VLOW_TBL,
  SRV_PCOUP_CHN1_MAX_NUM_TX_LEVELS, SRV_PCOUP_EQU_NUM_COEF_CHN << 1, SRV_PCOUP_CHN1_LINE_DRV_CONF,
  srvPlcCoupDaccTableCenA,
  srvPlcCoupPredistCoefChn1High, srvPlcCoupPredistCoefChn1Low
};

static const SRV_PLC_PCOUP_CHANNEL_DATA srvPlcCoupChn3Data = {
  SRV_PCOUP_CHN3_RMS_HIGH_TBL, SRV_PCOUP_CHN3_RMS_VLOW_TBL,
  SRV_PCOUP_CHN3_THRS_HIGH_TBL, SRV_PCOUP_CHN3_THRS_VLOW_TBL,
  SRV_PCOUP_CHN3_GAIN_HIGH_TBL, SRV_PCOUP_CHN3_GAIN_VLOW_TBL,
  SRV_PCOUP_CHN3_MAX_NUM_TX_LEVELS, SRV_PCOUP_EQU_NUM_COEF_CHN << 1, SRV_PCOUP_CHN3_LINE_DRV_CONF,
  srvPlcCoupDaccTableFcc,
  srvPlcCoupPredistCoefDummy, srvPlcCoupPredistCoefDummy
};

static const SRV_PLC_PCOUP_CHANNEL_DATA srvPlcCoupChn4Data = {
  SRV_PCOUP_CHN4_RMS_HIGH_TBL, SRV_PCOUP_CHN4_RMS_VLOW_TBL,
  SRV_PCOUP_CHN4_THRS_HIGH_TBL, SRV_PCOUP_CHN4_THRS_VLOW_TBL,
  SRV_PCOUP_CHN4_GAIN_HIGH_TBL, SRV_PCOUP_CHN4_GAIN_VLOW_TBL,
  SRV_PCOUP_CHN4_MAX_NUM_TX_LEVELS, SRV_PCOUP_EQU_NUM_COEF_CHN << 1, SRV_PCOUP_CHN4_LINE_DRV_CONF,
  srvPlcCoupDaccTableFcc,
  srvPlcCoupPredistCoefDummy, srvPlcCoupPredistCoefDummy
};

static const SRV_PLC_PCOUP_CHANNEL_DATA srvPlcCoupChn5Data = {
  SRV_PCOUP_CHN5_RMS_HIGH_TBL, SRV_PCOUP_CHN5_RMS_VLOW_TBL,
  SRV_PCOUP_CHN5_THRS_HIGH_TBL, SRV_PCOUP_CHN5_THRS_VLOW_TBL,
  SRV_PCOUP_CHN5_GAIN_HIGH_TBL, SRV_PCOUP_CHN5_GAIN_VLOW_TBL,
  SRV_PCOUP_CHN5_MAX_NUM_TX_LEVELS, SRV_PCOUP_EQU_NUM_COEF_CHN << 1, SRV_PCOUP_CHN5_LINE_DRV_CONF,
  srvPlcCoupDaccTableFcc,
  srvPlcCoupPredistCoefDummy, srvPlcCoupPredistCoefDummy
};

static const SRV_PLC_PCOUP_CHANNEL_DATA srvPlcCoupChn6Data = {
  SRV_PCOUP_CHN6_RMS_HIGH_TBL, SRV_PCOUP_CHN6_RMS_VLOW_TBL,
  SRV_PCOUP_CHN6_THRS_HIGH_TBL, SRV_PCOUP_CHN6_THRS_VLOW_TBL,
  SRV_PCOUP_CHN6_GAIN_HIGH_TBL, SRV_PCOUP_CHN6_GAIN_VLOW_TBL,
  SRV_PCOUP_CHN6_MAX_NUM_TX_LEVELS, SRV_PCOUP_EQU_NUM_COEF_CHN << 1, SRV_PCOUP_CHN6_LINE_DRV_CONF,
  srvPlcCoupDaccTableFcc,
  srvPlcCoupPredistCoefDummy, srvPlcCoupPredistCoefDummy
};

static const SRV_PLC_PCOUP_CHANNEL_DATA srvPlcCoupChn7Data = {
  SRV_PCOUP_CHN7_RMS_HIGH_TBL, SRV_PCOUP_CHN7_RMS_VLOW_TBL,
  SRV_PCOUP_CHN7_THRS_HIGH_TBL, SRV_PCOUP_CHN7_THRS_VLOW_TBL,
  SRV_PCOUP_CHN7_GAIN_HIGH_TBL, SRV_PCOUP_CHN7_GAIN_VLOW_TBL,
  SRV_PCOUP_CHN7_MAX_NUM_TX_LEVELS, SRV_PCOUP_EQU_NUM_COEF_CHN << 1, SRV_PCOUP_CHN7_LINE_DRV_CONF,
  srvPlcCoupDaccTableFcc,
  srvPlcCoupPredistCoefDummy, srvPlcCoupPredistCoefDummy
};

static const SRV_PLC_PCOUP_CHANNEL_DATA srvPlcCoupChn8Data = {
  SRV_PCOUP_CHN8_RMS_HIGH_TBL, SRV_PCOUP_CHN8_RMS_VLOW_TBL,
  SRV_PCOUP_CHN8_THRS_HIGH_TBL, SRV_PCOUP_CHN8_THRS_VLOW_TBL,
  SRV_PCOUP_CHN8_GAIN_HIGH_TBL, SRV_PCOUP_CHN8_GAIN_VLOW_TBL,
  SRV_PCOUP_CHN8_MAX_NUM_TX_LEVELS, SRV_PCOUP_EQU_NUM_COEF_CHN << 1, SRV_PCOUP_CHN8_LINE_DRV_CONF,
  srvPlcCoupDaccTableFcc,
  srvPlcCoupPredistCoefDummy, srvPlcCoupPredistCoefDummy
};

static const SRV_PLC_PCOUP_CHANNEL_DATA * srvPlcCoupChnData[] = {
    NULL,
    &srvPlcCoupChn1Data,
    NULL,
    &srvPlcCoupChn3Data,
    &srvPlcCoupChn4Data,
    &srvPlcCoupChn5Data,
    &srvPlcCoupChn6Data,
    &srvPlcCoupChn7Data,
    &srvPlcCoupChn8Data,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

// *****************************************************************************
// *****************************************************************************
// Section: File scope functions
// *****************************************************************************
// *****************************************************************************

SRV_PLC_PCOUP_CHANNEL SRV_PCOUP_Get_Default_Channel( void )
{
  return SRV_PCOUP_DEFAULT_CHANNEL;
}

SRV_PLC_PCOUP_CHANNEL_DATA * SRV_PCOUP_Get_Channel_Data(SRV_PLC_PCOUP_CHANNEL channel)
{
    if ((channel >= CHN1) && (channel <= CHN7_CHN8))
    {
        return (SRV_PLC_PCOUP_CHANNEL_DATA *)srvPlcCoupChnData[channel];
    }
    else
    {
        return NULL;
    }
}
