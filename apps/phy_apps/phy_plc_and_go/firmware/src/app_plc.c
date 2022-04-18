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

#define div_round(a, b)      (((a) + (b >> 1)) / (b))

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

APP_PLC_DATA appPlc;
APP_PLC_DATA_TX appPlcTx;

static CACHE_ALIGN uint8_t appPlcPibDataBuffer[CACHE_ALIGNED_SIZE_GET(APP_PLC_PIB_BUFFER_SIZE)];
static CACHE_ALIGN uint8_t appPlcTxDataBuffer[CACHE_ALIGNED_SIZE_GET(APP_PLC_BUFFER_SIZE)];

static void APP_PLC_SetInitialConfiguration ( void )
{
    /* Get PLC PHY version */
    appPlc.plcPIB.id = PLC_ID_VERSION_NUM;
    appPlc.plcPIB.length = 11;
    DRV_PLC_PHY_PIBGet(appPlc.drvPl360Handle, &appPlc.plcPIB);
    appPlcTx.pl360PhyVersion = *(uint32_t *)appPlc.plcPIB.pData;
    
    /* Set PLC TX configuration by default */
    APP_PLC_SetModScheme(SCHEME_DBPSK_C);
    appPlcTx.pl360Tx.time = 0;
    appPlcTx.pl360Tx.attenuation = 0;
    appPlcTx.pl360Tx.forced = 1;
    appPlcTx.pl360Tx.bufferId = TX_BUFFER_0;
    appPlcTx.pl360Tx.mode = TX_MODE_RELATIVE;
    appPlcTx.pl360Tx.pTransmitData = appPlcTx.pDataTx;
    appPlcTx.pl360Tx.dataLength = 0;
    
    /* Set channel configuration by default */
    APP_PLC_SetChannel(SRV_PCOUP_Get_Default_Channel());

    /* Force Transmission to VLO mode by default in order to maximize signal level in anycase */
    /* Disable autodetect mode */
    appPlcTx.txAuto = 0;
    appPlc.plcPIB.id = PLC_ID_CFG_AUTODETECT_IMPEDANCE;
    appPlc.plcPIB.length = 1;
    *appPlc.plcPIB.pData = appPlcTx.txAuto;
    DRV_PLC_PHY_PIBSet(appPlc.drvPl360Handle, &appPlc.plcPIB);

    /* Set VLO mode */
    appPlcTx.txImpedance = 2;
    appPlc.plcPIB.id = PLC_ID_CFG_IMPEDANCE;
    appPlc.plcPIB.length = 1;
    *appPlc.plcPIB.pData = appPlcTx.txImpedance;
    DRV_PLC_PHY_PIBSet(appPlc.drvPl360Handle, &appPlc.plcPIB);

}

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

static void APP_PLC_PVDDMonitorCb( SRV_PVDDMON_CMP_MODE cmpMode, uintptr_t context )
{
    (void)context;
    
    if (cmpMode == SRV_PVDDMON_CMP_MODE_OUT)
    {
        /* PLC Transmission is not permitted */
        DRV_PLC_PHY_EnableTX(appPlc.drvPl360Handle, false);
        appPlc.pvddMonTxEnable = false;
        /* Restart PVDD Monitor to check when VDD is within the comparison window */
        SRV_PVDDMON_Restart(SRV_PVDDMON_CMP_MODE_IN);
    }
    else
    {
        /* PLC Transmission is permitted again */
        DRV_PLC_PHY_EnableTX(appPlc.drvPl360Handle, true);
        appPlc.pvddMonTxEnable = true;
        /* Restart PVDD Monitor to check when VDD is out of the comparison window */
        SRV_PVDDMON_Restart(SRV_PVDDMON_CMP_MODE_OUT);
    }
}

static void APP_PLC_SleepModeDisableCb( uintptr_t context )
{
    /* Avoid warning */
    (void)context;

    /* Apply PLC initial configuration */
    APP_PLC_SetInitialConfiguration();

    /* Set PLC state */
    appPlc.state = APP_PLC_STATE_WAITING;
}

static void APP_PLC_ExceptionCb(DRV_PLC_PHY_EXCEPTION exceptionObj, uintptr_t context )
{
    /* Avoid warning */
    (void)context;
    (void)exceptionObj;

    /* Update PLC TX Status */
    appPlc.plcTxState = APP_PLC_TX_STATE_IDLE;
    /* Restart PLC task */
    appPlc.state = APP_PLC_STATE_IDLE;
}

static void APP_PLC_DataCfmCb(DRV_PLC_PHY_TRANSMISSION_CFM_OBJ *cfmObj, uintptr_t context )
{
    /* Avoid warning */
    (void)context;

    /* Update PLC TX Status */
    appPlc.plcTxState = APP_PLC_TX_STATE_IDLE;
    
    /* Capture TX result of the last transmission */
    appPlc.lastTxResult = cfmObj->result;
}

