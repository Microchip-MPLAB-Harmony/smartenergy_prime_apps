/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app_bootloader.c

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
#include "app_bootloader.h"
#include "definitions.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

/* PRIME Bootloader version (in fixed position in flash) */
#ifdef __GNUC__
const unsigned char PRBO_version[32] __attribute__((section(".frwvrs"))) = PRBO_VERSION;
#endif
#ifdef __ICCARM__
#pragma location = ".frwvrs"
__root const char PRBO_version[32] = PRBO_VERSION;
#endif

/* User signature */
static uint8_t userSignBuf[BOOT_FLASH_PAGE_SIZE];
static uint8_t *bootConfig;

/* Temporal buffer to store the flash pages content (in blocks of pages) */
static uint8_t pageBlock[BOOT_FLASH_16PAGE_SIZE];

/* Counter of page blocks */
static uint8_t pagesCounter;

/* Bootloader state */
static BOOT_STATE bootState;

// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_BOOTLOADER_Initialize function.

    Application strings and buffers are be defined outside this structure.
 */

APP_BOOTLOADER_DATA app_bootloaderData;

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

static void lAPP_BOOTLOADER_UpdateUserSignature(uint8_t pagesCnt,
        BOOT_STATE state) {
    /* Update values */
    bootConfig[16] = pagesCnt;
    bootConfig[17] = (uint8_t) state;

    /* Erase the user signature */
    SEFC0_UserSignatureErase(BOOT_USER_SIGNATURE_BLOCK);

    /* Update the user signature */
    (void) SEFC0_UserSignatureWrite((void*) userSignBuf,
            (uint32_t) BOOT_USER_SIGNATURE_SIZE_64,
            (SEFC_USERSIGNATURE_BLOCK) BOOT_USER_SIGNATURE_BLOCK,
            (SEFC_USERSIGNATURE_PAGE) BOOT_USER_SIGNATURE_PAGE);
}

static bool lAPP_BOOTLOADER_IsSwapCmd(uint32_t imgSize,
        uint32_t srcAddr,
        uint32_t dstAddr) {
    if ((imgSize == 0UL) || (srcAddr == 0UL) || (dstAddr == 0UL) ||
            (imgSize == 0xFFFFFFFFUL) ||
            (srcAddr == 0xFFFFFFFFUL) || (dstAddr == 0xFFFFFFFFUL)) {
        return false;
    }

    if (srcAddr == dstAddr) {
        return false;
    }

    if ((bootState == BOOT_IDLE) && (pagesCounter > 0)) {
        return false;
    }

    if (bootState > BOOT_COPIED_BUFF_TO_APP) {
        return false;
    }

    return true;
}

static uint8_t lAPP_BOOTLOADER_DeletePage(uint32_t addr) {
    SEFC0_RegionUnlock(addr);

    while (SEFC0_IsBusy()) {
        ;
    }

    SEFC0_PageErase(addr); // Function erases 16 pages

    while (SEFC0_IsBusy()) {
        ;
    }

    return 1;
}

static uint8_t lAPP_BOOTLOADER_CopyPage(uint32_t srcAddr, uint32_t dstAddr) {
    uint8_t *page;
    uint8_t i;

    (void) memcpy(pageBlock, (uint8_t *) (srcAddr), BOOT_FLASH_16PAGE_SIZE);
    page = &pageBlock[0];

    for (i = 0; i < BOOT_FLASH_PAGES_NUMBER; i++) {
        if (SEFC0_PageWrite((uint32_t *) page, dstAddr) == false) {
            return 0;
        }

        while (SEFC0_IsBusy()) {
            ;
        }

        page += BOOT_FLASH_PAGE_SIZE;
        dstAddr += BOOT_FLASH_PAGE_SIZE;
    }

    return 1;
}

static uint8_t lAPP_BOOTLOADER_VerifyPage(uint8_t *ramPage,
        uint8_t *flashPage,
        uint16_t pageSize) {
    uint16_t i = 0;

    while (i < pageSize) {
        if (ramPage[i] != flashPage[i]) {
            return 0;
        }

        i++;
    }

    return 1;
}

