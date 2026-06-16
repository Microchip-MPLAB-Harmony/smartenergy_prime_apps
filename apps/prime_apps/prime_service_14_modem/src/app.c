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
#include "bsp/bsp.h"
#include "service/firmware_upgrade/srv_firmware_upgrade.h"

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

/* ECDSA P-256 public key used to verify FU images. SEC1 uncompressed
 * format: 1-byte 0x04 prefix + 32-byte X + 32-byte Y = 65 bytes total.
 * The signing tool uses the matching private key to sign the bundle;
 * SRV_FU_VerifyImage runs SHA-256 over the image (minus the trailing
 * signLength bytes) and verifies the captured signature against this
 * key via Crypto_DigiSign_Ecdsa_Verify.
 *
 * The pointer is handed to the FU service via SRV_FU_SetECDSAPublicKey
 * during init; the FU service stores the pointer (no copy), so the
 * buffer must outlive it -- file-scope const is fine.
 */
#define APP_LENGTH_ECDSA_KEY      65

static uint8_t appPubECDSAKey[APP_LENGTH_ECDSA_KEY] =
    {0x04,0x8d,0x6c,0x28,0x44,0xf6,0x47,0xc5,0x95,0x6b,0xa4,0x97,0xfd,0x86,0xbb,
     0xe4,0xfc,0xe3,0x90,0xe5,0x03,0xfa,0xe9,0xe0,0x4f,0x0d,0xe9,0x03,0x92,0x70,
     0x46,0x7b,0xa9,0xc4,0x80,0xf5,0x27,0xf0,0xad,0xf0,0xc9,0x5d,0x83,0x99,0xee,
     0xaa,0x1d,0x73,0x9a,0xb3,0xc9,0xa8,0x93,0x17,0xa9,0x3d,0x7f,0x7c,0x62,0xbb,
     0x51,0x57,0xb0,0x2e,0x9f};

/* New PRIME stack pointer */
static const PRIME_API *newPrimeApi;

/* Enable swapping of stack location */
static uint32_t volatile fuSwapEn;

/* SW0 handling state. Pressing SW0 (PA15, active-low) for ~APP_SW0_DEBOUNCE_TICKS
 * consecutive APP_Tasks iterations writes BOOT_MODE_UART_PENDING into
 * the SST26 BOOT_FLAG sector and resets the device, so the next boot
 * lands in the bootloader's UART recovery loop. Equivalent to the PIB
 * 0xA000 path but without needing PRIME network access -- intended as a
 * physical emergency entry for HW debug. */
#define APP_SW0_DEBOUNCE_TICKS              (1000U)

typedef enum
{
    APP_SW0_IDLE = 0,
    APP_SW0_WAIT_BOOT_MODE_OK,
} APP_SW0_STATE;

static APP_SW0_STATE appSw0State    = APP_SW0_IDLE;
static uint16_t      appSw0LowCount = 0U;

/* Firmware-swap state machine. After the PRIME stack signals a FU
 * result that needs a swap (success or revert), the swap is no
 * longer a one-shot synchronous call -- SRV_FU_SwapFirmware now
 * kicks an async write of BOOT_MODE_INFO into SST26, which takes
 * ~28 ms (sector erase + page program) and is driven by SRV_FU_Tasks
 * during the main loop. The application polls
 * SRV_FU_ExtMemBootModeStatus until it transitions to OK and only
 * then triggers the reset. */
typedef enum
{
    APP_SWAP_IDLE = 0,
    APP_SWAP_WAIT_BOOT_MODE_OK,
} APP_SWAP_STATE;

static APP_SWAP_STATE appSwapState = APP_SWAP_IDLE;

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
    /* v3 hand-off: kick the BOOT_MODE_INFO write and let
     * lAPP_PollSwap drive the rest. SRV_FU_SwapFirmware is now async
     * (it calls SRV_FU_ExtMemBootModeSet under the hood), so this
     * function only schedules the operation; the reset happens later
     * once the SST26 page program completes.
     *
     * If the FU service is busy (Set returns false), the request is
     * dropped silently here -- the BS will time out and retry the
     * swap, which is the safe behaviour. */
    if (SRV_FU_SwapFirmware() == true)
    {
        appSwapState = APP_SWAP_WAIT_BOOT_MODE_OK;
    }
}

