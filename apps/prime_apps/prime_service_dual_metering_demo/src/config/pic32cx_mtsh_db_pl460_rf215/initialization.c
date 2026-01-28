/*******************************************************************************
  System Initialization File

  File Name:
    initialization.c

  Summary:
    This file contains source code necessary to initialize the system.

  Description:
    This file contains source code necessary to initialize the system.  It
    implements the "SYS_Initialize" function, defines the configuration bits,
    and allocates any necessary global system resources,
 *******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
* Copyright (C) 2025 Microchip Technology Inc. and its subsidiaries.
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

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
#include "configuration.h"
#include "definitions.h"
#include "device.h"


// ****************************************************************************
// ****************************************************************************
// Section: Configuration Bits
// ****************************************************************************
// ****************************************************************************
#pragma config SECURITY_BIT = CLEAR
#pragma config BOOT_MODE = 0x3
#pragma config PLANE_SELECTION = CLEAR
#pragma config ERASE_FUNCTION_LOCK = 0x0




// *****************************************************************************
// *****************************************************************************
// Section: Driver Initialization Data
// *****************************************************************************
// *****************************************************************************
/* Following MISRA-C rules are deviated in the below code block */
/* MISRA C-2023 Rule 7.2 - Deviation record ID - H3_MISRAC_2023_R_7_2_DR_1 */
/* MISRA C-2023 Rule 11.1 - Deviation record ID - H3_MISRAC_2023_R_11_1_DR_1 */
/* MISRA C-2023 Rule 11.3 - Deviation record ID - H3_MISRAC_2023_R_11_3_DR_1 */
/* MISRA C-2023 Rule 11.8 - Deviation record ID - H3_MISRAC_2023_R_11_8_DR_1 */
// <editor-fold defaultstate="collapsed" desc="DRV_RF215 Initialization Data">

/* RF215 Driver Initialization Data */
static const DRV_RF215_INIT drvRf215InitData = {
    /* Pointer to SPI PLIB is busy function */
    .spiPlibIsBusy = FLEXCOM5_SPI_IsTransmitterBusy,

    /* Pointer to SPI Write and Read function */
    .spiPlibWriteRead = FLEXCOM5_SPI_WriteRead,

    /* Pointer to SPI Register Callback function */
    .spiPlibSetCallback = FLEXCOM5_SPI_CallbackRegister,

    /* Interrupt source ID for DMA */
    .dmaIntSource = FLEXCOM5_IRQn,

    /* Interrupt source ID for SYS_TIME */
    .sysTimeIntSource = TC0_CH0_IRQn,

    /* Interrupt source ID for PLC external interrupt */
    .plcExtIntSource = DRV_PLC_EXT_INT_SRC,

    /* Initial PHY frequency band and operating mode for Sub-GHz transceiver */
    .rf09PhyBandOpmIni = SUN_FSK_BAND_863_OPM1,

    /* Initial PHY frequency channel number for Sub-GHz transceiver */
    .rf09PhyChnNumIni = 0,

};

// </editor-fold>

// <editor-fold defaultstate="collapsed" desc="DRV_METROLOGY Initialization Data">

/* MISRA C-2023 deviation block start */
/* MISRA C-2023 Rule 8.4 deviated once. Deviation record ID - H3_MISRAC_2023_R_8_4_DR_1 */

/* Metrology Driver Initialization Data */
DRV_METROLOGY_INIT drvMetrologyInitData = {

    /* MET bin destination address */
    .regBaseAddress = DRV_METROLOGY_REG_BASE_ADDRESS,

    /* MET Binary start address */
    .binStartAddress = (uint32_t)&met_bin_start,

    /* MET Binary end address */
    .binEndAddress = (uint32_t)&met_bin_end,

};

/* MISRA C-2023 deviation block end */

// </editor-fold>

// <editor-fold defaultstate="collapsed" desc="SRV_USI Instance 0 Initialization Data">

static uint8_t CACHE_ALIGN srvUSI0ReadBuffer[SRV_USI0_RD_BUF_SIZE] = {0};
static uint8_t CACHE_ALIGN srvUSI0WriteBuffer[SRV_USI0_WR_BUF_SIZE] = {0};


static const SRV_USI_USART_INTERFACE srvUsi0InitDataFLEXCOM2 = {
    .readCallbackRegister = (USI_USART_PLIB_READ_CALLBACK_REG)FLEXCOM2_USART_ReadCallbackRegister,
    .readData = (USI_USART_PLIB_WRRD)FLEXCOM2_USART_Read,
    .writeData = (USI_USART_PLIB_WRRD)FLEXCOM2_USART_Write,
};

