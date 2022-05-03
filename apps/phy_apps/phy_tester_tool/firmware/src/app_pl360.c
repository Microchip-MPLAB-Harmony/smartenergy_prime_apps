/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It
    implements the logic of the application's state machine and it may call
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <string.h>
#include "definitions.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_Initialize function.

    Application strings and buffers are be defined outside this structure.
*/

CACHE_ALIGN APP_DATA appData;
    
static CACHE_ALIGN uint8_t pPLCDataTxBuffer[CACHE_ALIGNED_SIZE_GET(APP_PLC_DATA_BUFFER_SIZE)];
static CACHE_ALIGN uint8_t pPLCDataRxBuffer[CACHE_ALIGNED_SIZE_GET(APP_PLC_DATA_BUFFER_SIZE)];
static CACHE_ALIGN uint8_t pPLCDataPIBBuffer[CACHE_ALIGNED_SIZE_GET(APP_PLC_PIB_BUFFER_SIZE)];
static CACHE_ALIGN uint8_t pSerialDataBuffer[CACHE_ALIGNED_SIZE_GET(APP_SERIAL_DATA_BUFFER_SIZE)];

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************
void Timer1_Callback (uintptr_t context)
{
    appData.tmr1Expired = true;
}

void Timer2_Callback (uintptr_t context)
{
    appData.tmr2Expired = true;
}

static void APP_PLC_SetCouplingConfiguration(DRV_PLC_PHY_CHANNEL channel)
{
    SRV_PCOUP_Set_Channel_Config(appData.drvPl360Handle, channel);
    
    /* Optional ***************************************************/
    /* Disable AUTO mode and set VLO behavior by default in order to
        * maximize signal level in any case */
    appData.plcPIB.id = PLC_ID_CFG_AUTODETECT_IMPEDANCE;
    appData.plcPIB.length = 1;
    *appData.plcPIB.pData = 0;
    DRV_PLC_PHY_PIBSet(appData.drvPl360Handle, &appData.plcPIB);

    appData.plcPIB.id = PLC_ID_CFG_IMPEDANCE;
    appData.plcPIB.length = 1;
    *appData.plcPIB.pData = 2;
    DRV_PLC_PHY_PIBSet(appData.drvPl360Handle, &appData.plcPIB);
}

static void APP_PLCExceptionCb(DRV_PLC_PHY_EXCEPTION exceptionObj,
        uintptr_t context)
{
    /* Avoid warning */
    (void)context;

    switch (exceptionObj)
    {
        case DRV_PLC_PHY_EXCEPTION_UNEXPECTED_KEY:
            appData.plc_phy_err_unexpected++;
            break;

        case DRV_PLC_PHY_EXCEPTION_CRITICAL_ERROR:
            appData.plc_phy_err_critical++;
            break;

        case DRV_PLC_PHY_EXCEPTION_RESET:
            appData.plc_phy_err_reset++;
            break;

        default:
            appData.plc_phy_err_unknow++;
	}

	appData.plc_phy_exception = true;
}

static void APP_PLCDataIndCb(DRV_PLC_PHY_RECEPTION_OBJ *indObj, uintptr_t context)
{
    /* Avoid warning */
    (void)context;

    /* Send Received PLC message through USI */
    if (indObj->dataLength)
    {
        size_t length;

        /* Start Timer: LED blinking for each received message */
        USER_PLC_IND_LED_On();
        appData.tmr2Handle = SYS_TIME_CallbackRegisterMS(Timer2_Callback, 0,
                LED_BLINK_PLC_MSG_MS, SYS_TIME_SINGLE);

        /* Add received message */
        length = SRV_PSERIAL_SerialRxMessage(appData.pSerialData, indObj);
        /* Send through USI */
        SRV_USI_Send_Message(appData.srvUSIHandle, SRV_USI_PROT_ID_PHY,
                appData.pSerialData, length);
    }
}

