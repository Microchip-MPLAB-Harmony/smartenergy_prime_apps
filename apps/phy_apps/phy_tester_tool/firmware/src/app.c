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

/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app_plc.c

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

CACHE_ALIGN APP_PLC_DATA appPlcData;
    
static CACHE_ALIGN uint8_t pPLCDataTxBuffer[CACHE_ALIGNED_SIZE_GET(APP_PLC_DATA_BUFFER_SIZE)];
static CACHE_ALIGN uint8_t pPLCDataPIBBuffer[CACHE_ALIGNED_SIZE_GET(APP_PLC_PIB_BUFFER_SIZE)];
static CACHE_ALIGN uint8_t pSerialDataBuffer[CACHE_ALIGNED_SIZE_GET(APP_SERIAL_DATA_BUFFER_SIZE)];

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************
static void APP_Timer1_Callback (uintptr_t context)
{
    appPlcData.tmr1Expired = true;
}

static void APP_Timer2_Callback (uintptr_t context)
{
    appPlcData.tmr2Expired = true;
}

static void APP_PLCSetCouplingConfiguration(DRV_PLC_PHY_CHANNEL channel)
{
    SRV_PCOUP_SetChannelConfig(appPlcData.drvPlcHandle, channel);
    
    /* Optional ***************************************************/
    /* Disable AUTO mode and set VLO behavior by default in order to
     * maximize signal level in any case */
    appPlcData.plcPIB.id = PLC_ID_CFG_AUTODETECT_IMPEDANCE;
    appPlcData.plcPIB.length = 1;
    *appPlcData.plcPIB.pData = 0;
    DRV_PLC_PHY_PIBSet(appPlcData.drvPlcHandle, &appPlcData.plcPIB);

    appPlcData.plcPIB.id = PLC_ID_CFG_IMPEDANCE;
    appPlcData.plcPIB.length = 1;
    *appPlcData.plcPIB.pData = 2;
    DRV_PLC_PHY_PIBSet(appPlcData.drvPlcHandle, &appPlcData.plcPIB);
}

static void APP_PLCExceptionCb(DRV_PLC_PHY_EXCEPTION exceptionObj,
        uintptr_t context)
{
    /* Avoid warning */
    (void)context;

    switch (exceptionObj)
    {
        case DRV_PLC_PHY_EXCEPTION_UNEXPECTED_KEY:
            appPlcData.plc_phy_err_unexpected++;
            break;

        case DRV_PLC_PHY_EXCEPTION_CRITICAL_ERROR:
            appPlcData.plc_phy_err_critical++;
            break;

        case DRV_PLC_PHY_EXCEPTION_RESET:
            appPlcData.plc_phy_err_reset++;
            break;

        default:
            appPlcData.plc_phy_err_unknow++;
	}

	appPlcData.plc_phy_exception = true;
}

static void APP_PLCDataIndCb(DRV_PLC_PHY_RECEPTION_OBJ *indObj, uintptr_t context)
{
    /* Avoid warning */
    (void)context;

    /* Send Received PLC message through USI */
    if (indObj->dataLength)
    {
        size_t length;

        /* Turn on indication LED and start timer to turn it off */
        SYS_TIME_TimerDestroy(appPlcData.tmr2Handle);
        appPlcData.tmr2Expired = false;
        USER_PLC_IND_LED_On();
        appPlcData.tmr2Handle = SYS_TIME_CallbackRegisterMS(APP_Timer2_Callback, 0,
                LED_BLINK_PLC_MSG_MS, SYS_TIME_SINGLE);

        /* Add received message */
        length = SRV_PSERIAL_SerialRxMessage(appPlcData.pSerialData, indObj);
        /* Send through USI */
        SRV_USI_Send_Message(appPlcData.srvUSIHandle, SRV_USI_PROT_ID_PHY,
                appPlcData.pSerialData, length);
    }
}

static void APP_PLCDataCfmCb(DRV_PLC_PHY_TRANSMISSION_CFM_OBJ *cfmObj, uintptr_t context)
{
    size_t length;

    /* Avoid warning */
    (void)context;
    
    /* Add received message */
    length = SRV_PSERIAL_SerialCfmMessage(appPlcData.pSerialData, cfmObj);
    /* Send through USI */
    SRV_USI_Send_Message(appPlcData.srvUSIHandle, SRV_USI_PROT_ID_PHY,
            appPlcData.pSerialData, length);

}

