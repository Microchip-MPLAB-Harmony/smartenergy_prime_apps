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
    PLC PHY Cpupling service. It helps to configure the PLC PHY Coupling 
    parameters through PLC Driver PIB interface.
*******************************************************************************/

//DOM-IGNORE-BEGIN
/*******************************************************************************
* Copyright (C) 2022 Microchip Technology Inc. and its subsidiaries.
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
#include "service/pcoup/srv_pcoup.h"

// *****************************************************************************
/* PLC PHY Coupling equalization data

  Summary:
    Holds the Tx equalization coefficients tables.

  Description:
    Pre-distorsion applies specific gain factor for each carrier, compensating 
    the frequency response of the external analog filter, and equalizing the 
    the transmitted signal.

  Remarks:
    Values are defined in srv_pcoup.h file. Different values for HIGH and VLOW 
    modes
 */

static const uint16_t srvPlcCoupPredistCoefChn1High[SRV_PCOUP_EQU_NUM_COEF_CHN] = SRV_PCOUP_PRED_CHN1_HIGH_TBL;
static const uint16_t srvPlcCoupPredistCoefChn1Low[SRV_PCOUP_EQU_NUM_COEF_CHN] = SRV_PCOUP_PRED_CHN1_VLOW_TBL;

static const uint16_t srvPlcCoupPredistCoefDummy[SRV_PCOUP_EQU_NUM_COEF_CHN] = SRV_PCOUP_PRED_NOT_USED;

/* Configuration values of internal DACC peripheral */
static const uint32_t srvPlcCoupDaccTableCenA[17] = SRV_PCOUP_DACC_CENA_TBL;

static const uint32_t srvPlcCoupDaccTableFcc[17] = SRV_PCOUP_DACC_FCC_TBL;


/* PLC PHY Coupling data

  Summary:
    PLC PHY Coupling data.

  Description:
    This structure contains all the data required to set the PLC PHY Coupling 
    parameters, for each PRIME channel.

  Remarks:
    Values are defined in srv_pcoup.h file
 */

static const SRV_PLC_PCOUP_CHANNEL_DATA srvPlcCoupChn1Data = {
  SRV_PCOUP_CHN1_RMS_HIGH_TBL, SRV_PCOUP_CHN1_RMS_VLOW_TBL,
  SRV_PCOUP_CHN1_THRS_HIGH_TBL, SRV_PCOUP_CHN1_THRS_VLOW_TBL,
  srvPlcCoupDaccTableCenA,
  srvPlcCoupPredistCoefChn1High, srvPlcCoupPredistCoefChn1Low,
  SRV_PCOUP_CHN1_GAIN_HIGH_TBL, SRV_PCOUP_CHN1_GAIN_VLOW_TBL,
  SRV_PCOUP_CHN1_MAX_NUM_TX_LEVELS, SRV_PCOUP_CHN1_LINE_DRV_CONF
  
};

static const SRV_PLC_PCOUP_CHANNEL_DATA srvPlcCoupChn2Data = {
  SRV_PCOUP_CHN2_RMS_HIGH_TBL, SRV_PCOUP_CHN2_RMS_VLOW_TBL,
  SRV_PCOUP_CHN2_THRS_HIGH_TBL, SRV_PCOUP_CHN2_THRS_VLOW_TBL,
  srvPlcCoupDaccTableFcc,
  srvPlcCoupPredistCoefDummy, srvPlcCoupPredistCoefDummy,
  SRV_PCOUP_CHN2_GAIN_HIGH_TBL, SRV_PCOUP_CHN2_GAIN_VLOW_TBL,
  SRV_PCOUP_CHN2_MAX_NUM_TX_LEVELS, SRV_PCOUP_CHN2_LINE_DRV_CONF
};

static const SRV_PLC_PCOUP_CHANNEL_DATA srvPlcCoupChn3Data = {
  SRV_PCOUP_CHN3_RMS_HIGH_TBL, SRV_PCOUP_CHN3_RMS_VLOW_TBL,
  SRV_PCOUP_CHN3_THRS_HIGH_TBL, SRV_PCOUP_CHN3_THRS_VLOW_TBL,
  srvPlcCoupDaccTableFcc,
  srvPlcCoupPredistCoefDummy, srvPlcCoupPredistCoefDummy,
  SRV_PCOUP_CHN3_GAIN_HIGH_TBL, SRV_PCOUP_CHN3_GAIN_VLOW_TBL,
  SRV_PCOUP_CHN3_MAX_NUM_TX_LEVELS, SRV_PCOUP_CHN3_LINE_DRV_CONF
};

