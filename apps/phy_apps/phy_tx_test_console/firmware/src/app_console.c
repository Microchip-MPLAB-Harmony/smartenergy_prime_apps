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

CACHE_ALIGN APP_CONSOLE_DATA appConsole;

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

const char * schemeRatesDescription[] = {
    "Robust DBPSK ................. 5.2 kbit/s",
    "Robust DQPSK ................. 10.2 kbit/s",
    "DBPSK + Convolutional Code ... 20.8 kbit/s",
    "DQPSK + Convolutional Code ... 40.6 kbit/s",
    "D8PSK + Convolutional Code ... 58.9 kbit/s",
    "DBPSK ........................ 40.6 kbit/s",
    "DQPSK ........................ 76.3 kbit/s",
    "D8PSK ........................ 107.9 kbit/s"
};

const char * frameDescription[] = {
  "MODE_TYPE_A",
  0,
  "MODE_TYPE_B",
  "MODE_TYPE_BC"
};

const char * schemeDescription[] = {
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
         (data == '\t') || (data == '\n'))
    {
        return 1;
    }

    return 0;
}

static void APP_CONSOLE_ReadRestart( uint8_t numCharPending )
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

static bool APP_CONSOLE_CharToHex(char value, uint8_t *hex)
{
    if ((value >= '0') && (value <= '9'))
    {
        *hex = value - 0x30;
    }
    else if ((value >= 'A') && (value <= 'F'))
    {
        *hex = value - 0x37;
    }
    else if ((value >= 'a') && (value <= 'f'))
    {
        *hex = value - 0x57;
    }
    else
    {
        return 0;
    }

    return 1;
}

static bool APP_CONSOLE_SetTxBuffer(char *buffer)
{
	switch (*buffer)
    {
		case '0':
            appPlcTx.plcPhyTx.bufferId = TX_BUFFER_0;
            break;
            
        case '1':
            appPlcTx.plcPhyTx.bufferId = TX_BUFFER_1;
            break;
            
        default:
            return false;
    }

    return true;
}

static bool APP_CONSOLE_SetAttenuationLevel(char *level)
{
	uint8_t attLevel;
    uint8_t attLevelHex;

    if (APP_CONSOLE_CharToHex(*level++, &attLevelHex))
    {
        attLevel = attLevelHex << 4;

        attLevelHex = *level;
        if (APP_CONSOLE_CharToHex(*level, &attLevelHex))
        {
            attLevel += attLevelHex;

            if ((attLevel <= 0x1F) || (attLevel == 0xFF)) {
                appPlcTx.plcPhyTx.attenuation = attLevel;
                return true;
            }
        }
    }

    return false;
}

static uint8_t APP_CONSOLE_SetScheme(char *scheme)
{
    DRV_PLC_PHY_SCH sch;
    uint8_t index;
    
    switch (*scheme)
    {
		case '0':
            sch = SCHEME_DBPSK;
            index = 5;
            break;
            
        case '1':
            sch = SCHEME_DQPSK;
            index = 6;
            break;
            
        case '2':
            sch = SCHEME_D8PSK;
            index = 7;
            break;
            
		case '3':
            sch = SCHEME_DBPSK_C;
            index = 2;
            break;
            
        case '4':
            sch = SCHEME_DQPSK_C;
            index = 3;
            break;
            
        case '5':
            sch = SCHEME_D8PSK_C;
            index = 4;
            break;
            
        case '6':
            sch = SCHEME_R_DBPSK;
            index = 0;
            break;
            
        case '7':
            sch = SCHEME_R_DQPSK;
            index = 1;
            break;
            
        default:
            return 0xFF;        
    }
    
    APP_PLC_SetModScheme(sch);

    return index;
}

static uint8_t APP_CONSOLE_SetDisableRX(char *disable)
{
    appPlcTx.plcPhyTx.csma.senseCount = 0;
    appPlcTx.plcPhyTx.csma.senseDelayMs = 0;

    switch (*disable)
    {
		case 'Y':
        case 'y':
            appPlcTx.plcPhyTx.csma.disableRx = 1;

            break;
            
        case 'N':
        case 'n':
            appPlcTx.plcPhyTx.csma.disableRx = 0;
            break;
            
        default:
            return false;
    }
    
    return true;
}

