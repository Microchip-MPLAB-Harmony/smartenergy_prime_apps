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
#define APP_LENGTH_ECDSA_KEY      65

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

/* ECDSA P-256 public key used to verify FU images. */
static uint8_t appPubECDSAKey[APP_LENGTH_ECDSA_KEY] =
    {0x04,0x8d,0x6c,0x28,0x44,0xf6,0x47,0xc5,0x95,0x6b,0xa4,0x97,0xfd,0x86,0xbb,
     0xe4,0xfc,0xe3,0x90,0xe5,0x03,0xfa,0xe9,0xe0,0x4f,0x0d,0xe9,0x03,0x92,0x70,
     0x46,0x7b,0xa9,0xc4,0x80,0xf5,0x27,0xf0,0xad,0xf0,0xc9,0x5d,0x83,0x99,0xee,
     0xaa,0x1d,0x73,0x9a,0xb3,0xc9,0xa8,0x93,0x17,0xa9,0x3d,0x7f,0x7c,0x62,0xbb,
     0x51,0x57,0xb0,0x2e,0x9f};


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
    /* Initialize PRIME stack with the new pointer */
    if (newPrimeApi == (PRIME_API *)PRIME_SN_FWSTACK14_ADDRESS)
    {
        PRIME_Restart((uint32_t *)newPrimeApi, PRIME_VERSION_1_4);
    } 
    else
    {
        PRIME_Restart((uint32_t *)newPrimeApi, PRIME_VERSION_1_3);
    }

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

            /* Initialize result callback for version swap request */
            SRV_FU_RegisterCallbackSwapVersion(lAPP_PrimeVersionSwapRequest);

            /* Initialize FU result callback */
            SRV_FU_RegisterCallbackFuResult(lAPP_PrimeFuResultHandler);
            
            /* Pass the public key to FU module */
            SRV_FU_SetECDSAPublicKey(appPubECDSAKey, APP_LENGTH_ECDSA_KEY);
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