static const SRV_PLC_PCOUP_CHANNEL_DATA srvPlcCoupChn4Data = {
  SRV_PCOUP_CHN4_RMS_HIGH_TBL, SRV_PCOUP_CHN4_RMS_VLOW_TBL,
  SRV_PCOUP_CHN4_THRS_HIGH_TBL, SRV_PCOUP_CHN4_THRS_VLOW_TBL,
  srvPlcCoupDaccTableFcc,
  srvPlcCoupPredistCoefDummy, srvPlcCoupPredistCoefDummy,
  SRV_PCOUP_CHN4_GAIN_HIGH_TBL, SRV_PCOUP_CHN4_GAIN_VLOW_TBL,
  SRV_PCOUP_CHN4_MAX_NUM_TX_LEVELS, SRV_PCOUP_CHN4_LINE_DRV_CONF
};

static const SRV_PLC_PCOUP_CHANNEL_DATA srvPlcCoupChn5Data = {
  SRV_PCOUP_CHN5_RMS_HIGH_TBL, SRV_PCOUP_CHN5_RMS_VLOW_TBL,
  SRV_PCOUP_CHN5_THRS_HIGH_TBL, SRV_PCOUP_CHN5_THRS_VLOW_TBL,
  srvPlcCoupDaccTableFcc,
  srvPlcCoupPredistCoefDummy, srvPlcCoupPredistCoefDummy,
  SRV_PCOUP_CHN5_GAIN_HIGH_TBL, SRV_PCOUP_CHN5_GAIN_VLOW_TBL,
  SRV_PCOUP_CHN5_MAX_NUM_TX_LEVELS, SRV_PCOUP_CHN5_LINE_DRV_CONF
};

static const SRV_PLC_PCOUP_CHANNEL_DATA srvPlcCoupChn6Data = {
  SRV_PCOUP_CHN6_RMS_HIGH_TBL, SRV_PCOUP_CHN6_RMS_VLOW_TBL,
  SRV_PCOUP_CHN6_THRS_HIGH_TBL, SRV_PCOUP_CHN6_THRS_VLOW_TBL,
  srvPlcCoupDaccTableFcc,
  srvPlcCoupPredistCoefDummy, srvPlcCoupPredistCoefDummy,
  SRV_PCOUP_CHN6_GAIN_HIGH_TBL, SRV_PCOUP_CHN6_GAIN_VLOW_TBL,
  SRV_PCOUP_CHN6_MAX_NUM_TX_LEVELS, SRV_PCOUP_CHN6_LINE_DRV_CONF
};

static const SRV_PLC_PCOUP_CHANNEL_DATA srvPlcCoupChn7Data = {
  SRV_PCOUP_CHN7_RMS_HIGH_TBL, SRV_PCOUP_CHN7_RMS_VLOW_TBL,
  SRV_PCOUP_CHN7_THRS_HIGH_TBL, SRV_PCOUP_CHN7_THRS_VLOW_TBL,
  srvPlcCoupDaccTableFcc,
  srvPlcCoupPredistCoefDummy, srvPlcCoupPredistCoefDummy,
  SRV_PCOUP_CHN7_GAIN_HIGH_TBL, SRV_PCOUP_CHN7_GAIN_VLOW_TBL,
  SRV_PCOUP_CHN7_MAX_NUM_TX_LEVELS, SRV_PCOUP_CHN7_LINE_DRV_CONF
};

static const SRV_PLC_PCOUP_CHANNEL_DATA srvPlcCoupChn8Data = {
  SRV_PCOUP_CHN8_RMS_HIGH_TBL, SRV_PCOUP_CHN8_RMS_VLOW_TBL,
  SRV_PCOUP_CHN8_THRS_HIGH_TBL, SRV_PCOUP_CHN8_THRS_VLOW_TBL,
  srvPlcCoupDaccTableFcc,
  srvPlcCoupPredistCoefDummy, srvPlcCoupPredistCoefDummy,
  SRV_PCOUP_CHN8_GAIN_HIGH_TBL, SRV_PCOUP_CHN8_GAIN_VLOW_TBL,
  SRV_PCOUP_CHN8_MAX_NUM_TX_LEVELS, SRV_PCOUP_CHN8_LINE_DRV_CONF
};

