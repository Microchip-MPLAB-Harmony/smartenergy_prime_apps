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
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    app_console.h

  Summary:
    This header file provides prototypes and definitions for the application.

  Description:
    This header file provides function prototypes and data type definitions for
    the application.  Some of these are required by the system (such as the
    "APP_CONSOLE_Initialize" and "APP_CONSOLE_Tasks" prototypes) and some of them are only used
    internally by the application (such as the "APP_CONSOLE_STATES" definition).  Both
    are defined here for convenience.
*******************************************************************************/

#ifndef _APP_CONSOLE_H
#define _APP_CONSOLE_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "configuration.h"
#include "definitions.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

extern "C" {

#endif
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Type Definitions
// *****************************************************************************
// *****************************************************************************
#define SERIAL_BUFFER_SIZE        512
#define LED_RX_OFF_RATE_MS        100    
    
// *****************************************************************************
/* Application states

  Summary:
    Application states enumeration

  Description:
    This enumeration defines the valid application states.  These states
    determine the behavior of the application at various times.
*/

typedef enum
{
    /* Application's state machine's initial state. */
    APP_CONSOLE_STATE_IDLE = 0,
    APP_CONSOLE_STATE_INIT,
    APP_CONSOLE_STATE_WAIT_PLC,
    APP_CONSOLE_STATE_SHOW_MENU,
    APP_CONSOLE_STATE_CONSOLE,
    APP_CONSOLE_STATE_SET_BUFFER,
    APP_CONSOLE_STATE_SET_ATT_LEVEL,
    APP_CONSOLE_STATE_SET_SCHEME,
    APP_CONSOLE_STATE_SET_DISABLE_RX,
    APP_CONSOLE_STATE_SET_FRAME_TYPE,
    APP_CONSOLE_STATE_SET_TIME_PERIOD,
    APP_CONSOLE_STATE_SET_DATA_LEN,
    APP_CONSOLE_STATE_SET_DATA,
    APP_CONSOLE_STATE_SET_CHANNEL,
    APP_CONSOLE_STATE_SET_AUTODETECT,
    APP_CONSOLE_STATE_VIEW_CONFIG,
    APP_CONSOLE_STATE_TX,
    APP_CONSOLE_STATE_ERROR
            
} APP_CONSOLE_STATES;

// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    Application strings and buffers are be defined outside this structure.
 */

typedef struct
{
    /* The application's current state */
    SYS_TIME_HANDLE tmr1Handle;   
    
    APP_CONSOLE_STATES state;

    char *pTransmitChar;
    
    char *pReceivedChar;

    char* pNextChar;
    
    size_t dataLength;
    
    uint8_t numCharToReceive;
    
    bool echoEnable;

} APP_CONSOLE_DATA;

extern APP_CONSOLE_DATA appConsole;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Routines
// *****************************************************************************
// *****************************************************************************
/* These routines are called by drivers when certain events occur.
*/

// *****************************************************************************
// *****************************************************************************
// Section: Console Interface Definition
// *****************************************************************************
// *****************************************************************************
#define STRING_EOL    "\r"
#define STRING_HEADER "\r\n-- MCHP PLC Tx Console Application --\r\n" \
	"-- Compiled: "__DATE__ " "__TIME__ " --\r\n" \
	"-- HOST version: "DRV_PLC_PHY_HOST_DESC " --\r\n"

#define MENU_HEADER "\n\r-- Menu Configuration --------------\n\r" \
	"0: Select Buffer to Transmit\n\r" \
    "1: Select Attenuation Level\n\r" \
	"2: Select Modulation/Scheme\n\r" \
    "3: Disable Rx in Transmission\n\r" \
    "4: Select Frame Type to Transmit\n\r" \
	"5: Select Time period between messages to transmit(us.)\n\r" \
	"6: Enter Data to transmit\n\r" \
	"7: Select Channel to Transmit\n\r" \
    "8: Config Auto-Detect Impedance\n\r" \
	"v: View TX configuration values\n\r" \
	"e: Execute transmission application\n\r" \
	"c: Enable/Disable ECHO console\n\r" \
	"otherwise: Display this main menu\n\n\r"

#define MENU_SCHEME "\n\r-- PRIME Scheme --------------\r\n"	\
	"0: DBPSK\n\r" \
	"1: DQPSK\n\r" \
	"2: D8PSK\n\r" \
    "3: DBPSK_C\n\r" \
	"4: DQPSK_C\n\r" \
	"5: D8PSK_C\n\r" \
	"6: R_DQPSK\n\r" \
	"7: R_D8PSK\n\r"

#define MENU_FRAME "\n\r-- PRIME Frame Type --------------\r\n"	\
	"0: TYPE_A\n\r" \
	"2: TYPE_B\n\r"	\
	"3: TYPE_BC\n\r"

#define MENU_DATA_MODE "\n\r-- Select Data Mode --------------\r\n" \
	"0: Random Data\n\r" \
	"1: Fixed Data\n\r"

#define MENU_CHANNEL "\n\r-- PRIME Channel --------------\r\n"	\
	"1  : CENELEC A\n\r" \
	"3-8: FCC\n\r"	

#define MENU_AUTODETECT "\n\r-- Autodetect Impedance Mode --------------\r\n"	\
	"0: Autodetect mode\n\r" \
	"1: Fixed Mode. High Impedance\n\r" \
	"2: Fixed Mode. Low Impedance\n\r" \
	"3: Fixed Mode. Vlow Impedance\n\r"

#define MENU_PROMPT "\n\rPHY-Console>"

/*******************************************************************************
  Function:
    void APP_CONSOLE_Initialize ( void )

  Summary:
     MPLAB Harmony application initialization routine.

  Description:
    This function initializes the Harmony application.  It places the
    application in its initial state and prepares it to run so that its
    APP_CONSOLE_Tasks function can be called.

  Precondition:
    All other system initialization routines should be called before calling
    this routine (in "SYS_Initialize").

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    APP_CONSOLE_Initialize();
    </code>

  Remarks:
    This routine must be called from the SYS_Initialize function.
*/

void APP_CONSOLE_Initialize ( void );


/*******************************************************************************
  Function:
    void APP_CONSOLE_Tasks ( void )

  Summary:
    MPLAB Harmony Demo application tasks function

  Description:
    This routine is the Harmony Demo application's tasks function.  It
    defines the application's state machine and core logic.

  Precondition:
    The system and application initialization ("SYS_Initialize") should be
    called before calling this.

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    APP_CONSOLE_Tasks();
    </code>

  Remarks:
    This routine must be called from SYS_Tasks() routine.
 */

void APP_CONSOLE_Tasks( void );

// *****************************************************************************
/* Function:
    void APP_CONSOLE_Print(const char *format, ...)

 

  Summary:
    Formats and prints a message with a variable number of arguments to the
    console.

 

  Description:
    This function formats and prints a message with a variable number of
    arguments to the console.

 

  Precondition:
    APP_CONSOLE_Initialize must have returned a valid object handle.

 

  Parameters:
    format          - Pointer to a buffer containing the format string for
                      the message to be displayed.
    ...             - Zero or more optional parameters to be formated as
                      defined by the format string.

 

  Returns:
    None.

 

  Example:
    <code>
    // In source code
    int result;

 

    result = SomeOperation();
    if (result > MAX_VALUE)
    {
        APP_CONSOLE_Print("Result of %d exceeds max value\r\n", result);
    }
    </code>

 

  Remarks:
    The format string and arguments follow the printf convention.

 

*/

 

void APP_CONSOLE_Print(const char *format, ...);

#endif /* _APP_CONSOLE_H */

//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

/*******************************************************************************
 End of File
 */

