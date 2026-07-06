/*******************************************************************************
  NVMCTRL Driver Implementation File

  Company:
    Microchip Technology Inc.

  File Name:
    drv_nvmctrl.c

  Summary:
    Bare-metal driver for the SAMD20 internal flash controller.

  Description:
    Direct register-level driver: no Harmony. See drv_nvmctrl.h for the
    rationale behind each function. Register
    access follows the same convention used in the modem project's
    plib_nvmctrl.c: writes to the CTRLA command register always OR in the
    NVMCTRL_CTRLA_CMDEX_KEY value (0xA5) that the controller requires to
    actually execute the command; the ADDR register is loaded with a
    halfword index (address >> 1) instead of a byte address.
*******************************************************************************/

//DOM-IGNORE-BEGIN
/*
Copyright (C) 2026 Microchip Technology Inc., and its subsidiaries. All rights reserved.

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

#include "drv_nvmctrl.h"
#include "device.h"

// *****************************************************************************
// *****************************************************************************
// Section: Public Functions
// *****************************************************************************
// *****************************************************************************

void DRV_NVMCTRL_Initialize(void)
{
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

    /* MISRA C-2023 deviation block start */
    /* MISRA C-2023 Rule 11.4 deviated once. Deviation record ID - H3_MISRAC_2023_R_11_4_DR_1 */
    pDest = (uint32_t *) address;
    /* MISRA C-2023 deviation block end */
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
