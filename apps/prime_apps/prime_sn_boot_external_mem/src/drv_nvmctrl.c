/*******************************************************************************
  NVMCTRL Driver Implementation File

  Company:
    Microchip Technology Inc.

  File Name:
    drv_nvmctrl.c

  Summary:
    Bare-metal driver for the SAMD20 internal flash controller.

  Description:
    See drv_nvmctrl.h for the rationale behind each function. Register
    access follows the same convention used in the modem project's
    plib_nvmctrl.c: writes to the CTRLA command register always OR in the
    NVMCTRL_CTRLA_CMDEX_KEY value (0xA5) that the controller requires to
    actually execute the command; the ADDR register is loaded with a
    halfword index (address >> 1) instead of a byte address.
*******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "drv_nvmctrl.h"
#include "device.h"

// *****************************************************************************
// *****************************************************************************
// Section: Public Functions
// *****************************************************************************
// *****************************************************************************

void DRV_NVMCTRL_Initialize(void)
{
    /* MANW = 1 so the write is triggered explicitly by the WP command
     * (matching how the modem project's srv_storage drives NVMCTRL). One
     * read wait state keeps us safe even if CPU frequency ever climbs. */
    NVMCTRL_REGS->NVMCTRL_CTRLB = NVMCTRL_CTRLB_MANW_Msk
                                | NVMCTRL_CTRLB_RWS(1UL);
}

void DRV_NVMCTRL_RegionUnlock(uint32_t address)
{
    NVMCTRL_REGS->NVMCTRL_ADDR = address >> 1U;
    NVMCTRL_REGS->NVMCTRL_CTRLA = (uint16_t) (NVMCTRL_CTRLA_CMD_UR_Val
                                            | NVMCTRL_CTRLA_CMDEX_KEY);

    DRV_NVMCTRL_WaitReady();
}

void DRV_NVMCTRL_RowErase(uint32_t address)
{
    NVMCTRL_REGS->NVMCTRL_ADDR = address >> 1U;
    NVMCTRL_REGS->NVMCTRL_CTRLA = (uint16_t) (NVMCTRL_CTRLA_CMD_ER_Val
                                            | NVMCTRL_CTRLA_CMDEX_KEY);

    DRV_NVMCTRL_WaitReady();
}

void DRV_NVMCTRL_PageWrite(const uint32_t *data, uint32_t address)
{
    uint32_t *pDest;
    uint32_t i;

    /* Fill the page buffer by writing 16 words directly to the target
     * address. No flash cycle is triggered yet because MANW = 1. */
    pDest = (uint32_t *) address;
    for (i = 0U; i < (DRV_NVMCTRL_PAGE_SIZE / 4U); i++)
    {
        pDest[i] = data[i];
    }

    /* Trigger the write. */
    NVMCTRL_REGS->NVMCTRL_ADDR = address >> 1U;
    NVMCTRL_REGS->NVMCTRL_CTRLA = (uint16_t) (NVMCTRL_CTRLA_CMD_WP_Val
                                            | NVMCTRL_CTRLA_CMDEX_KEY);

    DRV_NVMCTRL_WaitReady();
}

bool DRV_NVMCTRL_IsBusy(void)
{
    return ((NVMCTRL_REGS->NVMCTRL_INTFLAG & NVMCTRL_INTFLAG_READY_Msk)
            != NVMCTRL_INTFLAG_READY_Msk);
}

void DRV_NVMCTRL_WaitReady(void)
{
    while (DRV_NVMCTRL_IsBusy())
    {
        /* spin */
    }
}

/*******************************************************************************
 End of File
*/
