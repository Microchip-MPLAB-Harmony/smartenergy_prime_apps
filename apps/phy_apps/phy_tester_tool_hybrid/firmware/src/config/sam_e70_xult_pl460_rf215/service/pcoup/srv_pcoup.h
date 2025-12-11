/*******************************************************************************
  PLC PHY Coupling Service Library Interface Header File

  Company
    Microchip Technology Inc.

  File Name
    srv_pcoup.h

  Summary
    PLC PHY Coupling service library interface.

  Description
    The Microchip G3-PLC and PRIME implementations include default PHY layer
    configuration values optimized for the Evaluation Kits. With the help of
    the PHY Calibration Tool it is possible to obtain the optimal configuration
    values for the customer's hardware implementation. Refer to the online
    documentation for more details about the available configuration values and
    their purpose.

  Remarks:
    This service provides the required information to be included on PLC
    projects for PL360/PL460 in order to apply the custom calibration.
*******************************************************************************/

// DOM-IGNORE-BEGIN
/*
Copyright (C) 2023, Microchip Technology Inc., and its subsidiaries. All rights reserved.

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
// DOM-IGNORE-END

#ifndef SRV_PCOUP_H    // Guards against multiple inclusion
#define SRV_PCOUP_H


// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include "system/system.h"
#include "driver/plc/phy/drv_plc_phy.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    extern "C" {

#endif
// DOM-IGNORE-END

/* PLC PRIME PHY default channel */
#define SRV_PCOUP_DEFAULT_CHANNEL                CHN6

/* PLC PRIME PHY Channel for impedance detection */
#define SRV_PCOUP_CHANNEL_IMP_DET                CHN3

/* List of channels that support impedance detection */
#define SRV_PCOUP_CHANNEL_LIST_IMP_DET           32509

/* PLC PRIME PHY Channel List */
#define SRV_PCOUP_CHANNEL_LIST                   32767

/* Equalization number of coefficients (number of carriers) */
#define SRV_PCOUP_EQU_NUM_COEF_CHN               97U
#define SRV_PCOUP_EQU_NUM_COEF_2_CHN             (SRV_PCOUP_EQU_NUM_COEF_CHN << 1)

/* RMS_CALC Carrier Mask size in bytes */
#define SRV_PCOUP_CARRIER_MASK_SIZE_CHN          13U
#define SRV_PCOUP_CARRIER_MASK_SIZE_2_CHN        25U

/* Equalization coefficients tables */
#define SRV_PCOUP_PRED_CHN1_HIGH_TBL             {0x756E, 0x7396, 0x730A, 0x72EB, 0x72B2, 0x7433, 0x755E, 0x75D7, 0x769E, 0x76A4, 0x77C3, 0x7851, 0x7864, 0x78A0, \
                                                 0x78BA, 0x7918, 0x79B6, 0x79E9, 0x7ACC, 0x7B06, 0x7B30, 0x7B27, 0x7C1E, 0x7B96, 0x7A76, 0x7B12, 0x7AFD, 0x7C40, \
                                                 0x7C5E, 0x7B48, 0x7B8A, 0x7C64, 0x7C42, 0x7BCD, 0x7AFD, 0x7A5F, 0x7A03, 0x7A9D, 0x7A1A, 0x7A4A, 0x79FC, 0x7984, \
                                                 0x7A0D, 0x79CC, 0x792E, 0x780D, 0x7676, 0x75E4, 0x747A, 0x7251, 0x707E, 0x6E96, 0x6E30, 0x6D44, 0x6DBD, 0x6C9A, \
                                                 0x6C3C, 0x6CF8, 0x6CA4, 0x6CDF, 0x6C59, 0x6B2C, 0x6CB9, 0x6C1F, 0x6B6D, 0x6BF5, 0x6AF0, 0x6A55, 0x6955, 0x674F, \
                                                 0x6841, 0x685D, 0x670F, 0x6904, 0x6967, 0x6B01, 0x6C31, 0x6C2A, 0x6D82, 0x6F58, 0x6E62, 0x6F18, 0x6EE7, 0x7069, \
                                                 0x717B, 0x7120, 0x7170, 0x72FB, 0x7491, 0x75B3, 0x75A2, 0x7664, 0x784A, 0x7A52, 0x7B51, 0x7D5A, 0x7FFF}

