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

#include "app.h"
#include "definitions.h"
#include "user.h"
#include "modem.h"

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

/* New PRIME stack pointer */
static const PRIME_API *newPrimeApi;

/* Enable swapping of stack location */
static uint32_t volatile fuSwapEn;
static uint32_t volatile versionSwapEn;

/* PRIME regions configuration */
//SRV_FU_REGION_CGF fuRegion;

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

static void lAPP_PrimeVersionSwapRequest(SRV_FU_TRAFFIC_VERSION traffic)
{
    /* Compare current PRIME pointer with detected traffic */
    if (traffic == SRV_FU_TRAFFIC_VER_PRIME_1_4) 
    {
        newPrimeApi = (PRIME_API *)PRIME_SN_FWSTACK14_ADDRESS;
        versionSwapEn = APP_VERSION_ENABLE_SWAP;

    } 
    else if (traffic == SRV_FU_TRAFFIC_VER_PRIME_1_3) 
    {
        newPrimeApi = (PRIME_API *)PRIME_SN_FWSTACK13_ADDRESS;
        versionSwapEn = APP_VERSION_ENABLE_SWAP;
    }
    else
    {
        // Do nothing
    }
}

static void lAPP_SwapStackVersion(void)
{
    uint32_t *nvicCpr0;
    uint32_t *nvicSer0;
    uint32_t *nvicCer0;
    uint32_t temp;

    /* Hold interrupt system */
    nvicSer0 = (uint32_t *)NVIC_ISER0;
    temp = *nvicSer0;

    /* Clear pending interrupts */
    nvicCer0 = (uint32_t *)NVIC_ICER0;
    nvicCpr0 = (uint32_t *)NVIC_ICPR0;
    *nvicCer0 = 0xFFFFFFFF;
    *nvicCpr0 = 0xFFFFFFFF;

    /* Reset PLC */
    DRV_PLC_HAL_Reset();

    /* Restore interrupt system */
    *nvicSer0 = temp;

    /* Initialize PRIME stack with the new pointer */
    PRIME_Restart((uint32_t *)newPrimeApi);

    /* Initialize Modem application */
    APP_Modem_Initialize(); /* Needed to set up callbacks */
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
    versionSwapEn = 0;

    /* Initialize modem application */
    APP_Modem_Initialize();

    /* Initialize application variables */
    appData.timerLedExpired = false;
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
            /* Register timer callback to blink LED */
            SYS_TIME_HANDLE timeHandle = SYS_TIME_CallbackRegisterMS(
                    lAPP_TimeExpiredSetFlag, (uintptr_t) &appData.timerLedExpired,
                    APP_LED_BLINK_PERIOD_MS, SYS_TIME_PERIODIC);

            if (timeHandle != SYS_TIME_HANDLE_INVALID)
            {
                appData.state = APP_STATE_SERVICE_TASKS;
                SYS_CONSOLE_MESSAGE(APP_STRING_HEADER);
            }

            /* Initialize result callback for version swap request */
            SRV_FU_RegisterCallbackSwapVersion(lAPP_PrimeVersionSwapRequest);

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

            /* Check if stack must be swapped */
            if (versionSwapEn == APP_VERSION_ENABLE_SWAP)
            {
                versionSwapEn = 0;
                lAPP_SwapStackVersion();
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
