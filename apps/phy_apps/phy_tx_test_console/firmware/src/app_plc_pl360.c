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
#define DRV_PLC_PHY_INDEX_0   0
#define APP_PLC_CONFIG_KEY  0xA55A

/* PLC Driver Initialization Data (initialization.c) */
extern DRV_PLC_PHY_INIT drvPlcPhyInitData;

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

CACHE_ALIGN APP_PLC_DATA appPlc;
CACHE_ALIGN APP_PLC_DATA_TX appPlcTx;

static CACHE_ALIGN uint8_t appPlcPibDataBuffer[CACHE_ALIGNED_SIZE_GET(APP_PLC_PIB_BUFFER_SIZE)];
static CACHE_ALIGN uint8_t appPlcTxDataBuffer[CACHE_ALIGNED_SIZE_GET(APP_PLC_BUFFER_SIZE)];

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************
void Timer1_Callback (uintptr_t context)
{
    appPlc.tmr1Expired = true;
}

void Timer2_Callback (uintptr_t context)
{
    appPlc.tmr2Expired = true;
}

static void APP_PLC_ExceptionCb(DRV_PLC_PHY_EXCEPTION exceptionObj, uintptr_t context )
{
    /* Avoid warning */
    (void)context;

    /* Update PLC TX Status */
    appPlc.plcTxState = APP_PLC_TX_STATE_IDLE;
    /* Restore TX configuration */
    appPlc.state = APP_PLC_STATE_READ_CONFIG;
}

static void APP_PLC_DataCfmCb(DRV_PLC_PHY_TRANSMISSION_CFM_OBJ *cfmObj, uintptr_t context )
{
    /* Avoid warning */
    (void)context;

    /* Update PLC TX Status */
    appPlc.plcTxState = APP_PLC_TX_STATE_IDLE;

    /* Handle result of transmission : Show it through Console */
    switch(cfmObj->result)
    {
        case DRV_PLC_PHY_TX_RESULT_PROCESS:
            APP_CONSOLE_Print("  TX_RESULT_PROCESS\r\n");
            break;
        case DRV_PLC_PHY_TX_RESULT_SUCCESS:
            APP_CONSOLE_Print("  TX_RESULT_SUCCESS\r\n");
            break;
        case DRV_PLC_PHY_TX_RESULT_INV_LENGTH:
            APP_CONSOLE_Print("  TX_RESULT_INV_LENGTH\r\n");
            break;
        case DRV_PLC_PHY_TX_RESULT_BUSY_CH:
            APP_CONSOLE_Print("  TX_RESULT_BUSY_CH\r\n");
            break;
        case DRV_PLC_PHY_TX_RESULT_BUSY_TX:
            APP_CONSOLE_Print("  TX_RESULT_BUSY_TX\r\n");
            break;
        case DRV_PLC_PHY_TX_RESULT_BUSY_RX:
            APP_CONSOLE_Print("  TX_RESULT_BUSY_RX\r\n");
            break;
        case DRV_PLC_PHY_TX_RESULT_INV_SCHEME:
            APP_CONSOLE_Print("  TX_RESULT_INV_SCHEME\r\n");
            break;
        case DRV_PLC_PHY_TX_RESULT_TIMEOUT:
            APP_CONSOLE_Print("  TX_RESULT_TIMEOUT\r\n");
            break;
        case DRV_PLC_PHY_TX_RESULT_INV_BUFFER:
            APP_CONSOLE_Print("  TX_RESULT_INV_BUFFER\r\n");
            break;
        case DRV_PLC_PHY_TX_RESULT_INV_MODE:
            APP_CONSOLE_Print("  TX_RESULT_INV_MODE\r\n");
            break;
        case DRV_PLC_PHY_TX_RESULT_INV_TX_MODE:
            APP_CONSOLE_Print("  TX_RESULT_INV_TX_MODE\r\n");
            break;
        case DRV_PLC_PHY_TX_RESULT_CANCELLED:
            APP_CONSOLE_Print("  TX_RESULT_CANCELLED\r\n");
            break;
        case DRV_PLC_PHY_TX_RESULT_NO_TX:
            APP_CONSOLE_Print("  TX_RESULT_NO_TX\r\n");
            break;
    }
}