static void APP_PLCDataCfmCb(DRV_PLC_PHY_TRANSMISSION_CFM_OBJ *cfmObj, uintptr_t context)
{
    size_t length;

    /* Avoid warning */
    (void)context;
    
    /* Add received message */
    length = SRV_PSERIAL_SerialCfmMessage(appData.pSerialData, cfmObj);
    /* Send through USI */
    SRV_USI_Send_Message(appData.srvUSIHandle, SRV_USI_PROT_ID_PHY,
            appData.pSerialData, length);

}

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************
void APP_USIPhyProtocolEventHandler(uint8_t *pData, size_t length)
{
    /* Message received from PLC Tool - USART */
    SRV_PSERIAL_COMMAND command;

    /* Protection for invalid us_length */
    if (!length)
    {
        return;
    }

    /* Process received message */
    command = SRV_PSERIAL_GetCommand(pData);

    switch (command) {
        case SRV_PSERIAL_CMD_PHY_GET_CFG:
        {
            /* Extract PIB information */
            SRV_PSERIAL_ParseGetPIB(&appData.plcPIB, pData);

            if (DRV_PLC_PHY_PIBGet(appData.drvPl360Handle, &appData.plcPIB))
            {
                size_t len;

                /* Serialize PIB data */
                len = SRV_PSERIAL_SerialGetPIB(appData.pSerialData, &appData.plcPIB);
                /* Send through USI */
                SRV_USI_Send_Message(appData.srvUSIHandle, SRV_USI_PROT_ID_PHY,
                        appData.pSerialData, len);
            }
        }
        break;

        case SRV_PSERIAL_CMD_PHY_SET_CFG:
        {
            bool sendUSIResponse = false;
            
            /* Extract PIB information */
            SRV_PSERIAL_ParseSetPIB(&appData.plcPIB, pData);
            
            /* Manage Channels configuration */
            if (appData.plcPIB.id == PLC_ID_CHANNEL_CFG)
            {
                DRV_PLC_PHY_CHANNEL channel;
                
                channel = *appData.plcPIB.pData;
                
                if ((appData.channel != channel) && (SRV_PCOUP_Get_Channel_Config(channel) != NULL))
                {
                    if (DRV_PLC_PHY_PIBSet(appData.drvPl360Handle, &appData.plcPIB))
                    {
                            /* Update channel application data */
                            appData.channel = channel;
                            /* Set configuration for PLC */
                            APP_PLC_SetCouplingConfiguration(appData.channel);
                            
                            sendUSIResponse = true;
                    }
                }
            } 
            else if (DRV_PLC_PHY_PIBSet(appData.drvPl360Handle, &appData.plcPIB))
            {
                sendUSIResponse = true;
            }
            
            if (sendUSIResponse)
            {
                size_t len;

                /* Serialize PIB data */
                len = SRV_PSERIAL_SerialSetPIB(&appData.pSerialData[1], &appData.plcPIB);
                /* Send through USI */
                SRV_USI_Send_Message(appData.srvUSIHandle, SRV_USI_PROT_ID_PHY,
                        appData.pSerialData, len);
            }
        }
        break;

        case SRV_PSERIAL_CMD_PHY_SEND_MSG:
        {
            /* Set PLC TX State to wait Tx confirmation */
            appData.plcTxState = APP_PLC_TX_STATE_WAIT_TX_CFM;

            /* Capture and parse data from USI */
            SRV_PSERIAL_ParseTxMessage(&appData.plcTxObj, pData);

            /* Send Message through PLC */
            DRV_PLC_PHY_TxRequest(appData.drvPl360Handle, &appData.plcTxObj);
        }
        break;

        default:
            break;
    }
}

// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************
void on_reset(void)
{
    /* Configure PIO interface with PLC device ASAP */
    PIO_Initialize();
}

/*******************************************************************************
  Function:
    void APP_PL360_Initialize(void)

  Remarks:
    See prototype in app.h.
 */

void APP_PL360_Initialize(void)
{
    /* Place the App state machine in its initial state. */
    appData.state = APP_STATE_IDLE;

    /* Init Timer handler */
    appData.tmr1Handle = SYS_TIME_HANDLE_INVALID;
    appData.tmr2Handle = SYS_TIME_HANDLE_INVALID;
    appData.tmr1Expired = false;
    appData.tmr2Expired = false;

    /* Reset PLC exceptions statistics */
    appData.plc_phy_err_unexpected = 0;
    appData.plc_phy_err_critical = 0;
    appData.plc_phy_err_reset = 0;
    appData.plc_phy_err_unknow = 0;

    appData.state = APP_STATE_INIT;

    /* Initialize PLC buffers */
    appData.plcTxObj.pTransmitData = pPLCDataTxBuffer;
    appData.plcRxObj.pReceivedData = pPLCDataRxBuffer;
    appData.plcPIB.pData = pPLCDataPIBBuffer;
    appData.pSerialData = pSerialDataBuffer;
    
    /* Init PLC TX status */
    appData.plcTxState = APP_PLC_TX_STATE_IDLE;
    
    /* Init Channel */
    appData.channel = SRV_PCOUP_Get_Default_Channel();
}


/******************************************************************************
  Function:
    void APP_PL360_Tasks(void)

  Remarks:
    See prototype in app.h.
 */
