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
#define SYS_TIME_MAX_TIMERS                         (30)
#define SYS_TIME_HW_COUNTER_WIDTH                   (32)
#define SYS_TIME_HW_COUNTER_PERIOD                  (4294967295U)
#define SYS_TIME_HW_COUNTER_HALF_PERIOD             (SYS_TIME_HW_COUNTER_PERIOD>>1)
#define SYS_TIME_CPU_CLOCK_FREQUENCY                (200000000)
#define SYS_TIME_COMPARE_UPDATE_EXECUTION_CYCLES    (232)

#define SYS_CONSOLE_INDEX_0                       0





#define SYS_CMD_ENABLE
#define SYS_CMD_DEVICE_MAX_INSTANCES       SYS_CONSOLE_DEVICE_MAX_INSTANCES
#define SYS_CMD_PRINT_BUFFER_SIZE          1024U
#define SYS_CMD_BUFFER_DMA_READY



#define SYS_DEBUG_ENABLE
#define SYS_DEBUG_GLOBAL_ERROR_LEVEL       SYS_ERROR_DEBUG
#define SYS_DEBUG_BUFFER_DMA_READY
#define SYS_DEBUG_USE_CONSOLE


#define SYS_CONSOLE_DEVICE_MAX_INSTANCES   			(1U)
#define SYS_CONSOLE_UART_MAX_INSTANCES 	   			(1U)
#define SYS_CONSOLE_USB_CDC_MAX_INSTANCES 	   		(0U)
#define SYS_CONSOLE_PRINT_BUFFER_SIZE        		(1024U)




// *****************************************************************************
// *****************************************************************************
// Section: Driver Configuration
// *****************************************************************************
// *****************************************************************************
/* Memory Driver Global Configuration Options */
#define DRV_MEMORY_INSTANCES_NUMBER          (2U)
/* RF215 Driver Configuration Options */
#define DRV_RF215_INDEX_0                     0U
#define DRV_RF215_CLIENTS_NUMBER              1U
#define DRV_RF215_TX_BUFFERS_NUMBER           2U
#define DRV_RF215_EXT_INT_PIN                 SYS_PORT_PIN_PB25
#define DRV_RF215_RESET_PIN                   SYS_PORT_PIN_PB26
#define DRV_RF215_LED_TX_PIN                  SYS_PORT_PIN_PC21
#define DRV_RF215_LED_RX_PIN                  SYS_PORT_PIN_PC20
#define DRV_RF215_NUM_TRX                     1U
#define DRV_RF215_FCS_LEN                     0U
#define DRV_RF215_MAX_PSDU_LEN                571U
#define DRV_RF215_MAX_TX_TIME_DELAY_ERROR_US  1000U
#define DRV_RF215_TIME_SYNC_EXECUTION_CYCLES  180U
#define DRV_RF215_TX_COMMAND_EXECUTION_CYCLES 1400U