static const SRV_PLC_PCOUP_CHANNEL_DATA * srvPlcCoupChnData[16] = {
    NULL,
    &srvPlcCoupChn1Data,
    &srvPlcCoupChn2Data,
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

DRV_PLC_PHY_CHANNEL SRV_PCOUP_Get_Default_Channel( void )
{
  return SRV_PCOUP_DEFAULT_CHANNEL;
}

SRV_PLC_PCOUP_CHANNEL_DATA * SRV_PCOUP_Get_Channel_Config(DRV_PLC_PHY_CHANNEL channel)
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

bool SRV_PCOUP_Set_Channel_Config(DRV_HANDLE handle, DRV_PLC_PHY_CHANNEL channel)
{
  SRV_PLC_PCOUP_CHANNEL_DATA *pCoupValues;
  DRV_PLC_PHY_PIB_OBJ pibObj;
  bool result;  

  /* Get PLC PHY Coupling parameters for the desired transmission channel */
  pCoupValues = SRV_PCOUP_Get_Channel_Config(channel);

  if (pCoupValues == NULL)
  {
    /* Transmission channel not recognized */
    return false;
  }

  /* Set PLC PHY Coupling parameters */
  pibObj.id = PLC_ID_IC_DRIVER_CFG;
  pibObj.length = 1;
  pibObj.pData = &pCoupValues->lineDrvConf;
  result = DRV_PLC_PHY_PIBSet(handle, &pibObj);

  pibObj.id = PLC_ID_NUM_TX_LEVELS;
  pibObj.pData = &pCoupValues->numTxLevels;
  result &= DRV_PLC_PHY_PIBSet(handle, &pibObj);

  pibObj.id = PLC_ID_DACC_TABLE_CFG;
  pibObj.length = 17 << 2;
  pibObj.pData = (uint8_t *)pCoupValues->daccTable;
  result &= DRV_PLC_PHY_PIBSet(handle, &pibObj);  

  pibObj.id = PLC_ID_MAX_RMS_TABLE_HI;
  pibObj.length = sizeof(pCoupValues->rmsHigh);
  pibObj.pData = (uint8_t *)pCoupValues->rmsHigh;
  result &= DRV_PLC_PHY_PIBSet(handle, &pibObj);

  pibObj.id = PLC_ID_MAX_RMS_TABLE_VLO;
  pibObj.pData = (uint8_t *)pCoupValues->rmsVLow;
  result &= DRV_PLC_PHY_PIBSet(handle, &pibObj);

  pibObj.id = PLC_ID_THRESHOLDS_TABLE_HI;
  pibObj.length = sizeof(pCoupValues->thrsHigh);
  pibObj.pData = (uint8_t *)pCoupValues->thrsHigh;
  result &= DRV_PLC_PHY_PIBSet(handle, &pibObj);

  pibObj.id = PLC_ID_THRESHOLDS_TABLE_VLO;
  pibObj.pData = (uint8_t *)pCoupValues->thrsVLow;
  result &= DRV_PLC_PHY_PIBSet(handle, &pibObj);

  pibObj.id = PLC_ID_GAIN_TABLE_HI;
  pibObj.length = sizeof(pCoupValues->gainHigh);
  pibObj.pData = (uint8_t *)pCoupValues->gainHigh;
  result &= DRV_PLC_PHY_PIBSet(handle, &pibObj);

  pibObj.id = PLC_ID_GAIN_TABLE_VLO;
  pibObj.pData = (uint8_t *)pCoupValues->gainVLow;
  result &= DRV_PLC_PHY_PIBSet(handle, &pibObj);

  pibObj.id = PLC_ID_PREDIST_COEF_TABLE_HI;
  pibObj.length = SRV_PCOUP_EQU_NUM_COEF_CHN << 1;
  pibObj.pData = (uint8_t *)pCoupValues->equHigh;
  result &= DRV_PLC_PHY_PIBSet(handle, &pibObj);

  pibObj.id = PLC_ID_PREDIST_COEF_TABLE_VLO;
  pibObj.pData = (uint8_t *)pCoupValues->equVlow;
  result &= DRV_PLC_PHY_PIBSet(handle, &pibObj);

  return result;
}
