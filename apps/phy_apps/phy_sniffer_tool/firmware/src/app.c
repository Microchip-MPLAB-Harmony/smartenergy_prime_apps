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
 ******************************************************************************/

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

static void APP_PLCDataIndCb(DRV_PLC_PHY_RECEPTION_OBJ *indObj, uintptr_t context)
{
    /* Avoid warning */
    (void)context;

    /* Send Received PLC message through USI */
    if (indObj->dataLength) {
        size_t length;
        
        /* Report RX Symbols */
        appPlcData.plcPIB.id = PLC_ID_RX_PAY_SYMBOLS;
        appPlcData.plcPIB.length = 2;
        DRV_PLC_PHY_PIBGet(appPlcData.drvPlcHandle, &appPlcData.plcPIB);

        SRV_PSNIFFER_SetRxPayloadSymbols(*(uint16_t *)appPlcData.plcPIB.pData);

        /* Serialize received message */
        length = SRV_PSNIFFER_SerialRxMessage(appPlcData.pSerialData, indObj);
        /* Send through USI */
        SRV_USI_Send_Message(appPlcData.srvUSIHandle, SRV_USI_PROT_ID_SNIF_PRIME,
                appPlcData.pSerialData, length);

        /* Turn on indication LED and start timer to turn it off */
        SYS_TIME_TimerDestroy(appPlcData.tmr2Handle);
        appPlcData.tmr2Expired = false;
        USER_PLC_IND_LED_On();
        appPlcData.tmr2Handle = SYS_TIME_CallbackRegisterMS(APP_Timer2_Callback, 0,
                LED_BLINK_PLC_MSG_MS, SYS_TIME_SINGLE);
    }
}

void APP_USIPhyProtocolEventHandler(uint8_t *pData, size_t length)
{
    SRV_PSNIFFER_COMMAND command;

    /* Protection for invalid length */
    if (!length)
    {
        return;
    }

    /* Process received command */
    command = SRV_PSNIFFER_GetCommand(pData);

    switch (command)
    {
        case SRV_PSNIFFER_CMD_SET_PLC_CHANNEL:
        {
            DRV_PLC_PHY_CHANNEL channel;
            
            channel = *(pData + 1);
            
            if ((appPlcData.channel != channel) && (channel >= CHN1) && (channel <= CHN7_CHN8))
            {
                appPlcData.channel = channel;
                
                /* Set channel configuration */
                appPlcData.plcPIB.id = PLC_ID_CHANNEL_CFG;
                appPlcData.plcPIB.length = 1;
                *appPlcData.plcPIB.pData = channel;
                DRV_PLC_PHY_PIBSet(appPlcData.drvPlcHandle, &appPlcData.plcPIB);

                /* Update channel in PSniffer */
                SRV_PSNIFFER_SetPLCChannel(appPlcData.channel);
            }

            break;
        }

        default:
            break;
    }
}

// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

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

    /* Initialize PLC objects */
    appPlcData.plcPIB.pData = pPLCDataPIBBuffer;
    appPlcData.pSerialData = pSerialDataBuffer;
    
    /* Init Channel */
    appPlcData.channel = CHN1;
}


/******************************************************************************
  Function:
    void APP_Tasks(void)

  Remarks:
    See prototype in app.h.
 */
void APP_Tasks(void)
{
    /* Update Watchdog */
    CLEAR_WATCHDOG();
    
    /* Signalling */
    if (appPlcData.tmr1Expired)
    {
        appPlcData.tmr1Expired = false;
        USER_BLINK_LED_Toggle();
    }
    
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
                DRV_PLC_PHY_DataIndCallbackRegister(appPlcData.drvPlcHandle,
                        APP_PLCDataIndCb, DRV_PLC_PHY_INDEX);

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
                        SRV_USI_PROT_ID_SNIF_PRIME, APP_USIPhyProtocolEventHandler);
                
                /* Set channel configuration */
                appPlcData.plcPIB.id = PLC_ID_CHANNEL_CFG;
                appPlcData.plcPIB.length = 1;
                *appPlcData.plcPIB.pData = appPlcData.channel;
                DRV_PLC_PHY_PIBSet(appPlcData.drvPlcHandle, &appPlcData.plcPIB);
                /* Update channel in PSniffer */
                SRV_PSNIFFER_SetPLCChannel(appPlcData.channel);

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
                appPlcData.state = APP_PLC_STATE_READY;
            }
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
            /* TODO: Handle error in application's state machine. */
            break;
        }
    }
}


/*******************************************************************************
 End of File
 */
