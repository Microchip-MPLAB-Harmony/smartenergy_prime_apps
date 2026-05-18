/*******************************************************************************
  Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    Entry point of the bare-metal SAMD20 bootloader.

  Description:
    The reset handler in startup_samd20_bootloader.c runs first after
    reset: it sets up the stack, zero-initializes .bss, copies .data from
    flash to RAM, and then calls main(). This main() simply forwards to
    APP_BOOTLOADER_Main(), which never returns.
*******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "app_bootloader.h"

// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************

int main(void)
{
    APP_BOOTLOADER_Main();

    /* APP_BOOTLOADER_Main is marked noreturn. This return statement is
     * only here to keep the C standard happy. */
    return 0;
}

/*******************************************************************************
 End of File
*/
