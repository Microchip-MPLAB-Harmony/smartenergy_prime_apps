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

//DOM-IGNORE-BEGIN
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
//DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "app.h"

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

APP_DATA appData;

static uint8_t plcDataPibBuffer[APP_PLC_PIB_BUFFER_SIZE];
static uint8_t plcSnifferDataBuffer[APP_PLC_SNIFFER_BUFFER_SIZE];

// *****************************************************************************
// *****************************************************************************
// Section: Function Declarations
// *****************************************************************************
// *****************************************************************************

static void _APP_TimerSyncInit();
static uint64_t _APP_PlcTimeToSysTime(uint32_t plcTime);
static uint32_t _APP_SysTimeToUS(uint64_t sysTime);

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

static void _APP_TimeExpired(uintptr_t context)
{
    *((bool *) context) = true;
}

static void _APP_PlcExceptionCb(DRV_PLC_PHY_EXCEPTION exception, uintptr_t ctxt)
{
    if (exception == DRV_PLC_PHY_EXCEPTION_RESET)
    {
        /* Initialize timer synchronization after reset */
        _APP_TimerSyncInit();
    }
}

static void _APP_PlcDataIndCb(DRV_PLC_PHY_RECEPTION_OBJ *indObj, uintptr_t ctxt)
{
    uint64_t rxSysTime;
    size_t length;

    /* Avoid warning */
    (void) ctxt;

    /* Start Timer: LED blinking for each received message */
    USER_PLC_IND_LED_On();
    SYS_TIME_TimerDestroy(appData.tmr2Handle);
    appData.tmr2Expired = false;
    appData.tmr2Handle = SYS_TIME_CallbackRegisterMS(_APP_TimeExpired,
            (uintptr_t) &appData.tmr2Expired, LED_BLINK_PLC_MSG_MS, SYS_TIME_SINGLE);

    /* Report RX Symbols */
    appData.plcPIB.id = PLC_ID_RX_PAY_SYMBOLS;
    appData.plcPIB.length = 2;
    DRV_PLC_PHY_PIBGet(appData.drvPlcHandle, &appData.plcPIB);
    SRV_PSNIFFER_SetRxPayloadSymbols(*(uint16_t *)appData.plcPIB.pData);

    /* Convert RX time to SYS_TIME units and convert to 32-bit US counter */
    rxSysTime = _APP_PlcTimeToSysTime(indObj->timeIni);
    indObj->timeIni = _APP_SysTimeToUS(rxSysTime);

    /* Serialize received PLC message */
    length = SRV_PSNIFFER_SerialRxMessage(plcSnifferDataBuffer, indObj);

    /* Send through USI */
    SRV_USI_Send_Message(appData.srvUSIHandle, SRV_USI_PROT_ID_SNIF_PRIME,
            plcSnifferDataBuffer, length);
}

static void _APP_RfRxIndCb(DRV_RF215_RX_INDICATION_OBJ* indObj, uintptr_t ctxt)
{
    DRV_RF215_PHY_CFG_OBJ rfPhyConfig;
    uint8_t* pRfSnifferData;
    size_t rfSnifferDataSize;
    uint16_t rfPayloadSymbols;
    uint16_t rfChannel;

    /* Avoid warning */
    (void) ctxt;

    /* Get payload symbols in the received message */
    DRV_RF215_GetPib(appData.drvRf215Handle, RF215_PIB_PHY_RX_PAY_SYMBOLS, &rfPayloadSymbols);

    /* Get RF PHY configuration */
    DRV_RF215_GetPib(appData.drvRf215Handle, RF215_PIB_PHY_CONFIG, &rfPhyConfig);
    DRV_RF215_GetPib(appData.drvRf215Handle, RF215_PIB_PHY_CHANNEL_NUM, &rfChannel);

    /* Serialize received RF message */
    pRfSnifferData = SRV_RSNIFFER_SerialRxMessage(indObj, &rfPhyConfig,
            rfPayloadSymbols, rfChannel, &rfSnifferDataSize);

    /* Send through USI */
    SRV_USI_Send_Message(appData.srvUSIHandle, SRV_USI_PROT_ID_SNIF_PRIME,
            pRfSnifferData, rfSnifferDataSize);
}