#define SRV_PCOUP_PRED_CHN1_VLOW_TBL             {0x7FFF, 0x7F2B, 0x7E38, 0x7CD3, 0x7B38, 0x7972, 0x77D6, 0x7654, 0x74AE, 0x7288, 0x70C0, 0x6E9A, 0x6D24, 0x6B80, \
                                                 0x6A2F, 0x6852, 0x674E, 0x65DA, 0x652E, 0x637E, 0x6292, 0x6142, 0x60CC, 0x5FF8, 0x5F6D, 0x5EC2, 0x5E6F, 0x5E55, \
                                                 0x5E43, 0x5E02, 0x5E5B, 0x5EB3, 0x5F4A, 0x5FD7, 0x604C, 0x60FC, 0x61F3, 0x6297, 0x63A9, 0x643D, 0x654A, 0x6634, \
                                                 0x675C, 0x6824, 0x6910, 0x69A4, 0x6A73, 0x6B6F, 0x6C15, 0x6CCD, 0x6D64, 0x6E4B, 0x6ED3, 0x6F44, 0x6F85, 0x70A1, \
                                                 0x70AF, 0x71B2, 0x7149, 0x71F3, 0x7203, 0x7279, 0x71FB, 0x72B4, 0x7281, 0x72A4, 0x7262, 0x72BD, 0x7295, 0x72CC, \
                                                 0x729E, 0x7288, 0x7244, 0x7279, 0x726C, 0x7230, 0x71B9, 0x70D8, 0x7045, 0x7052, 0x6F8D, 0x6F3D, 0x6EB0, 0x6E6A, \
                                                 0x6E76, 0x6E1C, 0x6D7A, 0x6D84, 0x6D50, 0x6D45, 0x6CF2, 0x6CA9, 0x6C92, 0x6CBA, 0x6C69, 0x6C27, 0x6C02}
#define SRV_PCOUP_PRED_NOT_USED                  {0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, \
                                                 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, \
                                                 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, \
                                                 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, \
                                                 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, \
                                                 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, \
                                                 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF}

#define SRV_PCOUP_PRED_2CHN_NOT_USED             {0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, \
                                                 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, \
                                                 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, \
                                                 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, \
                                                 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, \
                                                 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, \
                                                 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, \
                                                 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, \
                                                 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, \
                                                 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, \
                                                 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, \
                                                 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, \
                                                 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, \
                                                 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF}

/* RMS_CALC Carrier Mask tables */
#define SRV_PCOUP_CHN1_CARRIER_MASK_HIGH_TBL     {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x80}
#define SRV_PCOUP_CHN1_CARRIER_MASK_VLOW_TBL     {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x80}

#define SRV_PCOUP_CHN2_CARRIER_MASK_HIGH_TBL     {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x80}
#define SRV_PCOUP_CHN2_CARRIER_MASK_VLOW_TBL     {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x80}

#define SRV_PCOUP_CHN3_CARRIER_MASK_HIGH_TBL     {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x80}
#define SRV_PCOUP_CHN3_CARRIER_MASK_VLOW_TBL     {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x80}

#define SRV_PCOUP_CHN4_CARRIER_MASK_HIGH_TBL     {0xFF, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
#define SRV_PCOUP_CHN4_CARRIER_MASK_VLOW_TBL     {0xFF, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}

#define SRV_PCOUP_CHN5_CARRIER_MASK_HIGH_TBL     {0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
#define SRV_PCOUP_CHN5_CARRIER_MASK_VLOW_TBL     {0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}

#define SRV_PCOUP_CHN6_CARRIER_MASK_HIGH_TBL     {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F, 0xFF, 0x80}
#define SRV_PCOUP_CHN6_CARRIER_MASK_VLOW_TBL     {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F, 0xFF, 0x80}

#define SRV_PCOUP_CHN7_CARRIER_MASK_HIGH_TBL     {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xFF, 0xFF, 0x80}
#define SRV_PCOUP_CHN7_CARRIER_MASK_VLOW_TBL     {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xFF, 0xFF, 0x80}

#define SRV_PCOUP_CHN8_CARRIER_MASK_HIGH_TBL     {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0xFF, 0xFF, 0x80}
#define SRV_PCOUP_CHN8_CARRIER_MASK_VLOW_TBL     {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0xFF, 0xFF, 0x80}

#define SRV_PCOUP_CHN12_CARRIER_MASK_HIGH_TBL    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xFF, 0xC0}
#define SRV_PCOUP_CHN12_CARRIER_MASK_VLOW_TBL    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xFF, 0xC0}

#define SRV_PCOUP_CHN23_CARRIER_MASK_HIGH_TBL    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F, 0xFF, 0xF0, 0x00, 0x00, 0x00}
#define SRV_PCOUP_CHN23_CARRIER_MASK_VLOW_TBL    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F, 0xFF, 0xF0, 0x00, 0x00, 0x00}

#define SRV_PCOUP_CHN34_CARRIER_MASK_HIGH_TBL    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xFF, 0xFF, 0xFF, 0xF0, 0x00, 0x00, \
                                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
#define SRV_PCOUP_CHN34_CARRIER_MASK_VLOW_TBL    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xFF, 0xFF, 0xFF, 0xF0, 0x00, 0x00, \
                                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}

