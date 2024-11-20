/*******************************************************************************
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    app_bootloader.h

  Summary:
    This header file provides prototypes and definitions for the application.

  Description:
    This header file provides function prototypes and data type definitions for
    the application.  Some of these are required by the system (such as the
    "APP_BOOTLOADER_Initialize" and "APP_BOOTLOADER_Tasks" prototypes) and some of them are only used
    internally by the application (such as the "APP_BOOTLOADER_STATES" definition).  Both
    are defined here for convenience.
*******************************************************************************/

#ifndef _APP_BOOTLOADER_H
#define _APP_BOOTLOADER_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
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

/* PRIME Bootloader version */
#define PRBO_VERSION                                 "ATMBOOTPIC32CX0202"
    
/* Bootloader address */
#define BOOT_START_ADDRESS                            IFLASH0_ADDR

/* Bootloader configuration */
#define BOOT_CONFIG_OFFSET_USER_SIGN                  112
#define BOOT_CONFIG_KEY                               0x55AA55AA
#define BOOT_BUFFER_ADDR                              0x0100E000

/* Firmware configuration */
#define BOOT_FLASH_PAGE_SIZE                          IFLASH0_PAGE_SIZE
#define BOOT_FLASH_PAGES_PER_SECTOR                   128
#define BOOT_FLASH_SECTOR_SIZE                        (BOOT_FLASH_PAGES_PER_SECTOR * BOOT_FLASH_PAGE_SIZE)
#define BOOT_FLASH_16PAGE_SIZE                        (BOOT_FLASH_PAGE_SIZE << 4)
#define BOOT_FLASH_PAGES_NUMBER                       (BOOT_FLASH_16PAGE_SIZE / BOOT_FLASH_PAGE_SIZE)

/* Region configuration */
#define BOOT_FIRST_SECTOR_START_ADDRESS               (IFLASH0_ADDR + 0x00010000)
#define BOOT_FLASH_APP_FIRMWARE_START_ADDRESS         BOOT_FIRST_SECTOR_START_ADDRESS
#define BOOT_FLASH_APP_FIRMWARE_RESET_ADDRESS         (BOOT_FIRST_SECTOR_START_ADDRESS + 4)

/* User signature configuration */
#define BOOT_USER_SIGNATURE_BLOCK                     1    // BLOCK_0
#define BOOT_USER_SIGNATURE_PAGE                      0    // PAGE_0
#define BOOT_USER_SIGNATURE_SIZE_8                    BOOT_FLASH_PAGE_SIZE
#define BOOT_USER_SIGNATURE_SIZE_64                   (BOOT_FLASH_PAGE_SIZE / sizeof(uint64_t))
    
/* Bootloader states */
typedef enum {
    BOOT_IDLE,
    BOOT_COPIED_FU_TO_BUFF,
    BOOT_COPIED_APP_TO_FU,
    BOOT_COPIED_BUFF_TO_APP
} BOOT_STATE;
    
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
    APP_BOOTLOADER_STATE_INIT=0,
    APP_BOOTLOADER_STATE_SERVICE_TASKS,

} APP_BOOTLOADER_STATES;


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
    APP_BOOTLOADER_STATES state;

} APP_BOOTLOADER_DATA;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Routines
// *****************************************************************************
// *****************************************************************************
/* These routines are called by drivers when certain events occur.
*/

// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_BOOTLOADER_Initialize ( void )

  Summary:
     MPLAB Harmony application initialization routine.

  Description:
    This function initializes the Harmony application.  It places the
    application in its initial state and prepares it to run so that its
    APP_BOOTLOADER_Tasks function can be called.

  Precondition:
    All other system initialization routines should be called before calling
    this routine (in "SYS_Initialize").

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    APP_BOOTLOADER_Initialize();
    </code>

  Remarks:
    This routine must be called from the SYS_Initialize function.
*/

void APP_BOOTLOADER_Initialize ( void );


/*******************************************************************************
  Function:
    void APP_BOOTLOADER_Tasks ( void )

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
    APP_BOOTLOADER_Tasks();
    </code>

  Remarks:
    This routine must be called from SYS_Tasks() routine.
 */

void APP_BOOTLOADER_Tasks( void );

//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif /* _APP_BOOTLOADER_H */

/*******************************************************************************
 End of File
 */

