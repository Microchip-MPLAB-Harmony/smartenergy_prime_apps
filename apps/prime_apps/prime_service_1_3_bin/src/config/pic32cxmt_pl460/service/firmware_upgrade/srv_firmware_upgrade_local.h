/*******************************************************************************
  Company:
    Microchip Technology Inc.

  File Name:
    srv_firmware_upgrade_local.h

  Summary:
    PRIME Firmware Upgrade Service Interface Local Header File.

  Description:
    PRIME Firmware Upgrade Service Interface Local Header File.

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

#ifndef SRV_FIRMWARE_UPGRADE_LOCAL_H
#define SRV_FIRMWARE_UPGRADE_LOCAL_H


// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "driver/memory/drv_memory.h"
#include "system/system_media.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    extern "C" {

#endif
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Type Definitions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Firmware Upgrade Memory States

  Summary:
    Firmware Upgrade Memory States enumeration

  Description:
    This enumeration defines the valid Firmware Upgrade Memory states. These states
    determine the behavior of the application at various times.
*/

typedef enum
{
    /* Open the flash driver */
    SRV_FU_MEM_STATE_OPEN_DRIVER,

    /* Get the geometry details */
    SRV_FU_MEM_STATE_GEOMETRY_GET,

    /* Write one block to Memory */
    SRV_FU_MEM_STATE_WRITE_ONE_BLOCK,
    
    /* Wait end of writing one block to Memory */
    SRV_FU_MEM_STATE_WRITE_WAIT_END,
   
    /* Read From Memory */
    SRV_FU_MEM_STATE_READ_MEMORY,

    /* Erase Flash */
    SRV_FU_MEM_STATE_ERASE_FLASH,

    /* Calculate CRC */
    SRV_FU_CALCULATE_CRC_BLOCK,
       
    /* Wait for transfer to complete */
    SRV_FU_MEM_STATE_XFER_WAIT,

    /* Transfer success */
    SRV_FU_MEM_STATE_SUCCESS,

    /* Wait for commands */
    SRV_FU_MEM_STATE_CMD_WAIT,

    /* Attached memory cannot be initialized */
    SRV_FU_MEM_UNINITIALIZED

} SRV_FU_MEM_STATES;

// *****************************************************************************
/* Memory information Data

  Summary:
    Holds the relevant memory information

  Description:
    This structure holds the relevant memory information

  Remarks:
    -
 */

typedef struct
{
    /* Memory transactions's current state */
    SRV_FU_MEM_STATES state;

    /* Driver Handle */
    DRV_HANDLE memoryHandle;

    /* Erase/Write/Read Command Handles */
    DRV_MEMORY_COMMAND_HANDLE eraseHandle;
    DRV_MEMORY_COMMAND_HANDLE writeHandle;
    DRV_MEMORY_COMMAND_HANDLE readHandle;

    uint32_t startAdressFuRegion;

    uint32_t sizeFuRegion;

    uint32_t eraseBlockStart;
    uint32_t numFuRegionEraseBlocks;

    uint32_t writePageSize;
    uint32_t writeAddress;
    uint32_t writeSize;
    uint32_t bytesWritten;

    uint32_t retrieveAddress;
    
    uint32_t readPageSize;

} SRV_FU_MEM_INFO;



// *****************************************************************************
/* CRC calculation states

  Summary:
    CRC calculation states

  Description:
    States of the CRC calculation

  Remarks:
    -
 */

typedef enum
{
  SRV_FU_CRC_IDLE,
  SRV_FU_CRC_WAIT_READ_BLOCK,
  SRC_FU_CRC_CALCULATING
} SRV_FU_CRC_STATE;


// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    }

#endif
// DOM-IGNORE-END

#endif // #ifndef SRV_FIRMWARE_UPGRADE_LOCAL_H
/*******************************************************************************
 End of File
*/