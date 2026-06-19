/*******************************************************************************
  PRIME Reset Handler Service Implementation.

  Company:
    Microchip Technology Inc.

  File Name:
    srv_reset_handler.c

  Summary:
    Source code for the PRIME Reset Handle service implementation.

  Description:
    The Reset Handler service provides a simple interface to trigger system
    resets and to manage and store reset causes.This file contains the source
    code for the implementation of this service.
*******************************************************************************/

///DOM-IGNORE-BEGIN
/*
Copyright (C) 2026, Microchip Technology Inc., and its subsidiaries. All rights reserved.

The software and documentation is provided by microchip and its contributors
"as is" and any express, implied or statutory warranties, including, but not
limited to, the implied warranties of merchantability, fitness for a particular
purpose and non-infringement of third party intellectual property rights are
disclaimed to the fullest extent permitted by law. In no event shall microchip
or its contributors be liable for any direct, indirect, incidental, special,
exemplary, or consequential damages (including, but not limited to, procurement
of substitute goods or services; loss of use, data, or profits; or business
interruption) however caused and on any theory of liability, whether in contract,
strict liability, or tort (including negligence or otherwise) arising in any way
out of the use of the software and documentation, even if advised of the
possibility of such damage.

Except as expressly permitted hereunder and subject to the applicable license terms
for any third-party software incorporated in the software and any applicable open
source software license terms, no license or other rights, whether express or
implied, are granted under any patent or other intellectual property rights of
Microchip or any third party.
*/
//DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "definitions.h"
#include "srv_reset_handler.h"
#include "device.h"
#include "interrupts.h"
#include "service/storage/srv_storage.h"

// *****************************************************************************
// *****************************************************************************
// Section: Non-volatile data slot layout (stored in emulated EEPROM)
//
// Reset cause and fault context are persisted across resets through the
// non-volatile data slots provided by the storage service:
//   slot 5  : (reset_count << 16) | resetType
//   slot 6  : PC
//   slot 7  : LR
//   slot 8  : PSR
//   slot 9  : HFSR  (always 0 on Cortex-M0+)
//   slot 10 : CFSR  (always 0 on Cortex-M0+)
//   slot 11 : R0
//   slot 12 : R1
//   slot 13 : R2
//   slot 14 : R3
//   slot 15 : R12
// *****************************************************************************
// *****************************************************************************

#define RESET_INFO_SLOT  5U
#define DUMP_SLOT_BASE   5U    /* block covers slots 5..15 in one R-M-E-W */
#define DUMP_SLOT_COUNT 11U

/* MISRA deviation: these must be volatile globals so the debugger can inspect
 * them after reset if EEPROM readback is not available. */
volatile uint32_t saved_r0;
volatile uint32_t saved_r1;
volatile uint32_t saved_r2;
volatile uint32_t saved_r3;
volatile uint32_t saved_r12;
volatile uint32_t saved_lr;
volatile uint32_t saved_pc;
volatile uint32_t saved_psr;
volatile uint32_t saved_hfsr;   /* unused on M0+, kept for ABI compat */
volatile uint32_t saved_cfsr;   /* unused on M0+, kept for ABI compat */

/* Hardware reset cause latched at boot from PM_RCAUSE. Cached here (not written
 * to the storage slot) so SRV_RESET_HANDLER_Initialize() has NO dependency on
 * SRV_STORAGE_Initialize() -- PM_RCAUSE is readable straight out of reset.
 * Query it with SRV_RESET_HANDLER_GetResetCause(). */
static volatile SRV_RESET_HANDLER_RESET_CAUSE gBootResetCause = RESET_HANDLER_GENERAL_RESET;

// *****************************************************************************
// *****************************************************************************
// Section: File scope functions
// *****************************************************************************
// *****************************************************************************

static void lSRV_RESET_HANDLER_StoreResetInfo(SRV_RESET_HANDLER_RESET_CAUSE resetType)
{
    uint32_t resetInfo;
    uint16_t numResets;

    /* Read and increase number of resets since start-up */
    numResets = (uint16_t)(SRV_STORAGE_ReadNonVolatileData(RESET_INFO_SLOT) >> 16);
    ++numResets;

    /* Store reset information: high 16 bits = count, low 16 bits = cause */
    resetInfo = ((uint32_t)numResets << 16) | (uint32_t)resetType;
    SRV_STORAGE_WriteNonVolatileData(RESET_INFO_SLOT, resetInfo);
}

void DumpStack(uint32_t stack[]) __attribute__((noreturn));