void APP_PL360_Tasks(void)
{
    WDT_Clear();
    
    /* Signalling: LED Toggle */
    if (appData.tmr1Expired)
    {
        appData.tmr1Expired = false;
        USER_BLINK_LED_Toggle();
    }
    
    /* Signalling: PLC RX */
    if (appData.tmr2Expired)
    {
        appData.tmr2Expired = false;
        USER_PLC_IND_LED_Off();
    }
    
    /* Check the application's current state. */
    switch(appData.state)
    {
        /* Application's initial state. */
        case APP_STATE_INIT:
        {
            /* Open PLC driver : Start uploading process */
            appData.drvPl360Handle = DRV_PLC_PHY_Open(DRV_PLC_PHY_INDEX, NULL);

            if (appData.drvPl360Handle != DRV_HANDLE_INVALID)
            {
                /* Set Application to next state */
                appData.state = APP_STATE_REGISTER;
            }
            else
            {
                /* Set Application to ERROR state */
                appData.state = APP_STATE_ERROR;
            }
            break;
        }

        /* Waiting to PLC transceiver be opened and register callback functions */
        case APP_STATE_REGISTER:
        {
            /* Check PLC transceiver */
            if (DRV_PLC_PHY_Status(DRV_PLC_PHY_INDEX) == SYS_STATUS_READY)
            {
                /* Register PLC callback */
                DRV_PLC_PHY_ExceptionCallbackRegister(appData.drvPl360Handle,
                        APP_PLCExceptionCb, DRV_PLC_PHY_INDEX);
                DRV_PLC_PHY_DataIndCallbackRegister(appData.drvPl360Handle,
                        APP_PLCDataIndCb, DRV_PLC_PHY_INDEX);
                DRV_PLC_PHY_TxCfmCallbackRegister(appData.drvPl360Handle,
                        APP_PLCDataCfmCb, DRV_PLC_PHY_INDEX);

                /* Open USI Service */
                appData.srvUSIHandle = SRV_USI_Open(SRV_USI_INDEX_0);

                if (appData.srvUSIHandle != DRV_HANDLE_INVALID)
                {
                    /* Set Application to next state */
                    appData.state = APP_STATE_CONFIG_USI;
                }
                else
                {
                    /* Set Application to ERROR state */
                    appData.state = APP_STATE_ERROR;
                }
            }
            break;
        }

        case APP_STATE_CONFIG_USI:
        {
            if (SRV_USI_Status(appData.srvUSIHandle) == SRV_USI_STATUS_CONFIGURED)
            {
                /* Register USI callback */
                SRV_USI_CallbackRegister(appData.srvUSIHandle,
                        SRV_USI_PROT_ID_PHY, APP_USIPhyProtocolEventHandler);

                if (appData.tmr1Handle == SYS_TIME_HANDLE_INVALID)
                {
                    /* Register Timer Callback */
                    appData.tmr1Handle = SYS_TIME_CallbackRegisterMS(
                            Timer1_Callback, 0, LED_BLINK_RATE_MS,
                            SYS_TIME_PERIODIC);
                }
                else
                {
                    SYS_TIME_TimerStart(appData.tmr1Handle);
                }

                /* Set Application to next state */
                appData.state = APP_STATE_CONFIG_PLC;
            }
            break;
        }

        case APP_STATE_CONFIG_PLC:
        {
            /* Set configuration for PLC */
            APP_PLC_SetCouplingConfiguration(appData.channel);
            
            /* Set channel configuration */
            appData.plcPIB.id = PLC_ID_CHANNEL_CFG;
            appData.plcPIB.length = 1;
            *appData.plcPIB.pData = appData.channel;
            DRV_PLC_PHY_PIBSet(appData.drvPl360Handle, &appData.plcPIB);
            
            /* Set Application to next state */
            appData.state = APP_STATE_READY;
            break;
        }

        case APP_STATE_READY:
        {
            /* Check USI status in case of USI device has been reset */
            if (SRV_USI_Status(appData.srvUSIHandle) == SRV_USI_STATUS_NOT_CONFIGURED)
            {
                /* Set Application to next state */
                appData.state = APP_STATE_CONFIG_USI;  
                SYS_TIME_TimerStop(appData.tmr1Handle);
                /* Disable Blink Led */
                USER_BLINK_LED_Off();
            }
            break;
        }

        case APP_STATE_ERROR:
        {
            /* Handle error in application's state machine */
            break;
        }

        /* The default state should never be executed. */
        default:
        {
            break;
        }
    }
}


/*******************************************************************************
 End of File
 */
