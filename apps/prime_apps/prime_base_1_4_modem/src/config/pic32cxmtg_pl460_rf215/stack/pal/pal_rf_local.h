/*******************************************************************************
  Company:
    Microchip Technology Inc.

  File Name:
    pal_rf_local.h

  Summary:
    Platform Abstraction Layer (PAL) RF Interface Local header file.

  Description:
    Platform Abstraction Layer (PAL) RF Interface Local header file.
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

#ifndef PAL_RF_LOCAL_H
#define PAL_RF_LOCAL_H

// *****************************************************************************
// *****************************************************************************
// Section: File includes
// *****************************************************************************
// *****************************************************************************

#include <stdbool.h>
#include <stdint.h>
#include "configuration.h"
#include "driver/driver_common.h"
#include "driver/rf215/drv_rf215.h"
#include "pal_types.h"

// *****************************************************************************
// *****************************************************************************
// Section: Data Types
// *****************************************************************************
// *****************************************************************************
// *****************************************************************************
/* RF PAL Module Status

  Summary:
    Identifies the current status/state of the RF PAL module.

  Description:
    This enumeration identifies the current status/state of the RF PAL module.

  Remarks:
    This enumeration is the return type for the PAL_RF_Status routine. The
    upper layer must ensure that PAL_RF_Status returns PAL_RF_STATUS_READY
    before performing RF PAL operations.
*/
typedef enum
{
    PAL_RF_STATUS_UNINITIALIZED = SYS_STATUS_UNINITIALIZED,
    PAL_RF_STATUS_BUSY = SYS_STATUS_BUSY,
    PAL_RF_STATUS_READY = SYS_STATUS_READY,
    PAL_RF_STATUS_ERROR = SYS_STATUS_ERROR,
    PAL_RF_STATUS_INVALID_OBJECT = SYS_STATUS_ERROR_EXTENDED - 1,
} PAL_RF_STATUS;

typedef struct
{
    uint8_t *pData;
    uint8_t buffId;
    DRV_RF215_TX_HANDLE txHandle;
} PAL_RF_TX_DATA;

// *****************************************************************************
/* PAL RF Data

  Summary:
    Holds PAL RF internal data.

  Description:
    This data type defines the data required to handle the PAL RF module.

  Remarks:
    None.
*/
typedef struct
{
    PAL_CALLBACKS rfCallbacks;

    PAL_RF_STATUS status;

    SYS_STATUS drvPhyStatus;

    DRV_HANDLE drvRfPhyHandle;

    DRV_RF215_PHY_CFG_OBJ rfPhyConfig;

    PAL_RF_TX_DATA txData[DRV_RF215_TX_BUFFERS_NUMBER];

    uint16_t currentChannel;

    uint16_t rfChannelsNumber;

    PAL_USI_SNIFFER_CB snifferCallback;

    SRV_USI_HANDLE usiHandler;

} PAL_RF_DATA;

#endif // #ifndef PAL_RF_LOCAL_H
/*******************************************************************************
 End of File
*/