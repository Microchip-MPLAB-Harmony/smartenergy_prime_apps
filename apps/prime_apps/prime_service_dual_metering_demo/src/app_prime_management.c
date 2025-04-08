/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app_prime_management.c

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

#include "definitions.h"
#include "app_prime_management.h"
#include "prime_metrology.h"

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
    This structure should be initialized by the APP_PRIME_MANAGEMENT_Initialize function.

    Application strings and buffers are be defined outside this structure.
*/

APP_PRIME_MANAGEMENT_DATA app_prime_managementData;

/* New PRIME stack pointer */
static const PRIME_API *newPrimeApi;

/* Enable swapping of stack location */
static uint32_t volatile fuSwapEn;
static uint32_t volatile versionSwapEn;

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

    /* Initialize metrology application */
    APP_PRIME_METROLOGY_Initialize(); /* Needed to set up callbacks */
}


// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_PRIME_MANAGEMENT_Initialize ( void )

  Remarks:
    See prototype in app_prime_management.h.
 */

void APP_PRIME_MANAGEMENT_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    app_prime_managementData.state = APP_PRIME_MANAGEMENT_STATE_INIT;

    /* Initialize swap flags */
    fuSwapEn = 0;
    versionSwapEn = 0;

    /* Initialize metrology application */
    APP_PRIME_METROLOGY_Initialize();
}

/******************************************************************************
  Function:
    void APP_PRIME_MANAGEMENT_Tasks ( void )

  Remarks:
    See prototype in app_prime_management.h.
 */

void APP_PRIME_MANAGEMENT_Tasks ( void )
{

    /* Check the application's current state. */
    switch ( app_prime_managementData.state )
    {
        /* Application's initial state. */
        case APP_PRIME_MANAGEMENT_STATE_INIT:
        {
            /* Initialize result callback for version swap request */
            SRV_FU_RegisterCallbackSwapVersion(lAPP_PrimeVersionSwapRequest);

            /* Initialize FU result callback */
            SRV_FU_RegisterCallbackFuResult(lAPP_PrimeFuResultHandler);
            break;
        }

        case APP_PRIME_MANAGEMENT_STATE_SERVICE_TASKS:
        {
            APP_PRIME_METROLOGY_Tasks();

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

        /* TODO: implement your application state machine.*/


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