void _APP_UsiSnifferEventCb(uint8_t *pData, size_t length)
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

            if ((appData.plcChannel != channel) && (channel >= CHN1) && (channel <= CHN7_CHN8))
            {
                appData.plcChannel = channel;

                /* Set channel configuration */
                appData.plcPIB.id = PLC_ID_CHANNEL_CFG;
                appData.plcPIB.length = 1;
                *appData.plcPIB.pData = channel;
                DRV_PLC_PHY_PIBSet(appData.drvPlcHandle, &appData.plcPIB);

                /* Initialize timer synchronization with channel update */
                _APP_TimerSyncInit();

                /* Update channel in PSniffer */
                SRV_PSNIFFER_SetPLCChannel(channel);
            }

            break;
        }

        case SRV_RSNIFFER_CMD_SET_RF_BAND_OPM_CHANNEL:
        {
            uint16_t rfBandOpMode, rfChannel;

            /* Parse Band, Operating Mode and Channel parameters */
            SRV_RSNIFFER_ParseConfigCommand(pData, &rfBandOpMode, &rfChannel);

            /* Set configuration in RF PHY */
            DRV_RF215_SetPib(appData.drvRf215Handle, RF215_PIB_PHY_BAND_OPERATING_MODE, &rfBandOpMode);
            DRV_RF215_SetPib(appData.drvRf215Handle, RF215_PIB_PHY_CHANNEL_NUM, &rfChannel);
            break;
        }

        default:
            break;
    }
}

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************

static uint64_t _APP_TimerSyncRead(uint32_t* plcTime)
{
    uint64_t sysTime;

    /* Enter critical region to ensure constant delay between timers read */
    SYS_INT_Disable();

    /* Read current PLC driver and SYS_TIME timers */
    appData.plcPIB.id = PLC_ID_TIME_REF_ID;
    appData.plcPIB.length = 4;
    sysTime = SYS_TIME_Counter64Get();
    DRV_PLC_PHY_PIBGet(appData.drvPlcHandle, &appData.plcPIB);

    /* Leave critical region */
    SYS_INT_Enable();

    /* Compensate difference between timer reads */
    sysTime += SYS_TIME_USToCount(5);

    *plcTime = *((uint32_t*) appData.plcPIB.pData);
    return sysTime;
}

static void _APP_TimerSyncInit()
{
    /* Get initial timer references */
    appData.syncSysTimeRef = _APP_TimerSyncRead(&appData.syncPlcTimeRef);

    /* Initialize relative frequency F_SysTime/F_PLC [uQ1.24] */
    appData.syncTimerRelFreq = (uint32_t) (((uint64_t) SYS_TIME_FrequencyGet() << 24) / 1000000);

    /* Program first interrupt after 50 ms (5 us deviation with 100 PPM) */
    SYS_TIME_TimerDestroy(appData.tmrSyncHandle);
    appData.syncTimeDelay = 50;
    appData.tmrSyncHandle = SYS_TIME_CallbackRegisterMS(_APP_TimeExpired,
            (uintptr_t) &appData.tmrSyncExpired, appData.syncTimeDelay, SYS_TIME_SINGLE);

    if (appData.tmrSyncHandle == SYS_TIME_HANDLE_INVALID)
    {
        appData.syncTimeDelay = 0;
        appData.tmrSyncExpired = true;
    }
    else
    {
        appData.tmrSyncExpired = false;
    }
}

static inline void _APP_TimerSyncUpdate(void)
{
    uint64_t sysTime;
    uint32_t plcTime;
    uint64_t delaySysTime;
    uint32_t delayPlcTime;

    /* Get current SYS_TIME and PLC timers */
    sysTime = _APP_TimerSyncRead(&plcTime);

    /* Compute delays from reference (last read) */
    delaySysTime = sysTime - appData.syncSysTimeRef;
    delayPlcTime = plcTime - appData.syncPlcTimeRef;

    /* Compute relative frequency F_SysTime/F_PLC [uQ1.24] */
    appData.syncTimerRelFreq = (uint32_t) DIV_ROUND((uint64_t) delaySysTime << 24, delayPlcTime);

    /* Update reference for next synchronization */
    appData.syncSysTimeRef = sysTime;
    appData.syncPlcTimeRef = plcTime;

    if (appData.syncTimeDelay != 5000)
    {
        switch (appData.syncTimeDelay)
        {
            case 0:
                /* Next sync after 50 ms (5 us deviation with 100 PPM) */
                appData.syncTimeDelay = 50;
                break;

            case 50:
                /* Next sync after 250 ms (5 us deviation with 20 PPM) */
                appData.syncTimeDelay = 250;
                break;

            case 250:
                /* Next sync after 1 second (5 us deviation with 5 PPM) */
                appData.syncTimeDelay = 1000;
                break;

            default:
                /* Next sync after 5 seconds (5 us deviation with 1 PPM) */
                appData.syncTimeDelay = 5000;
                break;
        }
    }

    /* Program next interrupt */
    appData.tmrSyncHandle = SYS_TIME_CallbackRegisterMS(_APP_TimeExpired,
            (uintptr_t) &appData.tmrSyncExpired, appData.syncTimeDelay, SYS_TIME_SINGLE);

    if (appData.tmrSyncHandle == SYS_TIME_HANDLE_INVALID)
    {
        appData.syncTimeDelay = 0;
        appData.tmrSyncExpired = true;
    }
    else
    {
        appData.tmrSyncExpired = false;
    }
}

