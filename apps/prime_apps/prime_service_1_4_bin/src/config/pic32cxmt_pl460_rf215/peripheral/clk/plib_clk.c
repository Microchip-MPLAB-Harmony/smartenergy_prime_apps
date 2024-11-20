/*******************************************************************************
* Copyright (C) 2022 Microchip Technology Inc. and its subsidiaries.
*
* Subject to your compliance with these terms, you may use Microchip software
* and any derivatives exclusively with Microchip products. It is your
* responsibility to comply with third party license terms applicable to your
* use of third party software (including open source software) that may
* accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
* EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
* WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
* PARTICULAR PURPOSE.
*
* IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
* INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
* WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
* BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
* FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
* ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
*******************************************************************************/
#include <stdbool.h>
#include "plib_clk.h"
#include "peripheral/rstc/plib_rstc.h"

#define PLLA_RECOMMENDED_ACR    0x0F020038U
#define PLLB_RECOMMENDED_ACR    0x28020038U
#define PLLC_RECOMMENDED_ACR    0x28020038U

#define PLLA_UPDT_STUPTIM_VAL   0x02U
#define PLLB_UPDT_STUPTIM_VAL   0x00U
#define PLLC_UPDT_STUPTIM_VAL   0x00U

#define NOP()    asm("NOP")

typedef struct pmc_pll_cfg_tag
{
    uint32_t ctrl0;
    uint32_t ctrl1;
    uint32_t ctrl2;
    uint32_t ssr;
    uint32_t acr;
    uint32_t stuptim;
}pmc_pll_cfg_t;






/*********************************************************************************
                            Initialize Slow Clock (SLCK)
*********************************************************************************/
static void SlowClockInitialize(void)
{
    /* 32KHz Crystal Oscillator is selected as the Slow Clock (SLCK) source.
       Enable 32KHz Crystal Oscillator  */
    SUPC_REGS->SUPC_CR = SUPC_CR_KEY_PASSWD | SUPC_CR_TDXTALSEL_XTAL;

    /* Wait until Slow Clock (SLCK) is switched from RC */
    while ((SUPC_REGS->SUPC_SR & SUPC_SR_TDOSCSEL_XTAL) == 0U)
    {
        /* Wait for status to set */
    }
}

void CLK_TDSCLKSelectXTAL(void)
{
    /* 32KHz Crystal Oscillator is selected as the Slow Clock (SLCK) source.
    Enable 32KHz Crystal Oscillator  */

    SUPC_REGS->SUPC_CR = SUPC_CR_KEY_PASSWD | SUPC_CR_TDXTALSEL_XTAL;

    /* Wait until Slow Clock (SLCK) is switched from RC */
    while ((SUPC_REGS->SUPC_SR & SUPC_SR_TDOSCSEL_XTAL) == 0U)
    {
        /* Wait for status to set */
    }
}



void CLK_EnableMainRCOscillator(void)
{
    /* Enable the RC Oscillator */
    PMC_REGS->CKGR_MOR = PMC_REGS->CKGR_MOR | (CKGR_MOR_KEY_PASSWD | CKGR_MOR_MOSCRCEN_Msk);

    while ((PMC_REGS->PMC_SR & PMC_SR_MOSCRCS_Msk) != PMC_SR_MOSCRCS_Msk)
    {
        /* Wait for the RC Oscillator to stabilize */
    }
}

void CLK_DisableMainRCOscillator(void)
{
    /* Disable the RC Oscillator */
    PMC_REGS->CKGR_MOR = CKGR_MOR_KEY_PASSWD | (PMC_REGS->CKGR_MOR & ~CKGR_MOR_MOSCRCEN_Msk);
}

void CLK_EnableMainXTALOscillator(void)
{
    /* Enable Main Crystal Oscillator */
    PMC_REGS->CKGR_MOR = (PMC_REGS->CKGR_MOR & ~CKGR_MOR_MOSCXTST_Msk) | CKGR_MOR_KEY_PASSWD | CKGR_MOR_MOSCXTST(0U) | CKGR_MOR_MOSCXTEN_Msk;

    while ((PMC_REGS->PMC_SR & PMC_SR_MOSCXTS_Msk) == 0U)
    {
        /* Wait until the main oscillator clock is ready */
    }
}

void CLK_DisableMainXTALOscillator(void)
{
    /* Disable the XTAL Oscillator */
    PMC_REGS->CKGR_MOR = CKGR_MOR_KEY_PASSWD | (PMC_REGS->CKGR_MOR & ~CKGR_MOR_MOSCXTEN_Msk);
}