static void APP_PLC_DataIndCb( DRV_PLC_PHY_RECEPTION_OBJ *indObj, uintptr_t context )
{
    /* Avoid warning */
    (void)context;

    if (indObj->dataLength)
    {
        USER_PLC_IND_LED_On();
        /* Start signal timer */
        appPlc.tmr2Handle = SYS_TIME_CallbackRegisterMS(Timer2_Callback, 0, LED_PLC_RX_MSG_RATE_MS, SYS_TIME_SINGLE);
    }
}

// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************
void APP_PLC_PL360_SetChannel ( SRV_PLC_PCOUP_CHANNEL channel )
{
    appPlcTx.channel = channel;

    /* Set channel configuration */
    appPlc.plcPIB.id = PLC_ID_CHANNEL_CFG;
    appPlc.plcPIB.length = 1;
    *appPlc.plcPIB.pData = channel;
    DRV_PLC_PHY_PIBSet(appPlc.drvPl360Handle, &appPlc.plcPIB);

    /* Apply PLC coupling configuration for the selected channel */
    SRV_PCOUP_SetChannelConfig(appPlc.drvPl360Handle, channel);

}

void APP_PLC_PL360_SetModScheme ( DRV_PLC_PHY_SCH scheme )
{
    appPlcTx.pl360Tx.scheme = scheme;
    switch(scheme)
    {
        case SCHEME_DBPSK:
            appPlcTx.pl360Tx.frameType = FRAME_TYPE_A;
            /* Add 7 bytes that are sent in header (Type A) */
            appPlcTx.maxPsduLen = 756 + 7;
            break;
        case SCHEME_DQPSK:
            appPlcTx.pl360Tx.frameType = FRAME_TYPE_A;
            appPlcTx.maxPsduLen = 1512 + 7;
            break;
        case SCHEME_D8PSK:
            appPlcTx.pl360Tx.frameType = FRAME_TYPE_A;
            appPlcTx.maxPsduLen = 2268 + 7;
            break;
        case SCHEME_DBPSK_C:
            appPlcTx.pl360Tx.frameType = FRAME_TYPE_A;
            appPlcTx.maxPsduLen = 377 + 7;
            break;
        case SCHEME_DQPSK_C:
            appPlcTx.pl360Tx.frameType = FRAME_TYPE_A;
            appPlcTx.maxPsduLen = 755 + 7;
            break;
        case SCHEME_D8PSK_C:
            appPlcTx.pl360Tx.frameType = FRAME_TYPE_A;
            appPlcTx.maxPsduLen = 1133 + 7;
            break;
        case SCHEME_R_DBPSK:
            /* Robust modulation: only supported in Type B */
            appPlcTx.pl360Tx.frameType = FRAME_TYPE_B;
            appPlcTx.maxPsduLen = 377;
            break;
        case SCHEME_R_DQPSK:
            /* Robust modulation: only supported in Type B */
            appPlcTx.pl360Tx.frameType = FRAME_TYPE_B;
            appPlcTx.maxPsduLen = 755;
            break;
    }

    /* Saturate to maximum data length allowed by PLC PHY (511) */
    if (appPlcTx.maxPsduLen > 511)
    {
        appPlcTx.maxPsduLen = 511;
    }
}

/*******************************************************************************
  Function:
    void APP_PLC_PL360_Initialize(void)

  Remarks:
    See prototype in app_plc.h.
 */
void APP_PLC_PL360_Initialize ( void )
{
    /* IDLE state is used to signal when application is started */
    appPlc.state = APP_PLC_STATE_IDLE;

    /* Init PLC PIB buffer */
    appPlc.plcPIB.pData = appPlcPibDataBuffer;

    /* Init PLC TX Buffer */
    appPlcTx.pl360Tx.pTransmitData = appPlcTxDataBuffer;

    /* Init Timer handler */
    appPlc.tmr1Handle = SYS_TIME_HANDLE_INVALID;
    appPlc.tmr2Handle = SYS_TIME_HANDLE_INVALID;
    appPlc.tmr1Expired = false;
    appPlc.tmr2Expired = false;

    /* Init signalling */
    appPlc.signalResetCounter = LED_RESET_BLINK_COUNTER;

    /* Init PLC TX status */
    appPlc.plcTxState = APP_PLC_TX_STATE_IDLE;
}

/******************************************************************************
  Function:
    void APP_PLC_PL360_Tasks ( void )

  Remarks:
    See prototype in app_plc.h.
 */