static void APP_PLC_DataIndCb( DRV_PLC_PHY_RECEPTION_OBJ *indObj, uintptr_t context )
{
    uint32_t crcCalc;
    uint32_t crcReceived;
    uint16_t dataLength;
    
    /* Avoid warning */
    (void)context;
    
    dataLength = indObj->dataLength;
    /* Check if data length is at least 5 bytes (1 data byte + 4 CRC bytes) */
    if ((dataLength > 4) && (dataLength < 512))
    {
        /* Discount 4 bytes corresponding to 32-bit CRC */
        dataLength -= 4;
        
        /* Compute PRIME 32-bit CRC. Use PCRC service */
        crcCalc = SRV_PCRC_GetValue(indObj->pReceivedData, dataLength, 
                PCRC_HT_USI, PCRC_CRC32, 0);
        
        /* Get CRC from last 4 bytes of the message */
        crcReceived = indObj->pReceivedData[dataLength + 3];
        crcReceived += (uint32_t)indObj->pReceivedData[dataLength + 2] << 8;
        crcReceived += (uint32_t)indObj->pReceivedData[dataLength + 1] << 16;
        crcReceived += (uint32_t)indObj->pReceivedData[dataLength] << 24;
        
        /* Check integrity of received message comparing the computed CRC with the received CRC in last 4 bytes */
        if (crcCalc == crcReceived)
        {
            /* CRC Ok. */
            /* Ignore CRC bytes */
            indObj->dataLength -= 4;
            /* Show Rx message via Console */
            APP_CONSOLE_ShowMessage(indObj);
        }
        else
        {
            APP_CONSOLE_Print("\rReceived message: CRC error\r\n");
        }
    }
    else
    {
        /* Length error: length in message content should never be more than total data length from PHY */
        APP_CONSOLE_Print("\rReceived message: Length error\r\n");
    }
    
    APP_CONSOLE_Print(MENU_CMD_PROMPT);
}

// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************
/*******************************************************************************
  Function:
    void APP_PLC_Initialize(void)

  Remarks:
    See prototype in app_plc.h.
 */
void APP_PLC_Initialize ( void )
{
    /* Init PLC PIB buffer */
    appPlc.plcPIB.pData = appPlcPibDataBuffer;

    /* Init PLC objects */
    appPlcTx.pDataTx = appPlcTxDataBuffer;
    appPlcTx.pl360Tx.pTransmitData = appPlcTx.pDataTx;
    
    /* Set PLC state */
    appPlc.state = APP_PLC_STATE_IDLE;
    
    /* Set PVDD Monitor tracking data */
    SRV_PVDDMON_Initialize();
    appPlc.pvddMonTxEnable = true;
    
    /* Init PLC TX status */
    appPlc.plcTxState = APP_PLC_TX_STATE_IDLE;

    /* Init Timer handler */
    appPlc.tmr1Handle = SYS_TIME_HANDLE_INVALID;
    appPlc.tmr2Handle = SYS_TIME_HANDLE_INVALID;
    appPlc.tmr1Expired = false;
    appPlc.tmr2Expired = false;
    
    /* Init Channel */
    appPlcTx.channel = SRV_PCOUP_Get_Default_Channel();
    
}

/******************************************************************************
  Function:
    void APP_PLC_Tasks ( void )

  Remarks:
    See prototype in app_plc.h.
 */