void DumpStack(uint32_t stack[])
{
    uint32_t dump[DUMP_SLOT_COUNT];
    uint16_t numResets;

    saved_r0   = stack[0];
    saved_r1   = stack[1];
    saved_r2   = stack[2];
    saved_r3   = stack[3];
    saved_r12  = stack[4];
    saved_lr   = stack[5];
    saved_pc   = stack[6];
    saved_psr  = stack[7];
    saved_hfsr = 0U;            /* not available on Cortex-M0+ */
    saved_cfsr = 0U;            /* not available on Cortex-M0+ */

    numResets = (uint16_t)(SRV_STORAGE_ReadNonVolatileData(RESET_INFO_SLOT) >> 16);
    ++numResets;

    dump[0]  = ((uint32_t)numResets << 16) | (uint32_t)RESET_HANDLER_HARD_FAULT_RESET;
    dump[1]  = saved_pc;
    dump[2]  = saved_lr;
    dump[3]  = saved_psr;
    dump[4]  = saved_hfsr;
    dump[5]  = saved_cfsr;
    dump[6]  = saved_r0;
    dump[7]  = saved_r1;
    dump[8]  = saved_r2;
    dump[9]  = saved_r3;
    dump[10] = saved_r12;

    SRV_STORAGE_WriteBlockNonVolatileData(DUMP_SLOT_BASE, DUMP_SLOT_COUNT, dump);

    /* Fault is unrecoverable: reboot so the next boot can read the dump. */
    NVIC_SystemReset();
}

/* Cortex-M0+ lacks ITE (ARMv7-M only). We use conditional branches instead.
 * __attribute__((naked)) prevents prolog/epilog so we can read MSP/PSP
 * exactly as they were at the instant of the exception.
 * This overrides the weak stub in exceptions.c.
 *
 * Use BL instead of B for the jump to DumpStack: in ARMv6-M the `b` branch
 * is limited to +/- 2 KB (R_ARM_THM_JUMP11) while `bl` reaches +/- 16 MB. Since
 * DumpStack is noreturn, clobbering LR is harmless. */
__attribute__((naked, noreturn))
void HardFault_Handler(void)
{
    __asm volatile (
        "  movs r0, #4         \n"
        "  mov  r1, lr         \n"
        "  tst  r0, r1         \n"
        "  beq  1f             \n"    /* bit 2 clear -> MSP was in use */
        "  mrs  r0, psp        \n"
        "  bl   DumpStack      \n"
        "1:                    \n"
        "  mrs  r0, msp        \n"
        "  bl   DumpStack      \n"
    );
}

// *****************************************************************************
// *****************************************************************************
// Section: Reset Handler Service Interface Implementation
// *****************************************************************************
// *****************************************************************************

void SRV_RESET_HANDLER_Initialize(void)
{
    uint8_t rcause = (uint8_t) PM_REGS->PM_RCAUSE;
    SRV_RESET_HANDLER_RESET_CAUSE cause;

    /* Latch the hardware reset cause directly from PM_RCAUSE. This deliberately
     * does NOT touch the storage slot, so it has no ordering dependency on
     * SRV_STORAGE_Initialize() (unlike the PIC32, whose cause lives in the
     * always-available SUPC GPBR). A software reset -- used by
     * SRV_RESET_HANDLER_RestartSystem() and the fault DumpStack() path -- reads
     * back as SYST here; the finer software/fault cause is already in the
     * storage slot, written at runtime before that reset. */
    if ((rcause & PM_RCAUSE_WDT_Msk) != 0U)
    {
        cause = RESET_HANDLER_WATCHDOG_RESET;
    }
    else if ((rcause & PM_RCAUSE_SYST_Msk) != 0U)
    {
        cause = RESET_HANDLER_SOFTWARE_RESET;
    }
    else if ((rcause & PM_RCAUSE_EXT_Msk) != 0U)
    {
        cause = RESET_HANDLER_USER_RESET;
    }
    else
    {
        /* POR / BOD12 / BOD33: first power-on / general reset. */
        cause = RESET_HANDLER_GENERAL_RESET;
    }

    gBootResetCause = cause;
}

SRV_RESET_HANDLER_RESET_CAUSE SRV_RESET_HANDLER_GetResetCause(void)
{
    return gBootResetCause;
}

void SRV_RESET_HANDLER_RestartSystem(SRV_RESET_HANDLER_RESET_CAUSE resetType)
{
    /* Persist reset cause + increment reset counter */
    lSRV_RESET_HANDLER_StoreResetInfo(resetType);

    /* Trigger software reset */
    NVIC_SystemReset();
}