void CLK_MainOscillatorSelectXTAL(void)
{
    /* Main Crystal Oscillator is selected as the Main Clock (MAINCK) source.
    Switch Main Clock (MAINCK) to Main Crystal Oscillator clock */

    PMC_REGS->CKGR_MOR = (PMC_REGS->CKGR_MOR | CKGR_MOR_MOSCSEL_Msk) | CKGR_MOR_KEY_PASSWD;
    while ((PMC_REGS->PMC_SR & PMC_SR_MOSCSELS_Msk) != PMC_SR_MOSCSELS_Msk)
    {
        /* Wait until MAINCK is switched to Main Crystal Oscillator */
    }
}

void CLK_MainOscillatorSelectRC(void)
{
    /* RC Oscillator is selected as the Main Clock (MAINCK) source.
    Switch Main Clock (MAINCK) to RC Oscillator clock */

    PMC_REGS->CKGR_MOR = (PMC_REGS->CKGR_MOR & ~CKGR_MOR_MOSCSEL_Msk) | CKGR_MOR_KEY_PASSWD;
    while ((PMC_REGS->PMC_SR & PMC_SR_MOSCSELS_Msk) != PMC_SR_MOSCSELS_Msk)
    {
        /* Wait until MAINCK is switched to RC Oscillator */
    }
}


/*********************************************************************************
                                Initialize CPU clock
*********************************************************************************/
static void CPUClockInitialize(void)
{
    /* Program PMC_CPU_CKR.PRES and wait for PMC_SR.MCKRDY to be set   */
    uint32_t reg = (PMC_REGS->PMC_CPU_CKR & ~PMC_CPU_CKR_PRES_Msk);
    PMC_REGS->PMC_CPU_CKR = (reg | PMC_CPU_CKR_PRES_CLK_1);
    while ((PMC_REGS->PMC_SR & PMC_SR_MCKRDY_Msk) != PMC_SR_MCKRDY_Msk)
    {
        /* Wait for status MCKRDY */
    }

    /* Program PMC_CPU_CKR.CSS and MCK dividers and Wait for PMC_SR.MCKRDY to be set    */
    reg = (PMC_REGS->PMC_CPU_CKR & ~(PMC_CPU_CKR_CSS_Msk |
                                     PMC_CPU_CKR_RATIO_MCK0DIV_Msk |
                                     PMC_CPU_CKR_RATIO_MCK0DIV2_Msk));
    reg |= (PMC_CPU_CKR_CSS_PLLACK1 | PMC_CPU_CKR_RATIO_MCK0DIV_Msk );
    PMC_REGS->PMC_CPU_CKR = reg;
    while ((PMC_REGS->PMC_SR & PMC_SR_MCKRDY_Msk) != PMC_SR_MCKRDY_Msk)
    {
        /* Wait for status MCKRDY */
    }

    /* Set coprocessor clock dummy prescaler */
    reg = (PMC_REGS->PMC_CPU_CKR & ~(PMC_CPU_CKR_CPPRES_Msk | PMC_CPU_CKR_RATIO_MCK1DIV_Msk));
    reg |= (PMC_CPU_CKR_CPPRES_CLK_2 | PMC_CPU_CKR_RATIO_MCK1DIV_Msk);
    PMC_REGS->PMC_CPU_CKR = reg;

    /* Program PMC_CPU_CKR.CPCSS and Wait for PMC_SR.CPMCKRDY to be set    */
    reg = (PMC_REGS->PMC_CPU_CKR & ~PMC_CPU_CKR_CPCSS_Msk);
    PMC_REGS->PMC_CPU_CKR = (reg | PMC_CPU_CKR_CPCSS_PLLBCK);
    while ((PMC_REGS->PMC_SR & PMC_SR_CPMCKRDY_Msk) != PMC_SR_CPMCKRDY_Msk)
    {
        /* Wait for status CPMCKRDY */
    }

    /* Program PMC_CPU_CKR.CPPRES and wait for PMC_SR.CPMCKRDY to be set   */
    reg = (PMC_REGS->PMC_CPU_CKR & ~(PMC_CPU_CKR_CPPRES_Msk | PMC_CPU_CKR_RATIO_MCK1DIV_Msk));
    reg |= (PMC_CPU_CKR_CPPRES_CLK_1 | PMC_CPU_CKR_RATIO_MCK1DIV_Msk);
    PMC_REGS->PMC_CPU_CKR = reg;
    while ((PMC_REGS->PMC_SR & PMC_SR_CPMCKRDY_Msk) != PMC_SR_CPMCKRDY_Msk)
    {
        /* Wait for status CPMCKRDY */
    }

    /* Enable co-processor bus clock  */
    PMC_REGS->PMC_SCER = (PMC_SCER_CPKEY_PASSWD | PMC_SCER_CPBMCK_Msk);
}