/* Metrology Configuration Options */
#define DRV_METROLOGY_REG_BASE_ADDRESS        0x20088000UL
/* Metrology Default Config: FEATURE_CTRL */
#define DRV_METROLOGY_CONF_FCTRL              0x300UL
/* Metrology Default Config: AFE SELECTION */
#define DRV_METROLOGY_CONF_AFE_SEL            0x0UL
/* Metrology Default Config: CHANNEL_MATRIX */
#define DRV_METROLOGY_CONF_CHN_MATRIX         0xfff3210fUL
/* Metrology Default Config: HARMONIC_CTRL */
#define DRV_METROLOGY_CONF_HARMONIC_CTRL      0x0UL
/* Metrology Default Config: Meter Type */
#define DRV_METROLOGY_CONF_MT                 0xccUL
/* Metrology Default Config: PULSE0_CTRL */
#define DRV_METROLOGY_CONF_PULSE0_CTRL        0x810001d0UL
/* Metrology Default Config: PULSE0_K_t */
#define DRV_METROLOGY_CONF_PULSE0_KT          0x500000UL
/* Metrology Default Config: PULSE1_CTRL */
#define DRV_METROLOGY_CONF_PULSE1_CTRL        0x810201d0UL
/* Metrology Default Config: PULSE1_K_t */
#define DRV_METROLOGY_CONF_PULSE1_KT          0x500000UL
/* Metrology Default Config: PULSE2_CTRL */
#define DRV_METROLOGY_CONF_PULSE2_CTRL        0x1d0UL
/* Metrology Default Config: PULSE2_K_t */
#define DRV_METROLOGY_CONF_PULSE2_KT          0x500000UL
/* Metrology Default Config: PULSE2_K_t */
#define DRV_METROLOGY_CONF_SYNTH_ADDR         0x0UL
/* Metrology Default Config: CREEP P */
#define DRV_METROLOGY_CONF_CREEP_P            0x2e9aUL
/* Metrology Default Config: CREEP PA */
#define DRV_METROLOGY_CONF_CREEP_PA           0x2e9aUL
/* Metrology Default Config: CREEP PB */
#define DRV_METROLOGY_CONF_CREEP_PB           0x2e9aUL
/* Metrology Default Config: CREEP PC */
#define DRV_METROLOGY_CONF_CREEP_PC           0x0UL
/* Metrology Default Config: CREEP Q */
#define DRV_METROLOGY_CONF_CREEP_Q            0x2e9aUL
/* Metrology Default Config: CREEP QA */
#define DRV_METROLOGY_CONF_CREEP_QA           0x2e9aUL
/* Metrology Default Config: CREEP QB */
#define DRV_METROLOGY_CONF_CREEP_QB           0x2e9aUL
/* Metrology Default Config: CREEP QC */
#define DRV_METROLOGY_CONF_CREEP_QC           0x0UL
/* Metrology Default Config: CREEP I */
#define DRV_METROLOGY_CONF_CREEP_I            0x212dUL
/* Metrology Default Config: CREEP IA */
#define DRV_METROLOGY_CONF_CREEP_IA           0x212dUL
/* Metrology Default Config: CREEP IB */
#define DRV_METROLOGY_CONF_CREEP_IB           0x212dUL
/* Metrology Default Config: CREEP IC */
#define DRV_METROLOGY_CONF_CREEP_IC           0x0UL
/* Metrology Default Config: CREEP S */
#define DRV_METROLOGY_CONF_CREEP_S            0x2e9aUL
/* Metrology Default Config: SWELLA */
#define DRV_METROLOGY_CONF_SWELLA             0x5e84f62UL
/* Metrology Default Config: SWELLB */
#define DRV_METROLOGY_CONF_SWELLB             0x5e84f62UL
/* Metrology Default Config: SWELLC */
#define DRV_METROLOGY_CONF_SWELLC             0x0UL
/* Metrology Default Config: SAGA */
#define DRV_METROLOGY_CONF_SAGA               0x1a2ec26UL
/* Metrology Default Config: SAGB */
#define DRV_METROLOGY_CONF_SAGB               0x1a2ec26UL
/* Metrology Default Config: SAGC */
#define DRV_METROLOGY_CONF_SAGC               0x0UL
/* Metrology Default Config: INTA */
#define DRV_METROLOGY_CONF_INTA               0x68bb0aUL
/* Metrology Default Config: INTB */
#define DRV_METROLOGY_CONF_INTB               0x68bb0aUL
/* Metrology Default Config: INTC */
#define DRV_METROLOGY_CONF_INTC               0x0UL
/* Metrology Default Config: Current conversion factor */
#define DRV_METROLOGY_CONF_KIA                0x9a523UL
/* Metrology Default Config: Voltage conversion factor */
#define DRV_METROLOGY_CONF_KVA                0x19cc00UL
/* Metrology Default Config: Current conversion factor */
#define DRV_METROLOGY_CONF_KIB                0x9a523UL
/* Metrology Default Config: Voltage conversion factor */
#define DRV_METROLOGY_CONF_KVB                0x19cc00UL
/* Metrology Default Config: Current conversion factor */
#define DRV_METROLOGY_CONF_KIC                0x0UL
/* Metrology Default Config: Voltage conversion factor */
#define DRV_METROLOGY_CONF_KVC                0x0UL
/* Metrology Default Config: Current conversion factor */
#define DRV_METROLOGY_CONF_KIN                0x0UL
/* Metrology Default Config: Voltage conversion factor */
#define DRV_METROLOGY_CONF_KVD                0x0UL
/* Metrology Default Config: AFE_CTRL */
#define DRV_METROLOGY_CONF_AFE_CTRL           0x0UL