#define SRV_PCOUP_CHN45_CARRIER_MASK_HIGH_TBL    {0xFF, 0xFF, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
#define SRV_PCOUP_CHN45_CARRIER_MASK_VLOW_TBL    {0xFF, 0xFF, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}

#define SRV_PCOUP_CHN56_CARRIER_MASK_HIGH_TBL    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, \
                                                 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0}
#define SRV_PCOUP_CHN56_CARRIER_MASK_VLOW_TBL    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, \
                                                 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0}

#define SRV_PCOUP_CHN67_CARRIER_MASK_HIGH_TBL    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F, 0xFF, 0xFF, 0xFF, 0xC0}
#define SRV_PCOUP_CHN67_CARRIER_MASK_VLOW_TBL    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F, 0xFF, 0xFF, 0xFF, 0xC0}

#define SRV_PCOUP_CHN78_CARRIER_MASK_HIGH_TBL    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00}
#define SRV_PCOUP_CHN78_CARRIER_MASK_VLOW_TBL    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00}

/* DACC configuration tables */
#define SRV_PCOUP_DACC_CENA_TBL                  {0x0UL, 0x21200000UL, 0x73f0000UL, 0x3f3f0000UL, 0xcccUL, 0x0UL, \
                                                 0xa92c00ffUL, 0x1a1a1a1aUL, 0x20200000UL, 0x4400UL, 0xfd20005UL, 0x3aaUL, \
                                                 0xf0000000UL, 0x1020f0UL, 0x3aaUL, 0xf0000000UL, 0x1020ffUL}

#define SRV_PCOUP_DACC_FCC_TBL                   {0x0UL, 0x0UL, 0x100UL, 0x100UL, 0x0UL, 0x0UL, \
                                                 0xffff00ffUL, 0x1b1b1b1bUL, 0x0UL, 0x0UL, 0x6UL, 0x355UL, \
                                                 0x0UL, 0x1020f0UL, 0x355UL, 0x0UL, 0x1020ffUL}

#define SRV_PCOUP_DACC_2CHN_TBL                  {0x0UL, 0x0UL, 0x100UL, 0x100UL, 0x0UL, 0x0UL, \
                                                 0xffff00ffUL, 0x17171717UL, 0x0UL, 0x0UL, 0x6UL, 0x355UL, \
                                                 0x0UL, 0x1020f0UL, 0x355UL, 0x0UL, 0x1020ffUL}


/* PLC PHY Coupling parameters for each PRIME channel */
#define SRV_PCOUP_CHN1_RMS_HIGH_TBL              {1725, 1522, 1349, 1202, 1071, 957, 855, 764}
#define SRV_PCOUP_CHN1_RMS_VLOW_TBL              {4874, 4427, 3986, 3555, 3157, 2795, 2470, 2184}
#define SRV_PCOUP_CHN1_THRS_HIGH_TBL             {0, 0, 0, 0, 0, 0, 0, 0, 1467, 1292, 1145, 1019, 910, 811, 725, 648}
#define SRV_PCOUP_CHN1_THRS_VLOW_TBL             {0, 0, 0, 0, 0, 0, 0, 0, 8479, 7515, 6665, 5874, 5192, 4576, 4030, 3557}
#define SRV_PCOUP_CHN1_GAIN_HIGH_TBL             {81, 40, 128}
#define SRV_PCOUP_CHN1_GAIN_VLOW_TBL             {256, 128, 281}
#define SRV_PCOUP_CHN1_LINE_DRV_CONF             8
#define SRV_PCOUP_CHN1_MAX_NUM_TX_LEVELS         8

#define SRV_PCOUP_CHN2_RMS_HIGH_TBL              {2809, 2458, 2157, 1891, 1658, 1456, 1283, 1126}
#define SRV_PCOUP_CHN2_RMS_VLOW_TBL              {4873, 4485, 4074, 3674, 3269, 2905, 2569, 2265}
#define SRV_PCOUP_CHN2_THRS_HIGH_TBL             {0, 0, 0, 0, 0, 0, 0, 0, 2520, 2203, 1927, 1694, 1484, 1304, 1146, 1010}
#define SRV_PCOUP_CHN2_THRS_VLOW_TBL             {0, 0, 0, 0, 0, 0, 0, 0, 7315, 6456, 5692, 4999, 4378, 3838, 3362, 2947}
#define SRV_PCOUP_CHN2_GAIN_HIGH_TBL             {120, 60, 256}
#define SRV_PCOUP_CHN2_GAIN_VLOW_TBL             {287, 128, 322}
#define SRV_PCOUP_CHN2_LINE_DRV_CONF             5
#define SRV_PCOUP_CHN2_MAX_NUM_TX_LEVELS         8

