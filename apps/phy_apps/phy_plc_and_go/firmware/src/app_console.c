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
    app_console.c

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
#include <stdarg.h>
#include <math.h>
#include "definitions.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************
#define CTRL_S_KEY         0x13
#define BACKSPACE_KEY      0x08
#define DELETE_KEY         0x7F

#define DIV_ROUND(a, b)    (((a) + (b >> 1)) / (b))

static va_list sArgs = {0};

/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_Initialize function.

    Application strings and buffers are be defined outside this structure.
*/

APP_CONSOLE_DATA appConsole;

static CACHE_ALIGN char pTransmitBuffer[CACHE_ALIGNED_SIZE_GET(SERIAL_BUFFER_SIZE)];
static CACHE_ALIGN char pReceivedBuffer[CACHE_ALIGNED_SIZE_GET(SERIAL_BUFFER_SIZE)];

const char * channelDescription[] = {
    "",
    "Channel 1 (42 - 89 kHz, CENELEC-A)",
    "Channel 2 (97 - 144 kHz, CENELEC-BCD)",
    "Channel 3 (151 - 198 kHz, FCC)",
    "Channel 4 (206 - 253 kHz, FCC)",
    "Channel 5 (261 - 308 kHz, FCC)",
    "Channel 6 (315 - 362 kHz, FCC)",
    "Channel 7 (370 - 417 kHz, FCC)",
    "Channel 8 (425 - 472 kHz, FCC)"
};

const char * schemeDescription[] = {
    "Robust DBPSK ................. 5.2 kbit/s",
    "Robust DQPSK ................. 10.2 kbit/s",
    "DBPSK + Convolutional Code ... 20.8 kbit/s",
    "DQPSK + Convolutional Code ... 40.6 kbit/s",
    "D8PSK + Convolutional Code ... 58.9 kbit/s",
    "DBPSK ........................ 40.6 kbit/s",
    "DQPSK ........................ 76.3 kbit/s",
    "D8PSK ........................ 107.9 kbit/s"
};

const DRV_PLC_PHY_SCH schemeList[] = {
  SCHEME_R_DBPSK,
  SCHEME_R_DQPSK,
  SCHEME_DBPSK_C,
  SCHEME_DQPSK_C,
  SCHEME_D8PSK_C,
  SCHEME_DBPSK,
  SCHEME_DQPSK,
  SCHEME_D8PSK
};

const char * schemeNames[] = {
  "DBPSK",
  "DQPSK",
  "D8PSK",
  0,
  "DBPSK_C",
  "DQPSK_C",
  "D8PSK_C",
  0,
  0,
  0,
  0,
  0,
  "R_DBPSK",
  "R_DQPSK"
};

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************


// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************

static bool APP_CONSOLE_CheckIsPrintable(char data)
{
    if (((data >= 32) && (data <= 126)) ||
        ((data >= 128) && (data <= 254)) ||
         (data == '\t') || (data == '\n') || (data == '\r'))
    {
        return 1;
    }

    return 0;
}

static void APP_CONSOLE_ReadRestart( uint16_t numCharPending )
{
    appConsole.pNextChar = appConsole.pReceivedChar;
    appConsole.dataLength = 0;
    appConsole.numCharToReceive = numCharPending;
}

static uint8_t APP_CONSOLE_ReadSerialChar( void )
{
    if (appConsole.numCharToReceive > 0)
    {
        if (SYS_CONSOLE_Read(SYS_CONSOLE_INDEX_0, (void*)appConsole.pNextChar, 1) > 0)
        {
            /* Success */
            if ((*(appConsole.pNextChar) == '\r') || (*(appConsole.pNextChar) == '\n'))
            {
                appConsole.numCharToReceive = 0;
                return appConsole.dataLength;
            }
            
            if (*(appConsole.pNextChar) == CTRL_S_KEY)
            {
                *appConsole.pReceivedChar = CTRL_S_KEY;
                appConsole.numCharToReceive = 0;
                appConsole.dataLength = 1;
                return appConsole.dataLength;
            }
            
            if ((*(appConsole.pNextChar) == BACKSPACE_KEY) || (*(appConsole.pNextChar) == DELETE_KEY))
            {
                /* Remove character from data buffer */
                if (appConsole.dataLength > 0)
                {
                    APP_CONSOLE_Print("\b \b");
                    appConsole.dataLength--;
                    appConsole.pNextChar--;
                    return appConsole.dataLength;
                }
            }

            if (APP_CONSOLE_CheckIsPrintable(*appConsole.pNextChar))
            {
                if (appConsole.echoEnable)
                {
                    SYS_CONSOLE_Write(SYS_CONSOLE_INDEX_0, appConsole.pNextChar, 1);
                }

                appConsole.dataLength++;
                appConsole.pNextChar++;
                appConsole.numCharToReceive--;
            }
        }
    }
    
    return appConsole.dataLength;
}