static uint64_t _APP_PlcTimeToSysTime(uint32_t plcTime)
{
    int64_t delayAux, delaySysTime;
    int32_t delayPlc;

    /* Compute PLC PHY delay time since last synchronization */
    delayPlc = (int32_t) (plcTime - appData.syncPlcTimeRef);

    /* Convert PLC PHY delay to Host delay (frequency deviation) */
    delayAux = (int64_t) delayPlc * appData.syncTimerRelFreq;
    delaySysTime = (delayAux + (1UL << 23)) >> 24;

    /* Compute SYS_TIME */
    return appData.syncSysTimeRef + delaySysTime;
}

static uint32_t _APP_SysTimeToUS(uint64_t sysTime)
{
    uint64_t sysTimeDiff;
    uint32_t sysTimeDiffNumHigh, sysTimeDiffRemaining;
    uint32_t timeUS = appData.plcSnifferPrevTimeUS;

    /* Difference between current and previous system time */
    sysTimeDiff = sysTime - appData.plcSnifferPrevSysTime;
    sysTimeDiffNumHigh = (uint32_t) (sysTimeDiff / 0x10000000);
    sysTimeDiffRemaining = (uint32_t) (sysTimeDiff % 0x10000000);

    /* Convert system time to microseconds and add to previous time */
    timeUS += (SYS_TIME_CountToUS(0x10000000) * sysTimeDiffNumHigh);
    timeUS += SYS_TIME_CountToUS(sysTimeDiffRemaining);

    /* Store times for next computation */
    appData.plcSnifferPrevSysTime = sysTime;
    appData.plcSnifferPrevTimeUS = timeUS;

    return timeUS;
}

// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_Initialize ( void )

  Remarks:
    See prototype in app.h.
 */

void APP_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    appData.state = APP_STATE_INIT;

    /* Initialize Timer handles */
    appData.tmr1Handle = SYS_TIME_HANDLE_INVALID;
    appData.tmr2Handle = SYS_TIME_HANDLE_INVALID;
    appData.tmrSyncHandle = SYS_TIME_HANDLE_INVALID;
    appData.tmr1Expired = false;
    appData.tmr2Expired = false;

    /* Initialize driver handles */
    appData.drvRf215Handle = DRV_HANDLE_INVALID;
    appData.drvPlcHandle = DRV_HANDLE_INVALID;
    appData.srvUSIHandle = DRV_HANDLE_INVALID;

    /* Initialize PLC objects */
    appData.plcPIB.pData = plcDataPibBuffer;

    /* Init Channel */
    appData.plcChannel = CHN1;
}

/******************************************************************************
  Function:
    void APP_Tasks ( void )

  Remarks:
    See prototype in app.h.
 */