#define SRV_PCOUP_CHN3_RMS_HIGH_TBL              {725, 642, 568, 503, 445, 395, 350, 310}
#define SRV_PCOUP_CHN3_RMS_VLOW_TBL              {2826, 2597, 2365, 2128, 1901, 1688, 1494, 1322}
#define SRV_PCOUP_CHN3_THRS_HIGH_TBL             {0, 0, 0, 0, 0, 0, 0, 0, 545, 482, 427, 378, 334, 296, 262, 233}
#define SRV_PCOUP_CHN3_THRS_VLOW_TBL             {0, 0, 0, 0, 0, 0, 0, 0, 8280, 7336, 6495, 5741, 5072, 4480, 3957, 3493}
#define SRV_PCOUP_CHN3_GAIN_HIGH_TBL             {30, 15, 256}
#define SRV_PCOUP_CHN3_GAIN_VLOW_TBL             {287, 128, 322}
#define SRV_PCOUP_CHN3_LINE_DRV_CONF             5
#define SRV_PCOUP_CHN3_MAX_NUM_TX_LEVELS         8

#define SRV_PCOUP_CHN4_RMS_HIGH_TBL              {4401, 3953, 3544, 3178, 2848, 2550, 2283, 2044}
#define SRV_PCOUP_CHN4_RMS_VLOW_TBL              {7515, 6847, 6185, 5532, 4920, 4359, 3861, 3416}
#define SRV_PCOUP_CHN4_THRS_HIGH_TBL             {0, 0, 0, 0, 0, 0, 0, 0, 3249, 2918, 2617, 2349, 2105, 1885, 1689, 1513}
#define SRV_PCOUP_CHN4_THRS_VLOW_TBL             {0, 0, 0, 0, 0, 0, 0, 0, 17622, 15870, 14283, 12851, 11559, 10412, 9370, 8435}
#define SRV_PCOUP_CHN4_GAIN_HIGH_TBL             {60, 30, 256}
#define SRV_PCOUP_CHN4_GAIN_VLOW_TBL             {256, 128, 287}
#define SRV_PCOUP_CHN4_LINE_DRV_CONF             5
#define SRV_PCOUP_CHN4_MAX_NUM_TX_LEVELS         8

#define SRV_PCOUP_CHN5_RMS_HIGH_TBL              {4382, 3919, 3505, 3135, 2804, 2507, 2243, 2005}
#define SRV_PCOUP_CHN5_RMS_VLOW_TBL              {9138, 8340, 7542, 6767, 6044, 5382, 4789, 4258}
#define SRV_PCOUP_CHN5_THRS_HIGH_TBL             {0, 0, 0, 0, 0, 0, 0, 0, 3804, 3403, 3044, 2723, 2435, 2179, 1949, 1743}
#define SRV_PCOUP_CHN5_THRS_VLOW_TBL             {0, 0, 0, 0, 0, 0, 0, 0, 12364, 11115, 9963, 8933, 7999, 7169, 6414, 5743}
#define SRV_PCOUP_CHN5_GAIN_HIGH_TBL             {85, 40, 256}
#define SRV_PCOUP_CHN5_GAIN_VLOW_TBL             {256, 128, 287}
#define SRV_PCOUP_CHN5_LINE_DRV_CONF             5
#define SRV_PCOUP_CHN5_MAX_NUM_TX_LEVELS         8

#define SRV_PCOUP_CHN6_RMS_HIGH_TBL              {2919, 2598, 2315, 2063, 1838, 1638, 1460, 1301}
#define SRV_PCOUP_CHN6_RMS_VLOW_TBL              {8622, 7866, 7095, 6373, 5691, 5080, 4525, 4034}
#define SRV_PCOUP_CHN6_THRS_HIGH_TBL             {0, 0, 0, 0, 0, 0, 0, 0, 2565, 2285, 2036, 1814, 1616, 1441, 1283, 1144}
#define SRV_PCOUP_CHN6_THRS_VLOW_TBL             {0, 0, 0, 0, 0, 0, 0, 0, 12460, 11102, 9889, 8802, 7833, 6973, 6199, 5523}
#define SRV_PCOUP_CHN6_GAIN_HIGH_TBL             {60, 30, 256}
#define SRV_PCOUP_CHN6_GAIN_VLOW_TBL             {256, 128, 287}
#define SRV_PCOUP_CHN6_LINE_DRV_CONF             5
#define SRV_PCOUP_CHN6_MAX_NUM_TX_LEVELS         8