/* PRIME PAL Configuration Options */
#define PRIME_PAL_INDEX                     0U
#define PRIME_PAL_SNIFFER_USI_INSTANCE      SRV_USI_INDEX_0


/* USI Service Instance 0 Configuration Options */
#define SRV_USI_INDEX_0                       0
#define SRV_USI0_RD_BUF_SIZE                  1024
#define SRV_USI0_WR_BUF_SIZE                  1024


/* USI Service Common Configuration Options */
#define SRV_USI_INSTANCES_NUMBER              1U
#define SRV_USI_USART_CONNECTIONS             1U
#define SRV_USI_CDC_CONNECTIONS               0U
#define SRV_USI_MSG_POOL_SIZE                 5U

/* PLC PHY Driver Configuration Options */
#define DRV_PLC_SECURE                        false
#define DRV_PLC_EXT_INT_SRC                   PIOA_IRQn
#define DRV_PLC_EXT_INT_PIO                   SYS_PORT_PIN_PA3
#define DRV_PLC_EXT_INT_PIN                   SYS_PORT_PIN_PA3
#define DRV_PLC_RESET_PIN                     SYS_PORT_PIN_PA18
#define DRV_PLC_TX_ENABLE_PIN                 SYS_PORT_PIN_PB1
#define DRV_PLC_THMON_PIN                     SYS_PORT_PIN_PA2
#define DRV_PLC_CSR_INDEX                     0
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
#define DRV_PLC_PHY_HOST_DESC                 "PIC32CX2051MTSH128"
#define DRV_PLC_PHY_HOST_MODEL                3U
#define DRV_PLC_PHY_HOST_BAND                 DRV_PLC_PHY_PROFILE

#define DRV_PLC_BIN_START_ADDRESS             0x10b8000
#define DRV_PLC_BIN_SIZE                      98304


/* Memory Driver Instance 1 Configuration */
#define DRV_MEMORY_INDEX_1                   1
#define DRV_MEMORY_CLIENTS_NUMBER_IDX1       1
#define DRV_MEMORY_BUF_Q_SIZE_IDX1    1
#define DRV_MEMORY_DEVICE_START_ADDRESS      0x1050000U
#define DRV_MEMORY_DEVICE_MEDIA_SIZE         256UL
#define DRV_MEMORY_DEVICE_MEDIA_SIZE_BYTES   (DRV_MEMORY_DEVICE_MEDIA_SIZE * 1024U)
#define DRV_MEMORY_DEVICE_PROGRAM_SIZE       512U
#define DRV_MEMORY_DEVICE_ERASE_SIZE         8192U


/* Memory Driver Instance 0 Configuration */
#define DRV_MEMORY_INDEX_0                   0
#define DRV_MEMORY_CLIENTS_NUMBER_IDX0       1
#define DRV_MEMORY_BUF_Q_SIZE_IDX0    1

/* SST26 Driver Instance Configuration */
#define DRV_SST26_INDEX                 (0U)
#define DRV_SST26_CLIENTS_NUMBER        (1U)
#define DRV_SST26_START_ADDRESS         (0x0U)
#define DRV_SST26_PAGE_SIZE             (256U)
#define DRV_SST26_ERASE_BUFFER_SIZE     (4096U)



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


/* PRIME SN Application Memory Region */
#define PRIME_SN_APP_ADDRESS                        0x1010000
#define PRIME_SN_APP_SIZE                           0x40000
/* PRIME SN v1.4 FW Stack Memory Region */
#define PRIME_SN_FWSTACK14_ADDRESS                  0x1090000
#define PRIME_SN_FWSTACK14_SIZE                     0x22000
/* PRIME SN PHY Layer Memory Region */
#define PRIME_SN_PHY_ADDRESS                        0x10b8000
#define PRIME_SN_PHY_SIZE                           0x18000
/* PRIME SN v1.3 FW Stack Memory Region */
#define PRIME_SN_FWSTACK13_ADDRESS                  0x10d0000
#define PRIME_SN_FWSTACK13_SIZE                     0x20000




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