static uint8_t CACHE_ALIGN srvUSI0USARTReadBuffer[128] = {0};

static const USI_USART_INIT_DATA srvUsi0InitData = {
    .plib = (void*)&srvUsi0InitDataFLEXCOM2,
    .pRdBuffer = (void*)srvUSI0ReadBuffer,
    .rdBufferSize = SRV_USI0_RD_BUF_SIZE,
    .usartReadBuffer = (void *)srvUSI0USARTReadBuffer,
    .usartBufferSize = 128,
};

/* srvUSIUSARTDevDesc declared in USI USART service implementation (srv_usi_usart.c) */

static const SRV_USI_INIT srvUSI0Init =
{
    .deviceInitData = (const void * const)&srvUsi0InitData,
    .consDevDesc = &srvUSIUSARTDevDesc,
    .deviceIndex = 0,
    .pWrBuffer = srvUSI0WriteBuffer,
    .wrBufferSize = SRV_USI0_WR_BUF_SIZE
};

// </editor-fold>
// <editor-fold defaultstate="collapsed" desc="_on_reset() critical function">
/* MISRA C-2023 deviation block start */
/* MISRA C-2023 Rule 8.4 deviated once. Deviation record ID - H3_MISRAC_2023_R_8_4_DR_1 */
/* MISRA C-2023 Rule 21.2 deviated once. Deviation record ID - H3_MISRAC_2023_R_21_2_DR_1 */

/* This routine must initialize the PL460 control pins as soon as possible */
/* after a power up reset to avoid risks on starting up PL460 device when */
/* pull up resistors are configured by default */
void _on_reset(void)
{
    /* Enable PIOA clock */
    PMC_REGS->PMC_PCR = PMC_PCR_CMD_Msk | PMC_PCR_EN_Msk | PMC_PCR_PID(ID_PIOA);
    while((PMC_REGS->PMC_CSR0 & PMC_CSR0_PID17_Msk) == 0U)
    {
        /* Wait for clock to be initialized */
    }
    /* Enable and Clear Reset Pin */
    SYS_PORT_PinOutputEnable(DRV_PLC_RESET_PIN);
    SYS_PORT_PinClear(DRV_PLC_RESET_PIN);
}

/* MISRA C-2023 deviation block end */

// </editor-fold>

// <editor-fold defaultstate="collapsed" desc="DRV_PLC_HAL Initialization Data">
/* HAL Interface Initialization for PLC transceiver */
static DRV_PLC_PLIB_INTERFACE drvPLCPlib = {

    /* SPI Transfer Setup */
    .spiPlibTransferSetup = (DRV_PLC_SPI_PLIB_TRANSFER_SETUP)FLEXCOM1_SPI_TransferSetup,

    /* SPI Is Busy */
    .spiIsBusy = FLEXCOM1_SPI_IsTransmitterBusy,

    /* SPI Write/Read */
    .spiWriteRead = FLEXCOM1_SPI_WriteRead,

    /* SPI clock frequency */
    .spiClockFrequency = DRV_PLC_SPI_CLK,

    /* PLC Reset Pin */
    .resetPin = DRV_PLC_RESET_PIN,

    /* PLC External Interrupt Pin */
    .extIntPin = DRV_PLC_EXT_INT_PIN,

    /* PLC External Interrupt Pio */
    .extIntPio = DRV_PLC_EXT_INT_PIO,

    /* PLC TX Enable Pin */
    .txEnablePin = DRV_PLC_TX_ENABLE_PIN,

    /* PLC External Interrupt Pin */
    .thMonPin = DRV_PLC_THMON_PIN,

};