#define SRV_PCOUP_CHN7_RMS_HIGH_TBL              {3468, 3083, 2740, 2436, 2165, 1926, 1712, 1523}
#define SRV_PCOUP_CHN7_RMS_VLOW_TBL              {8835, 8038, 7235, 6478, 5780, 5146, 4583, 4081}
#define SRV_PCOUP_CHN7_THRS_HIGH_TBL             {0, 0, 0, 0, 0, 0, 0, 0, 2914, 2590, 2301, 2047, 1819, 1618, 1438, 1279}
#define SRV_PCOUP_CHN7_THRS_VLOW_TBL             {0, 0, 0, 0, 0, 0, 0, 0, 15406, 13630, 12059, 10675, 9453, 8374, 7418, 6578}
#define SRV_PCOUP_CHN7_GAIN_HIGH_TBL             {60, 30, 256}
#define SRV_PCOUP_CHN7_GAIN_VLOW_TBL             {256, 128, 287}
#define SRV_PCOUP_CHN7_LINE_DRV_CONF             5
#define SRV_PCOUP_CHN7_MAX_NUM_TX_LEVELS         8

#define SRV_PCOUP_CHN8_RMS_HIGH_TBL              {2168, 1929, 1717, 1528, 1361, 1212, 1080, 962}
#define SRV_PCOUP_CHN8_RMS_VLOW_TBL              {10206, 9351, 8496, 7640, 6824, 6078, 5407, 4809}
#define SRV_PCOUP_CHN8_THRS_HIGH_TBL             {0, 0, 0, 0, 0, 0, 0, 0, 1628, 1448, 1288, 1146, 1021, 909, 810, 720}
#define SRV_PCOUP_CHN8_THRS_VLOW_TBL             {0, 0, 0, 0, 0, 0, 0, 0, 23125, 20543, 18202, 16117, 14268, 12632, 11193, 9912}
#define SRV_PCOUP_CHN8_GAIN_HIGH_TBL             {30, 15, 256}
#define SRV_PCOUP_CHN8_GAIN_VLOW_TBL             {287, 128, 322}
#define SRV_PCOUP_CHN8_LINE_DRV_CONF             5
#define SRV_PCOUP_CHN8_MAX_NUM_TX_LEVELS         8

#define SRV_PCOUP_CHN12_RMS_HIGH_TBL             {1346, 1184, 1041, 915, 807, 710, 626, 552}
#define SRV_PCOUP_CHN12_RMS_VLOW_TBL             {2503, 2285, 2064, 1849, 1645, 1457, 1288, 1141}
#define SRV_PCOUP_CHN12_THRS_HIGH_TBL            {0, 0, 0, 0, 0, 0, 0, 0, 1216, 1069, 940, 826, 728, 641, 564, 498}
#define SRV_PCOUP_CHN12_THRS_VLOW_TBL            {0, 0, 0, 0, 0, 0, 0, 0, 3712, 3276, 2889, 2538, 2227, 1954, 1718, 1510}
#define SRV_PCOUP_CHN12_GAIN_HIGH_TBL            {120, 60, 256}
#define SRV_PCOUP_CHN12_GAIN_VLOW_TBL            {287, 128, 322}
#define SRV_PCOUP_CHN12_LINE_DRV_CONF            5
#define SRV_PCOUP_CHN12_MAX_NUM_TX_LEVELS        8

#define SRV_PCOUP_CHN23_RMS_HIGH_TBL             {965, 853, 755, 668, 591, 524, 465, 411}
#define SRV_PCOUP_CHN23_RMS_VLOW_TBL             {3533, 3228, 2915, 2610, 2322, 2058, 1822, 1613}
#define SRV_PCOUP_CHN23_THRS_HIGH_TBL            {0, 0, 0, 0, 0, 0, 0, 0, 728, 644, 569, 503, 446, 395, 350, 310}
#define SRV_PCOUP_CHN23_THRS_VLOW_TBL            {0, 0, 0, 0, 0, 0, 0, 0, 10643, 9410, 8312, 7338, 6473, 5713, 5033, 4436}
#define SRV_PCOUP_CHN23_GAIN_HIGH_TBL            {30, 15, 256}
#define SRV_PCOUP_CHN23_GAIN_VLOW_TBL            {287, 128, 322}
#define SRV_PCOUP_CHN23_LINE_DRV_CONF            5
#define SRV_PCOUP_CHN23_MAX_NUM_TX_LEVELS        8

#define SRV_PCOUP_CHN34_RMS_HIGH_TBL             {1020, 903, 798, 706, 626, 554, 490, 435}
#define SRV_PCOUP_CHN34_RMS_VLOW_TBL             {3706, 3386, 3060, 2742, 2436, 2158, 1908, 1687}
#define SRV_PCOUP_CHN34_THRS_HIGH_TBL            {0, 0, 0, 0, 0, 0, 0, 0, 760, 672, 594, 526, 465, 412, 365, 323}
#define SRV_PCOUP_CHN34_THRS_VLOW_TBL            {0, 0, 0, 0, 0, 0, 0, 0, 11058, 9802, 8684, 7688, 6792, 6010, 5309, 4694}
#define SRV_PCOUP_CHN34_GAIN_HIGH_TBL            {30, 15, 256}
#define SRV_PCOUP_CHN34_GAIN_VLOW_TBL            {287, 128, 322}
#define SRV_PCOUP_CHN34_LINE_DRV_CONF            5
#define SRV_PCOUP_CHN34_MAX_NUM_TX_LEVELS        8