void APP_Tasks ( void )
{
    /* Update Watchdog */
    CLEAR_WATCHDOG();

    /* LED signaling */
    if (appData.tmr1Expired == true)
    {
        appData.tmr1Expired = false;
        USER_BLINK_LED_Toggle();
    }

    if (appData.tmr2Expired == true)
    {
        appData.tmr2Expired = false;
        USER_PLC_IND_LED_Off();
    }

    /* Check update synchronization between SYS_TIME and PLC timers */
    if (appData.tmrSyncExpired == true)
    {
        _APP_TimerSyncUpdate();
    }

    /* Check the application's current state. */
    switch(appData.state)
    {
        /* Application's initial state. */
        case APP_STATE_INIT:
        {
            /* Open PLC driver: Start uploading process */
            appData.drvPlcHandle = DRV_PLC_PHY_Open(DRV_PLC_PHY_INDEX, NULL);

            if (appData.drvPlcHandle != DRV_HANDLE_INVALID)
            {
                /* Set Application to next state */
                appData.state = APP_STATE_REGISTER_PLC;
            }
            else
            {
                /* PLC driver unavailable: Try to open RF215 driver */
                appData.state = APP_STATE_REGISTER_RF;
            }
            break;
        }

        /* Waiting to PLC driver be opened and register callback functions */
        case APP_STATE_REGISTER_PLC:
        {
            /* Check PLC transceiver */
            SYS_STATUS plcStatus = DRV_PLC_PHY_Status(DRV_PLC_PHY_INDEX);
            if (plcStatus == SYS_STATUS_READY)
            {
                /* Register PLC driver callback */
                DRV_PLC_PHY_DataIndCallbackRegister(appData.drvPlcHandle,
                        _APP_PlcDataIndCb, DRV_PLC_PHY_INDEX);
                DRV_PLC_PHY_ExceptionCallbackRegister(appData.drvPlcHandle,
                        _APP_PlcExceptionCb, DRV_PLC_PHY_INDEX);

                /* Set channel configuration */
                appData.plcPIB.id = PLC_ID_CHANNEL_CFG;
                appData.plcPIB.length = 1;
                *appData.plcPIB.pData = appData.plcChannel;
                DRV_PLC_PHY_PIBSet(appData.drvPlcHandle, &appData.plcPIB);

                /* Initialize timer synchronization with channel update */
                _APP_TimerSyncInit();

                /* Update channel in PSniffer */
                SRV_PSNIFFER_SetPLCChannel(appData.plcChannel);

                /* PLC driver opened successfully. Try to open RF215 driver. */
                appData.state = APP_STATE_REGISTER_RF;
            }
            else if (plcStatus == SYS_STATUS_ERROR)
            {
                /* PLC driver unavailable. Try to open RF215 driver. */
                appData.drvPlcHandle = DRV_HANDLE_INVALID;
                appData.state = APP_STATE_REGISTER_RF;
            }

            break;
        }

        /* Waiting to RF215 driver be opened and register callback functions */
        case APP_STATE_REGISTER_RF:
        {
            /* Check status of RF215 driver */
            SYS_STATUS rf215Status = DRV_RF215_Status(sysObj.drvRf215);
            if (rf215Status == SYS_STATUS_READY)
            {
                /* RF215 driver is ready to be opened */
                appData.drvRf215Handle = DRV_RF215_Open(DRV_RF215_INDEX_0, RF215_TRX_ID_RF09);
                if (appData.drvRf215Handle != DRV_HANDLE_INVALID)
                {
                    /* Register RF215 driver callback */
                    DRV_RF215_RxIndCallbackRegister(appData.drvRf215Handle, _APP_RfRxIndCb, 0);
                }
            }

            if ((rf215Status == SYS_STATUS_READY) || (rf215Status == SYS_STATUS_ERROR))
            {
                if ((appData.drvPlcHandle == DRV_HANDLE_INVALID) &&
                        (appData.drvRf215Handle == DRV_HANDLE_INVALID))
                {
                    /* Set Application to ERROR state */
                    appData.state = APP_STATE_ERROR;
                }
                else
                {
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
            }

            break;
        }

        case APP_STATE_CONFIG_USI:
        {
            if (SRV_USI_Status(appData.srvUSIHandle) == SRV_USI_STATUS_CONFIGURED)
            {
                /* Register USI callback */
                SRV_USI_CallbackRegister(appData.srvUSIHandle,
                        SRV_USI_PROT_ID_SNIF_PRIME, _APP_UsiSnifferEventCb);

                if (appData.tmr1Handle == SYS_TIME_HANDLE_INVALID)
                {
                    /* Register Timer Callback */
                    appData.tmr1Handle = SYS_TIME_CallbackRegisterMS(
                            _APP_TimeExpired, (uintptr_t) &appData.tmr1Expired,
                            LED_BLINK_RATE_MS, SYS_TIME_PERIODIC);
                }
                else
                {
                    SYS_TIME_TimerStart(appData.tmr1Handle);
                }

                /* Set Application to next state */
                appData.state = APP_STATE_READY;
            }
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
            /* TODO: Handle error in application's state machine. */
            break;
        }
    }
}


/*******************************************************************************
 End of File
 */