static void lAPP_PollSwap(void)
{
    SRV_FU_EXT_MEM_BOOT_MODE_STATUS status;

    if (appSwapState != APP_SWAP_WAIT_BOOT_MODE_OK)
    {
        return;
    }

    status = SRV_FU_ExtMemBootModeStatus();
    if (status == SRV_FU_EXT_MEM_BOOT_MODE_STATUS_OK)
    {
        /* BOOT_MODE_INFO is now in SST26. Reset hands control to the
         * bootloader, which reads it and dispatches install/revert. */
        SRV_RESET_HANDLER_RestartSystem(RESET_HANDLER_FU_RESET);
        /* unreachable */
    }
    else if (status == SRV_FU_EXT_MEM_BOOT_MODE_STATUS_ERROR)
    {
        /* Page program failed (DRV_MEMORY error). Drop the swap and
         * let the BS retry a full FU cycle. */
        appSwapState = APP_SWAP_IDLE;
    }
    /* IDLE / BUSY: keep waiting. */
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
    static void lAPP_HandleSw0 ( void )

  Summary:
    Polls SW0 (PA15, active-low) and triggers a reset into UART recovery
    when held for ~APP_SW0_DEBOUNCE_TICKS consecutive ticks.

  Description:
    Two-step state machine:

      APP_SW0_IDLE
        - Counts consecutive LOW polls. When the threshold is reached,
          calls SRV_FU_ExtMemBootModeSet(UART_PENDING). If the FU service
          is currently busy (e.g. mid telecarga) the call returns false
          and we keep polling -- the press does not get lost.

      APP_SW0_WAIT_BOOT_MODE_OK
        - The set was kicked. Polls SRV_FU_ExtMemBootModeStatus until
          OK (sector erase + page program completed in SST26) and then
          triggers NVIC_SystemReset. ERROR returns to IDLE so the user
          can try again.

    BSP_USER_BUTTON0 pin / pull configuration is owned by plib_port.c
    (regenerated by MCC). PA15 already comes up as an input that reads
    HIGH when released and LOW when pressed.
*/
static void lAPP_HandleSw0(void)
{
    SRV_FU_EXT_MEM_BOOT_MODE_STATUS status;

    if (appSw0State == APP_SW0_IDLE)
    {
        if (BSP_USER_BUTTON0_Get() == BSP_USER_BUTTON0_STATE_PRESSED)
        {
            appSw0LowCount++;
            if (appSw0LowCount >= APP_SW0_DEBOUNCE_TICKS)
            {
                appSw0LowCount = 0U;
                if (SRV_FU_ExtMemBootModeSet(
                        SRV_FU_EXT_MEM_BOOT_MODE_UART_PENDING,
                        0U, 0U) == true)
                {
                    appSw0State = APP_SW0_WAIT_BOOT_MODE_OK;
                }
                /* else: FU service busy; keep IDLE so we retry on the
                 * next press cycle. */
            }
        }
        else
        {
            appSw0LowCount = 0U;
        }
    }
    else /* APP_SW0_WAIT_BOOT_MODE_OK */
    {
        status = SRV_FU_ExtMemBootModeStatus();
        if (status == SRV_FU_EXT_MEM_BOOT_MODE_STATUS_OK)
        {
            NVIC_SystemReset();
            /* unreachable */
        }
        else if (status == SRV_FU_EXT_MEM_BOOT_MODE_STATUS_ERROR)
        {
            /* Sector erase or page program failed. Reset state and let
             * the operator try again. */
            appSw0State = APP_SW0_IDLE;
        }
        else
        {
            /* IDLE / BUSY -- keep waiting. */
        }
    }
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
    fuSwapEn = 0U;

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
            SYS_TIME_HANDLE timeHandle;

            /* Start of PRIME STack*/
            PRIME_Open(PRIME_INDEX_0);
            
            /* Register timer callback to blink LED */
            timeHandle = SYS_TIME_CallbackRegisterMS(
                    lAPP_TimeExpiredSetFlag, (uintptr_t) &appData.timerLedExpired,
                    APP_LED_BLINK_PERIOD_MS, SYS_TIME_PERIODIC);

            if (timeHandle != SYS_TIME_HANDLE_INVALID)
            {
                appData.state = APP_STATE_SERVICE_TASKS;
                SYS_CONSOLE_MESSAGE(APP_STRING_HEADER);
            }

            /* Initialize FU result callback */
            SRV_FU_RegisterCallbackFuResult(lAPP_PrimeFuResultHandler);

            /* Hand the ECDSA P-256 public key to the FU service so it
             * can verify the signature appended to the FU image. Without
             * this, SRV_FU_VerifyImage rejects every signed image
             * (dsaState stays at NO_PUBLIC_KEY). The same key is shared
             * with the dual_modem PIC32 build. */
            SRV_FU_SetECDSAPublicKey(appPubECDSAKey, APP_LENGTH_ECDSA_KEY);
            break;
        }

        case APP_STATE_SERVICE_TASKS:
        {
            APP_Modem_Tasks();

            /* Check if FU location must be swapped */
            if (fuSwapEn == APP_FU_ENABLE_SWAP)
            {
                fuSwapEn = 0U;
                lAPP_SwapFirmware();
            }

            /* Drive the async BOOT_MODE_INFO write that SwapFirmware
             * kicked. When it completes, this resets the device. */
            lAPP_PollSwap();

            /* Physical emergency entry to bootloader: SW0 held
             * triggers a reset into UART recovery. Polled every
             * APP_Tasks iteration. */
            lAPP_HandleSw0();

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