static uint8_t APP_CONSOLE_SetScheme(char scheme)
{
    uint8_t schemeIndex;
    
    schemeIndex = scheme - 0x30;
    
    if (schemeIndex >= 8)
    {
        return 0xFF;
    }
    
    APP_PLC_SetModScheme(schemeList[schemeIndex]);

    return schemeIndex;
}

static void APP_CONSOLE_ShowSetSchemeMenu( void )
{
    uint8_t index;
    
    APP_CONSOLE_Print("\r\n--- Tx Modulation Configuration Menu ---\r\n");
    APP_CONSOLE_Print("Select Modulation:\r\n");
    
    for (index = 0; index < 8; index++)
    {
        if (schemeList[index] == appPlcTx.plcPhyTx.scheme)
        {
            APP_CONSOLE_Print("->\t");
        }
        else
        {
            APP_CONSOLE_Print("\t");
        }
        
        APP_CONSOLE_Print("%u: %s\r\n", index, schemeDescription[index]);
        
    }
    
    APP_CONSOLE_Print(MENU_CMD_PROMPT);
}

static void APP_CONSOLE_ShowSetSleepMenu( void )
{
    APP_CONSOLE_Print("\r\n--- Enable/Disable Sleep Configuration Menu ---\r\n");
    APP_CONSOLE_Print("Select Sleep Mode:\r\n");
    
    if (appPlc.state != APP_PLC_STATE_SLEEP)
    {
        APP_CONSOLE_Print("->");
    }
    APP_CONSOLE_Print("\t0: Sleep mode off\r\n");

    if (appPlc.state == APP_PLC_STATE_SLEEP)
    {
        APP_CONSOLE_Print("->");
    }
    APP_CONSOLE_Print("\t1: Sleep mode on\r\n");
    APP_CONSOLE_Print(MENU_CMD_PROMPT);
}

static uint8_t APP_CONSOLE_SetChannel(char channel)
{
    DRV_PLC_PHY_CHANNEL chn;
    
    chn = (DRV_PLC_PHY_CHANNEL)(channel - 0x30);
    
    if ((appPlcTx.channel != chn) && (chn >= CHN1) && (chn <= CHN8))
    {
        APP_PLC_SetChannel(chn);
        return (uint8_t)chn;
    }

    return 0xFF;
}