void CLK_Core1BusMasterClkEnable(void)
{
    PMC_REGS->PMC_SCER = (PMC_SCER_CPKEY_PASSWD | PMC_SCER_CPBMCK_Msk);

    while ((PMC_REGS->PMC_SR & PMC_SR_CPMCKRDY_Msk) != PMC_SR_CPMCKRDY_Msk)
    {
        /* Wait for status CPMCKRDY */
    }
}

void CLK_Core1BusMasterClkDisable(void)
{
    PMC_REGS->PMC_SCDR = (PMC_SCDR_CPKEY_PASSWD | PMC_SCDR_CPBMCK_Msk);
}


/*********************************************************************************
                    Enable/Disable flash patch based on core frequency
*********************************************************************************/
static void CLK_ApplyFlashPatch(uint32_t cpuCLKFreq)
{
    SFR_REGS->SFR_WPMR = SFR_WPMR_WPKEY_PASSWD;

    if (cpuCLKFreq >= 160000000U)
    {
        /*Enable Flash high speed patch */
        SFR_REGS->SFR_FLASH = 0x0U;
    }
    else
    {
        /*Disable Flash high speed patch */;
        SFR_REGS->SFR_FLASH = SFR_FLASH_Msk;
    }

    SFR_REGS->SFR_WPMR = (SFR_WPMR_WPKEY_PASSWD | SFR_WPMR_WPEN_Msk);
}


/*********************************************************************************
                        Check Peripheral clock status
*********************************************************************************/
static bool PeripheralClockStatus(uint32_t periph_id)
{
    bool retval = false;
    uint32_t status = 0U;
    const uint32_t csr_offset[] = { PMC_CSR0_REG_OFST,
                                    PMC_CSR1_REG_OFST,
                                    PMC_CSR2_REG_OFST,
                                    PMC_CSR3_REG_OFST
                                    };
    uint32_t index = periph_id / 32U;
    if (index < (sizeof(csr_offset)/sizeof(csr_offset[0])))
    {
        status = (*(volatile uint32_t* const)((PMC_BASE_ADDRESS +
                                                        csr_offset[index])));
        retval = ((status & ((uint32_t)1U << (periph_id % 32U))) != 0U);
    }
    return retval;
}
/*********************************************************************************
                        Initialize Peripheral clocks
*********************************************************************************/
static void PeripheralClockInitialize(void)
{
    struct {
        uint8_t id;
        uint8_t clken;
        uint8_t gclken;
        uint8_t css;
        uint8_t divs;
    } periphList[] =
    {
        { ID_PERIPH_MAX + 1, 0, 0, 0, 0}//end of list marker
    };
    uint32_t count = sizeof(periphList)/sizeof(periphList[0]);
    uint32_t i = 0U;

    while((i < count) && (periphList[i].id != ((uint32_t)ID_PERIPH_MAX + 1U)))
    {
        PMC_REGS->PMC_PCR = PMC_PCR_CMD_Msk |\
                            PMC_PCR_GCLKEN(periphList[i].gclken) |\
                            PMC_PCR_EN(periphList[i].clken) |\
                            PMC_PCR_GCLKDIV(periphList[i].divs) |\
                            PMC_PCR_GCLKCSS(periphList[i].css) |\
                            PMC_PCR_PID(periphList[i].id);
        while(PeripheralClockStatus(periphList[i].id) == false)
        {
            /* Wait for clock to be initialized */
        }
        i++;
    }
}

/*********************************************************************************
                                Clock Initialize
*********************************************************************************/
void CLK_Initialize( void )
{
    if(RSTC_PMCResetStatusGet())
    {
        /* Initialize TD slow clock */
        SlowClockInitialize();

        /* Apply flash patch */
        CLK_ApplyFlashPatch(0);

        /* Initialize CPU clock */
        CPUClockInitialize();

        /* Initialize Peripheral clock */
        PeripheralClockInitialize();

        /* Disable Main RC Oscillator */
        CLK_DisableMainRCOscillator();
    }
    else
    {
        /* Apply flash patch */
        CLK_ApplyFlashPatch(0);
    }
}