static uint8_t lAPP_BOOTLOADER_SwapFwVersion(uint32_t imgSize,
        uint32_t fuBaseAddress,
        uint32_t appBaseAddress) {
    uint32_t bufferAddr;
    uint32_t pageOffset = 0;
    uint32_t temp;
    uint8_t pagesNumber;
    uint8_t i;
    uint8_t init;

    /* Temporary buffer of 8 pages */
    bufferAddr = BOOT_BUFFER_ADDR;

    /* Number of page blocks */
    temp = imgSize / BOOT_FLASH_16PAGE_SIZE;
    pagesNumber = (uint8_t) temp;
    if (imgSize % BOOT_FLASH_16PAGE_SIZE) {
        pagesNumber++;
    }

    /* Start at the last counter position */
    init = pagesCounter;
    for (i = init; i < pagesNumber; i++) {
        /* Set page offset */
        temp = (uint32_t) i;
        pageOffset = temp * BOOT_FLASH_16PAGE_SIZE;

        /* Check state */
        if ((bootState == BOOT_IDLE) || (bootState == BOOT_COPIED_BUFF_TO_APP)) {
            /* Delete temporary buffer */
            if (lAPP_BOOTLOADER_DeletePage(bufferAddr) == 0U) {
                return 0;
            }

            /* Copy FU into buffer */
            if (lAPP_BOOTLOADER_CopyPage(fuBaseAddress + pageOffset,
                    bufferAddr) == 0U) {
                return 0;
            }

            /* Verify the written data */
            /* If not successfully verified, repeat the page writing process */
            if (lAPP_BOOTLOADER_VerifyPage(pageBlock, (uint8_t *) bufferAddr,
                    BOOT_FLASH_16PAGE_SIZE) == 0U) {
                i--;
                continue;
            }

            /* Set new state */
            bootState = BOOT_COPIED_FU_TO_BUFF;

            /* Update user signature */
            lAPP_BOOTLOADER_UpdateUserSignature(pagesCounter, bootState);
        }

        /* Check state */
        if (bootState == BOOT_COPIED_FU_TO_BUFF) {
            /* Delete FU */
            if (lAPP_BOOTLOADER_DeletePage(fuBaseAddress + pageOffset) == 0U) {
                return 0;
            }

            /* Copy application into FU */
            if (lAPP_BOOTLOADER_CopyPage(appBaseAddress + pageOffset,
                    fuBaseAddress + pageOffset) == 0U) {
                return 0;
            }

            /* Verify the written data */
            /* If not successfully verified, repeat the page writing process */
            if (lAPP_BOOTLOADER_VerifyPage(pageBlock,
                    (uint8_t *) (fuBaseAddress + pageOffset),
                    BOOT_FLASH_16PAGE_SIZE) == 0U) {
                i--;
                continue;
            }

            /* Set new state */
            bootState = BOOT_COPIED_APP_TO_FU;

            /* Update user signature */
            lAPP_BOOTLOADER_UpdateUserSignature(pagesCounter, bootState);
        }

        /* Check state */
        if (bootState == BOOT_COPIED_APP_TO_FU) {
            /* Delete application */
            if (lAPP_BOOTLOADER_DeletePage(appBaseAddress + pageOffset) == 0U) {
                return 0;
            }

            /* Copy buffer into application */
            if (lAPP_BOOTLOADER_CopyPage(bufferAddr, appBaseAddress + pageOffset) == 0U) {
                return 0;
            }

            /* Verify the written data */
            /* If not successfully verified, repeat the page writing process */
            if (lAPP_BOOTLOADER_VerifyPage(pageBlock,
                    (uint8_t *) (appBaseAddress + pageOffset),
                    BOOT_FLASH_16PAGE_SIZE) == 0U) {
                i--;
                continue;
            }

            /* Set new state */
            bootState = BOOT_COPIED_BUFF_TO_APP;

            /* Increase counter */
            ++pagesCounter;

            /* Update user signature */
            lAPP_BOOTLOADER_UpdateUserSignature(pagesCounter, bootState);
        }
    }

    return 1;
}
// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_BOOTLOADER_Initialize ( void )

  Remarks:
    See prototype in app_bootloader.h.
 */

void APP_BOOTLOADER_Initialize(void) {
    /* Place the App state machine in its initial state. */
    app_bootloaderData.state = APP_BOOTLOADER_STATE_INIT;
}

/******************************************************************************
  Function:
    void APP_BOOTLOADER_Tasks ( void )

  Remarks:
    See prototype in app_bootloader.h.
 */