static void APP_CONSOLE_ShowSetChannelMenu( void )
{
    DRV_PLC_PHY_CHANNEL currentChannel;
    uint8_t index;
    
    APP_CONSOLE_Print("\r\n--- Tx/Rx Channel Configuration Menu ---\r\n");
    APP_CONSOLE_Print("Select channel:\r\n");
    
    currentChannel = appPlcTx.channel;
    
    for (index = 1; index <= 8; index++)
    {       
        if (index == currentChannel)
        {
            APP_CONSOLE_Print("->\t");
        }
        else
        {
            APP_CONSOLE_Print("\t");
        }
        
        APP_CONSOLE_Print("%u: %s\r\n", index, channelDescription[index]);
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
    void APP_CONSOLE_Initialize ( void )

  Remarks:
    See prototype in app_console.h.
 */

void APP_CONSOLE_Initialize ( void )
{
    /* Update state machine */
    appConsole.state = APP_CONSOLE_STATE_INIT;

    /* Init Reception data */
    appConsole.pTransmitChar = pTransmitBuffer;
    appConsole.pReceivedChar = pReceivedBuffer;
    appConsole.pNextChar = pReceivedBuffer;
    appConsole.dataLength = 0;
    appConsole.numCharToReceive = 0;
    
    /* Set ECHO ON by default */
    appConsole.echoEnable = true;

}

/******************************************************************************
  Function:
    void APP_CONSOLE_Tasks ( void )

  Remarks:
    See prototype in app_console.h.
 */

void APP_CONSOLE_Tasks ( void )
{
    /* Refresh WDG */
    CLEAR_WATCHDOG();
    
    /* Read console port */
    APP_CONSOLE_ReadSerialChar();

    /* Check the application's current state. */
    switch ( appConsole.state )
    {
        /* Application's initial state. */
        case APP_CONSOLE_STATE_INIT:
        {
            char data;
            SYS_STATUS ret;
            
            ret = SYS_CONSOLE_Status(SYS_CONSOLE_INDEX_0);
            
            if (ret == SYS_STATUS_READY)
            {
                /* Wait for any serial data to start Console application */
                if (SYS_CONSOLE_Read(SYS_CONSOLE_INDEX_0, &data, 1) > 0)
                {
                    appConsole.state = APP_CONSOLE_STATE_WAIT_PLC;
                }
            }
            break;
        }

        case APP_CONSOLE_STATE_WAIT_PLC:
        {
            /* Wait for PLC transceiver initialization */
            if (appPlc.state == APP_PLC_STATE_WAITING)
            {
                /* Show App Header */
                APP_CONSOLE_Print(STRING_HEADER);
            
                /* Show PLC PHY version */
                APP_CONSOLE_Print("PLC PHY binary loaded correctly\r\nPHY version: %02x.%02x.%02x.%02x", 
                        (uint8_t)(appPlcTx.plcPhyVersion >> 24), (uint8_t)(appPlcTx.plcPhyVersion >> 16),
                        (uint8_t)(appPlcTx.plcPhyVersion >> 8), (uint8_t)(appPlcTx.plcPhyVersion));
                
                /* Show PLC TX scheme */
                
                APP_CONSOLE_Print("\r\nPress 'CTRL+S' to enter configuration menu. " \
                    "Enter text and press 'ENTER' to trigger transmission\r\n>>> ");
                
                /* Set Console state */
                appConsole.state = APP_CONSOLE_STATE_SHOW_PROMPT;
            }
            break;
        }

        case APP_CONSOLE_STATE_SHOW_PROMPT:
        {
            /* Show console interface */
            APP_CONSOLE_Print(MENU_CMD_PROMPT);

            /* Waiting Console command */
            APP_CONSOLE_ReadRestart(SERIAL_BUFFER_SIZE);
            appConsole.state = APP_CONSOLE_STATE_CONSOLE;
            

            break;
        }

        case APP_CONSOLE_STATE_CONSOLE:
        {
            if (appConsole.numCharToReceive == 0)
            {
                switch(*appConsole.pReceivedChar)
                {
                    case CTRL_S_KEY:
                        appConsole.state = APP_CONSOLE_STATE_MENU;
                        APP_CONSOLE_ReadRestart(1);
                        APP_CONSOLE_Print("\n\r-- Configuration Menu --------------\r\n");
                        APP_CONSOLE_Print("Select parameter to configure:\r\n");
                        APP_CONSOLE_Print("\t0: Enable/Disable sleep mode\n\r");
                        APP_CONSOLE_Print("\t1: Tx Modulation\n\r");
                        APP_CONSOLE_Print("\t2: Tx/Rx Channel\n\r");
                        break;
                        
                    default:
                        if (appConsole.dataLength)
                        {
                            /* Transmit PLC message */
                            if (APP_PLC_SendData((uint8_t *)appConsole.pReceivedChar, appConsole.dataLength))
                            {
                                appConsole.state = APP_CONSOLE_STATE_WAIT_PLC_TX_CFM;
                            }
                            else
                            {
                                if (appPlc.pvddMonTxEnable == false)
                                {
                                    APP_CONSOLE_Print("\n\rTransmission is not available. PVDD Monitor out of thresholds.\r\n");
                                }
                                else
                                {
                                    APP_CONSOLE_Print("\n\rTransmission is not available in Sleep Mode\r\n");
                                }
                                appConsole.state = APP_CONSOLE_STATE_SHOW_PROMPT;
                            }
                        }
                        else
                        {
                            appConsole.state = APP_CONSOLE_STATE_SHOW_PROMPT;
                        }
                }
            }
        }
        break;
        
        case APP_CONSOLE_STATE_MENU:
        {
            if (appConsole.numCharToReceive == 0)
            {
                if (appConsole.pReceivedChar[0] == '0')
                {
                    appConsole.state = APP_CONSOLE_STATE_SET_SLEEP;
                    APP_CONSOLE_ShowSetSleepMenu();
                    APP_CONSOLE_ReadRestart(1);
                }
                else if (appConsole.pReceivedChar[0] == '1')
                {
                    appConsole.state = APP_CONSOLE_STATE_SET_SCHEME;
                    APP_CONSOLE_ShowSetSchemeMenu();
                    APP_CONSOLE_ReadRestart(1);
                }
                else if (appConsole.pReceivedChar[0] == '2')
                {
                    appConsole.state = APP_CONSOLE_STATE_SET_PLC_CHANNEL;
                    APP_CONSOLE_ShowSetChannelMenu();
                    APP_CONSOLE_ReadRestart(1);
                }
                else
                {
                    /* Try it again */
                    APP_CONSOLE_Print("\r\nERROR: Menu option is not permitted. Try again : ");
                    APP_CONSOLE_ReadRestart(1);
                }
            }
        }
        break;
            

        case APP_CONSOLE_STATE_WAIT_PLC_TX_CFM:
        {
            if (appPlc.plcTxState == APP_PLC_TX_STATE_IDLE)
            {
                APP_CONSOLE_Print("\r\nTx (%u bytes): ", appConsole.dataLength);
                switch(appPlc.lastTxResult)
                {
                    case DRV_PLC_PHY_TX_RESULT_PROCESS:
                        APP_CONSOLE_Print("  TX_RESULT_PROCESS");
                        break;
                    case DRV_PLC_PHY_TX_RESULT_SUCCESS:
                        APP_CONSOLE_Print("  TX_RESULT_SUCCESS");
                        break;
                    case DRV_PLC_PHY_TX_RESULT_INV_LENGTH:
                        APP_CONSOLE_Print("  TX_RESULT_INV_LENGTH");
                        break;
                    case DRV_PLC_PHY_TX_RESULT_BUSY_CH:
                        APP_CONSOLE_Print("  TX_RESULT_BUSY_CH");
                        break;
                    case DRV_PLC_PHY_TX_RESULT_BUSY_TX:
                        APP_CONSOLE_Print("  TX_RESULT_BUSY_TX");
                        break;
                    case DRV_PLC_PHY_TX_RESULT_BUSY_RX:
                        APP_CONSOLE_Print("  TX_RESULT_BUSY_RX");
                        break;
                    case DRV_PLC_PHY_TX_RESULT_INV_SCHEME:
                        APP_CONSOLE_Print("  TX_RESULT_INV_SCHEME");
                        break;
                    case DRV_PLC_PHY_TX_RESULT_TIMEOUT:
                        APP_CONSOLE_Print("  TX_RESULT_TIMEOUT");
                        break;
                    case DRV_PLC_PHY_TX_RESULT_INV_BUFFER:
                        APP_CONSOLE_Print("  TX_RESULT_INV_BUFFER");
                        break;
                    case DRV_PLC_PHY_TX_RESULT_INV_MODE:
                        APP_CONSOLE_Print("  TX_RESULT_INV_MODE");
                        break;
                    case DRV_PLC_PHY_TX_RESULT_INV_TX_MODE:
                        APP_CONSOLE_Print("  TX_RESULT_INV_TX_MODE");
                        break;
                    case DRV_PLC_PHY_TX_RESULT_CANCELLED:
                        APP_CONSOLE_Print("  TX_RESULT_CANCELLED");
                        break;
                    case DRV_PLC_PHY_TX_RESULT_HIGH_TEMP_120:
                        APP_CONSOLE_Print("  TX_RESULT_HIGH_TEMP_120");
                        break;
                    case DRV_PLC_PHY_TX_RESULT_HIGH_TEMP_110:
                        APP_CONSOLE_Print("  TX_RESULT_HIGH_TEMP_110");
                        break;
                    case DRV_PLC_PHY_TX_RESULT_NO_TX:
                        APP_CONSOLE_Print("  TX_RESULT_NO_TX");
                        break;
                }
                
                appConsole.state = APP_CONSOLE_STATE_SHOW_PROMPT;
            }
        }
        break;

        case APP_CONSOLE_STATE_SET_SLEEP:
        {
            if (appConsole.numCharToReceive == 0)
            {
                if (appConsole.pReceivedChar[0] == '1')
                {
                    if (APP_PLC_SetSleepMode(true))
                    {
                        APP_CONSOLE_Print("\r\nPLC Sleep mode enabled\r\n");
                    }
                    else
                    {
                        APP_CONSOLE_Print("\r\nPLC Sleep mode is already enabled\r\n");
                    }
                }
                else
                {
                    if (APP_PLC_SetSleepMode(false))
                    {
                        APP_CONSOLE_Print("\r\nPLC Sleep mode disabled\r\n");
                    }
                    else
                    {
                        APP_CONSOLE_Print("\r\nPLC Sleep mode is already disabled\r\n");
                    }
                }
                appConsole.state = APP_CONSOLE_STATE_SHOW_PROMPT;
            }
            break;
        }

        case APP_CONSOLE_STATE_SET_SCHEME:
        {
            if (appConsole.numCharToReceive == 0)
            {
                uint8_t schemeIndex;
                
                schemeIndex = APP_CONSOLE_SetScheme(appConsole.pReceivedChar[0]);
                if (schemeIndex != 0xFF)
                {
                    APP_CONSOLE_Print("\r\nTx Modulation: %.30s Max data length = %u\r\n", 
                            schemeDescription[schemeIndex], appPlcTx.maxPsduLen);
                    appConsole.state = APP_CONSOLE_STATE_SHOW_PROMPT;
                }
                else
                {
                    /* Try it again */
                    APP_CONSOLE_Print("\r\nERROR: Scheme not permitted. Try again : ");
                    APP_CONSOLE_ReadRestart(1);
                }
            }
            break;
        }
        
        case APP_CONSOLE_STATE_SET_PLC_CHANNEL:
        {
            if (appConsole.numCharToReceive == 0)
            {
                uint8_t chn;
                
                chn = APP_CONSOLE_SetChannel(appConsole.pReceivedChar[0]);
                if (chn != 0xFF)
                {
                    APP_CONSOLE_Print("\r\n%s\r\n", channelDescription[chn]);

                    appConsole.state = APP_CONSOLE_STATE_SHOW_PROMPT;
                }
                else
                {
                    APP_CONSOLE_Print("\r\nPLC Channel has not changed\r\n");
                    appConsole.state = APP_CONSOLE_STATE_SHOW_PROMPT;
                }
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

void APP_CONSOLE_Print(const char *format, ...)
{
    size_t len = 0;
    uint32_t numRetries = 10000;
    
    if (appConsole.state == APP_CONSOLE_STATE_INIT)
    {
        return;
    }

    while(SYS_CONSOLE_WriteCountGet(SYS_CONSOLE_INDEX_0))
    {
        if (numRetries--)
        {
            /* Maintain Console service */
            SYS_CONSOLE_Tasks(SYS_CONSOLE_INDEX_0);

            /* Refresh WDG */
            CLEAR_WATCHDOG();
        }
        else
        {
            return;
        }
    }

    va_start( sArgs, format );
    len = vsnprintf(appConsole.pTransmitChar, SERIAL_BUFFER_SIZE - 1, format, sArgs);
    va_end( sArgs );
    
    if (len > SERIAL_BUFFER_SIZE - 1)
    {
        len = SERIAL_BUFFER_SIZE - 1;
    }
    
    appConsole.pTransmitChar[len] = '\0';
    SYS_CONSOLE_Message(SYS_CONSOLE_INDEX_0, (const char *) appConsole.pTransmitChar);
}

void APP_CONSOLE_ShowMessage( DRV_PLC_PHY_RECEPTION_OBJ *plcRxObj )
{
    APP_CONSOLE_Print("\rRx (%s, RSSI %udBuV,  CINR %ddB): %.*s", 
            schemeNames[plcRxObj->scheme], 
            plcRxObj->rssiAvg, 
            DIV_ROUND((int16_t)plcRxObj->cinrAvg - 40, 4),
            plcRxObj->dataLength - 1, plcRxObj->pReceivedData + 1);
}


/*******************************************************************************
 End of File
 */