static uint8_t APP_CONSOLE_SetFrameType(char *mode)
{
    switch (*mode)
    {
		case '0':
            appPlcTx.plcPhyTx.frameType = FRAME_TYPE_A;
            break;
            
        case '2':
            appPlcTx.plcPhyTx.frameType = FRAME_TYPE_B;
            break;
            
        case '3':
            appPlcTx.plcPhyTx.frameType = FRAME_TYPE_BC;
            break;
            
        default:
            return false;
    }
    
    return true;
}

static bool APP_CONSOLE_SetTransmissionPeriod(char *pTime, size_t length)
{
    uint8_t index;
    uint8_t tmpValue;
    bool result = false;

    appPlcTx.plcPhyTx.timeIni = 0;

    for(index = length - 1; index > 0; index--)
    {
        if ((*pTime >= '0') && (*pTime <= '9')) {
				tmpValue = (*pTime - 0x30);
                appPlcTx.plcPhyTx.timeIni += (uint32_t)pow(10, index) * tmpValue;
                pTime++;

                result = true;
        }
        else
        {
            result = false;
            break;
        }
    }

    return result;
}

static bool APP_CONSOLE_SetDataLength(char *pDataLength, size_t length)
{
    uint16_t dataLength = 0;
    uint8_t index;
    uint8_t tmpValue;
    bool result = false;

    appPlcTx.plcPhyTx.dataLength = 0;

    for (index = length; index > 0; index--)
    {
        if ((*pDataLength >= '0') && (*pDataLength <= '9')) {
				tmpValue = (*pDataLength - 0x30);
                dataLength += (uint32_t)pow(10, index - 1) * tmpValue;
                pDataLength++;
                result = true;
        }
        else
        {
            result = false;
            break;
        }
    }
    
    if (result & (dataLength < APP_PLC_BUFFER_SIZE))
    {
        appPlcTx.plcPhyTx.dataLength = dataLength;
    }

    return result;
}

static bool APP_CONSOLE_SetDataMode(char *mode)
{
    size_t length;
    uint8_t* pData;
    uint32_t dataValue;
    bool result = true;

    length = appPlcTx.plcPhyTx.dataLength;
    pData = appPlcTx.plcPhyTx.pTransmitData;

	switch (*mode)
    {
		case '0':
            /* Random mode */
            length >>= 2;
            length++;
            while(length--)
            {
                dataValue = TRNG_ReadData();
                *pData++ = (uint8_t)dataValue;
                *pData++ = (uint8_t)(dataValue >> 8);
                *pData++ = (uint8_t)(dataValue >> 16);
                *pData++ = (uint8_t)(dataValue >> 24);
            }
			break;

		case '1':
            /* Fixed mode */
            dataValue = 0x30;
			while(length--)
            {
                *pData++ = (uint8_t)dataValue++;
                if (dataValue == 0x3A) {
                    dataValue = 0x30;
                }
            }
			break;

		default:
			result = false;
	}

    return result;
}

static bool APP_CONSOLE_SetChannel(char *channel)
{
    DRV_PLC_PHY_CHANNEL chn;
    SRV_PLC_PCOUP_CHANNEL_DATA *chnData;
    
    chn = (DRV_PLC_PHY_CHANNEL)(*channel - 0x30);
    chnData = SRV_PCOUP_GetChannelConfig(chn);
    
    /* Check if channel is available via MCC configuration */
    if (chnData != NULL)
    {
        APP_PLC_SetChannel(chn);
        return true;
    }
    
    return false;
}

static bool APP_CONSOLE_SetAutodetect(char *buffer)
{
	switch (*buffer)
    {
		case '0':
            appPlcTx.txAuto = 1;
            break;
            
        case '1':
            appPlcTx.txAuto = 0;
            appPlcTx.txImpedance = HI_STATE;
            break;
            
        case '2':
            appPlcTx.txAuto = 0;
            appPlcTx.txImpedance = LOW_STATE;
            break;
            
        case '3':
            appPlcTx.txAuto = 0;
            appPlcTx.txImpedance = VLO_STATE;
            break;
            
        default:
            return false;        
    }

    return true;
}