/* HAL Interface Initialization for PLC transceiver */
static DRV_PLC_HAL_INTERFACE drvPLCHalAPI = {

    /* PLC PLIB */
    .plcPlib = &drvPLCPlib,

    /* PLC HAL init */
    .init = (DRV_PLC_HAL_INIT)DRV_PLC_HAL_Init,

    /* PLC HAL setup */
    .setup = (DRV_PLC_HAL_SETUP)DRV_PLC_HAL_Setup,

    /* PLC transceiver reset */
    .reset = (DRV_PLC_HAL_RESET)DRV_PLC_HAL_Reset,

    /* PLC Get Thermal Monitor value */
    .getThermalMonitor = (DRV_PLC_HAL_GET_THMON)DRV_PLC_HAL_GetThermalMonitor,

    /* PLC Set TX Enable Pin */
    .setTxEnable = (DRV_PLC_HAL_SET_TXENABLE)DRV_PLC_HAL_SetTxEnable,

    /* PLC HAL Enable/Disable external interrupt */
    .enableExtInt = (DRV_PLC_HAL_ENABLE_EXT_INT)DRV_PLC_HAL_EnableInterrupts,

    /* PLC HAL Enable/Disable external interrupt */
    .getPinLevel = (DRV_PLC_HAL_GET_PIN_LEVEL)DRV_PLC_HAL_GetPinLevel,

    /* PLC HAL delay function */
    .delay = (DRV_PLC_HAL_DELAY)DRV_PLC_HAL_Delay,

    /* PLC HAL Transfer Bootloader Command */
    .sendBootCmd = (DRV_PLC_HAL_SEND_BOOT_CMD)DRV_PLC_HAL_SendBootCmd,

    /* PLC HAL Transfer Write/Read Command */
    .sendWrRdCmd = (DRV_PLC_HAL_SEND_WRRD_CMD)DRV_PLC_HAL_SendWrRdCmd,
};

// </editor-fold>

// <editor-fold defaultstate="collapsed" desc="DRV_PLC_PHY Initialization Data">

/* MISRA C-2023 deviation block start */
/* MISRA C-2023 Rule 8.4 deviated once. Deviation record ID - H3_MISRAC_2023_R_8_4_DR_1 */

/* PLC Driver Initialization Data */
DRV_PLC_PHY_INIT drvPlcPhyInitData = {

    /* SPI PLIB API  interface*/
    .plcHal = &drvPLCHalAPI,

    /* PLC PHY Number of clients */
    .numClients = DRV_PLC_PHY_CLIENTS_NUMBER_IDX,

    /* PLC Binary start address */
    .binStartAddress = DRV_PLC_BIN_START_ADDRESS,

    /* PLC Binary end address */
    .binEndAddress = DRV_PLC_BIN_START_ADDRESS + DRV_PLC_BIN_SIZE - 1,

    /* Secure Mode */
    .secure = DRV_PLC_SECURE,

};

/* MISRA C-2023 deviation block end */

// </editor-fold>
// <editor-fold defaultstate="collapsed" desc="DRV_MEMORY Instance 1 Initialization Data">

static uint8_t gDrvMemory1EraseBuffer[SEFC0_ERASE_BUFFER_SIZE] CACHE_ALIGN;

static DRV_MEMORY_CLIENT_OBJECT gDrvMemory1ClientObject[DRV_MEMORY_CLIENTS_NUMBER_IDX1];

static DRV_MEMORY_BUFFER_OBJECT gDrvMemory1BufferObject[DRV_MEMORY_BUF_Q_SIZE_IDX1];

static const DRV_MEMORY_DEVICE_INTERFACE drvMemory1DeviceAPI = {
    .Open               = DRV_SEFC0_Open,
    .Close              = DRV_SEFC0_Close,
    .Status             = DRV_SEFC0_Status,
    .SectorErase        = DRV_SEFC0_SectorErase,
    .Read               = DRV_SEFC0_Read,
    .PageWrite          = DRV_SEFC0_PageWrite,
    .EventHandlerSet    = NULL,
    .GeometryGet        = (DRV_MEMORY_DEVICE_GEOMETRY_GET)DRV_SEFC0_GeometryGet,
    .TransferStatusGet  = (DRV_MEMORY_DEVICE_TRANSFER_STATUS_GET)DRV_SEFC0_TransferStatusGet
};
static const DRV_MEMORY_INIT drvMemory1InitData =
{
    .memDevIndex                = 0,
    .memoryDevice               = &drvMemory1DeviceAPI,
    .isMemDevInterruptEnabled   = false,
    .isFsEnabled                = false,
    .ewBuffer                   = &gDrvMemory1EraseBuffer[0],
    .clientObjPool              = (uintptr_t)&gDrvMemory1ClientObject[0],
    .bufferObj                  = (uintptr_t)&gDrvMemory1BufferObject[0],
    .queueSize                  = DRV_MEMORY_BUF_Q_SIZE_IDX1,
    .nClientsMax                = DRV_MEMORY_CLIENTS_NUMBER_IDX1
};