static void APP_PVDDMonitorCb( SRV_PVDDMON_CMP_MODE cmpMode, uintptr_t context )
{
    (void)context;
    
    if (cmpMode == SRV_PVDDMON_CMP_MODE_OUT)
    {
        /* PLC Transmission is not permitted */
        DRV_PLC_PHY_EnableTX(appPlcData.drvPlcHandle, false);
        appPlcData.pvddMonTxEnable = false;
        /* Restart PVDD Monitor to check when VDD is within the comparison window */
        SRV_PVDDMON_Restart(SRV_PVDDMON_CMP_MODE_IN);
    }
    else
    {
        /* PLC Transmission is permitted again */
        DRV_PLC_PHY_EnableTX(appPlcData.drvPlcHandle, true);
        appPlcData.pvddMonTxEnable = true;
        /* Restart PVDD Monitor to check when VDD is out of the comparison window */
        SRV_PVDDMON_Restart(SRV_PVDDMON_CMP_MODE_OUT);
    }
}

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
            SRV_PSERIAL_ParseGetPIB(&appPlcData.plcPIB, pData);

            if (DRV_PLC_PHY_PIBGet(appPlcData.drvPlcHandle, &appPlcData.plcPIB))
            {
                size_t len;

                /* Serialize PIB data */
                len = SRV_PSERIAL_SerialGetPIB(appPlcData.pSerialData, &appPlcData.plcPIB);
                /* Send through USI */
                SRV_USI_Send_Message(appPlcData.srvUSIHandle, SRV_USI_PROT_ID_PHY,
                        appPlcData.pSerialData, len);
            }
        }
        break;

        case SRV_PSERIAL_CMD_PHY_SET_CFG:
        {
            bool sendUSIResponse = false;
            
            /* Extract PIB information */
            SRV_PSERIAL_ParseSetPIB(&appPlcData.plcPIB, pData);
            
            /* Manage Channels configuration */
            if (appPlcData.plcPIB.id == PLC_ID_CHANNEL_CFG)
            {
                DRV_PLC_PHY_CHANNEL channel;
                
                channel = *appPlcData.plcPIB.pData;
                
                if ((appPlcData.channel != channel) && (SRV_PCOUP_GetChannelConfig(channel) != NULL))
                {
                    if (DRV_PLC_PHY_PIBSet(appPlcData.drvPlcHandle, &appPlcData.plcPIB))
                    {
                            /* Update channel application data */
                            appPlcData.channel = channel;
                            /* Set configuration for PLC */
                            APP_PLCSetCouplingConfiguration(appPlcData.channel);
                            
                            sendUSIResponse = true;
                    }
                }
            } 
            else if (DRV_PLC_PHY_PIBSet(appPlcData.drvPlcHandle, &appPlcData.plcPIB))
            {
                sendUSIResponse = true;
            }
            
            if (sendUSIResponse)
            {
                size_t len;

                /* Serialize PIB data */
                len = SRV_PSERIAL_SerialSetPIB(&appPlcData.pSerialData[1], &appPlcData.plcPIB);
                /* Send through USI */
                SRV_USI_Send_Message(appPlcData.srvUSIHandle, SRV_USI_PROT_ID_PHY,
                        appPlcData.pSerialData, len);
            }
        }
        break;

        case SRV_PSERIAL_CMD_PHY_SEND_MSG:
        {
            if (appPlcData.pvddMonTxEnable)
            {
                /* Capture and parse data from USI */
                SRV_PSERIAL_ParseTxMessage(&appPlcData.plcTxObj, pData);

                /* Send Message through PLC */
                DRV_PLC_PHY_TxRequest(appPlcData.drvPlcHandle, &appPlcData.plcTxObj);
            }
            else
            {
                DRV_PLC_PHY_TRANSMISSION_CFM_OBJ cfmData;
                
                cfmData.timeIni = 0;
                cfmData.rmsCalc = 0;
                cfmData.result = DRV_PLC_PHY_TX_RESULT_NO_TX;
                APP_PLCDataCfmCb(&cfmData, 0);
            }
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
    void APP_Initialize(void)

  Remarks:
    See prototype in app.h.
 */

void APP_Initialize(void)
{
    /* Place the App state machine in its initial state. */
    appPlcData.state = APP_PLC_STATE_IDLE;

    /* Init Timer handler */
    appPlcData.tmr1Handle = SYS_TIME_HANDLE_INVALID;
    appPlcData.tmr2Handle = SYS_TIME_HANDLE_INVALID;
    appPlcData.tmr1Expired = false;
    appPlcData.tmr2Expired = false;

    /* Reset PLC exceptions statistics */
    appPlcData.plc_phy_err_unexpected = 0;
    appPlcData.plc_phy_err_critical = 0;
    appPlcData.plc_phy_err_reset = 0;
    appPlcData.plc_phy_err_unknow = 0;

    /* Initialize PLC buffers */
    appPlcData.plcTxObj.pTransmitData = pPLCDataTxBuffer;
    appPlcData.plcPIB.pData = pPLCDataPIBBuffer;
    appPlcData.pSerialData = pSerialDataBuffer;
    
    /* Init Channel */
    appPlcData.channel = SRV_PCOUP_GetDefaultChannel();
}


/******************************************************************************
  Function:
    void APP_Tasks(void)

  Remarks:
    See prototype in app.h.
 */
void APP_Tasks(void)
{
    CLEAR_WATCHDOG();
    
    /* Signalling: LED Toggle */
    if (appPlcData.tmr1Expired)
    {
        appPlcData.tmr1Expired = false;
        USER_BLINK_LED_Toggle();
    }
    
    /* Signalling: PLC RX */
    if (appPlcData.tmr2Expired)
    {
        appPlcData.tmr2Expired = false;
        USER_PLC_IND_LED_Off();
    }
    
    /* Check the application's current state. */
    switch(appPlcData.state)
    {
        /* Application's initial state. */
        case APP_PLC_STATE_IDLE:
        {
            /* Open PLC driver : Start uploading process */
            appPlcData.drvPlcHandle = DRV_PLC_PHY_Open(DRV_PLC_PHY_INDEX, NULL);

            if (appPlcData.drvPlcHandle != DRV_HANDLE_INVALID)
            {
                /* Set Application to next state */
                appPlcData.state = APP_PLC_STATE_REGISTER;
            }
            else
            {
                /* Set Application to ERROR state */
                appPlcData.state = APP_PLC_STATE_ERROR;
            }
            break;
        }

        /* Waiting to PLC transceiver be opened and register callback functions */
        case APP_PLC_STATE_REGISTER:
        {
            /* Check PLC transceiver */
            if (DRV_PLC_PHY_Status(DRV_PLC_PHY_INDEX) == SYS_STATUS_READY)
            {
                /* Register PLC callback */
                DRV_PLC_PHY_ExceptionCallbackRegister(appPlcData.drvPlcHandle,
                        APP_PLCExceptionCb, DRV_PLC_PHY_INDEX);
                DRV_PLC_PHY_DataIndCallbackRegister(appPlcData.drvPlcHandle,
                        APP_PLCDataIndCb, DRV_PLC_PHY_INDEX);
                DRV_PLC_PHY_TxCfmCallbackRegister(appPlcData.drvPlcHandle,
                        APP_PLCDataCfmCb, DRV_PLC_PHY_INDEX);

                /* Open USI Service */
                appPlcData.srvUSIHandle = SRV_USI_Open(SRV_USI_INDEX_0);

                if (appPlcData.srvUSIHandle != DRV_HANDLE_INVALID)
                {
                    /* Set Application to next state */
                    appPlcData.state = APP_PLC_STATE_CONFIG_USI;
                }
                else
                {
                    /* Set Application to ERROR state */
                    appPlcData.state = APP_PLC_STATE_ERROR;
                }
            }
            break;
        }

        case APP_PLC_STATE_CONFIG_USI:
        {
            if (SRV_USI_Status(appPlcData.srvUSIHandle) == SRV_USI_STATUS_CONFIGURED)
            {
                /* Register USI callback */
                SRV_USI_CallbackRegister(appPlcData.srvUSIHandle,
                        SRV_USI_PROT_ID_PHY, APP_USIPhyProtocolEventHandler);

                if (appPlcData.tmr1Handle == SYS_TIME_HANDLE_INVALID)
                {
                    /* Register Timer Callback */
                    appPlcData.tmr1Handle = SYS_TIME_CallbackRegisterMS(
                            APP_Timer1_Callback, 0, LED_BLINK_RATE_MS,
                            SYS_TIME_PERIODIC);
                }
                else
                {
                    SYS_TIME_TimerStart(appPlcData.tmr1Handle);
                }

                /* Set Application to next state */
                appPlcData.state = APP_PLC_STATE_CONFIG_PLC;
            }
            break;
        }

        case APP_PLC_STATE_CONFIG_PLC:
        {
            /* Set channel configuration */
            appPlcData.plcPIB.id = PLC_ID_CHANNEL_CFG;
            appPlcData.plcPIB.length = 1;
            *appPlcData.plcPIB.pData = appPlcData.channel;
            DRV_PLC_PHY_PIBSet(appPlcData.drvPlcHandle, &appPlcData.plcPIB);

            /* Set configuration for PLC */
            APP_PLCSetCouplingConfiguration(appPlcData.channel);

            /* Disable TX Enable at the beginning */
            DRV_PLC_PHY_EnableTX(appPlcData.drvPlcHandle, false);
            appPlcData.pvddMonTxEnable = false;
            /* Enable PLC PVDD Monitor Service */
            SRV_PVDDMON_CallbackRegister(APP_PVDDMonitorCb, 0);
            SRV_PVDDMON_Start(SRV_PVDDMON_CMP_MODE_IN);
            /* Set Application to next state */
            appPlcData.state = APP_PLC_STATE_READY;
            break;
        }

        case APP_PLC_STATE_READY:
        {
            /* Check USI status in case of USI device has been reset */
            if (SRV_USI_Status(appPlcData.srvUSIHandle) == SRV_USI_STATUS_NOT_CONFIGURED)
            {
                /* Set Application to next state */
                appPlcData.state = APP_PLC_STATE_CONFIG_USI;  
                SYS_TIME_TimerStop(appPlcData.tmr1Handle);
                /* Disable Blink Led */
                USER_BLINK_LED_Off();
            }
            break;
        }

        case APP_PLC_STATE_ERROR:
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