void APP_PLC_PL360_Tasks ( void )
{
    /* Signalling: LED Toggle */
    if (appPlc.tmr1Expired)
    {
        appPlc.tmr1Expired = false;
        USER_BLINK_LED_Toggle();

        if (appPlc.signalResetCounter)
        {
            appPlc.signalResetCounter--;
        }
    }

    /* Signalling: PLC RX */
    if (appPlc.tmr2Expired)
    {
        appPlc.tmr2Expired = false;
        USER_PLC_IND_LED_Off();
    }

    /* Check the application's current state. */
    switch ( appPlc.state )
    {
        case APP_PLC_STATE_IDLE:
        {
            /* Signalling when the application is starting */
            if (appPlc.signalResetCounter)
            {
                if (appPlc.tmr1Handle == SYS_TIME_HANDLE_INVALID)
                {
                    /* Init Timer to handle blinking led */
                    appPlc.tmr1Handle = SYS_TIME_CallbackRegisterMS(Timer1_Callback, 0, LED_RESET_BLINK_RATE_MS, SYS_TIME_PERIODIC);
                }
            }
            else
            {
                SYS_TIME_TimerDestroy(appPlc.tmr1Handle);
                appPlc.tmr1Handle = SYS_TIME_HANDLE_INVALID;

                /* Read configuration from NVM memory */
                appPlc.state = APP_PLC_STATE_READ_CONFIG;
            }
            break;
        }

        case APP_PLC_STATE_READ_CONFIG:
        {
            if (appNvm.state == APP_NVM_STATE_CMD_WAIT)
            {
                appNvm.pData = (uint8_t*)&appPlcTx;
                appNvm.dataLength = sizeof(appPlcTx);
                appNvm.state = APP_NVM_STATE_READ_MEMORY;

                appPlc.state = APP_PLC_STATE_CHECK_CONFIG;
            }
            break;
        }

        case APP_PLC_STATE_CHECK_CONFIG:
        {
            if (appNvm.state == APP_NVM_STATE_CMD_WAIT)
            {
                if (appPlcTx.configKey != APP_PLC_CONFIG_KEY)
                {
                    uint8_t index;
                    uint8_t* pData;

                    /* Set configuration by default */
                    appPlcTx.configKey = APP_PLC_CONFIG_KEY;
                    appPlcTx.pl360PhyVersion = 0;
                    appPlcTx.txImpedance = HI_STATE;
                    appPlcTx.txAuto = 1;
                    appPlcTx.pl360Tx.time = 1000000;
                    appPlcTx.pl360Tx.attenuation = 0;

                    /* Set PLC TX configuration by default */
                    APP_PLC_PL360_SetModScheme(SCHEME_DBPSK_C);
                    appPlcTx.pl360Tx.time = 1000000;
                    appPlcTx.pl360Tx.attenuation = 0;
                    appPlcTx.pl360Tx.forced = 1;
                    appPlcTx.pl360Tx.bufferId = TX_BUFFER_0;
                    appPlcTx.pl360Tx.mode = TX_MODE_RELATIVE;
                    appPlcTx.pl360Tx.dataLength = 64;
                    appPlcTx.pl360Tx.pTransmitData = appPlcTxDataBuffer;
                    pData = appPlcTx.pl360Tx.pTransmitData;
                    for(index = 0; index < appPlcTx.pl360Tx.dataLength; index++)
                    {
                        *pData++ = index;
                    }

                    /* Init Channel */
                    appPlcTx.channel = SRV_PCOUP_GetDefaultChannel();

                    /* Clear Transmission flag */
                    appPlcTx.inTx = false;
                }

                /* Initialize PLC driver */
                appPlc.state = APP_PLC_STATE_INIT;
            }
            break;
        }

        case APP_PLC_STATE_WRITE_CONFIG:
        {
            if (appNvm.state == APP_NVM_STATE_CMD_WAIT)
            {
                appNvm.pData = (uint8_t*)&appPlcTx;
                appNvm.dataLength = sizeof(appPlcTx);
                appNvm.state = APP_NVM_STATE_WRITE_MEMORY;

                appPlc.state = APP_PLC_STATE_WAIT_CONFIG;
            }
            break;
        }

        case APP_PLC_STATE_WAIT_CONFIG:
        {
            if (appNvm.state == APP_NVM_STATE_CMD_WAIT)
            {
                if (appPlcTx.inTx)
                {
                    appPlc.state = APP_PLC_STATE_TX;
                }
                else
                {
                    appPlc.state = APP_PLC_STATE_WAITING;
                }
            }
            break;
        }

        case APP_PLC_STATE_INIT:
        {
            /* Open PLC driver */
            appPlc.drvPl360Handle = DRV_PLC_PHY_Open(DRV_PLC_PHY_INDEX_0, NULL);

            if (appPlc.drvPl360Handle != DRV_HANDLE_INVALID)
            {
                appPlc.state = APP_PLC_STATE_OPEN;
            }
            else
            {
                appPlc.state = APP_PLC_STATE_ERROR;
            }
            break;
        }

        case APP_PLC_STATE_OPEN:
        {
            /* Check PLC transceiver */
            if (DRV_PLC_PHY_Status(DRV_PLC_PHY_INDEX_0) == SYS_STATUS_READY)
            {
                /* Configure PLC callbacks */
                DRV_PLC_PHY_ExceptionCallbackRegister(appPlc.drvPl360Handle, APP_PLC_ExceptionCb, DRV_PLC_PHY_INDEX_0);
                DRV_PLC_PHY_DataCfmCallbackRegister(appPlc.drvPl360Handle, APP_PLC_DataCfmCb, DRV_PLC_PHY_INDEX_0);
                DRV_PLC_PHY_DataIndCallbackRegister(appPlc.drvPl360Handle, APP_PLC_DataIndCb, DRV_PLC_PHY_INDEX_0);

                /* Set channel and apply PLC coupling configuration */
                APP_PLC_PL360_SetChannel(appPlcTx.channel);

                /* Init Timer to handle blinking led */
                appPlc.tmr1Handle = SYS_TIME_CallbackRegisterMS(Timer1_Callback, 0, LED_BLINK_RATE_MS, SYS_TIME_PERIODIC);

                /* Get PLC PHY version */
                appPlc.plcPIB.id = PLC_ID_VERSION_NUM;
                appPlc.plcPIB.length = 4;
                DRV_PLC_PHY_PIBGet(appPlc.drvPl360Handle, &appPlc.plcPIB);
                appPlcTx.pl360PhyVersion = *(uint32_t *)appPlc.plcPIB.pData;

                if (appPlcTx.inTx)
                {
                    /* Previous Transmission state */
                    appPlc.state = APP_PLC_STATE_TX;
                }
                else
                {
                    /* Nothing To Do */
                    appPlc.state = APP_PLC_STATE_WAITING;
                }
            }
            break;
        }

        case APP_PLC_STATE_WAITING:
        {
            break;
        }

        case APP_PLC_STATE_TX:
        {
            if (!appPlcTx.inTx)
            {
                DRV_PLC_PHY_PIB_OBJ pibObj;

                /* Apply TX configuration */
                /* Set Autodetect Mode */
                pibObj.id = PLC_ID_CFG_AUTODETECT_IMPEDANCE;
                pibObj.length = 1;
                pibObj.pData = (uint8_t *)&appPlcTx.txAuto;
                DRV_PLC_PHY_PIBSet(appPlc.drvPl360Handle, &pibObj);
                /* Set Impedance Mode */
                pibObj.id = PLC_ID_CFG_IMPEDANCE;
                pibObj.length = 1;
                pibObj.pData = (uint8_t *)&appPlcTx.txImpedance;
                DRV_PLC_PHY_PIBSet(appPlc.drvPl360Handle, &pibObj);

                /* Set Transmission Mode */
                appPlcTx.pl360Tx.mode = TX_MODE_RELATIVE;

                /* Set Transmission flag */
                appPlcTx.inTx = true;

                /* Store TX configuration */
                appPlc.state = APP_PLC_STATE_WRITE_CONFIG;
            }
            else
            {
                if (appPlc.plcTxState == APP_PLC_TX_STATE_IDLE)
                {
                    appPlc.plcTxState = APP_PLC_TX_STATE_WAIT_TX_CFM;
                    /* Send PLC message */
                    DRV_PLC_PHY_Send(appPlc.drvPl360Handle, &appPlcTx.pl360Tx);
                }
            }

            break;
        }

        case APP_PLC_STATE_STOP_TX:
        {
            /* Clear Transmission flag */
            appPlcTx.inTx = false;

            /* Store TX configuration */
            appPlc.state = APP_PLC_STATE_WRITE_CONFIG;

            /* Cancel last transmission */
            if (appPlc.plcTxState == APP_PLC_TX_STATE_WAIT_TX_CFM)
            {
                /* Send PLC Cancel message */
                appPlcTx.pl360Tx.mode = TX_MODE_CANCEL | TX_MODE_RELATIVE;
                DRV_PLC_PHY_Send(appPlc.drvPl360Handle, &appPlcTx.pl360Tx);
            }
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