void APP_PLC_Tasks ( void )
{    
    /* Signalling */
    if (appPlc.tmr1Expired)
    {
        appPlc.tmr1Expired = false;
        USER_BLINK_LED_Toggle();
    }
    
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
            SYS_STATUS ret;
            
            ret = SYS_CONSOLE_Status(SYS_CONSOLE_INDEX_0);
            
            /* Wait Console initialization */
            if (ret == SYS_STATUS_READY)
            {
                /* Initialize PLC driver */
                appPlc.state = APP_PLC_STATE_INIT;
            }
        }
        break;

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
                DRV_PLC_PHY_TxCfmCallbackRegister(appPlc.drvPl360Handle, APP_PLC_DataCfmCb, DRV_PLC_PHY_INDEX_0);
                DRV_PLC_PHY_DataIndCallbackRegister(appPlc.drvPl360Handle, APP_PLC_DataIndCb, DRV_PLC_PHY_INDEX_0);
                DRV_PLC_PHY_SleepDisableCallbackRegister(appPlc.drvPl360Handle, APP_PLC_SleepModeDisableCb, DRV_PLC_PHY_INDEX_0);
                
                /* Apply PLC initial configuration */
                APP_PLC_SetInitialConfiguration();
                
                /* Enable PLC PVDD Monitor Service: ADC channel 0 */
                SRV_PVDDMON_CallbackRegister(APP_PLC_PVDDMonitorCb, 0);
                SRV_PVDDMON_Start(SRV_PVDDMON_CMP_MODE_OUT);
            
                /* Init Timer to handle blinking led */
                appPlc.tmr1Handle = SYS_TIME_CallbackRegisterMS(Timer1_Callback, 0, LED_BLINK_RATE_MS, SYS_TIME_PERIODIC);
                
                /* Set PLC state */
                appPlc.state = APP_PLC_STATE_WAITING;
            }
        }
        break;

        case APP_PLC_STATE_WAITING:
        {
            break;
        }

        case APP_PLC_STATE_WAITING_TX_CFM:
        {
            if (appPlc.plcTxState != APP_PLC_TX_STATE_WAIT_TX_CFM)
            {
                appPlc.state = APP_PLC_STATE_WAITING;
            }
            break;
        }

        case APP_PLC_STATE_SET_CHANNEL:
        {
            if (appPlc.plcTxState != APP_PLC_TX_STATE_WAIT_TX_CFM)
            {
                /* Set channel configuration */
                APP_PLC_SetChannel(appPlcTx.channel);
                appPlc.state = APP_PLC_STATE_WAITING;
            }
            
            break;
        }

        /* The default state should never be executed. */
        default:
        {
            /* Handle error in application's state machine. */
            break;
        }
    }
}

/*******************************************************************************
  Function:
    bool APP_PLC_Initialize(void)

  Remarks:
    See prototype in app_plc.h.
 */
bool APP_PLC_SendData ( uint8_t* pData, uint16_t length )
{
    if (appPlc.state == APP_PLC_STATE_WAITING)
    {
        if (appPlc.pvddMonTxEnable)
        {
            if ((length > 0) && (length <= (APP_PLC_BUFFER_SIZE - 4)))
            {
                uint32_t crcCalc;
                
                /* First byte should be reserved for MAC purposes in Type A frames */
                appPlcTx.pDataTx[0] = 0;
                memcpy(&appPlcTx.pDataTx[1], pData, length++);
                
                /* Compute PRIME 32-bit CRC and add to buffer after message. Use PCRC service */
                crcCalc = SRV_PCRC_GetValue(appPlcTx.pDataTx, length, PCRC_HT_USI, PCRC_CRC32, 0);
                appPlcTx.pDataTx[length++] = (uint8_t)(crcCalc >> 24);
                appPlcTx.pDataTx[length++] = (uint8_t)(crcCalc >> 16);
                appPlcTx.pDataTx[length++] = (uint8_t)(crcCalc >> 8);
                appPlcTx.pDataTx[length++] = (uint8_t)(crcCalc);

                /* Adjust new data length */
                appPlcTx.pl360Tx.dataLength = length;

                appPlc.plcTxState = APP_PLC_TX_STATE_WAIT_TX_CFM;

                DRV_PLC_PHY_TxRequest(appPlc.drvPl360Handle, &appPlcTx.pl360Tx);

                /* Set PLC state */
                if (appPlc.plcTxState == APP_PLC_TX_STATE_WAIT_TX_CFM)
                {
                    appPlc.state = APP_PLC_STATE_WAITING_TX_CFM;
                    return true;
                }
            }
        }
    }
    
    return false;
}

void APP_PLC_SetModScheme ( DRV_PLC_PHY_SCH scheme )
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

void APP_PLC_SetChannel ( DRV_PLC_PHY_CHANNEL channel )
{
    appPlcTx.channel = channel;
    
    /* Set channel configuration */
    appPlc.plcPIB.id = PLC_ID_CHANNEL_CFG;
    appPlc.plcPIB.length = 1;
    *appPlc.plcPIB.pData = appPlcTx.channel;
    DRV_PLC_PHY_PIBSet(appPlc.drvPl360Handle, &appPlc.plcPIB);
                
    /* Apply PLC coupling configuration for the selected channel */
    SRV_PCOUP_Set_Channel_Config(appPlc.drvPl360Handle, appPlcTx.channel);

}

bool APP_PLC_SetSleepMode ( bool enable )
{
    bool sleepIsEnabled = (appPlc.state == APP_PLC_STATE_SLEEP);
    
    if (sleepIsEnabled != enable)
    {
        DRV_PLC_PHY_Sleep(appPlc.drvPl360Handle, enable);
        if (enable)
        {
            appPlc.state = APP_PLC_STATE_SLEEP;
        }
        
        return true;
    }
    
    return false;
}

/*******************************************************************************
 End of File
 */