// </editor-fold>
// <editor-fold defaultstate="collapsed" desc="DRV_MEMORY Instance 0 Initialization Data">

static uint8_t gDrvMemory0EraseBuffer[DRV_SST26_ERASE_BUFFER_SIZE] CACHE_ALIGN;

static DRV_MEMORY_CLIENT_OBJECT gDrvMemory0ClientObject[DRV_MEMORY_CLIENTS_NUMBER_IDX0];

static DRV_MEMORY_BUFFER_OBJECT gDrvMemory0BufferObject[DRV_MEMORY_BUF_Q_SIZE_IDX0];

static const DRV_MEMORY_DEVICE_INTERFACE drvMemory0DeviceAPI = {
    .Open               = DRV_SST26_Open,
    .Close              = DRV_SST26_Close,
    .Status             = DRV_SST26_Status,
    .SectorErase        = DRV_SST26_SectorErase,
    .Read               = DRV_SST26_Read,
    .PageWrite          = DRV_SST26_PageWrite,
    .EventHandlerSet    = NULL,
    .GeometryGet        = (DRV_MEMORY_DEVICE_GEOMETRY_GET)DRV_SST26_GeometryGet,
    .TransferStatusGet  = (DRV_MEMORY_DEVICE_TRANSFER_STATUS_GET)DRV_SST26_TransferStatusGet
};
static const DRV_MEMORY_INIT drvMemory0InitData =
{
    .memDevIndex                = DRV_SST26_INDEX,
    .memoryDevice               = &drvMemory0DeviceAPI,
    .isMemDevInterruptEnabled   = false,
    .isFsEnabled                = false,
    .ewBuffer                   = &gDrvMemory0EraseBuffer[0],
    .clientObjPool              = (uintptr_t)&gDrvMemory0ClientObject[0],
    .bufferObj                  = (uintptr_t)&gDrvMemory0BufferObject[0],
    .queueSize                  = DRV_MEMORY_BUF_Q_SIZE_IDX0,
    .nClientsMax                = DRV_MEMORY_CLIENTS_NUMBER_IDX0
};

// </editor-fold>
// <editor-fold defaultstate="collapsed" desc="DRV_SST26 Initialization Data">

static const DRV_SST26_PLIB_INTERFACE drvSST26PlibAPI = {
    .CommandWrite   = QSPI_CommandWrite,
    .RegisterRead   = QSPI_RegisterRead,
    .RegisterWrite  = QSPI_RegisterWrite,
    .MemoryRead     = QSPI_MemoryRead,
    .MemoryWrite    = QSPI_MemoryWrite
};

static const DRV_SST26_INIT drvSST26InitData =
{
    .sst26Plib      = &drvSST26PlibAPI,
};
// </editor-fold>




// *****************************************************************************
// *****************************************************************************
// Section: System Data
// *****************************************************************************
// *****************************************************************************
/* Structure to hold the object handles for the modules in the system. */
SYSTEM_OBJECTS sysObj;

// *****************************************************************************
// *****************************************************************************
// Section: Library/Stack Initialization Data
// *****************************************************************************
// *****************************************************************************

// <editor-fold defaultstate="collapsed" desc="PRIME Initialization Data">

/* PRIME Initialization Data */
static PRIME_STACK_INIT primeInitData = {
    /* PAL index */
    .palIndex = PRIME_PAL_INDEX,
    
    /* Management Plane USI port */
    .mngPlaneUsiPort = PRIME_MNG_PLANE_USI_INDEX
};

// </editor-fold>


// *****************************************************************************
// *****************************************************************************
// Section: System Initialization
// *****************************************************************************
// *****************************************************************************
// <editor-fold defaultstate="collapsed" desc="SYS_TIME Initialization Data">

static const SYS_TIME_PLIB_INTERFACE sysTimePlibAPI = {
    .timerCallbackSet = (SYS_TIME_PLIB_CALLBACK_REGISTER)TC0_CH0_TimerCallbackRegister,
    .timerStart = (SYS_TIME_PLIB_START)TC0_CH0_TimerStart,
    .timerStop = (SYS_TIME_PLIB_STOP)TC0_CH0_TimerStop ,
    .timerFrequencyGet = (SYS_TIME_PLIB_FREQUENCY_GET)TC0_CH0_TimerFrequencyGet,
    .timerPeriodSet = (SYS_TIME_PLIB_PERIOD_SET)TC0_CH0_TimerPeriodSet,
    .timerCompareSet = (SYS_TIME_PLIB_COMPARE_SET)TC0_CH0_TimerCompareSet,
    .timerCounterGet = (SYS_TIME_PLIB_COUNTER_GET)TC0_CH0_TimerCounterGet,
};

