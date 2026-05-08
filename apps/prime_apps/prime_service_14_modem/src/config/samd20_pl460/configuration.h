/*******************************************************************************
  System Configuration Header

  File Name:
    configuration.h

  Summary:
    Build-time configuration header for the system defined by this project.

  Description:
    An MPLAB Project may have multiple configurations.  This file defines the
    build-time options for a single configuration.

  Remarks:
    This configuration header must not define any prototypes or data
    definitions (or include any files that do).  It only provides macro
    definitions for build-time configuration options

*******************************************************************************/

// DOM-IGNORE-BEGIN
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
// DOM-IGNORE-END

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
/*  This section Includes other configuration headers necessary to completely
    define this configuration.
*/

#include "user.h"
#include "device.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

extern "C" {

#endif
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: System Configuration
// *****************************************************************************
// *****************************************************************************



// *****************************************************************************
// *****************************************************************************
// Section: System Service Configuration
// *****************************************************************************
// *****************************************************************************
/* TIME System Service Configuration Options */
#define SYS_TIME_INDEX_0                            (0)
#define SYS_TIME_MAX_TIMERS                         (5)
#define SYS_TIME_HW_COUNTER_WIDTH                   (32)
#define SYS_TIME_HW_COUNTER_PERIOD                  (4294967295U)
#define SYS_TIME_HW_COUNTER_HALF_PERIOD             (SYS_TIME_HW_COUNTER_PERIOD>>1)
#define SYS_TIME_CPU_CLOCK_FREQUENCY                (47972352)
#define SYS_TIME_COMPARE_UPDATE_EXECUTION_CYCLES    (200)

#define SYS_CONSOLE_INDEX_0                       0





#define SYS_DEBUG_ENABLE
#define SYS_DEBUG_GLOBAL_ERROR_LEVEL       SYS_ERROR_DEBUG
#define SYS_DEBUG_BUFFER_DMA_READY
#define SYS_DEBUG_USE_CONSOLE


#define SYS_CONSOLE_DEVICE_MAX_INSTANCES   			(1U)
#define SYS_CONSOLE_UART_MAX_INSTANCES 	   			(1U)
#define SYS_CONSOLE_USB_CDC_MAX_INSTANCES 	   		(0U)
#define SYS_CONSOLE_PRINT_BUFFER_SIZE        		(200U)




// *****************************************************************************
// *****************************************************************************
// Section: Driver Configuration
// *****************************************************************************
// *****************************************************************************
/* Memory Driver Global Configuration Options */
#define DRV_MEMORY_INSTANCES_NUMBER          (1U)

/* USI Service Common Configuration Options */
#define SRV_USI_INSTANCES_NUMBER              1U
#define SRV_USI_USART_CONNECTIONS             1U
#define SRV_USI_CDC_CONNECTIONS               0U
#define SRV_USI_MSG_POOL_SIZE                 5U

/* PLC PHY Driver Configuration Options */
#define DRV_PLC_SECURE                        false
#define DRV_PLC_SPI_CS_PIN                    SYS_PORT_PIN_PB17
#define DRV_PLC_EXT_INT_SRC                   EIC_IRQn
#define DRV_PLC_EXT_INT_PIO                   SYS_PORT_PIN_PA28
#define DRV_PLC_EXT_INT_PIN                   EIC_PIN_8
#define DRV_PLC_RESET_PIN                     SYS_PORT_PIN_PB30
#define DRV_PLC_TX_ENABLE_PIN                 SYS_PORT_PIN_PA03
#define DRV_PLC_THMON_PIN                     SYS_PORT_PIN_PA27
#define DRV_PLC_SPI_CLK                       8000000

/* PLC Driver Identification */
#define DRV_PLC_PHY_INSTANCES_NUMBER          1U
#define DRV_PLC_PHY_INDEX                     0U
#define DRV_PLC_PHY_CLIENTS_NUMBER_IDX        1U
#define DRV_PLC_PHY_PROFILE                   4U
#define DRV_PLC_PHY_NUM_CARRIERS_PER_CHANNEL  97U
#define DRV_PLC_PHY_HOST_PRODUCT              0x3600U
#define DRV_PLC_PHY_HOST_VERSION              0x36000300UL
#define DRV_PLC_PHY_HOST_PHY                  0x36000003UL
#define DRV_PLC_PHY_HOST_DESC                 "ATSAMD20J18"
#define DRV_PLC_PHY_HOST_MODEL                3U
#define DRV_PLC_PHY_HOST_BAND                 DRV_PLC_PHY_PROFILE



/* Memory Driver Instance 0 Configuration */
#define DRV_MEMORY_INDEX_0                   0
#define DRV_MEMORY_CLIENTS_NUMBER_IDX0       1
#define DRV_MEMORY_BUF_Q_SIZE_IDX0    1

/* PRIME PAL Configuration Options */
#define PRIME_PAL_INDEX                     0U
#define PRIME_PAL_SNIFFER_USI_INSTANCE      SRV_USI_INDEX_0

/* SST26 Driver Instance Configuration */
#define DRV_SST26_INDEX                 (0U)
#define DRV_SST26_CLIENTS_NUMBER        (1U)
#define DRV_SST26_START_ADDRESS         (0x0U)
#define DRV_SST26_PAGE_SIZE             (256U)
#define DRV_SST26_ERASE_BUFFER_SIZE     (4096U)
#define DRV_SST26_CHIP_SELECT_PIN       SYS_PORT_PIN_PA17


/* USI Service Instance 0 Configuration Options */
#define SRV_USI_INDEX_0                       0
#define SRV_USI0_RD_BUF_SIZE                  300
#define SRV_USI0_WR_BUF_SIZE                  300



// *****************************************************************************
// *****************************************************************************
// Section: Middleware & Other Library Configuration
// *****************************************************************************
// *****************************************************************************
/* Service Firmware Upgrade RTOS configuration */
#define SRV_FU_RTOS_STACK_SIZE         3584U
#define SRV_FU_RTOS_TASK_PRIORITY          2U
#define SRV_FU_RTOS_TASK_DELAY_MS          1U

/* PRIME stack RTOS configuration */
#define PRIME_STACK_RTOS_STACK_SIZE         3584U
#define PRIME_STACK_RTOS_TASK_PRIORITY          2U
#define PRIME_STACK_RTOS_TASK_DELAY_MS          1U

/* PRIME Identification */
#define PRIME_INDEX_0                               0U
#define PRIME_INSTANCES_NUMBER                      1U

/* Management Plane USI port */
#define PRIME_MNG_PLANE_USI_INDEX                   0U





// *****************************************************************************
// *****************************************************************************
// Section: Application Configuration
// *****************************************************************************
// *****************************************************************************


//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif // CONFIGURATION_H
/*******************************************************************************
 End of File
*/
