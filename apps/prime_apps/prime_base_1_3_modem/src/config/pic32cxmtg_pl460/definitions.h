/*******************************************************************************
  System Definitions

  File Name:
    definitions.h

  Summary:
    project system definitions.

  Description:
    This file contains the system-wide prototypes and definitions for a project.

 *******************************************************************************/

//DOM-IGNORE-BEGIN
/*******************************************************************************
* Copyright (C) 2018 Microchip Technology Inc. and its subsidiaries.
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

#ifndef DEFINITIONS_H
#define DEFINITIONS_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "service/pcoup/srv_pcoup.h"
#include "peripheral/flexcom/usart/plib_flexcom7_usart.h"
#include "driver/memory/drv_memory.h"
#include "peripheral/adc/plib_adc.h"
#include "peripheral/flexcom/spi/master/plib_flexcom5_spi_master.h"
#include "peripheral/tc/plib_tc0.h"
#include "service/usi/srv_usi.h"
#include "service/usi/srv_usi_usart.h"
#include "peripheral/flexcom/usart/plib_flexcom0_usart.h"
#include "service/log_report/srv_log_report.h"
#include "system/time/sys_time.h"
#include "driver/plc/phy/drv_plc_phy_definitions.h"
#include "driver/plc/phy/drv_plc_phy.h"
#include "driver/plc/phy/drv_plc_phy_comm.h"
#include "driver/memory/drv_memory_sefc0.h"
#include "service/reset_handler/srv_reset_handler.h"
#include "peripheral/trng/plib_trng.h"
#include "service/psniffer/srv_psniffer.h"
#include "system/int/sys_int.h"
#include "system/ports/sys_ports.h"
#include "system/cache/sys_cache.h"
#include "osal/osal.h"
#include "system/debug/sys_debug.h"
#include "stack/pal/pal.h"
#include "peripheral/dwdt/plib_dwdt.h"
#include "peripheral/clk/plib_clk.h"
#include "peripheral/rstc/plib_rstc.h"
#include "peripheral/nvic/plib_nvic.h"
#include "peripheral/cmcc/plib_cmcc.h"
#include "peripheral/pio/plib_pio.h"
#include "peripheral/supc/plib_supc.h"
#include "bsp/bsp.h"
#include "service/pcrc/srv_pcrc.h"
#include "peripheral/sefc/plib_sefc0.h"
#include "peripheral/sefc/plib_sefc1.h"
#include "service/firmware_upgrade/srv_firmware_upgrade.h"
#include "stack/prime/prime_stack.h"
#include "stack/prime/hal_api/hal_api.h"
#include "stack/prime/prime_api/prime_api.h"
#include "stack/prime/prime_api/prime_api_defs.h"
#include "stack/prime/prime_api/prime_api_types.h"
#include "stack/prime/prime_api/prime_hal_wrapper.h"
#include "stack/prime/mac/mac.h"
#include "stack/prime/mac/mac_defs.h"
#include "stack/prime/mac/mac_pib.h"
#include "stack/prime/mngp/mngp.h"
#include "stack/prime/mngp/bmng_api.h"
#include "stack/prime/mngp/bmng_defs.h"
#include "stack/prime/conv/sscs/null/cl_null.h"
#include "stack/prime/conv/sscs/null/cl_null_api.h"
#include "stack/prime/conv/sscs/iec_4_32/cl_432.h"
#include "stack/prime/conv/sscs/iec_4_32/cl_432_api.h"
#include "stack/prime/conv/sscs/iec_4_32/cl_432_defs.h"
#include "service/user_pib/srv_user_pib.h"
#include "service/pvddmon/srv_pvddmon.h"
#include "service/storage/srv_storage.h"
#include "system/console/sys_console.h"
#include "system/console/src/sys_console_uart_definitions.h"
#include "service/time_management/srv_time_management.h"
#include "app.h"



// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

extern "C" {

#endif
// DOM-IGNORE-END

/* Device Information */
#define DEVICE_NAME          "PIC32CX2051MTG128"
#define DEVICE_ARCH          "CORTEX-M4"
#define DEVICE_FAMILY        "PIC32CX_MT"
#define DEVICE_SERIES        "PIC32CXMTG"

/* CPU clock frequency */
#define CPU_CLOCK_FREQUENCY 200000000U

// *****************************************************************************
// *****************************************************************************
// Section: System Functions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* System Initialization Function

  Function:
    void SYS_Initialize( void *data )

  Summary:
    Function that initializes all modules in the system.

  Description:
    This function initializes all modules in the system, including any drivers,
    services, middleware, and applications.

  Precondition:
    None.

  Parameters:
    data            - Pointer to the data structure containing any data
                      necessary to initialize the module. This pointer may
                      be null if no data is required and default initialization
                      is to be used.

  Returns:
    None.

  Example:
    <code>
    SYS_Initialize ( NULL );

    while ( true )
    {
        SYS_Tasks ( );
    }
    </code>

  Remarks:
    This function will only be called once, after system reset.
*/

void SYS_Initialize( void *data );

// *****************************************************************************
/* System Tasks Function

Function:
    void SYS_Tasks ( void );

Summary:
    Function that performs all polled system tasks.

Description:
    This function performs all polled system tasks by calling the state machine
    "tasks" functions for all polled modules in the system, including drivers,
    services, middleware and applications.

Precondition:
    The SYS_Initialize function must have been called and completed.

Parameters:
    None.

Returns:
    None.

Example:
    <code>
    SYS_Initialize ( NULL );

    while ( true )
    {
        SYS_Tasks ( );
    }
    </code>

Remarks:
    If the module is interrupt driven, the system will call this routine from
    an interrupt context.
*/

void SYS_Tasks ( void );

// *****************************************************************************
// *****************************************************************************
// Section: Type Definitions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* System Objects

Summary:
    Structure holding the system's object handles

Description:
    This structure contains the object handles for all objects in the
    MPLAB Harmony project's system configuration.

Remarks:
    These handles are returned from the "Initialize" functions for each module
    and must be passed into the "Tasks" function for each module.
*/

typedef struct
{
    SYS_MODULE_OBJ  sysTime;

    SYS_MODULE_OBJ drvPlcPhy;
    SYS_MODULE_OBJ  drvMemory0;
    SYS_MODULE_OBJ  sysConsole0;

    SYS_MODULE_OBJ  sysDebug;

    SYS_MODULE_OBJ primeStack;

    SYS_MODULE_OBJ srvUSI0;

} SYSTEM_OBJECTS;

// *****************************************************************************
// *****************************************************************************
// Section: extern declarations
// *****************************************************************************
// *****************************************************************************



extern SYSTEM_OBJECTS sysObj;

//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif /* DEFINITIONS_H */
/*******************************************************************************
 End of File
*/

