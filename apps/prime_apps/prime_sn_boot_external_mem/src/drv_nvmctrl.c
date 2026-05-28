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
    /* CTRLB layout matches the modem project's plib_nvmctrl_Initialize.
     * Earlier versions of this driver only set MANW + RWS, which left
     * READMODE and SLEEPPRM at their reset defaults; some SAMD20 silicon
     * is sensitive to the missing READMODE bits and the corruption only
     * shows up the SECOND time a row is programmed (the first install
     * after a chip-erase appears to work because virgin flash is 0xFF
     * and any write produces the right bytes regardless of erase
     * success).
     *
     *   MANW = 1                        manual page write trigger
     *   RWS  = 1                        one read wait state at 8 MHz
     *   READMODE = NO_MISS_PENALTY      Harmony default
     *   SLEEPPRM = WAKEONACCESS         Harmony default
     */
    NVMCTRL_REGS->NVMCTRL_CTRLB = NVMCTRL_CTRLB_READMODE_NO_MISS_PENALTY
                                | NVMCTRL_CTRLB_SLEEPPRM_WAKEONACCESS
                                | NVMCTRL_CTRLB_RWS(1UL)
                                | NVMCTRL_CTRLB_MANW_Msk;
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

void DRV_NVMCTRL_CacheInvalidate(void)
{
    /* CMD = INVALL (0x46) drops every cache line in NVMCTRL's flash cache.
     * No ADDR is needed; the command targets the whole cache. WaitReady
     * after issue, same pattern as every other CTRLA command in this
     * driver. */
    NVMCTRL_REGS->NVMCTRL_CTRLA = (uint16_t) (NVMCTRL_CTRLA_CMD_INVALL_Val
                                            | NVMCTRL_CTRLA_CMDEX_KEY);

    DRV_NVMCTRL_WaitReady();
}

DRV_NVMCTRL_ERROR DRV_NVMCTRL_GetError(void)
{
    uint16_t          status;
    DRV_NVMCTRL_ERROR result;

    status = NVMCTRL_REGS->NVMCTRL_STATUS;

    if ((status & NVMCTRL_STATUS_NVME_Msk) != 0U)
    {
        result = DRV_NVMCTRL_ERROR_NVM;
    }
    else if ((status & NVMCTRL_STATUS_LOCKE_Msk) != 0U)
    {
        result = DRV_NVMCTRL_ERROR_LOCK;
    }
    else if ((status & NVMCTRL_STATUS_PROGE_Msk) != 0U)
    {
        result = DRV_NVMCTRL_ERROR_PROG;
    }
    else
    {
        result = DRV_NVMCTRL_ERROR_NONE;
    }

    /* Clear all three error bits via W1C so the next call only sees
     * errors that occurred after this one. Mirrors how Harmony's
     * NVMCTRL_ErrorGet handles status. The companion INTFLAG.ERROR bit
     * is also W1C and gets cleared here for the same reason. */
    if (result != DRV_NVMCTRL_ERROR_NONE)
    {
        NVMCTRL_REGS->NVMCTRL_STATUS = (uint16_t) (NVMCTRL_STATUS_NVME_Msk
                                                | NVMCTRL_STATUS_LOCKE_Msk
                                                | NVMCTRL_STATUS_PROGE_Msk);
        NVMCTRL_REGS->NVMCTRL_INTFLAG = NVMCTRL_INTFLAG_ERROR_Msk;
    }

    return result;
}

/*******************************************************************************
 End of File
*/