void APP_BOOTLOADER_Tasks(void) {
    uint32_t imageSize;
    uint32_t origAddr;
    uint32_t destAddr;
    uint32_t cfgKey;
    uint8_t i;

    /* Check the application's current state. */
    switch (app_bootloaderData.state) {
            /* Application's initial state. */
        case APP_BOOTLOADER_STATE_INIT:
        {
            bool appInitialized = true;

            if (appInitialized) {
                /* Ensure all priority bits are assigned as preemption */
                /* priority bits. */
                NVIC_SetPriorityGrouping(__NVIC_PRIO_BITS);
                __set_BASEPRI(0);

                /* Disable User Signature write protection */
                SEFC0_WriteProtectionSet(0);

                /* Enable write and read User Signature rights */
                /* (block 0 / area 1)  */
                SEFC0_UserSignatureRightsSet(
                        SEFC_EEFC_USR_RDENUSB1_Msk | SEFC_EEFC_USR_WRENUSB1_Msk);

                app_bootloaderData.state = APP_BOOTLOADER_STATE_SERVICE_TASKS;
            }

            break;
        }

        case APP_BOOTLOADER_STATE_SERVICE_TASKS:
        {
            /* Read the boot configuration from user signature */
            (void) SEFC0_UserSignatureRead((void*) userSignBuf,
                    (uint32_t) BOOT_USER_SIGNATURE_SIZE_64,
                    (SEFC_USERSIGNATURE_BLOCK) BOOT_USER_SIGNATURE_BLOCK,
                    (SEFC_USERSIGNATURE_PAGE) BOOT_USER_SIGNATURE_PAGE);

            /* Get boot configuration */
            bootConfig = userSignBuf + BOOT_CONFIG_OFFSET_USER_SIGN;
            cfgKey = ((uint32_t) bootConfig[3]) << 24;
            cfgKey += ((uint32_t) bootConfig[2]) << 16;
            cfgKey += ((uint32_t) bootConfig[1]) << 8;
            cfgKey += (uint32_t) bootConfig[0];

            /* Check configuration key */
            if (cfgKey != BOOT_CONFIG_KEY) {
                /* Clear boot configuration */
                (void) memset(bootConfig, 0, 16);
                /* Set configuration key */
                bootConfig[0] = (uint8_t) (BOOT_CONFIG_KEY);
                bootConfig[1] = (uint8_t) ((BOOT_CONFIG_KEY >> 8) & 0xFF);
                bootConfig[2] = (uint8_t) ((BOOT_CONFIG_KEY >> 16) & 0xFF);
                bootConfig[3] = (uint8_t) ((BOOT_CONFIG_KEY >> 24) & 0xFF);
                /* Update user signature */
                lAPP_BOOTLOADER_UpdateUserSignature(0, BOOT_IDLE);
            }
            else {
                /* Get boot information */
                imageSize = (uint32_t) (bootConfig[7]) << 24;
                imageSize += (uint32_t) (bootConfig[6]) << 16;
                imageSize += (uint32_t) (bootConfig[5]) << 8;
                imageSize += (uint32_t) (bootConfig[4]);
                origAddr = (uint32_t) (bootConfig[11]) << 24;
                origAddr += (uint32_t) (bootConfig[10]) << 16;
                origAddr += (uint32_t) (bootConfig[9]) << 8;
                origAddr += (uint32_t) (bootConfig[8]);
                destAddr = (uint32_t) (bootConfig[15]) << 24;
                destAddr += (uint32_t) (bootConfig[14]) << 16;
                destAddr += (uint32_t) (bootConfig[13]) << 8;
                destAddr += (uint32_t) (bootConfig[12]);
                pagesCounter = bootConfig[16];
                bootState = bootConfig[17];

                /* Check if swap fw is needed. If not, load defaults. */
                if (lAPP_BOOTLOADER_IsSwapCmd(imageSize, origAddr,
                        destAddr) == true) {
                    /* Swap fw */
                    (void) lAPP_BOOTLOADER_SwapFwVersion(imageSize,
                            origAddr,
                            destAddr);

                    /* Clear boot configuration (leave configuration key) */
                    (void) memset(&bootConfig[4], 0, 16);

                    /* Update user signature */
                    lAPP_BOOTLOADER_UpdateUserSignature(0, BOOT_IDLE);
                }
            }

            __disable_irq();

            /* Disable SysTick */
            SysTick->CTRL = 0;

            /* Disable IRQs & clear pending IRQs */
            for (i = 0; i < 8; i++) {
                NVIC->ICER[i] = 0xFFFFFFFF;
                NVIC->ICPR[i] = 0xFFFFFFFF;
            }

            /* Modify vector table location */
            __DSB();
            __ISB();

            /* Set the stack pointer to the start of the firmware application */
            __set_MSP(*(int *) (BOOT_FLASH_APP_FIRMWARE_START_ADDRESS));

            /* Offset the start of the vector table (first 6 bits must be 
             * zero) */
            /* The register containing the offset, from 0x00000000, is at 
             * 0xE000ED08 */
            SCB->VTOR = ((uint32_t) BOOT_FLASH_APP_FIRMWARE_START_ADDRESS &
                    SCB_VTOR_TBLOFF_Msk);
            __DSB();
            __ISB();
            __enable_irq();
            /* * (int *) 0xE000ED08 = FIRMWARE_START_ADDRESS; */

            /* Jump to the start of the firmware, casting the address as 
             * function pointer to the start of the firmware. We want to jump 
             * to the address of the reset. */

            /* Handler function, that is the value that is being pointed at 
             * position FIRMWARE_RESET_ADDRESS */

            void (*runFirmware)(void) = NULL;
            runFirmware = (void (*)(void))(*(uint32_t *)
                    BOOT_FLASH_APP_FIRMWARE_RESET_ADDRESS);
            runFirmware();

            while (1) {
                ;
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