#define SRV_PCOUP_CHN45_RMS_HIGH_TBL             {2364, 2118, 1897, 1698, 1520, 1360, 1216, 1088}
#define SRV_PCOUP_CHN45_RMS_VLOW_TBL             {4020, 3638, 3267, 2910, 2580, 2285, 2023, 1790}
#define SRV_PCOUP_CHN45_THRS_HIGH_TBL            {0, 0, 0, 0, 0, 0, 0, 0, 1717, 1537, 1378, 1234, 1105, 989, 886, 792}
#define SRV_PCOUP_CHN45_THRS_VLOW_TBL            {0, 0, 0, 0, 0, 0, 0, 0, 9403, 8476, 7635, 6868, 6187, 5571, 5021, 4520}
#define SRV_PCOUP_CHN45_GAIN_HIGH_TBL            {60, 30, 256}
#define SRV_PCOUP_CHN45_GAIN_VLOW_TBL            {256, 128, 287}
#define SRV_PCOUP_CHN45_LINE_DRV_CONF            5
#define SRV_PCOUP_CHN45_MAX_NUM_TX_LEVELS        8

#define SRV_PCOUP_CHN56_RMS_HIGH_TBL             {930, 830, 741, 661, 590, 526, 470, 419}
#define SRV_PCOUP_CHN56_RMS_VLOW_TBL             {2876, 2613, 2355, 2109, 1882, 1677, 1494, 1331}
#define SRV_PCOUP_CHN56_THRS_HIGH_TBL            {0, 0, 0, 0, 0, 0, 0, 0, 826, 737, 658, 587, 524, 468, 417, 373}
#define SRV_PCOUP_CHN56_THRS_VLOW_TBL            {0, 0, 0, 0, 0, 0, 0, 0, 3835, 3432, 3066, 2737, 2444, 2181, 1947, 1737}
#define SRV_PCOUP_CHN56_GAIN_HIGH_TBL            {60, 30, 256}
#define SRV_PCOUP_CHN56_GAIN_VLOW_TBL            {256, 128, 287}
#define SRV_PCOUP_CHN56_LINE_DRV_CONF            5
#define SRV_PCOUP_CHN56_MAX_NUM_TX_LEVELS        8

#define SRV_PCOUP_CHN67_RMS_HIGH_TBL             {1761, 1565, 1392, 1238, 1101, 980, 872, 775}
#define SRV_PCOUP_CHN67_RMS_VLOW_TBL             {4588, 4160, 3743, 3346, 2983, 2657, 2366, 2107}
#define SRV_PCOUP_CHN67_THRS_HIGH_TBL            {0, 0, 0, 0, 0, 0, 0, 0, 1498, 1332, 1184, 1053, 937, 833, 741, 660}
#define SRV_PCOUP_CHN67_THRS_VLOW_TBL            {0, 0, 0, 0, 0, 0, 0, 0, 7678, 6807, 6035, 5351, 4745, 4212, 3739, 3319}
#define SRV_PCOUP_CHN67_GAIN_HIGH_TBL            {60, 30, 256}
#define SRV_PCOUP_CHN67_GAIN_VLOW_TBL            {256, 128, 287}
#define SRV_PCOUP_CHN67_LINE_DRV_CONF            5
#define SRV_PCOUP_CHN67_MAX_NUM_TX_LEVELS        8

#define SRV_PCOUP_CHN78_RMS_HIGH_TBL             {1066, 949, 844, 751, 669, 597, 533, 476}
#define SRV_PCOUP_CHN78_RMS_VLOW_TBL             {5202, 4760, 4308, 3869, 3456, 3078, 2740, 2439}
#define SRV_PCOUP_CHN78_THRS_HIGH_TBL            {0, 0, 0, 0, 0, 0, 0, 0, 822, 731, 650, 579, 516, 460, 411, 367}
#define SRV_PCOUP_CHN78_THRS_VLOW_TBL            {0, 0, 0, 0, 0, 0, 0, 0, 10641, 9390, 8291, 7325, 6476, 5730, 5072, 4497}
#define SRV_PCOUP_CHN78_GAIN_HIGH_TBL            {30, 15, 256}
#define SRV_PCOUP_CHN78_GAIN_VLOW_TBL            {287, 128, 322}
#define SRV_PCOUP_CHN78_LINE_DRV_CONF            5
#define SRV_PCOUP_CHN78_MAX_NUM_TX_LEVELS        8


