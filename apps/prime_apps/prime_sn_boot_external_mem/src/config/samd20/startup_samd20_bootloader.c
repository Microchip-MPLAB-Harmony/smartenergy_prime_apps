/*******************************************************************************
  Startup File for the SAMD20 Bootloader (Bare-metal)

  Company:
    Microchip Technology Inc.

  File Name:
    startup_samd20_bootloader.c

  Summary:
    Minimal startup code for the SAMD20J18 bootloader. Defines the interrupt
    vector table at flash offset 0x00000000 and the reset handler.

  Description:
    This startup does NOT use Harmony, the XC32 CRT, or any C library init.
    The reset handler performs the bare minimum work:
      1) Zero-initializes the .bss section.
      2) Copies the .data section from its flash load address (LMA) to RAM.
      3) Calls main().
      4) Loops forever if main() returns (which should not happen).

    The vector table holds valid addresses only for the initial stack
    pointer and the reset handler. All other entries point to
    Default_Handler, which loops forever: peripheral interrupts are never
    enabled by the bootloader, so reaching Default_Handler indicates a bug.

    See BOOTLOADER_FROM_RAM_DESIGN.md §4.1 for the design rationale.
*******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include <stdbool.h>

// *****************************************************************************
// *****************************************************************************
// Section: External Symbols Provided by the Linker Script
// *****************************************************************************
// *****************************************************************************

extern uint32_t _sbss;      /* start of .bss (VMA in RAM)             */
extern uint32_t _ebss;      /* end of .bss (exclusive)                */
extern uint32_t _sdata;     /* start of .data (VMA in RAM)            */
extern uint32_t _edata;     /* end of .data (exclusive)               */
extern uint32_t _sidata;    /* start of .data (LMA in flash, source)  */
extern uint32_t _estack;    /* top of stack (initial MSP value)       */

// *****************************************************************************
// *****************************************************************************
// Section: Forward Declarations
// *****************************************************************************
// *****************************************************************************

extern int main(void);

void Reset_Handler(void);
void Default_Handler(void);

// *****************************************************************************
// *****************************************************************************
// Section: Default Interrupt Handler
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void Default_Handler ( void )

  Summary:
    Catch-all handler for every interrupt not explicitly implemented.

  Description:
    The bootloader never enables peripheral interrupts, therefore reaching
    this handler indicates a firmware bug or a spurious hardware event.
    Looping here allows a connected debugger to observe the program
    counter; on a release build the watchdog will trigger a reset.
*/

void Default_Handler(void)
{
    while (true)
    {
        /* intentional infinite loop */
    }
}

// *****************************************************************************
// *****************************************************************************
// Section: Interrupt Vector Table
//
// Cortex-M0+ requires the first two words to be:
//   [0] = Initial MSP value (top of stack)
//   [1] = Reset handler address
//
// The remaining 14 ARM system vectors + 25 SAMD20 peripheral vectors all
// point to Default_Handler. The whole table is 176 B; we align it to 256 B
// because SCB->VTOR requires that alignment on this implementation.
// *****************************************************************************
// *****************************************************************************

__attribute__((section(".vectors"), aligned(256), used))
const void *const vectors[] =
{
    /* Cortex-M0+ system vectors */
    (const void *) &_estack,            /*  0: initial MSP      */
    (const void *) Reset_Handler,       /*  1: Reset            */
    (const void *) Default_Handler,     /*  2: NMI              */
    (const void *) Default_Handler,     /*  3: HardFault        */
    (const void *) 0,                   /*  4-10: reserved      */
    (const void *) 0,
    (const void *) 0,
    (const void *) 0,
    (const void *) 0,
    (const void *) 0,
    (const void *) 0,
    (const void *) Default_Handler,     /* 11: SVCall           */
    (const void *) 0,                   /* 12-13: reserved      */
    (const void *) 0,
    (const void *) Default_Handler,     /* 14: PendSV           */
    (const void *) Default_Handler,     /* 15: SysTick          */

    /* SAMD20 peripheral vectors 0-24 */
    (const void *) Default_Handler,     /*  0: PM               */
    (const void *) Default_Handler,     /*  1: SYSCTRL          */
    (const void *) Default_Handler,     /*  2: WDT              */
    (const void *) Default_Handler,     /*  3: RTC              */
    (const void *) Default_Handler,     /*  4: EIC              */
    (const void *) Default_Handler,     /*  5: NVMCTRL          */
    (const void *) Default_Handler,     /*  6: EVSYS            */
    (const void *) Default_Handler,     /*  7: SERCOM0          */
    (const void *) Default_Handler,     /*  8: SERCOM1          */
    (const void *) Default_Handler,     /*  9: SERCOM2          */
    (const void *) Default_Handler,     /* 10: SERCOM3          */
    (const void *) Default_Handler,     /* 11: SERCOM4          */
    (const void *) Default_Handler,     /* 12: SERCOM5          */
    (const void *) Default_Handler,     /* 13: TC0              */
    (const void *) Default_Handler,     /* 14: TC1              */
    (const void *) Default_Handler,     /* 15: TC2              */
    (const void *) Default_Handler,     /* 16: TC3              */
    (const void *) Default_Handler,     /* 17: TC4              */
    (const void *) Default_Handler,     /* 18: TC5              */
    (const void *) Default_Handler,     /* 19: TC6              */
    (const void *) Default_Handler,     /* 20: TC7              */
    (const void *) Default_Handler,     /* 21: ADC              */
    (const void *) Default_Handler,     /* 22: AC               */
    (const void *) Default_Handler,     /* 23: DAC              */
    (const void *) Default_Handler,     /* 24: PTC              */
};

// *****************************************************************************
// *****************************************************************************
// Section: Reset Handler
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void Reset_Handler ( void )

  Summary:
    First code to execute after power-on or reset.

  Description:
    Prepares the C runtime environment (zeros .bss, copies .data from
    flash to RAM) and then calls main(). If main() returns, loops forever.

  Parameters:
    None.

  Returns:
    None.
*/

void Reset_Handler(void)
{
    uint32_t *pBss;
    uint32_t *pData;
    const uint32_t *pInitData;

    /* Zero-initialize the .bss section. */
    pBss = &_sbss;
    while (pBss < &_ebss)
    {
        *pBss = 0U;
        pBss++;
    }

    /* Copy the .data section from flash (LMA) to RAM (VMA). For the
     * bootloader .data is normally empty, but we handle it for
     * completeness and future-proofing. */
    pData     = &_sdata;
    pInitData = &_sidata;
    while (pData < &_edata)
    {
        *pData = *pInitData;
        pData++;
        pInitData++;
    }

    (void) main();

    /* main() should not return; loop forever if it does. */
    while (true)
    {
        /* intentional infinite loop */
    }
}

/*******************************************************************************
 End of File
*/