static void APP_CONSOLE_ShowConfiguration(void)
{
    APP_CONSOLE_Print("\n\r-- Configuration Info --------------\r\n");
    APP_CONSOLE_Print("-I- PHY Version: 0x%08X\n\r", (unsigned int)appPlcTx.plcPhyVersion);

    if (appPlcTx.plcPhyTx.attenuation == 0xFF)
    {
        APP_CONSOLE_Print("-I- TX Attenuation: 0xFF (no signal)\n\r");
    }
    else
    {
        APP_CONSOLE_Print("-I- TX Attenuation: 0x%02X\n\r", (unsigned int)appPlcTx.plcPhyTx.attenuation);
    }
    
    APP_CONSOLE_Print("-I- Modulation Scheme: %s\n\r", schemeDescription[appPlcTx.plcPhyTx.scheme]);
    APP_CONSOLE_Print("-I- Disable RX: %u\n\r", appPlcTx.plcPhyTx.csma.disableRx);
    APP_CONSOLE_Print("-I- PRIME mode: %s\n\r", frameDescription[appPlcTx.plcPhyTx.frameType]);
    APP_CONSOLE_Print("-I- Time Period: %u\n\r", (unsigned int)appPlcTx.plcPhyTx.timeIni);
	APP_CONSOLE_Print("-I- Data Len: %u\n\r", (unsigned int)appPlcTx.plcPhyTx.dataLength);
    if (appPlcTx.plcPhyTx.pTransmitData[0] == 0x30)
    {
		APP_CONSOLE_Print("-I- Fixed Data\r\n");
	}
    else
    {
		APP_CONSOLE_Print("-I- Random Data\r\n");
	}
    
    if (appPlcTx.txAuto)
    {
		APP_CONSOLE_Print("-I- Branch Mode : Autodetect - ");
	}
    else
    {
		APP_CONSOLE_Print("-I- Branch Mode : Fixed - ");
	}

	if (appPlcTx.txImpedance == HI_STATE)
    {
		APP_CONSOLE_Print("High Impedance \r\n");
	}
    else
    {
		APP_CONSOLE_Print("Very Low Impedance \r\n");
	}

	APP_CONSOLE_Print("-I- PRIME channel: %s\n\r", channelDescription[appPlcTx.channel]);
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
    /* Place the App state machine in its initial state. */
    appConsole.state = APP_CONSOLE_STATE_IDLE;

    /* Init Timer handler */
    appConsole.tmr1Handle = SYS_TIME_HANDLE_INVALID;

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
            appConsole.state = APP_CONSOLE_STATE_WAIT_PLC;

            /* Show App Header */
            APP_CONSOLE_Print(STRING_HEADER);

            break;
        }

        case APP_CONSOLE_STATE_WAIT_PLC:
        {
            /* Wait for PLC transceiver initialization */
            if (appPlc.state == APP_PLC_STATE_WAITING)
            {
                /* Show Console menu */
                appConsole.state = APP_CONSOLE_STATE_SHOW_MENU;
            }
            else if (appPlc.state == APP_PLC_STATE_TX)
            {
                /* Set TX state */
                appConsole.state = APP_CONSOLE_STATE_TX;
            }
            break;
        }

        case APP_CONSOLE_STATE_SHOW_MENU:
        {
            /* Show console interface */
            APP_CONSOLE_Print(MENU_HEADER);
            
            /* Show console prompt */
            APP_CONSOLE_Print(MENU_PROMPT);

            /* Waiting Console command */
            appConsole.state = APP_CONSOLE_STATE_CONSOLE;
            APP_CONSOLE_ReadRestart(1);

            break;
        }

        case APP_CONSOLE_STATE_CONSOLE:
        {
            if (appConsole.numCharToReceive == 0)
            {
                switch(*appConsole.pReceivedChar)
                {
                    case '0':
                        appConsole.state = APP_CONSOLE_STATE_SET_BUFFER;
                        APP_CONSOLE_Print("\r\nEnter the buffer to use in TX [0,1] :");
                        APP_CONSOLE_ReadRestart(1);
                        break;
                        
                    case '1':
                        appConsole.state = APP_CONSOLE_STATE_SET_ATT_LEVEL;
                        APP_CONSOLE_Print("\r\nEnter attenuation level using 2 digits [00..FF][use FF for signal 0] : ");
                        APP_CONSOLE_ReadRestart(2);
                        break;

                    case '2':
                        appConsole.state = APP_CONSOLE_STATE_SET_SCHEME;
                        APP_CONSOLE_Print(MENU_SCHEME);
                        APP_CONSOLE_ReadRestart(1);
                        break;

                    case '3':
                        appConsole.state = APP_CONSOLE_STATE_SET_DISABLE_RX;
                        APP_CONSOLE_Print("\r\nForce disable RX in TX [Y/N] : ");
                        APP_CONSOLE_ReadRestart(1);
                        break;

                    case '4':
                        appConsole.state = APP_CONSOLE_STATE_SET_FRAME_TYPE;
                        APP_CONSOLE_Print(MENU_FRAME);
                        APP_CONSOLE_ReadRestart(1);
                        break;

                    case '5':
                        appConsole.state = APP_CONSOLE_STATE_SET_TIME_PERIOD;
                        APP_CONSOLE_Print("\r\nEnter transmission period in us. (max. 10 digits and value min 2100 us): ");
                        APP_CONSOLE_ReadRestart(10);
                        break;

                    case '6':
                        appConsole.state = APP_CONSOLE_STATE_SET_DATA_LEN;
                        APP_CONSOLE_Print("\r\nEnter length of data to transmit in bytes (max. 512 bytes): ");
                        APP_CONSOLE_ReadRestart(3);
                        break;

                    case '7':
                        appConsole.state = APP_CONSOLE_STATE_SET_CHANNEL;
                        APP_CONSOLE_Print(MENU_CHANNEL);
                        APP_CONSOLE_ReadRestart(1);
                        break;

                    case '8':
                        appConsole.state = APP_CONSOLE_STATE_SET_AUTODETECT;
                        APP_CONSOLE_Print(MENU_AUTODETECT);
                        APP_CONSOLE_ReadRestart(1);
                        break;

                    case 'v':
                    case 'V':
                        appConsole.state = APP_CONSOLE_STATE_VIEW_CONFIG;
                        break;

                    case 'e':
                    case 'E':
                        APP_CONSOLE_Print("\r\nStart transmission, type 'x' to cancel.\r\n");
                        appPlc.state = APP_PLC_STATE_TX;
                        appConsole.state = APP_CONSOLE_STATE_TX;
                        APP_CONSOLE_ReadRestart(1);
                        break;

                    case 'c':
                    case 'C':
                        if (appConsole.echoEnable)
                        {
                            appConsole.echoEnable = false;
                            APP_CONSOLE_Print("\r\nConsole ECHO disabled.\r\n");
                        }
                        else
                        {
                            appConsole.echoEnable = true;
                            APP_CONSOLE_Print("\r\nConsole ECHO enabled.\r\n");
                        }
                        
                        appConsole.state = APP_CONSOLE_STATE_SHOW_MENU;
                        break;

                    default:
                        /* Discard character */
                        appConsole.state = APP_CONSOLE_STATE_ERROR;
                        break;

                }
            }

            break;
        }

        case APP_CONSOLE_STATE_SET_BUFFER:
        {
            if (appConsole.numCharToReceive == 0)
            {
                if (APP_CONSOLE_SetTxBuffer(appConsole.pReceivedChar))
                {
                    APP_CONSOLE_Print("\r\nSet TX Buffer ID = %u\r\n",
                            (unsigned int)appPlcTx.plcPhyTx.bufferId);
                    appConsole.state = APP_CONSOLE_STATE_SHOW_MENU;
                }
                else
                {
                    /* Try it again */
                    APP_CONSOLE_Print("\r\nERROR: TX Buffer ID not permitted. Try again : ");
                    APP_CONSOLE_ReadRestart(1);
                }
            }
            break;
        }

        case APP_CONSOLE_STATE_SET_ATT_LEVEL:
        {
            if (appConsole.numCharToReceive == 0)
            {
                if (APP_CONSOLE_SetAttenuationLevel(appConsole.pReceivedChar))
                {
                    APP_CONSOLE_Print("\r\nSet Attenuation level = 0x%02x\r\n",
                            (unsigned int)appPlcTx.plcPhyTx.attenuation);
                    appConsole.state = APP_CONSOLE_STATE_SHOW_MENU;
                }
                else
                {
                    /* Try it again */
                    APP_CONSOLE_Print("\r\nERROR: Attenuation level not permitted. Try again : ");
                    APP_CONSOLE_ReadRestart(2);
                }
            }
            break;
        }

        case APP_CONSOLE_STATE_SET_SCHEME:
        {
            if (appConsole.numCharToReceive == 0)
            {
                uint8_t schemeIndex;
                
                schemeIndex = APP_CONSOLE_SetScheme(appConsole.pReceivedChar);
                if (schemeIndex != 0xFF)
                {
                    APP_CONSOLE_Print("\r\nTx Modulation: %.30s Max data length = %u\r\n", 
                            schemeRatesDescription[schemeIndex], appPlcTx.maxPsduLen);
                    appConsole.state = APP_CONSOLE_STATE_SHOW_MENU;
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

        case APP_CONSOLE_STATE_SET_DISABLE_RX:
        {
            if (appConsole.numCharToReceive == 0)
            {
                if (APP_CONSOLE_SetDisableRX(appConsole.pReceivedChar))
                {
                    if (appPlcTx.plcPhyTx.csma.disableRx)
                    {
                        APP_CONSOLE_Print("\r\nDisable RX in TX\r\n");
                    }
                    else
                    {
                        APP_CONSOLE_Print("\r\nEnable RX in TX\r\n");
                    }
                    appConsole.state = APP_CONSOLE_STATE_SHOW_MENU;
                }
                else
                {
                    /* Try it again */
                    APP_CONSOLE_Print("\r\nERROR: Selection not permitted. Try again : ");
                    APP_CONSOLE_ReadRestart(1);
                }
            }
            break;
        }

        case APP_CONSOLE_STATE_SET_FRAME_TYPE:
        {
            if (appConsole.numCharToReceive == 0)
            {
                if (APP_CONSOLE_SetFrameType(appConsole.pReceivedChar))
                {
                    APP_CONSOLE_Print("\r\nSet frame type: %s\r\n", frameDescription[appPlcTx.plcPhyTx.frameType]);
                    appConsole.state = APP_CONSOLE_STATE_SHOW_MENU;
                }
                else
                {
                    /* Try it again */
                    APP_CONSOLE_Print("\r\nERROR: Selection not permitted. Try again : ");
                    APP_CONSOLE_ReadRestart(1);
                }
            }
            break;
        }

        case APP_CONSOLE_STATE_SET_TIME_PERIOD:
        {
            if (appConsole.numCharToReceive == 0)
            {
                if (APP_CONSOLE_SetTransmissionPeriod(appConsole.pReceivedChar, appConsole.dataLength))
                {
                    APP_CONSOLE_Print("\r\nSet Time Period = %u us.\r\n",
                            (unsigned int)appPlcTx.plcPhyTx.timeIni);
                    appConsole.state = APP_CONSOLE_STATE_SHOW_MENU;
                }
                else
                {
                    /* Try it again */
                    APP_CONSOLE_Print("\r\nERROR: Time Period not defined. Try again : ");
                    APP_CONSOLE_ReadRestart(10);
                }
                break;
            }
        }

        case APP_CONSOLE_STATE_SET_DATA_LEN:
        {
            if (appConsole.numCharToReceive == 0)
            {
                if (APP_CONSOLE_SetDataLength(appConsole.pReceivedChar, appConsole.dataLength))
                {
                    APP_CONSOLE_Print("\r\nSet Data Length = %u bytes\r\n",
                            (unsigned int)appPlcTx.plcPhyTx.dataLength);

                    /* Set Data content */
                    APP_CONSOLE_Print(MENU_DATA_MODE);
                    appConsole.state = APP_CONSOLE_STATE_SET_DATA;
                    APP_CONSOLE_ReadRestart(1);
                }
                else
                {
                    /* Try it again */
                    APP_CONSOLE_Print("\r\nERROR: Data length is not permitted. Try again : ");
                    APP_CONSOLE_ReadRestart(3);
                }
            }
            break;
        }

        case APP_CONSOLE_STATE_SET_DATA:
        {
            if (appConsole.numCharToReceive == 0)
            {
                if (APP_CONSOLE_SetDataMode(appConsole.pReceivedChar))
                {
                    APP_CONSOLE_Print("\r\nSet Data mode successfully\r\n");
                    appConsole.state = APP_CONSOLE_STATE_SHOW_MENU;
                }
                else
                {
                    /* Try it again */
                    APP_CONSOLE_Print("\r\nERROR: Data Mode not permitted. Try again : ");
                    APP_CONSOLE_ReadRestart(1);
                }
            }
            break;
        }

        case APP_CONSOLE_STATE_SET_CHANNEL:
        {
            if (appConsole.numCharToReceive == 0)
            {
                if (APP_CONSOLE_SetChannel(appConsole.pReceivedChar))
                {
                    APP_CONSOLE_Print("\r\nSet channel %u\r\n", (unsigned int)appPlcTx.channel);
                    appConsole.state = APP_CONSOLE_STATE_SHOW_MENU;
                }
                else
                {
                    /* Try it again */
                    APP_CONSOLE_Print("\r\nERROR: Channel is not available. Try again : ");
                    APP_CONSOLE_ReadRestart(1);
                }
            }
            break;
        }

        case APP_CONSOLE_STATE_SET_AUTODETECT:
        {
            if (appConsole.numCharToReceive == 0)
            {
                if (APP_CONSOLE_SetAutodetect(appConsole.pReceivedChar))
                {
                    if (appPlcTx.txAuto)
                    {
                        APP_CONSOLE_Print("\r\nAutodetect mode selected\r\n");
                    }
                    else
                    {
                        if (appPlcTx.txImpedance == HI_STATE)
                        {
                            APP_CONSOLE_Print("\r\nFix high impedance mode selected\r\n");
                        }
                        else if (appPlcTx.txImpedance == LOW_STATE)
                        {
                            APP_CONSOLE_Print("\r\nFix low impedance mode selected\r\n");
                        }
                        else
                        {
                            APP_CONSOLE_Print("\r\nFix very low impedance mode selected\r\n");
                        }
                    }
                    appConsole.state = APP_CONSOLE_STATE_SHOW_MENU;
                }
                else
                {
                    /* Try it again */
                    APP_CONSOLE_Print("\r\nERROR: Selection is not permitted. Try again : ");
                    APP_CONSOLE_ReadRestart(1);
                }
            }
            break;
        }

        case APP_CONSOLE_STATE_VIEW_CONFIG:
        {
            APP_CONSOLE_ShowConfiguration();
            appConsole.state = APP_CONSOLE_STATE_SHOW_MENU;
            break;
        }

        case APP_CONSOLE_STATE_TX:
        {
            if (appConsole.numCharToReceive == 0)
            {
                if ((appConsole.pReceivedChar[0] == 'x') || (appConsole.pReceivedChar[0] == 'X'))
                {
                    APP_CONSOLE_Print("\r\nCancel transmission\r\n");
                    appPlc.state = APP_PLC_STATE_STOP_TX;
                    appConsole.state = APP_CONSOLE_STATE_SHOW_MENU;
                }
                else
                {
                    APP_CONSOLE_ReadRestart(1);
                }
            }
            break;
        }

        case APP_CONSOLE_STATE_ERROR:
        {
            APP_CONSOLE_Print("\r\nERROR: Unknown received character\r\n");
            appConsole.state = APP_CONSOLE_STATE_SHOW_MENU;
            break;
        }

        /* The default state should never be executed. */
        default:
        {
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


/*******************************************************************************
 End of File
 */