// *****************************************************************************
// *****************************************************************************
// Section: Data Types
// *****************************************************************************
// *****************************************************************************
// *****************************************************************************

/* PLC PHY Coupling data

  Summary:
    PLC PHY Coupling data.

  Description:
    This structure contains all the data required to set the PLC PHY Coupling
    parameters, for a specific PRIME channel.

  Remarks:
    Equalization coefficients and DACC table are not stored in the structure,
    just pointers to arrays were they are actually stored. This allows to use
    the same type for different PRIME channels.
*/

typedef struct
{
    /* Target RMS values in HIGH mode for dynamic Tx gain */
    uint32_t rmsHigh[8];

    /* Target RMS values in VLOW mode for dynamic Tx gain */
    uint32_t rmsVLow[8];

    /* Threshold RMS values in HIGH mode for dynamic Tx mode */
    uint32_t thrsHigh[16];

    /* Threshold RMS values in VLOW mode for dynamic Tx mode */
    uint32_t thrsVLow[16];

    /* Pointer to values for configuration of PLC DACC peripheral, according to
       hardware coupling design and PLC device (PL360/PL460) */
    const uint32_t * daccTable;

    /* Pointer to Tx equalization coefficients table in HIGH mode.
       There is one coefficient for each carrier in the used band */
    const uint16_t * equHigh;

    /* Pointer to Tx equalization coefficients table in VLOW mode.
       There is one coefficient for each carrier in the used band */
    const uint16_t * equVlow;
  
    /* Pointer to RMS_CALC carrier mask table in HIGH mode.
       There is one bit for each carrier in the used band */
    const uint8_t * carrierMaskHigh;

    /* Pointer to RMS_CALC carrier mask table in VLOW mode.
       There is one bit for each carrier in the used band */
    const uint8_t * carrierMaskVlow;

    /* Tx gain values for HIGH mode [HIGH_INI, HIGH_MIN, HIGH_MAX] */
    uint16_t gainHigh[3];

    /* Tx gain values for VLOW mode [VLOW_INI, VLOW_MIN, VLOW_MAX] */
    uint16_t gainVLow[3];

    /* Number of Tx attenuation levels (1 dB step) supporting dynamic Tx mode */
    uint8_t numTxLevels;

    /* Configuration of the PLC Tx Line Driver, according to hardware coupling
       design and PLC device (PL360/PL460) */
    uint8_t lineDrvConf;

} SRV_PLC_PCOUP_CHANNEL_DATA;

// *****************************************************************************
// *****************************************************************************
// Section: Service Interface Functions
// *****************************************************************************
// *****************************************************************************

/***************************************************************************
  Function:
    DRV_PLC_PHY_CHANNEL SRV_PCOUP_GetDefaultChannel(void)

  Summary:
    Get the default PRIME channel.

  Description:
    This function allows to get the PRIME channel used by default.

  Precondition:
    None.

  Parameters:
    None.

  Returns:
    Default PRIME channel.

  Example:
    <code>
    DRV_PLC_PHY_CHANNEL plcDefaultChannel;
    DRV_PLC_PHY_PIB_OBJ pibObj;

    plcDefaultChannel = SRV_PCOUP_GetDefaultChannel();

    pibObj.id = PLC_ID_CHANNEL_CFG;
    pibObj.length = 1;
    pibObj.pData = &plcDefaultChannel;
    DRV_PLC_PHY_PIBSet(handle, &pibObj);

    SRV_PCOUP_SetChannelConfig(handle, plcDefaultChannel);
    </code>

  Remarks:
    None.
  ***************************************************************************/

DRV_PLC_PHY_CHANNEL SRV_PCOUP_GetDefaultChannel( void );

/***************************************************************************
  Function:
    SRV_PLC_PCOUP_CHANNEL_DATA * SRV_PCOUP_GetChannelConfig (DRV_PLC_PHY_CHANNEL channel)

  Summary:
    Get the PLC PHY Coupling parameters for the specified PRIME channel.

  Description:
    This function allows to get the PLC PHY Coupling parameters for the
    specified PRIME channel. These parameters can be sent to the PLC device
    through PLC Driver PIB interface (DRV_PLC_PHY_PIBSet).

  Precondition:
    None.

  Parameters:
    channel   - PRIME channel for which the parameters are requested

  Returns:
    - Pointer to PLC PHY Coupling parameters
      - if channel parameter is valid
    - *NULL*
      - if channel parameter is not valid

  Example:
    <code>
    SRV_PLC_PCOUP_CHANNEL_DATA *pCoupChannelData;

    pCoupChannelData = SRV_PCOUP_GetChannelConfig
(SRV_PCOUP_DEFAULT_CHANNEL);
    </code>

  Remarks:
    If SRV_PCOUP_SetChannelConfig is used to set the PLC PHY Coupling
    parameters, this function is not needed.
  ***************************************************************************/