static const SYS_TIME_INIT sysTimeInitData =
{
    .timePlib = &sysTimePlibAPI,
    .hwTimerIntNum = TC0_CH0_IRQn,
};

// </editor-fold>
// <editor-fold defaultstate="collapsed" desc="SYS_CONSOLE Instance 0 Initialization Data">


static const SYS_CONSOLE_UART_PLIB_INTERFACE sysConsole0UARTPlibAPI =
{
    .read_t = (SYS_CONSOLE_UART_PLIB_READ)FLEXCOM0_USART_Read,
    .readCountGet = (SYS_CONSOLE_UART_PLIB_READ_COUNT_GET)FLEXCOM0_USART_ReadCountGet,
    .readFreeBufferCountGet = (SYS_CONSOLE_UART_PLIB_READ_FREE_BUFFFER_COUNT_GET)FLEXCOM0_USART_ReadFreeBufferCountGet,
    .write_t = (SYS_CONSOLE_UART_PLIB_WRITE)FLEXCOM0_USART_Write,
    .writeCountGet = (SYS_CONSOLE_UART_PLIB_WRITE_COUNT_GET)FLEXCOM0_USART_WriteCountGet,
    .writeFreeBufferCountGet = (SYS_CONSOLE_UART_PLIB_WRITE_FREE_BUFFER_COUNT_GET)FLEXCOM0_USART_WriteFreeBufferCountGet,
};

static const SYS_CONSOLE_UART_INIT_DATA sysConsole0UARTInitData =
{
    .uartPLIB = &sysConsole0UARTPlibAPI,
};

static const SYS_CONSOLE_INIT sysConsole0Init =
{
    .deviceInitData = (const void*)&sysConsole0UARTInitData,
    .consDevDesc = &sysConsoleUARTDevDesc,
    .deviceIndex = 0,
};



// </editor-fold>


static const SYS_CMD_INIT sysCmdInit =
{
    .moduleInit = {0},
    .consoleCmdIOParam = (uint8_t) SYS_CMD_SINGLE_CHARACTER_READ_CONSOLE_IO_PARAM,
	.consoleIndex = 0,
};


static const SYS_DEBUG_INIT debugInit =
{
    .moduleInit = {0},
    .errorLevel = SYS_DEBUG_GLOBAL_ERROR_LEVEL,
    .consoleIndex = 0,
};





// *****************************************************************************
// *****************************************************************************
// Section: Local initialization functions
// *****************************************************************************
// *****************************************************************************

/* MISRAC 2023 deviation block end */

/*******************************************************************************
  Function:
    void SYS_Initialize ( void *data )

  Summary:
    Initializes the board, services, drivers, application and other modules.

  Remarks:
 */

