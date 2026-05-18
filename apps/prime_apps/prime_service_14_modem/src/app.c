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
#include "app.h"
#include "user.h"
#include "modem.h"

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

/* New PRIME stack pointer */
static const PRIME_API *newPrimeApi;

/* Enable swapping of stack location */
static uint32_t volatile fuSwapEn;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

/* TODO:  Add any necessary callback functions.
*/

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************

static void lAPP_SwapFirmware(void)
{
    /* Swap firmware */
    if (SRV_FU_SwapFirmware() == true)
    {
        /* Trigger reset to launch bootloader */
        SRV_RESET_HANDLER_RestartSystem(RESET_HANDLER_FU_RESET);
    }
}

static void lAPP_PrimeFuResultHandler(SRV_FU_RESULT fuResult)
{
    switch (fuResult) 
    {
        case SRV_FU_RESULT_SUCCESS:
            /* Update FU pointer */
            fuSwapEn = APP_FU_ENABLE_SWAP;
            break;

        case SRV_FU_RESULT_CRC_ERROR:
            /* Nothing to do - FU will restart automatically */
            break;

        case SRV_FU_RESULT_CANCEL:
            /* Nothing to do */
            break;

        case SRV_FU_RESULT_FW_CONFIRM:
            /* Nothing to do */
            break;

        case SRV_FU_RESULT_FW_REVERT:
            /* Revert FU pointer */
            fuSwapEn = APP_FU_ENABLE_SWAP;
            break;

        case SRV_FU_RESULT_ERROR:
            /* Nothing to do */
            break;

        case SRV_FU_RESULT_SIGNATURE_ERROR:
            /* Nothing to do */
            break;

        case SRV_FU_RESULT_IMAGE_ERROR:
            /* Nothing to do */
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

static void lAPP_TimeExpiredSetFlag(uintptr_t context)
{
    /* Context holds the flag's address */
    *((bool *) context) = true;
}


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

    /* Initialize swap flags */
    fuSwapEn = 0;

    /* Initialize modem application */
    APP_Modem_Initialize();

    /* Initialize application variables */
    appData.timerLedExpired = false;

    WDT_Enable();
}


/******************************************************************************
  Function:
    void APP_Tasks ( void )

  Remarks:
    See prototype in app.h.
 */

void APP_Tasks ( void )
{
    /* Refresh Watchdog */
    CLEAR_WATCHDOG();

    /* Signaling: LED Toggle */
    if (appData.timerLedExpired == true)
    {
        appData.timerLedExpired = false;
        USER_BLINK_LED_Toggle();
    }

    /* Check the application's current state. */
    switch ( appData.state )
    {
        /* Application's initial state. */
        case APP_STATE_INIT:
        {
            /* Start of PRIME STack*/
            PRIME_Open(PRIME_INDEX_0);
            
            /* Register timer callback to blink LED */
            SYS_TIME_HANDLE timeHandle = SYS_TIME_CallbackRegisterMS(
                    lAPP_TimeExpiredSetFlag, (uintptr_t) &appData.timerLedExpired,
                    APP_LED_BLINK_PERIOD_MS, SYS_TIME_PERIODIC);

            if (timeHandle != SYS_TIME_HANDLE_INVALID)
            {
                appData.state = APP_STATE_SERVICE_TASKS;
                SYS_CONSOLE_MESSAGE(APP_STRING_HEADER);
            }

            /* Initialize FU result callback */
            SRV_FU_RegisterCallbackFuResult(lAPP_PrimeFuResultHandler);
            break;
        }

        case APP_STATE_SERVICE_TASKS:
        {
            APP_Modem_Tasks();

            /* Check if FU location must be swapped */
            if (fuSwapEn == APP_FU_ENABLE_SWAP)
            {
                fuSwapEn = 0;
                lAPP_SwapFirmware();
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