SRV_PLC_PCOUP_CHANNEL_DATA * SRV_PCOUP_GetChannelConfig (DRV_PLC_PHY_CHANNEL channel);

/***************************************************************************
  Function:
    bool SRV_PCOUP_SetChannelConfig(DRV_HANDLE handle, DRV_PLC_PHY_CHANNEL channel);

  Summary:
    Set the PLC PHY Coupling parameters for the specified PRIME channel.

  Description:
    This function allows to set the PLC PHY Coupling parameters for the
    specified PRIME channel, using the PLC Driver PIB interface
    (DRV_PLC_PHY_PIBSet).

  Precondition:
    DRV_PLC_PHY_Open must have been called to obtain a valid opened device
    handle.

  Parameters:
    handle   - A valid instance handle, returned from DRV_PLC_PHY_Open
    channel  - PRIME channel for which the parameters will be set

  Returns:
    - true
      - Successful configuration
    - false
      - if channel parameter is not valid
      - if there is an error when using the PLC Driver PIB interface

  Example:
    <code>
    bool result;

    result = SRV_PCOUP_SetChannelConfig(handle, CHN5);
    </code>

  Remarks:
    None.
  ***************************************************************************/

bool SRV_PCOUP_SetChannelConfig(DRV_HANDLE handle, DRV_PLC_PHY_CHANNEL channel);

/***************************************************************************
  Function:
    uint16_t SRV_PCOUP_GetChannelList(void)

  Summary:
    Get the PRIME channel list.

  Description:
    This function allows to get the PRIME channel list.

  Precondition:
    None.

  Parameters:
    None.

  Returns:
    PRIME channel list. The channel list is a bitmask corresponding to the 
    following values.
    Single channel:
      Channel 1 : bit 0
      Channel 2 : bit 1
      Channel 3 : bit 2
      Channel 4 : bit 3
      Channel 5 : bit 4
      Channel 6 : bit 5
      Channel 7 : bit 6
      Channel 8 : bit 7
    In double channel:
      Channel 1-2 : bit 8
      Channel 2-3 : bit 9
      Channel 3-4 : bit 10
      Channel 4-5 : bit 11
      Channel 5-6 : bit 12
      Channel 6-7 : bit 13
      Channel 7-8 : bit 14

  Example:
    <code>
    uint16_t plcChannelList;

    plcChannelList = SRV_PCOUP_GetChannelList();

    </code>

  Remarks:
    None.
  ***************************************************************************/

uint16_t SRV_PCOUP_GetChannelList(void);

/***************************************************************************
  Function:
    uint16_t SRV_PCOUP_GetChannelListImpedanceDetection(void)

  Summary:
    Get the PRIME channel list valid for impedance detection.

  Description:
    This function allows to get the PRIME channel list valid for impedance detection.

  Precondition:
    None.

  Parameters:
    None.

  Returns:
    PRIME channel list valid for impedance detection. The channel list is a bitmask
    corresponding to the following values.
    Single channel:
      Channel 1 : bit 0
      Channel 2 : bit 1
      Channel 3 : bit 2
      Channel 4 : bit 3
      Channel 5 : bit 4
      Channel 6 : bit 5
      Channel 7 : bit 6
      Channel 8 : bit 7
    In double channel:
      Channel 1-2 : bit 8
      Channel 2-3 : bit 9
      Channel 3-4 : bit 10
      Channel 4-5 : bit 11
      Channel 5-6 : bit 12
      Channel 6-7 : bit 13
      Channel 7-8 : bit 14

  Example:
    <code>
    uint16_t plcChannelListImpDetect;

    plcChannelListImpDetect = SRV_PCOUP_GetChannelListImpedanceDetection();

    </code>

  Remarks:
    None.
  ***************************************************************************/

uint16_t SRV_PCOUP_GetChannelListImpedanceDetection(void);

/***************************************************************************
  Function:
    DRV_PLC_PHY_CHANNEL SRV_PCOUP_GetChannelImpedanceDetection(void)

  Summary:
    Get the PRIME channel for impedance detection.

  Description:
    This function allows to get the PRIME channel that should be used for impedance detection.

  Precondition:
    None.

  Parameters:
    None.

  Returns:
    PRIME channel that should be used for impedance detection.

  Example:
    <code>
    DRV_PLC_PHY_CHANNEL plcChannelImpDetect;

    plcChannelImpDetect = SRV_PCOUP_GetChannelImpedanceDetection();

    </code>

  Remarks:
    None.
  ***************************************************************************/

DRV_PLC_PHY_CHANNEL SRV_PCOUP_GetChannelImpedanceDetection(void);

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    }

#endif
// DOM-IGNORE-END

#endif //SRV_PCOUP_H