void SYS_Initialize ( void* data )
{

    /* MISRAC 2023 deviation block start */
    /* MISRA C-2023 Rule 2.2 deviated in this file.  Deviation record ID -  H3_MISRAC_2023_R_2_2_DR_1 */


    SEFC0_Initialize();

    SEFC1_Initialize();
  
    DWDT_Initialize();
    CLOCK_Initialize();
    RSTC_Initialize();

    PIO_Initialize();
    SUPC_Initialize();




    FLEXCOM5_SPI_Initialize();

    RTC_Initialize();

    FLEXCOM0_USART_Initialize();

    FLEXCOM1_SPI_Initialize();

    FLEXCOM2_USART_Initialize();

	ICM_Initialize();
    BSP_Initialize();
    ADC_Initialize();
 
    TC0_CH0_TimerInitialize(); 
     
    
	TRNG_Initialize();

    QSPI_Initialize();

    /* MISRAC 2023 deviation block start */
    /* Following MISRA-C rules deviated in this block  */
    /* MISRA C-2023 Rule 11.3 - Deviation record ID - H3_MISRAC_2023_R_11_3_DR_1 */
    /* MISRA C-2023 Rule 11.8 - Deviation record ID - H3_MISRAC_2023_R_11_8_DR_1 */

    /* Initialize RF215 Driver Instance */
    sysObj.drvRf215 = DRV_RF215_Initialize(DRV_RF215_INDEX_0, (SYS_MODULE_INIT *)&drvRf215InitData);

    /* Initialize Metrology Driver Instance */
    sysObj.drvMet = DRV_METROLOGY_Initialize((SYS_MODULE_INIT *)&drvMetrologyInitData, RSTC_ResetCauseGet());
    

    /* Initialize Firmware Upgrade */
    SRV_FU_Initialize();


    /* Initialize PRIME Storage service */
    SRV_STORAGE_Initialize();

    /* Initialize USI Service Instance 0 */
    sysObj.srvUSI0 = SRV_USI_Initialize(SRV_USI_INDEX_0, (SYS_MODULE_INIT *)&srvUSI0Init);

    /* Initialize PLC Phy Driver Instance */
    sysObj.drvPlcPhy = DRV_PLC_PHY_Initialize(DRV_PLC_PHY_INDEX, (SYS_MODULE_INIT *)&drvPlcPhyInitData);
    (void) PIO_PinInterruptCallbackRegister((PIO_PIN)DRV_PLC_EXT_INT_PIN, DRV_PLC_PHY_ExternalInterruptHandler, sysObj.drvPlcPhy);


    sysObj.drvMemory1 = DRV_MEMORY_Initialize((SYS_MODULE_INDEX)DRV_MEMORY_INDEX_1, (SYS_MODULE_INIT *)&drvMemory1InitData);


    sysObj.drvMemory0 = DRV_MEMORY_Initialize((SYS_MODULE_INDEX)DRV_MEMORY_INDEX_0, (SYS_MODULE_INIT *)&drvMemory0InitData);

    /* Initialize PRIME Reset Handler service */
    SRV_RESET_HANDLER_Initialize();
    sysObj.drvSST26 = DRV_SST26_Initialize((SYS_MODULE_INDEX)DRV_SST26_INDEX, (SYS_MODULE_INIT *)&drvSST26InitData);

    /* Initialize PRIME User PIBs service */
    SRV_USER_PIB_Initialize();

    /* Initialize PVDD Monitor Service */
    SRV_PVDDMON_Initialize();
    DRV_SLCDC_Initialize();


    /* MISRA C-2012 Rule 11.3, 11.8 deviated below. Deviation record ID -  
    H3_MISRAC_2012_R_11_3_DR_1 & H3_MISRAC_2012_R_11_8_DR_1*/
        
    sysObj.sysTime = SYS_TIME_Initialize(SYS_TIME_INDEX_0, (SYS_MODULE_INIT *)&sysTimeInitData);
    
    /* MISRAC 2012 deviation block end */
    /* MISRA C-2012 Rule 11.3, 11.8 deviated below. Deviation record ID -  
     H3_MISRAC_2012_R_11_3_DR_1 & H3_MISRAC_2012_R_11_8_DR_1*/
        sysObj.sysConsole0 = SYS_CONSOLE_Initialize(SYS_CONSOLE_INDEX_0, (SYS_MODULE_INIT *)&sysConsole0Init);
   /* MISRAC 2012 deviation block end */
    sysObj.sysCommand = (uint32_t) SYS_CMD_Initialize((SYS_MODULE_INIT*)&sysCmdInit);

    /* MISRA C-2012 Rule 11.3, 11.8 deviated below. Deviation record ID -  
     H3_MISRAC_2012_R_11_3_DR_1 & H3_MISRAC_2012_R_11_8_DR_1*/
        
    sysObj.sysDebug = SYS_DEBUG_Initialize(SYS_DEBUG_INDEX_0, (SYS_MODULE_INIT*)&debugInit);

    /* MISRAC 2012 deviation block end */


/* Initialize PRIME */

/* MISRA C-2023 deviation block start */
/* MISRA C-2023 Rule 11.3 deviated twice. Deviation record ID - H3_MISRAC_2023_R_11_3_DR_1 */
    sysObj.primeStack = PRIME_Initialize(PRIME_INDEX_0, (SYS_MODULE_INIT *)&primeInitData);


    /* MISRAC 2023 deviation block end */
    APP_METROLOGY_Initialize();
    APP_CONSOLE_Initialize();
    APP_DATALOG_Initialize();
    APP_DISPLAY_Initialize();
    APP_ENERGY_Initialize();
    APP_EVENTS_Initialize();
    APP_PRIME_MANAGEMENT_Initialize();


    NVIC_Initialize();


    /* MISRAC 2023 deviation block end */
}

/*******************************************************************************
 End of File
*/
