/*******************************************************************************
  Metrology Driver.

  Company:
    Microchip Technology Inc.

  File Name:
    drv_metrology.c

  Summary:
    Metrology Driver source file.

  Description:
    None
*******************************************************************************/

//DOM-IGNORE-BEGIN
/*
Copyright (C) 2023, Microchip Technology Inc., and its subsidiaries. All rights reserved.

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
// Section: Include Files
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "system/int/sys_int.h"
#include "drv_metrology.h"
#include "drv_metrology_definitions.h"
#include "drv_metrology_local.h"
#include "peripheral/pio/plib_pio.h"
#include "peripheral/rstc/plib_rstc.h"
#include "peripheral/clk/plib_clk.h"
#include "interrupts.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Data
// *****************************************************************************
// *****************************************************************************

typedef enum {
    PENERGY = 0U,
    QENERGY = 1U,
} DRV_METROLOGY_ENERGY_TYPE;

/* This is the driver instance object array. */
static DRV_METROLOGY_OBJ gDrvMetObj;

static CACHE_ALIGN uint32_t sCaptureBuffer[CACHE_ALIGNED_SIZE_GET(MET_CAPTURE_BUF_SIZE)];

static const DRV_METROLOGY_REGS_CONTROL gDrvMetControlDefault =
{
    STATE_CTRL_STATE_CTRL_RESET_Val,                  /* 00 STATE_CTRL */
    (uint32_t)(DRV_METROLOGY_CONF_FCTRL),             /* 01 FEATURE_CTRL */
    (uint32_t)(DRV_METROLOGY_CONF_HARMONIC_CTRL),     /* 02 HARMONIC CTRL */
    (uint32_t)(DRV_METROLOGY_CONF_MT),                /* 03 METER_TYPE sensor_type =0 CT, 1 SHUNT, 2 ROGOWSKI */
    (uint32_t)(0x00000000UL),                         /* 04 M M=50->50Hz M=60->60Hz */
    (uint32_t)(0x00001130UL),                         /* 05 N_MAX 4400=0x1130 */
    (uint32_t)(DRV_METROLOGY_CONF_PULSE0_CTRL),       /* 06 PULSE0_CTRL */
    (uint32_t)(DRV_METROLOGY_CONF_PULSE1_CTRL),       /* 07 PULSE1_CTRL */
    (uint32_t)(DRV_METROLOGY_CONF_PULSE2_CTRL),       /* 08 PULSE2_CTRL */
    (uint32_t)(DRV_METROLOGY_CONF_PKT),               /* 09 P_K_T */
    (uint32_t)(DRV_METROLOGY_CONF_PKT),               /* 10 Q_K_T */
    (uint32_t)(DRV_METROLOGY_CONF_PKT),               /* 11 I_K_T */
    (uint32_t)(DRV_METROLOGY_CONF_PKT),               /* 12 S_K_T */
    (uint32_t)(DRV_METROLOGY_CONF_CREEP_P),           /* 13 CREEP_THR_P */
    (uint32_t)(DRV_METROLOGY_CONF_CREEP_Q),           /* 14 CREEP_THR_Q */
    (uint32_t)(DRV_METROLOGY_CONF_CREEP_I),           /* 15 CREEP_THR_I */
    (uint32_t)(DRV_METROLOGY_CONF_CREEP_S),           /* 16 CREEP_THR_S */
    (uint32_t)(0x00000000UL),                         /* 17 POWER_OFFSET_CTRL */
    (uint32_t)(0x00000000UL),                         /* 18 POWER_OFFSET_P */
    (uint32_t)(0x00000000UL),                         /* 19 POWER_OFFSET_Q */
    (uint32_t)(0x00000000UL),                         /* 20 POWER_OFFSET_S */
    (uint32_t)(DRV_METROLOGY_CONF_SWELL),             /* 21 SWELL_THR_VA */
    (uint32_t)(DRV_METROLOGY_CONF_SWELL),             /* 22 SWELL_THR_VB */
    (uint32_t)(DRV_METROLOGY_CONF_SWELL),             /* 23 SWELL_THR_VC */
    (uint32_t)(DRV_METROLOGY_CONF_SAG),               /* 24 SAG_THR_VA */
    (uint32_t)(DRV_METROLOGY_CONF_SAG),               /* 25 SAG_THR_VB */
    (uint32_t)(DRV_METROLOGY_CONF_SAG),               /* 26 SAG_THR_VC */
    (uint32_t)(DRV_METROLOGY_CONF_KI),                /* 27 K_IA */
    (uint32_t)(DRV_METROLOGY_CONF_KV),                /* 28 K_VA */
    (uint32_t)(DRV_METROLOGY_CONF_KI),                /* 29 K_IB */
    (uint32_t)(DRV_METROLOGY_CONF_KV),                /* 30 K_VB */
    (uint32_t)(DRV_METROLOGY_CONF_KI),                /* 31 K_IC */
    (uint32_t)(DRV_METROLOGY_CONF_KV),                /* 32 K_VC */
    (uint32_t)(DRV_METROLOGY_CONF_KI),                /* 33 K_IN */
    (uint32_t)(0x20000000UL),                         /* 34 CAL_M_IA */
    (uint32_t)(0x20000000UL),                         /* 35 CAL_M_VA */
    (uint32_t)(0x20000000UL),                         /* 36 CAL_M_IB */
    (uint32_t)(0x20000000UL),                         /* 37 CAL_M_VB */
    (uint32_t)(0x20000000UL),                         /* 38 CAL_M_IC */
    (uint32_t)(0x20000000UL),                         /* 39 CAL_M_VC */
    (uint32_t)(0x20000000UL),                         /* 40 CAL_M_IN */
    (uint32_t)(0x00000000UL),                         /* 41 CAL_PH_IA */
    (uint32_t)(0x00000000UL),                         /* 42 CAL_PH_VA */
    (uint32_t)(0x00000000UL),                         /* 43 CAL_PH_IB */
    (uint32_t)(0x00000000UL),                         /* 44 CAL_PH_VB */
    (uint32_t)(0x00000000UL),                         /* 45 CAL_PH_IC */
    (uint32_t)(0x00000000UL),                         /* 46 CAL_PH_VC */
    (uint32_t)(0x00000000UL),                         /* 47 CAL_PH_IN */
    (uint32_t)(DRV_METROLOGY_CONF_WAVEFORM),          /* 48 CAPTURE_CTRL */
    (uint32_t)(DRV_METROLOGY_CAPTURE_BUF_SIZE),       /* 49 CAPTURE_BUFF_SIZE */
    (uint32_t)(sCaptureBuffer),                       /* 50 CAPTURE_ADDR */
    (uint32_t)(0x00000000UL),                         /* 51 RESERVED_C48 */
    (uint32_t)(0x00000000UL),                         /* 52 RESERVED_C49 */
    (uint32_t)(0x00000000UL),                         /* 53 RESERVED_C50 */
    (uint32_t)(DRV_METROLOGY_CONF_ATS2023),           /* 54 ATSENSE_CTRL_20_23 */
    (uint32_t)(DRV_METROLOGY_CONF_ATS2427),           /* 55 ATSENSE_CTRL_24_27 */
    (uint32_t)(0x00000003UL),                         /* 56 ATSENSE_CTRL_28_2B: MSB_MODE=0,OSR=3 */
    (uint32_t)(0x00000000UL),                         /* 57 RESERVED_C54 */
    (uint32_t)(0x00000000UL),                         /* 58 POWER_OFFSET_P_A */
    (uint32_t)(0x00000000UL),                         /* 59 POWER_OFFSET_P_B */
    (uint32_t)(0x00000000UL),                         /* 60 POWER_OFFSET_P_C */
    (uint32_t)(0x00000000UL),                         /* 61 POWER_OFFSET_Q_A */
    (uint32_t)(0x00000000UL),                         /* 62 POWER_OFFSET_Q_B */
    (uint32_t)(0x00000000UL)                          /* 63 POWER_OFFSET_Q_C */
};

// *****************************************************************************
// *****************************************************************************
// Section: File scope functions
// *****************************************************************************
// *****************************************************************************

static double lDRV_Metrology_GetDouble(int64_t value)
{
    if (value < 0)
    {
        value = -value;
    }

    return (double)value;
}

void IPC1_InterruptHandler (void)
{
    uint32_t status = IPC1_REGS->IPC_ISR;
    status &= IPC1_REGS->IPC_IMR;

    if ((status & DRV_METROLOGY_IPC_INIT_IRQ_MSK) != 0UL)
    {
        if (gDrvMetObj.metRegisters->MET_STATUS.STATUS == STATUS_STATUS_RESET)
        {
            gDrvMetObj.status = DRV_METROLOGY_STATUS_INIT_DSP;
        }
    }

    if ((status & DRV_METROLOGY_IPC_INTEGRATION_IRQ_MSK) != 0UL)
    {
        if (gDrvMetObj.metRegisters->MET_STATUS.STATUS == STATUS_STATUS_DSP_RUNNING)
        {
            /* Update Accumulators Data */
            (void) memcpy(&gDrvMetObj.metAccData, &gDrvMetObj.metRegisters->MET_ACCUMULATORS, sizeof(DRV_METROLOGY_REGS_ACCUMULATORS));
            /* Store samples in period */
            gDrvMetObj.samplesInPeriod = gDrvMetObj.metRegisters->MET_STATUS.N;
            /* Update Frequency Data */
            gDrvMetObj.metFreqData.freq = gDrvMetObj.metRegisters->MET_STATUS.FREQ;
            gDrvMetObj.metFreqData.freqA = gDrvMetObj.metRegisters->MET_STATUS.FREQ_VA;
            gDrvMetObj.metFreqData.freqB = gDrvMetObj.metRegisters->MET_STATUS.FREQ_VB;
            gDrvMetObj.metFreqData.freqC = gDrvMetObj.metRegisters->MET_STATUS.FREQ_VC;
            if (gDrvMetObj.harmonicAnalysisData.holdRegs == false)
            {
                /* Update Harmonics Data */
                (void) memcpy(&gDrvMetObj.metHarData, &gDrvMetObj.metRegisters->MET_HARMONICS, sizeof(DRV_METROLOGY_REGS_HARMONICS));
            }
        }

        gDrvMetObj.integrationFlag = true;
    }

    IPC1_REGS->IPC_ICCR = status;

    gDrvMetObj.ipcInterruptFlag = true;

}

static double lDRV_Metrology_GetHarmonicRMS(int32_t real, int32_t imag, uint32_t k)
{
    double res, dre, dim, kd;
    double intPart, decPart;
    uint32_t intPartU, decPartU;
    uint32_t measure;

    /* k [uQ22.10] */
    intPartU = (k >> 10);
    decPartU = (k & 0x3FFUL);
    intPart = (double)intPartU;
    decPart = (double)decPartU;
    kd = decPart / 1024.0;
    kd += intPart;

    /* Get Real contribution */
    if (real < 0)
    {
        measure = (uint32_t)-real;
    }
    else
    {
        measure = (uint32_t)real;
    }

    /* 13.18 */
    intPartU = (measure >> 18);
    decPartU = (measure & 0x3FFFFUL);
    intPart = (double)intPartU;
    decPart = (double)decPartU;
    dre = decPart / 262144.0;
    dre += intPart;
    dre *= kd;
    dre *= dre;

    /* Get Imaginary contribution */
    if (imag < 0)
    {
        measure = (uint32_t)-imag;
    }
    else
    {
        measure = (uint32_t)imag;
    }

    /* 13.18 */
    intPartU = (measure >> 18);
    decPartU = (measure & 0x3FFFFUL);
    intPart = (double)intPartU;
    decPart = (double)decPartU;
    dim = decPart / 262144.0;
    dim += intPart;
    dim *= kd;
    dim *= dim;

    res = dre + dim;
    if (res > 0.0)
    {
        res = sqrt(res);
        res *= sqrt(2.0);
        res /= (double)gDrvMetObj.samplesInPeriod;
    }

    return res;
}

static uint32_t lDRV_Metrology_GetVIRMS(uint64_t val, uint32_t k_x)
{
    double m;

    m = (double)val;
    m = (m / (double)DIV_Q_FACTOR);
    m = (m / (double)gDrvMetObj.samplesInPeriod);
    if (m < 0.0)
    {
        m = 0.0;
    }
    else
    {
        m = sqrt(m);
        m = m * (double)k_x / (double)DIV_GAIN;
        m = m * VI_ACCURACY_DOUBLE;
    }

    return ((uint32_t)(m));
}

static uint32_t lDRV_Metrology_GetInxRMS(uint64_t val)
{
    double m;

    m = (double)val;
    m = (m / (double)DIV_Inx_Q_FACTOR);
    m = (m / (double)gDrvMetObj.samplesInPeriod);
    if (m < 0.0)
    {
        m = 0.0;
    }
    else
    {
        m = sqrt(m);
        m = m * (double)10000U;
    }

    return ((uint32_t)(m));
}

static uint32_t lDRV_Metrology_GetPQ(int64_t val, uint32_t k_ix, uint32_t k_vx)
{
    double m;
    double divisor, mult;

    m = lDRV_Metrology_GetDouble(val);

    m = m / (double)DIV_Q_FACTOR;
    m = m / (double)gDrvMetObj.samplesInPeriod;
    mult = (double)k_ix * (double)k_vx;
    divisor = (double)DIV_GAIN * (double)DIV_GAIN;
    m = (m * mult) / divisor;
    m = m * PQS_ACCURACY_DOUBLE;

    return ((uint32_t)(m));
}

static unsigned char lDRV_Metrology_CheckPQDir(int64_t val)
{
    if (val < 0)
    {
        return 1U;
    }
    else
    {
        return 0U;
    }
}

static uint32_t lDRV_Metrology_GetS(uint64_t i_val, uint64_t v_val, uint32_t k_ix, uint32_t k_vx)
{
    double i_m, v_m;
    double s_m;

    /* Get i and v values in double format */
    i_m = (double)i_val;
    v_m = (double)v_val;
    i_m = (i_m / (double)DIV_Q_FACTOR);
    v_m = (v_m / (double)DIV_Q_FACTOR);
    /* Accumulated to mean value */
    i_m = (i_m / (double)gDrvMetObj.samplesInPeriod);
    v_m = (v_m / (double)gDrvMetObj.samplesInPeriod);

    /* Obtain RMS value for Current */
    if (i_m < 0.0)
    {
        /* Protection against negative value */
        i_m = 0.0;
    }
    else
    {
        /* Square root */
        i_m = sqrt(i_m);
        /* Convert to Amps */
        i_m = i_m * (double)k_ix / (double)DIV_GAIN;
    }

    /* Obtain RMS value for Voltage */
    if (v_m < 0.0)
    {
        /* Protection against negative value */
        v_m = 0.0;
    }
    else
    {
        /* Square root */
        v_m = sqrt(v_m);
        /* Convert to Volts */
        v_m = v_m * (double)k_vx / (double)DIV_GAIN;
    }

    /* Compute Apparent Power as Irms*Vrms */
    s_m = i_m * v_m;
    s_m = s_m * PQS_ACCURACY_DOUBLE;

    return ((uint32_t)(s_m));
}

static uint32_t lDRV_Metrology_GetAngle(int64_t p, int64_t q)
{
    double m, pd, qd;
    int32_t n;
    uint32_t angle;

    pd = (double)p;
    qd = (double)q;
    m = atan2(qd, pd);
    m = 180.0 * m;
    m = m * ANGLE_ACCURACY_DOUBLE;
    m = m / CONST_Pi;
    n = (int32_t)m;

    if (n < 0)
    {
        /* Get the positive value and set the MSB */
        n = -n;
        angle = (uint32_t)n;
        angle |= 0x80000000UL;
    }
    else
    {
        angle = (uint32_t)n;
    }

    return angle;
}

static uint32_t lDRV_Metrology_GetPQEnergy(DRV_METROLOGY_ENERGY_TYPE id)
{
    double m, k;
    double divisor;
    double ki, kv;

    divisor = (double)DIV_GAIN * (double)DIV_GAIN;

    /* Calculated as absolute values */
    if (id == PENERGY)
    {
        m = lDRV_Metrology_GetDouble(gDrvMetObj.metAccData.P_A);
        ki = (double)gDrvMetObj.metRegisters->MET_CONTROL.K_IA;
        kv = (double)gDrvMetObj.metRegisters->MET_CONTROL.K_VA;
        m = (m * ki * kv) / divisor;    /* m =m*k_v*k_i */
        m = m / (double)DIV_Q_FACTOR;   /* k =k/2^40 */
        k = m / SAMPLING_FREQ;          /* k =k/fs */

        m = lDRV_Metrology_GetDouble(gDrvMetObj.metAccData.P_B);
        ki = (double)gDrvMetObj.metRegisters->MET_CONTROL.K_IB;
        kv = (double)gDrvMetObj.metRegisters->MET_CONTROL.K_VB;
        m = (m * ki * kv) / divisor;    /* m =m*k_v*k_i */
        m = m / (double)DIV_Q_FACTOR;   /* k =k/2^40 */
        k += m / SAMPLING_FREQ;         /* k =k/fs */

        m = lDRV_Metrology_GetDouble(gDrvMetObj.metAccData.P_C);
        ki = (double)gDrvMetObj.metRegisters->MET_CONTROL.K_IC;
        kv = (double)gDrvMetObj.metRegisters->MET_CONTROL.K_VC;
        m = (m * ki * kv) / divisor;    /* m =m*k_v*k_i */
        m = m / (double)DIV_Q_FACTOR;   /* k =k/2^40 */
        k += m / SAMPLING_FREQ;         /* k =k/fs */
    }
    else
    {
        /* reactive energy */
        m = lDRV_Metrology_GetDouble(gDrvMetObj.metAccData.Q_A);
        ki = (double)gDrvMetObj.metRegisters->MET_CONTROL.K_IA;
        kv = (double)gDrvMetObj.metRegisters->MET_CONTROL.K_VA;
        m = (m * ki * kv) / divisor;    /* m =m*k_v*k_i */
        m = m / (double)DIV_Q_FACTOR;   /* k =k/2^40 */
        k = m / SAMPLING_FREQ;          /* k =k/fs */

        m = lDRV_Metrology_GetDouble(gDrvMetObj.metAccData.Q_B);
        ki = (double)gDrvMetObj.metRegisters->MET_CONTROL.K_IB;
        kv = (double)gDrvMetObj.metRegisters->MET_CONTROL.K_VB;
        m = (m * ki * kv) / divisor;    /* m =m*k_v*k_i */
        m = m / (double)DIV_Q_FACTOR;   /* k =k/2^40 */
        k += m / SAMPLING_FREQ;         /* k =k/fs */

        m = lDRV_Metrology_GetDouble(gDrvMetObj.metAccData.Q_C);
        ki = (double)gDrvMetObj.metRegisters->MET_CONTROL.K_IC;
        kv = (double)gDrvMetObj.metRegisters->MET_CONTROL.K_VC;
        m = (m * ki * kv) / divisor;    /* m =m*k_v*k_i */
        m = m / (double)DIV_Q_FACTOR;   /* k =k/2^40 */
        k += m / SAMPLING_FREQ;         /* k =k/fs */
    }

    k = k / SECS_IN_HOUR_DOUBLE; /* xxxxxx (Wh/Varh) */
    k = k * 10000.0;        /* *10000 (kWh/kVarh) */

    return ((uint32_t)(k));  /* xxxx (kWh/kVarh) */
}

static void lDRV_Metrology_IpcInitialize (void)
{
    /* Clear interrupts */
    IPC1_REGS->IPC_ICCR = 0xFFFFFFFFUL;
    /* Enable interrupts */
    IPC1_REGS->IPC_IECR = DRV_METROLOGY_IPC_INIT_IRQ_MSK |
        DRV_METROLOGY_IPC_INTEGRATION_IRQ_MSK;
}

static uint32_t lDRV_Metrology_CorrectCalibrationAngle(uint32_t measured, double reference)
{
    double bams, phase_correction;
    int64_t correction_angle;

    if ((measured & 0x80000000UL) != 0UL)
    {
        uint32_t measured_angle;
        measured_angle = 0x7FFFFFFFUL & measured;
        correction_angle = (int64_t)measured_angle;
        correction_angle = -correction_angle;
    }
    else
    {
        correction_angle = (int64_t)measured;
    }

    /* Get diff with reference */
    correction_angle = correction_angle - (int64_t)reference;

    /* Correction angle should be between -180 and 180 degrees */
    while (correction_angle < (-18000000))
    {
        correction_angle += 36000000;
    }

    while (correction_angle > 18000000)
    {
        correction_angle -= 36000000;
    }

    bams = (double)correction_angle;
    bams = bams * (60.00 / gDrvMetObj.calibrationData.freq);
    bams = bams / 18000000.00; /* get bams and remove precision adjust */

    phase_correction = bams * 2147483648.00; /* sQ0.31 */
    correction_angle = (int64_t)phase_correction;

    return (uint32_t)correction_angle;
}

static void lDRV_METROLOGY_UpdateMeasurements(void)
{
    uint32_t *afeMeasure = NULL;
    uint32_t stateFlagReg;

    /* Get State Flag Register */
    stateFlagReg = gDrvMetObj.metRegisters->MET_STATUS.STATE_FLAG;

    /* Update Measure values */
    afeMeasure = gDrvMetObj.metAFEData.measure;

    afeMeasure[MEASURE_UA_RMS] = lDRV_Metrology_GetVIRMS(gDrvMetObj.metAccData.V_A, gDrvMetObj.metRegisters->MET_CONTROL.K_VA);
    afeMeasure[MEASURE_UB_RMS] = lDRV_Metrology_GetVIRMS(gDrvMetObj.metAccData.V_B, gDrvMetObj.metRegisters->MET_CONTROL.K_VB);
    afeMeasure[MEASURE_UC_RMS] = lDRV_Metrology_GetVIRMS(gDrvMetObj.metAccData.V_C, gDrvMetObj.metRegisters->MET_CONTROL.K_VC);

    afeMeasure[MEASURE_IA_RMS] = lDRV_Metrology_GetVIRMS(gDrvMetObj.metAccData.I_A, gDrvMetObj.metRegisters->MET_CONTROL.K_IA);
    afeMeasure[MEASURE_IB_RMS] = lDRV_Metrology_GetVIRMS(gDrvMetObj.metAccData.I_B, gDrvMetObj.metRegisters->MET_CONTROL.K_IB);
    afeMeasure[MEASURE_IC_RMS] = lDRV_Metrology_GetVIRMS(gDrvMetObj.metAccData.I_C, gDrvMetObj.metRegisters->MET_CONTROL.K_IC);

    afeMeasure[MEASURE_INI_RMS] = lDRV_Metrology_GetInxRMS(gDrvMetObj.metAccData.I_Ni);
    afeMeasure[MEASURE_INM_RMS] = lDRV_Metrology_GetVIRMS(gDrvMetObj.metAccData.I_Nm, gDrvMetObj.metRegisters->MET_CONTROL.K_IN);
    afeMeasure[MEASURE_INMI_RMS] = lDRV_Metrology_GetInxRMS(gDrvMetObj.metAccData.I_Nmi);

    afeMeasure[MEASURE_PA]  = lDRV_Metrology_GetPQ(gDrvMetObj.metAccData.P_A, gDrvMetObj.metRegisters->MET_CONTROL.K_IA, gDrvMetObj.metRegisters->MET_CONTROL.K_VA);
    gDrvMetObj.metAFEData.afeEvents.paDir = lDRV_Metrology_CheckPQDir(gDrvMetObj.metAccData.P_A);
    afeMeasure[MEASURE_PB]  = lDRV_Metrology_GetPQ(gDrvMetObj.metAccData.P_B, gDrvMetObj.metRegisters->MET_CONTROL.K_IB, gDrvMetObj.metRegisters->MET_CONTROL.K_VB);
    gDrvMetObj.metAFEData.afeEvents.pbDir = lDRV_Metrology_CheckPQDir(gDrvMetObj.metAccData.P_B);
    afeMeasure[MEASURE_PC]  = lDRV_Metrology_GetPQ(gDrvMetObj.metAccData.P_C, gDrvMetObj.metRegisters->MET_CONTROL.K_IC, gDrvMetObj.metRegisters->MET_CONTROL.K_VC);
    gDrvMetObj.metAFEData.afeEvents.pcDir = lDRV_Metrology_CheckPQDir(gDrvMetObj.metAccData.P_C);

    afeMeasure[MEASURE_QA]  = lDRV_Metrology_GetPQ(gDrvMetObj.metAccData.Q_A, gDrvMetObj.metRegisters->MET_CONTROL.K_IA, gDrvMetObj.metRegisters->MET_CONTROL.K_VA);
    gDrvMetObj.metAFEData.afeEvents.qaDir = lDRV_Metrology_CheckPQDir(gDrvMetObj.metAccData.Q_A);
    afeMeasure[MEASURE_QB]  = lDRV_Metrology_GetPQ(gDrvMetObj.metAccData.Q_B, gDrvMetObj.metRegisters->MET_CONTROL.K_IB, gDrvMetObj.metRegisters->MET_CONTROL.K_VB);
    gDrvMetObj.metAFEData.afeEvents.qbDir = lDRV_Metrology_CheckPQDir(gDrvMetObj.metAccData.Q_B);
    afeMeasure[MEASURE_QC]  = lDRV_Metrology_GetPQ(gDrvMetObj.metAccData.Q_C, gDrvMetObj.metRegisters->MET_CONTROL.K_IC, gDrvMetObj.metRegisters->MET_CONTROL.K_VC);
    gDrvMetObj.metAFEData.afeEvents.qcDir = lDRV_Metrology_CheckPQDir(gDrvMetObj.metAccData.Q_C);

    afeMeasure[MEASURE_SA]  = lDRV_Metrology_GetS(gDrvMetObj.metAccData.I_A, gDrvMetObj.metAccData.V_A, gDrvMetObj.metRegisters->MET_CONTROL.K_IA, gDrvMetObj.metRegisters->MET_CONTROL.K_VA);
    afeMeasure[MEASURE_SB]  = lDRV_Metrology_GetS(gDrvMetObj.metAccData.I_B, gDrvMetObj.metAccData.V_B, gDrvMetObj.metRegisters->MET_CONTROL.K_IB, gDrvMetObj.metRegisters->MET_CONTROL.K_VB);
    afeMeasure[MEASURE_SC]  = lDRV_Metrology_GetS(gDrvMetObj.metAccData.I_C, gDrvMetObj.metAccData.V_C, gDrvMetObj.metRegisters->MET_CONTROL.K_IC, gDrvMetObj.metRegisters->MET_CONTROL.K_VC);

    afeMeasure[MEASURE_UAF_RMS] = lDRV_Metrology_GetVIRMS(gDrvMetObj.metAccData.V_A_F, gDrvMetObj.metRegisters->MET_CONTROL.K_VA);
    afeMeasure[MEASURE_UBF_RMS] = lDRV_Metrology_GetVIRMS(gDrvMetObj.metAccData.V_B_F, gDrvMetObj.metRegisters->MET_CONTROL.K_VB);
    afeMeasure[MEASURE_UCF_RMS] = lDRV_Metrology_GetVIRMS(gDrvMetObj.metAccData.V_C_F, gDrvMetObj.metRegisters->MET_CONTROL.K_VC);

    afeMeasure[MEASURE_IAF_RMS] = lDRV_Metrology_GetVIRMS(gDrvMetObj.metAccData.I_A_F, gDrvMetObj.metRegisters->MET_CONTROL.K_IA);
    afeMeasure[MEASURE_IBF_RMS] = lDRV_Metrology_GetVIRMS(gDrvMetObj.metAccData.I_B_F, gDrvMetObj.metRegisters->MET_CONTROL.K_IB);
    afeMeasure[MEASURE_ICF_RMS] = lDRV_Metrology_GetVIRMS(gDrvMetObj.metAccData.I_C_F, gDrvMetObj.metRegisters->MET_CONTROL.K_IC);

    afeMeasure[MEASURE_IMNF_RMS] = lDRV_Metrology_GetVIRMS(gDrvMetObj.metAccData.I_Nm_F, gDrvMetObj.metRegisters->MET_CONTROL.K_IN);

    afeMeasure[MEASURE_PAF]  = lDRV_Metrology_GetPQ(gDrvMetObj.metAccData.P_A_F, gDrvMetObj.metRegisters->MET_CONTROL.K_IA, gDrvMetObj.metRegisters->MET_CONTROL.K_VA);
    gDrvMetObj.metAFEData.afeEvents.pafDir = lDRV_Metrology_CheckPQDir(gDrvMetObj.metAccData.P_A_F);
    afeMeasure[MEASURE_PBF]  = lDRV_Metrology_GetPQ(gDrvMetObj.metAccData.P_B_F, gDrvMetObj.metRegisters->MET_CONTROL.K_IB, gDrvMetObj.metRegisters->MET_CONTROL.K_VB);
    gDrvMetObj.metAFEData.afeEvents.pbfDir = lDRV_Metrology_CheckPQDir(gDrvMetObj.metAccData.P_B_F);
    afeMeasure[MEASURE_PCF]  = lDRV_Metrology_GetPQ(gDrvMetObj.metAccData.P_C_F, gDrvMetObj.metRegisters->MET_CONTROL.K_IC, gDrvMetObj.metRegisters->MET_CONTROL.K_VC);
    gDrvMetObj.metAFEData.afeEvents.pcfDir = lDRV_Metrology_CheckPQDir(gDrvMetObj.metAccData.P_C_F);

    afeMeasure[MEASURE_QAF]  = lDRV_Metrology_GetPQ(gDrvMetObj.metAccData.Q_A_F, gDrvMetObj.metRegisters->MET_CONTROL.K_IA, gDrvMetObj.metRegisters->MET_CONTROL.K_VA);
    gDrvMetObj.metAFEData.afeEvents.qafDir = lDRV_Metrology_CheckPQDir(gDrvMetObj.metAccData.Q_A_F);
    afeMeasure[MEASURE_QBF]  = lDRV_Metrology_GetPQ(gDrvMetObj.metAccData.Q_B_F, gDrvMetObj.metRegisters->MET_CONTROL.K_IB, gDrvMetObj.metRegisters->MET_CONTROL.K_VB);
    gDrvMetObj.metAFEData.afeEvents.qbfDir = lDRV_Metrology_CheckPQDir(gDrvMetObj.metAccData.Q_B_F);
    afeMeasure[MEASURE_QCF]  = lDRV_Metrology_GetPQ(gDrvMetObj.metAccData.Q_C_F, gDrvMetObj.metRegisters->MET_CONTROL.K_IC, gDrvMetObj.metRegisters->MET_CONTROL.K_VC);
    gDrvMetObj.metAFEData.afeEvents.qcfDir = lDRV_Metrology_CheckPQDir(gDrvMetObj.metAccData.Q_C_F);

    afeMeasure[MEASURE_SAF]  = lDRV_Metrology_GetS(gDrvMetObj.metAccData.I_A_F, gDrvMetObj.metAccData.V_A_F, gDrvMetObj.metRegisters->MET_CONTROL.K_IA, gDrvMetObj.metRegisters->MET_CONTROL.K_VA);
    afeMeasure[MEASURE_SBF]  = lDRV_Metrology_GetS(gDrvMetObj.metAccData.I_B_F, gDrvMetObj.metAccData.V_B_F, gDrvMetObj.metRegisters->MET_CONTROL.K_IB, gDrvMetObj.metRegisters->MET_CONTROL.K_VB);
    afeMeasure[MEASURE_SCF]  = lDRV_Metrology_GetS(gDrvMetObj.metAccData.I_C_F, gDrvMetObj.metAccData.V_C_F, gDrvMetObj.metRegisters->MET_CONTROL.K_IC, gDrvMetObj.metRegisters->MET_CONTROL.K_VC);

    afeMeasure[MEASURE_PT]  = afeMeasure[MEASURE_PA] + afeMeasure[MEASURE_PB] + afeMeasure[MEASURE_PC];
    gDrvMetObj.metAFEData.afeEvents.ptDir = lDRV_Metrology_CheckPQDir(gDrvMetObj.metAccData.P_A + gDrvMetObj.metAccData.P_B + gDrvMetObj.metAccData.P_C);

    afeMeasure[MEASURE_QT]  = afeMeasure[MEASURE_QA] + afeMeasure[MEASURE_QB] + afeMeasure[MEASURE_QC];
    gDrvMetObj.metAFEData.afeEvents.qtDir = lDRV_Metrology_CheckPQDir(gDrvMetObj.metAccData.Q_A + gDrvMetObj.metAccData.Q_B + gDrvMetObj.metAccData.Q_C);

    afeMeasure[MEASURE_PTF]  = afeMeasure[MEASURE_PAF] + afeMeasure[MEASURE_PBF] + afeMeasure[MEASURE_PCF];
    gDrvMetObj.metAFEData.afeEvents.ptfDir = lDRV_Metrology_CheckPQDir(gDrvMetObj.metAccData.P_A_F + gDrvMetObj.metAccData.P_B_F + gDrvMetObj.metAccData.P_C_F);

    afeMeasure[MEASURE_QTF]  = afeMeasure[MEASURE_QAF] + afeMeasure[MEASURE_QBF] + afeMeasure[MEASURE_QCF];
    gDrvMetObj.metAFEData.afeEvents.qtfDir = lDRV_Metrology_CheckPQDir(gDrvMetObj.metAccData.Q_A_F + gDrvMetObj.metAccData.Q_B_F + gDrvMetObj.metAccData.Q_C_F);

    afeMeasure[MEASURE_ST]  = afeMeasure[MEASURE_SA] + afeMeasure[MEASURE_SB] + afeMeasure[MEASURE_SC];

    afeMeasure[MEASURE_STF]  = afeMeasure[MEASURE_SAF] + afeMeasure[MEASURE_SBF] + afeMeasure[MEASURE_SCF];

    afeMeasure[MEASURE_FREQ]  = (gDrvMetObj.metFreqData.freq * FREQ_ACCURACY_INT) >> FREQ_Q;
    afeMeasure[MEASURE_FREQA]  = (gDrvMetObj.metFreqData.freqA * FREQ_ACCURACY_INT) >> FREQ_Q;
    afeMeasure[MEASURE_FREQB]  = (gDrvMetObj.metFreqData.freqB * FREQ_ACCURACY_INT) >> FREQ_Q;
    afeMeasure[MEASURE_FREQC]  = (gDrvMetObj.metFreqData.freqC * FREQ_ACCURACY_INT) >> FREQ_Q;

    afeMeasure[MEASURE_ANGLEA]  = lDRV_Metrology_GetAngle(gDrvMetObj.metAccData.P_A, gDrvMetObj.metAccData.Q_A);
    afeMeasure[MEASURE_ANGLEB]  = lDRV_Metrology_GetAngle(gDrvMetObj.metAccData.P_B, gDrvMetObj.metAccData.Q_B);
    afeMeasure[MEASURE_ANGLEC]  = lDRV_Metrology_GetAngle(gDrvMetObj.metAccData.P_C, gDrvMetObj.metAccData.Q_C);
    afeMeasure[MEASURE_ANGLEN]  = lDRV_Metrology_GetAngle(gDrvMetObj.metAccData.P_N, gDrvMetObj.metAccData.Q_N);

    gDrvMetObj.metAFEData.energy += lDRV_Metrology_GetPQEnergy(PENERGY);

    /* Update Swell/Sag events */
    gDrvMetObj.metAFEData.afeEvents.sagA = (stateFlagReg & STATUS_STATE_FLAG_SAG_DET_VA_Msk) > 0U? 1U : 0U;
    gDrvMetObj.metAFEData.afeEvents.sagB = (stateFlagReg & STATUS_STATE_FLAG_SAG_DET_VB_Msk) > 0U? 1U : 0U;
    gDrvMetObj.metAFEData.afeEvents.sagC = (stateFlagReg & STATUS_STATE_FLAG_SAG_DET_VC_Msk) > 0U? 1U : 0U;
    gDrvMetObj.metAFEData.afeEvents.swellA = (stateFlagReg & STATUS_STATE_FLAG_SWELL_DET_VA_Msk) > 0U? 1U : 0U;
    gDrvMetObj.metAFEData.afeEvents.swellB = (stateFlagReg & STATUS_STATE_FLAG_SWELL_DET_VB_Msk) > 0U? 1U : 0U;
    gDrvMetObj.metAFEData.afeEvents.swellC = (stateFlagReg & STATUS_STATE_FLAG_SWELL_DET_VC_Msk) > 0U? 1U : 0U;

}

static bool lDRV_METROLOGY_UpdateCalibrationValues(void)
{
    DRV_METROLOGY_CALIBRATION * pCalibrationData;
    DRV_METROLOGY_REGS_ACCUMULATORS * pMetAccRegs;

    pCalibrationData = &gDrvMetObj.calibrationData;
    pMetAccRegs = &gDrvMetObj.metAccData;

    if (pCalibrationData->numIntegrationPeriods != 0U)
    {
        pCalibrationData->numIntegrationPeriods--;

        /* Update Accumulators */
        pCalibrationData->dspAccIa += pMetAccRegs->I_A;
        pCalibrationData->dspAccIb += pMetAccRegs->I_B;
        pCalibrationData->dspAccIc += pMetAccRegs->I_C;
        pCalibrationData->dspAccIn += pMetAccRegs->I_Nm;
        pCalibrationData->dspAccUa += pMetAccRegs->V_A;
        pCalibrationData->dspAccUb += pMetAccRegs->V_B;
        pCalibrationData->dspAccUc += pMetAccRegs->V_C;
        pCalibrationData->dspAccPa += pMetAccRegs->P_A_F;
        pCalibrationData->dspAccPb += pMetAccRegs->P_B_F;
        pCalibrationData->dspAccPc += pMetAccRegs->P_C_F;
        pCalibrationData->dspAccPn += pMetAccRegs->P_N_F;
        pCalibrationData->dspAccQa += pMetAccRegs->Q_A_F;
        pCalibrationData->dspAccQb += pMetAccRegs->Q_B_F;
        pCalibrationData->dspAccQc += pMetAccRegs->Q_C_F;
        pCalibrationData->dspAccQn += pMetAccRegs->Q_N_F;

        return false;
    }
    else
    {
        DRV_METROLOGY_REGS_CONTROL * pMetControlRegs;
        uint64_t m, m_div;
        uint32_t k;

        pMetControlRegs = &gDrvMetObj.metRegisters->MET_CONTROL;

        /* The number of the required integration periods has been completed */
        /* Get Calibration Values */
        if ((pCalibrationData->references.lineId == PHASE_A) || (pCalibrationData->references.lineId == PHASE_T) ||
            (pCalibrationData->references.lineId == PHASE_TN))
        {
            /* Calibration I RMS */
            pCalibrationData->dspAccIa >>= 2U;
            k = lDRV_Metrology_GetVIRMS(pCalibrationData->dspAccIa, pMetControlRegs->K_IA);
            m = (uint64_t)pCalibrationData->references.aimIA;
            m = m << CAL_VI_Q;
            m_div = m / k;
            pMetControlRegs->CAL_M_IA = (uint32_t)m_div;
            if ((((m % k) * 10U) / k) > 4U)
            {
                pMetControlRegs->CAL_M_IA += 1U;
            }
            /* Calibration V RMS */
            pCalibrationData->dspAccUa >>= 2U;
            k = lDRV_Metrology_GetVIRMS(pCalibrationData->dspAccUa, pMetControlRegs->K_VA);
            m = (uint64_t)pCalibrationData->references.aimVA;
            m = m << CAL_VI_Q;
            m_div = m / k;
            pMetControlRegs->CAL_M_VA = (uint32_t)m_div;
            if ((((m % k) * 10U) / k) > 4U)
            {
                pMetControlRegs->CAL_M_VA += 1U;
            }

            /* Calibration Phase */
            pCalibrationData->dspAccPa /= 4;
            pCalibrationData->dspAccQa /= 4;
            k = lDRV_Metrology_GetAngle(pCalibrationData->dspAccPa, pCalibrationData->dspAccQa);
            pMetControlRegs->CAL_PH_IA = lDRV_Metrology_CorrectCalibrationAngle(k, pCalibrationData->references.angleA);

            pCalibrationData->result = true;
        }

        if ((pCalibrationData->references.lineId == PHASE_B) || (pCalibrationData->references.lineId == PHASE_T) ||
            (pCalibrationData->references.lineId == PHASE_TN))
        {
            /* Calibration I RMS */
            pCalibrationData->dspAccIb >>= 2U;
            k = lDRV_Metrology_GetVIRMS(pCalibrationData->dspAccIb, pMetControlRegs->K_IB);
            m = (uint64_t)pCalibrationData->references.aimIB;
            m = m << CAL_VI_Q;
            m_div = m / k;
            pMetControlRegs->CAL_M_IB = (uint32_t)m_div;
            if ((((m % k) * 10U) / k) > 4U)
            {
                pMetControlRegs->CAL_M_IB += 1U;
            }
            /* Calibration V RMS */
            pCalibrationData->dspAccUb >>= 2U;
            k = lDRV_Metrology_GetVIRMS(pCalibrationData->dspAccUb, pMetControlRegs->K_VB);
            m = (uint64_t)pCalibrationData->references.aimVB;
            m = m << CAL_VI_Q;
            m_div = m / k;
            pMetControlRegs->CAL_M_VB = (uint32_t)m_div;
            if ((((m % k) * 10U) / k) > 4U)
            {
                pMetControlRegs->CAL_M_VB += 1U;
            }

            /* Calibration Phase */
            pCalibrationData->dspAccPb /= 4;
            pCalibrationData->dspAccQb /= 4;
            k = lDRV_Metrology_GetAngle(pCalibrationData->dspAccPb, pCalibrationData->dspAccQb);
            pMetControlRegs->CAL_PH_IB = lDRV_Metrology_CorrectCalibrationAngle(k, pCalibrationData->references.angleB);

            pCalibrationData->result = true;
        }

        if ((pCalibrationData->references.lineId == PHASE_C) || (pCalibrationData->references.lineId == PHASE_T) ||
            (pCalibrationData->references.lineId == PHASE_TN))
        {
            /* Calibration I RMS */
            pCalibrationData->dspAccIc >>= 2U;
            k = lDRV_Metrology_GetVIRMS(pCalibrationData->dspAccIc, pMetControlRegs->K_IC);
            m = (uint64_t)pCalibrationData->references.aimIC;
            m = m << CAL_VI_Q;
            m_div = m / k;
            pMetControlRegs->CAL_M_IC = (uint32_t)m_div;
            if ((((m % k) * 10U) / k) > 4U)
            {
                pMetControlRegs->CAL_M_IC += 1U;
            }
            /* Calibration V RMS */
            pCalibrationData->dspAccUc >>= 2U;
            k = lDRV_Metrology_GetVIRMS(pCalibrationData->dspAccUc, pMetControlRegs->K_VC);
            m = (uint64_t)pCalibrationData->references.aimVC;
            m = m << CAL_VI_Q;
            m_div = m / k;
            pMetControlRegs->CAL_M_VC = (uint32_t)m_div;
            if ((((m % k) * 10U) / k) > 4U)
            {
                pMetControlRegs->CAL_M_VC += 1U;
            }

            /* Calibration Phase */
            pCalibrationData->dspAccPc /= 4;
            pCalibrationData->dspAccQc /= 4;
            k = lDRV_Metrology_GetAngle(pCalibrationData->dspAccPc, pCalibrationData->dspAccQc);
            pMetControlRegs->CAL_PH_IC = lDRV_Metrology_CorrectCalibrationAngle(k, pCalibrationData->references.angleC);

            pCalibrationData->result = true;
        }

        if ((pCalibrationData->references.lineId == PHASE_N) || (pCalibrationData->references.lineId == PHASE_TN))
        {
            /* Calibration I RMS */
            pCalibrationData->dspAccIn >>= 2U;
            k = lDRV_Metrology_GetVIRMS(pCalibrationData->dspAccIn, pMetControlRegs->K_IN);
            m = (uint64_t)pCalibrationData->references.aimIN;
            m = m << CAL_VI_Q;
            m_div = m / k;
            pMetControlRegs->CAL_M_IN = (uint32_t)m_div;
            if ((((m % k) * 10U) / k) > 4U)
            {
                pMetControlRegs->CAL_M_IN += 1U;
            }

            /* Calibration Phase */
            pCalibrationData->dspAccPn /= 4;
            pCalibrationData->dspAccQn /= 4;
            k = lDRV_Metrology_GetAngle(pCalibrationData->dspAccPn, pCalibrationData->dspAccQn);
            pMetControlRegs->CAL_PH_IN = lDRV_Metrology_CorrectCalibrationAngle(k, pCalibrationData->references.angleN);

            pCalibrationData->result = true;
        }

        /* Restore FEATURE_CTRL after calibration */
        pMetControlRegs->FEATURE_CTRL = pCalibrationData->featureCtrlBackup;

        /* Calibration has been completed */
        pCalibrationData->running = false;

        return true;
    }
}

static void lDRV_METROLOGY_UpdateHarmonicAnalysisValues(void)
{
    DRV_METROLOGY_HARMONICS_RMS *pHarmonicsRsp = gDrvMetObj.harmonicAnalysisData.pHarmonicAnalysisResponse;
    DRV_METROLOGY_REGS_HARMONICS *pHarData = &gDrvMetObj.metHarData;
    int32_t real, imag;
    uint32_t k;
    uint8_t index;
    uint32_t harmonicBitmap = gDrvMetObj.harmonicAnalysisData.harmonicBitmap;

    for (index = 0; index < DRV_METROLOGY_HARMONICS_MAX_ORDER; index++)
    {
        if ((harmonicBitmap & (1UL << index)) != 0U)
        {
            real = (int32_t)pHarData->I_A_m_R[index];
            imag = (int32_t)pHarData->I_A_m_I[index];
            k = gDrvMetObj.metRegisters->MET_CONTROL.K_IA;
            pHarmonicsRsp->Irms_A_m = lDRV_Metrology_GetHarmonicRMS(real, imag, k);

            real = (int32_t)pHarData->I_B_m_R[index];
            imag = (int32_t)pHarData->I_B_m_I[index];
            k = gDrvMetObj.metRegisters->MET_CONTROL.K_IB;
            pHarmonicsRsp->Irms_B_m = lDRV_Metrology_GetHarmonicRMS(real, imag, k);

            real = (int32_t)pHarData->I_C_m_R[index];
            imag = (int32_t)pHarData->I_C_m_I[index];
            k = gDrvMetObj.metRegisters->MET_CONTROL.K_IC;
            pHarmonicsRsp->Irms_C_m = lDRV_Metrology_GetHarmonicRMS(real, imag, k);

            real = (int32_t)pHarData->I_N_m_R[index];
            imag = (int32_t)pHarData->I_N_m_I[index];
            k = gDrvMetObj.metRegisters->MET_CONTROL.K_IN;
            pHarmonicsRsp->Irms_N_m = lDRV_Metrology_GetHarmonicRMS(real, imag, k);

            real = (int32_t)pHarData->V_A_m_R[index];
            imag = (int32_t)pHarData->V_A_m_I[index];
            k = gDrvMetObj.metRegisters->MET_CONTROL.K_VA;
            pHarmonicsRsp->Vrms_A_m = lDRV_Metrology_GetHarmonicRMS(real, imag, k);

            real = (int32_t)pHarData->V_B_m_R[index];
            imag = (int32_t)pHarData->V_B_m_I[index];
            k = gDrvMetObj.metRegisters->MET_CONTROL.K_VB;
            pHarmonicsRsp->Vrms_B_m = lDRV_Metrology_GetHarmonicRMS(real, imag, k);

            real = (int32_t)pHarData->V_C_m_R[index];
            imag = (int32_t)pHarData->V_C_m_I[index];
            k = gDrvMetObj.metRegisters->MET_CONTROL.K_VC;
            pHarmonicsRsp->Vrms_C_m = lDRV_Metrology_GetHarmonicRMS(real, imag, k);
        }
        else
        {
            pHarmonicsRsp->Irms_A_m = 0.0;
            pHarmonicsRsp->Irms_B_m = 0.0;
            pHarmonicsRsp->Irms_C_m = 0.0;
            pHarmonicsRsp->Irms_N_m = 0.0;
            pHarmonicsRsp->Vrms_A_m = 0.0;
            pHarmonicsRsp->Vrms_B_m = 0.0;
            pHarmonicsRsp->Vrms_C_m = 0.0;
        }

        pHarmonicsRsp++;
    }
}

// *****************************************************************************
// *****************************************************************************
// Section: Driver Common Interface Implementation
// *****************************************************************************
// *****************************************************************************

SYS_MODULE_OBJ DRV_METROLOGY_Initialize (SYS_MODULE_INIT * init, uint32_t resetCause)
{
    /* MISRA C-2012 deviation block start */
    /* MISRA C-2012 Rule 11.3 deviated once. Deviation record ID - H3_MISRAC_2012_R_11_3_DR_1 */
    DRV_METROLOGY_INIT *metInit = (DRV_METROLOGY_INIT *)init;
    /* MISRA C-2012 deviation block end */

    if ((gDrvMetObj.inUse == true) || (init == NULL))
    {
        return SYS_MODULE_OBJ_INVALID;
    }

	/* Clean the IPC interrupt generic flag */
    gDrvMetObj.ipcInterruptFlag = false;

    /* Disable IPC interrupts */
    (void) SYS_INT_SourceDisable(IPC1_IRQn);

	/* Clean the IPC interrupt flags */
    gDrvMetObj.integrationFlag = false;

    if (resetCause != RSTC_SR_RSTTYP(RSTC_SR_RSTTYP_WDT0_RST_Val))
    {
        uint32_t *pSrc;
        uint32_t *pDst;

        /* Assert reset of the coprocessor */
        RSTC_CoProcessorEnable(false);

        /* Disable coprocessor Clocks */
        CLK_Core1ProcessorClkDisable();

        gDrvMetObj.binStartAddress = metInit->binStartAddress;
        gDrvMetObj.binSize = metInit->binEndAddress - metInit->binStartAddress;

        /* Copy the Metrology bin file to SRAM1 */
        pSrc = (uint32_t *)gDrvMetObj.binStartAddress;
        pDst = (uint32_t *)IRAM1_ADDR;
        (void) memcpy(pDst, pSrc, gDrvMetObj.binSize);
    }

    /* Initialization of the interface with Metrology Lib */
    gDrvMetObj.metRegisters = (MET_REGISTERS *)metInit->regBaseAddress;
    gDrvMetObj.inUse = true;
    gDrvMetObj.integrationCallback = NULL;

    (void) memset(&gDrvMetObj.metAccData, 0, sizeof(DRV_METROLOGY_REGS_ACCUMULATORS));
    (void) memset(&gDrvMetObj.metHarData, 0, sizeof(DRV_METROLOGY_REGS_HARMONICS));
    (void) memset(&gDrvMetObj.calibrationData, 0, sizeof(DRV_METROLOGY_CALIBRATION));
    (void) memset(&gDrvMetObj.metAFEData, 0, sizeof(DRV_METROLOGY_AFE_DATA));

    /* Initialization of the Metrology object */
    gDrvMetObj.status = DRV_METROLOGY_STATUS_HALT;

    /* Configure IPC peripheral */
    lDRV_Metrology_IpcInitialize();

    return (SYS_MODULE_OBJ)&gDrvMetObj;
}

SYS_MODULE_OBJ DRV_METROLOGY_Reinitialize (SYS_MODULE_INIT * init)
{
    /* MISRA C-2012 deviation block start */
    /* MISRA C-2012 Rule 11.3 deviated once. Deviation record ID - H3_MISRAC_2012_R_11_3_DR_1 */
    DRV_METROLOGY_INIT *metInit = (DRV_METROLOGY_INIT *)init;
    /* MISRA C-2012 deviation block end */
    uint32_t *pSrc;
    uint32_t *pDst;

    if ((gDrvMetObj.inUse == false) || (init == NULL))
    {
        return SYS_MODULE_OBJ_INVALID;
    }

    /* Disable IPC interrupts */
    (void) SYS_INT_SourceDisable(IPC1_IRQn);

    /* Assert reset of the coprocessor and its peripherals */
    RSTC_CoProcessorEnable(false);
    RSTC_CoProcessorPeripheralEnable(false);

    /* Disable coprocessor Clocks */
    CLK_Core1ProcessorClkDisable();

    /* De-assert reset of the coprocessor peripherals */
    RSTC_CoProcessorPeripheralEnable(true);

    gDrvMetObj.binStartAddress = metInit->binStartAddress;
    gDrvMetObj.binSize = metInit->binEndAddress - metInit->binStartAddress;

    /* Copy the Metrology bin file to SRAM1 */
    pSrc = (uint32_t *)gDrvMetObj.binStartAddress;
    pDst = (uint32_t *)IRAM1_ADDR;
    (void) memcpy(pDst, pSrc, gDrvMetObj.binSize);

    /* Initialization of the interface with Metrology Lib */
    gDrvMetObj.metRegisters = (MET_REGISTERS *)metInit->regBaseAddress;

    (void) memset(&gDrvMetObj.metAccData, 0, sizeof(DRV_METROLOGY_REGS_ACCUMULATORS));
    (void) memset(&gDrvMetObj.metHarData, 0, sizeof(DRV_METROLOGY_REGS_HARMONICS));
    (void) memset(&gDrvMetObj.calibrationData, 0, sizeof(DRV_METROLOGY_CALIBRATION));
    (void) memset(&gDrvMetObj.metAFEData, 0, sizeof(DRV_METROLOGY_AFE_DATA));

    /* Initialization of the Metrology object */
    gDrvMetObj.status = DRV_METROLOGY_STATUS_HALT;

    lDRV_Metrology_IpcInitialize();

    return (SYS_MODULE_OBJ)&gDrvMetObj;
}

DRV_METROLOGY_RESULT DRV_METROLOGY_Open (DRV_METROLOGY_START_MODE mode, DRV_METROLOGY_REGS_CONTROL * pConfiguration)
{
    if (gDrvMetObj.inUse == false)
    {
        return DRV_METROLOGY_ERROR;
    }

    if (gDrvMetObj.status != DRV_METROLOGY_STATUS_HALT)
    {
        return DRV_METROLOGY_ERROR;
    }

    gDrvMetObj.status = DRV_METROLOGY_STATUS_WAITING_IPC;

    /* Enable IPC1 Interrupt Source */
    SYS_INT_SourceStatusClear(IPC1_IRQn);
    SYS_INT_SourceEnable(IPC1_IRQn);

    if (mode == DRV_METROLOGY_START_HARD)
    {
        /* Enable the coprocessor clock (Core 1) */
        CLK_Core1ProcessorClkEnable();

        /* De-assert the reset of the coprocessor (Core 1) */
        RSTC_CoProcessorEnable(true);

        /* Wait IPC Init interrupt */
        while(gDrvMetObj.status == DRV_METROLOGY_STATUS_WAITING_IPC)
        {
        }

        /* Keep Metrology Lib in reset */
        gDrvMetObj.metRegisters->MET_CONTROL.STATE_CTRL = STATE_CTRL_STATE_CTRL_RESET_Val;

        if ((pConfiguration != NULL) && (pConfiguration->ATSENSE_CTRL_20_23 != 0UL))
        {
            /* Overwrite STATE CTRL register */
            pConfiguration->STATE_CTRL = STATE_CTRL_STATE_CTRL_RESET_Val;
            /* Load external Control values */
            (void) memcpy(&gDrvMetObj.metRegisters->MET_CONTROL, pConfiguration, sizeof(DRV_METROLOGY_REGS_CONTROL));
        }
        else
        {
            /* Load default Control values */
            (void) memcpy(&gDrvMetObj.metRegisters->MET_CONTROL, &gDrvMetControlDefault, sizeof(DRV_METROLOGY_REGS_CONTROL));
        }

        /* Set Metrology Lib state as Init */
        gDrvMetObj.metRegisters->MET_CONTROL.STATE_CTRL = STATE_CTRL_STATE_CTRL_INIT_Val;

        while(gDrvMetObj.metRegisters->MET_STATUS.STATUS != STATUS_STATUS_READY)
        {
        }
    }

    gDrvMetObj.status = DRV_METROLOGY_STATUS_READY;

    return DRV_METROLOGY_SUCCESS;
}

DRV_METROLOGY_RESULT DRV_METROLOGY_Close (void)
{
    if (gDrvMetObj.inUse == false)
    {
        return DRV_METROLOGY_ERROR;
    }

    /* Disable IPC1 Interrupt Source */
    (void) SYS_INT_SourceDisable(IPC1_IRQn);
    NVIC_ClearPendingIRQ(IPC1_IRQn);

    /* Keep Metrology Lib in reset */
    gDrvMetObj.metRegisters->MET_CONTROL.STATE_CTRL = STATE_CTRL_STATE_CTRL_RESET_Val;
    /* Wait until the metrology resets */
    while (gDrvMetObj.metRegisters->MET_STATUS.STATUS != STATUS_STATUS_RESET)
    {
    }

    /* Update Driver state */
    gDrvMetObj.status = DRV_METROLOGY_STATUS_HALT;

    return DRV_METROLOGY_SUCCESS;

}

DRV_METROLOGY_RESULT DRV_METROLOGY_Start (void)
{
    if (gDrvMetObj.metRegisters->MET_STATUS.STATUS == STATUS_STATUS_READY)
    {
        /* Set Metrology Lib state as Run */
        gDrvMetObj.metRegisters->MET_CONTROL.STATE_CTRL = STATE_CTRL_STATE_CTRL_RUN_Val;
        /* Wait until the metrology running */
        while (gDrvMetObj.metRegisters->MET_STATUS.STATUS != STATUS_STATUS_DSP_RUNNING)
        {
        }
        gDrvMetObj.status = DRV_METROLOGY_STATUS_RUNNING;

        return DRV_METROLOGY_SUCCESS;
    }

    return DRV_METROLOGY_ERROR;
}

DRV_METROLOGY_STATUS DRV_METROLOGY_GetStatus(void)
{
    return gDrvMetObj.status;
}

DRV_METROLOGY_RESULT DRV_METROLOGY_IntegrationCallbackRegister (
    DRV_METROLOGY_CALLBACK callback
)
{
    if (callback == NULL)
    {
        return DRV_METROLOGY_ERROR;
    }

    gDrvMetObj.integrationCallback = callback;
    return DRV_METROLOGY_SUCCESS;
}

DRV_METROLOGY_RESULT DRV_METROLOGY_CalibrationCallbackRegister (
    DRV_METROLOGY_CALIBRATION_CALLBACK callback
)
{
    if (callback == NULL)
    {
        return DRV_METROLOGY_ERROR;
    }

    gDrvMetObj.calibrationCallback = callback;
    return DRV_METROLOGY_SUCCESS;
}

DRV_METROLOGY_RESULT DRV_METROLOGY_HarmonicAnalysisCallbackRegister (
    DRV_METROLOGY_HARMONICS_CALLBACK callback
)
{
    if (callback == NULL)
    {
        return DRV_METROLOGY_ERROR;
    }

    gDrvMetObj.harmonicAnalysisCallback = callback;
    return DRV_METROLOGY_SUCCESS;
}

void DRV_METROLOGY_Tasks(SYS_MODULE_OBJ object)
{
    if (object == SYS_MODULE_OBJ_INVALID)
    {
        /* Invalid system object */
        return;
    }

    if (gDrvMetObj.ipcInterruptFlag == false)
    {
        /* There are not IPC interrupts */
        return;
    }

    /* Clear IPC interrupt flag */
    gDrvMetObj.ipcInterruptFlag = false;

    if (gDrvMetObj.integrationFlag == true)
    {
        gDrvMetObj.integrationFlag = false;

        if (gDrvMetObj.harmonicAnalysisData.integrationPeriods > 0U)
        {
            gDrvMetObj.harmonicAnalysisData.integrationPeriods--;
        }

        /* Check if there is a calibration process running */
        if (gDrvMetObj.calibrationData.running == true)
        {
            if (lDRV_METROLOGY_UpdateCalibrationValues() == true)
            {
                /* Launch calibration callback */
                if (gDrvMetObj.calibrationCallback != NULL)
                {
                    gDrvMetObj.calibrationCallback(gDrvMetObj.calibrationData.result);
                }
            }
        }
        else
        {
            /* Update measurements from metrology library registers */
            lDRV_METROLOGY_UpdateMeasurements();

            /* Launch integration callback */
            if (gDrvMetObj.integrationCallback != NULL)
            {
                gDrvMetObj.integrationCallback();
            }

            /* Check if there is a harmonic analysis process running */
            if ((gDrvMetObj.harmonicAnalysisData.running == true) &&
                (gDrvMetObj.harmonicAnalysisData.integrationPeriods == 0U))
            {
                /* Prevent updating of harmonic registers */
                gDrvMetObj.harmonicAnalysisData.holdRegs = true;

                lDRV_METROLOGY_UpdateHarmonicAnalysisValues();
                /* Launch Harmonic Analysis callback */
                if (gDrvMetObj.harmonicAnalysisCallback != NULL)
                {
                    gDrvMetObj.harmonicAnalysisCallback(gDrvMetObj.harmonicAnalysisData.harmonicBitmap);
                }

                /* Reset harmonic registers update flag */
                gDrvMetObj.harmonicAnalysisData.holdRegs = false;
            }
        }
    }
}

DRV_METROLOGY_REGS_STATUS * DRV_METROLOGY_GetStatusData (void)
{
    return &gDrvMetObj.metRegisters->MET_STATUS;
}

DRV_METROLOGY_REGS_CONTROL * DRV_METROLOGY_GetControlData (void)
{
    return &gDrvMetObj.metRegisters->MET_CONTROL;
}

DRV_METROLOGY_REGS_CONTROL * DRV_METROLOGY_GetControlByDefault (void)
{
    /* MISRA C-2012 Rule 11.8 deviated below. Deviation record ID -
      H3_MISRAC_2012_R_11_8_DR_1*/
    return (DRV_METROLOGY_REGS_CONTROL *)&gDrvMetControlDefault;
   /* MISRAC 2012 deviation block end */
}

DRV_METROLOGY_REGS_ACCUMULATORS * DRV_METROLOGY_GetAccData (void)
{
    return &gDrvMetObj.metAccData;
}

DRV_METROLOGY_REGS_HARMONICS * DRV_METROLOGY_GetHarData (void)
{
    return &gDrvMetObj.metHarData;
}

DRV_METROLOGY_CALIBRATION_REFS * DRV_METROLOGY_GetCalibrationReferences (void)
{
    return &gDrvMetObj.calibrationData.references;
}

void DRV_METROLOGY_SetControl (DRV_METROLOGY_REGS_CONTROL * pControl)
{
    /* MISRA C-2012 Rule 11.8 deviated below. Deviation record ID -
      H3_MISRAC_2012_R_11_8_DR_1*/
    /* Keep State Control Register value */
    (void) memcpy((void *)&gDrvMetObj.metRegisters->MET_CONTROL.FEATURE_CTRL,
                  (void *)&pControl->FEATURE_CTRL,
                  sizeof(DRV_METROLOGY_REGS_CONTROL) - sizeof(uint32_t));
   /* MISRAC 2012 deviation block end */
}

uint32_t DRV_METROLOGY_GetEnergyValue(bool restartEnergy)
{
    uint32_t energy = gDrvMetObj.metAFEData.energy;

    if (restartEnergy == true)
    {
        gDrvMetObj.metAFEData.energy = 0UL;
    }

    return energy;
}

uint32_t DRV_METROLOGY_GetMeasureValue(DRV_METROLOGY_MEASURE_TYPE type)
{
    uint32_t value;

    value = gDrvMetObj.metAFEData.measure[type];

    if (type >= MEASURE_ANGLEA)
    {
        if ((value & 0x80000000UL) != 0U)
        {
            value &= 0x7FFFFFFFUL;
        }

        /* Absolute value should be between 0 and 180 degrees */
        while ((value > 18000000UL) == true)
        {
            value -= 36000000UL;
        }
    }

    return value;
}

DRV_METROLOGY_MEASURE_SIGN DRV_METROLOGY_GetMeasureSign(DRV_METROLOGY_MEASURE_TYPE type)
{
    DRV_METROLOGY_MEASURE_SIGN sign;

    if ((type == MEASURE_PA) && (gDrvMetObj.metAFEData.afeEvents.paDir == 1U))
    {
        sign = MEASURE_SIGN_NEGATIVE;
    }
    else if ((type == MEASURE_PB) && (gDrvMetObj.metAFEData.afeEvents.pbDir == 1U))
    {
        sign = MEASURE_SIGN_NEGATIVE;
    }
    else if ((type == MEASURE_PC) && (gDrvMetObj.metAFEData.afeEvents.pcDir == 1U))
    {
        sign = MEASURE_SIGN_NEGATIVE;
    }
    else if ((type == MEASURE_PT) && (gDrvMetObj.metAFEData.afeEvents.ptDir == 1U))
    {
        sign = MEASURE_SIGN_NEGATIVE;
    }
    else if ((type == MEASURE_QA) && (gDrvMetObj.metAFEData.afeEvents.qaDir == 1U))
    {
        sign = MEASURE_SIGN_NEGATIVE;
    }
    else if ((type == MEASURE_QB) && (gDrvMetObj.metAFEData.afeEvents.qbDir == 1U))
    {
        sign = MEASURE_SIGN_NEGATIVE;
    }
    else if ((type == MEASURE_QC) && (gDrvMetObj.metAFEData.afeEvents.qcDir == 1U))
    {
        sign = MEASURE_SIGN_NEGATIVE;
    }
    else if ((type == MEASURE_QT) && (gDrvMetObj.metAFEData.afeEvents.qtDir == 1U))
    {
        sign = MEASURE_SIGN_NEGATIVE;
    }
    else if ((type == MEASURE_PAF) && (gDrvMetObj.metAFEData.afeEvents.pafDir == 1U))
    {
        sign = MEASURE_SIGN_NEGATIVE;
    }
    else if ((type == MEASURE_PBF) && (gDrvMetObj.metAFEData.afeEvents.pbfDir == 1U))
    {
        sign = MEASURE_SIGN_NEGATIVE;
    }
    else if ((type == MEASURE_PCF) && (gDrvMetObj.metAFEData.afeEvents.pcfDir == 1U))
    {
        sign = MEASURE_SIGN_NEGATIVE;
    }
    else if ((type == MEASURE_PTF) && (gDrvMetObj.metAFEData.afeEvents.ptfDir == 1U))
    {
        sign = MEASURE_SIGN_NEGATIVE;
    }
    else if ((type == MEASURE_QAF) && (gDrvMetObj.metAFEData.afeEvents.qafDir == 1U))
    {
        sign = MEASURE_SIGN_NEGATIVE;
    }
    else if ((type == MEASURE_QBF) && (gDrvMetObj.metAFEData.afeEvents.qbfDir == 1U))
    {
        sign = MEASURE_SIGN_NEGATIVE;
    }
    else if ((type == MEASURE_QCF) && (gDrvMetObj.metAFEData.afeEvents.qcfDir == 1U))
    {
        sign = MEASURE_SIGN_NEGATIVE;
    }
    else if ((type == MEASURE_QTF) && (gDrvMetObj.metAFEData.afeEvents.qtfDir == 1U))
    {
        sign = MEASURE_SIGN_NEGATIVE;
    }
    else if ((type >= MEASURE_ANGLEA) && ((gDrvMetObj.metAFEData.measure[type] & 0x80000000UL) != 0U))
    {
        sign = MEASURE_SIGN_NEGATIVE;
    }
    else
    {
        sign = MEASURE_SIGN_POSITIVE;
    }

    return sign;
}

void DRV_METROLOGY_SetConfiguration(DRV_METROLOGY_CONFIGURATION * config)
{
    double res, divisor;
    uint64_t m;
    DRV_METROLOGY_REGS_CONTROL *pControl;
    uint32_t i;
    uint32_t reg, gain;

    /* Store Calibration parameters */
    gDrvMetObj.calibrationData.freq = config->freq;

    pControl = &gDrvMetObj.calibrationData.metControlConf;
    *pControl = gDrvMetObj.metRegisters->MET_CONTROL;

    reg = 1000000000UL / (config->mc);
    m = (uint64_t)reg;
    m = (m << GAIN_P_K_T_Q);
    m = m / 1000000UL;
    reg = (uint32_t)m;

    pControl->P_K_t = reg;
    pControl->Q_K_t = reg;
    pControl->I_K_t = reg;
    pControl->S_K_t = reg;

    reg = pControl->METER_TYPE;
    reg &= ~(METER_TYPE_SENSOR_TYPE_I_A_Msk | METER_TYPE_SENSOR_TYPE_I_B_Msk | METER_TYPE_SENSOR_TYPE_I_C_Msk);
    reg |= METER_TYPE_SENSOR_TYPE_I_A(config->st) |
            METER_TYPE_SENSOR_TYPE_I_B(config->st) | METER_TYPE_SENSOR_TYPE_I_C(config->st);
    pControl->METER_TYPE = reg;

    reg = pControl->ATSENSE_CTRL_20_23;
    reg &= ~(ATSENSE_CTRL_20_23_I0_GAIN_Msk | ATSENSE_CTRL_20_23_I1_GAIN_Msk | ATSENSE_CTRL_20_23_I2_GAIN_Msk);
    reg |= ATSENSE_CTRL_20_23_I0_GAIN(config->gain) |
            ATSENSE_CTRL_20_23_I1_GAIN(config->gain) | ATSENSE_CTRL_20_23_I2_GAIN(config->gain);
    pControl->ATSENSE_CTRL_20_23 = reg;

    reg = pControl->ATSENSE_CTRL_24_27;
    reg &= ~(ATSENSE_CTRL_24_27_I3_GAIN_Msk);
    reg |= ATSENSE_CTRL_24_27_I3_GAIN(config->gain);
    pControl->ATSENSE_CTRL_24_27 = reg;

    gain = (uint32_t)config->gain;
    gain = 1UL << gain;
    if (config->st == SENSOR_CT)
    {
        res = config->tr * 1000000.0; /* improve accuracy * (ohm -> uohm) */
        divisor = config->rl * (double)gain;
        res = res / divisor;
        m = (uint64_t)res;
        m = m << GAIN_VI_Q; /* format Q22.10 */
        m = m / 1000000UL; /* restore accuracy */
        i = (uint32_t)m;
    }
    else if (config->st == SENSOR_ROGOWSKI)
    {
        res = 1000000.0 / config->tr;
        divisor = 60.0 / config->freq;
        divisor *= (double)gain;
        res = res / divisor;
        res *= 10000.0; /* improve accuracy */
        m = (uint64_t)res;
        m = m << GAIN_VI_Q; /* format Q22.10 */
        m = m / 10000UL; /* restore accuracy */
        i = (uint32_t)m;
    }
    else if (config->st == SENSOR_SHUNT)
    {
        divisor = (double)gain * config->rl;
        res = 1000000.0 / divisor;
        res *= 10000.0; /* improve accuracy */
        m = (uint64_t)res;
        m = m << GAIN_VI_Q; /* format Q22.10 */
        m = m / 10000UL; /* restore accuracy */
        i = (uint32_t)m;
    }
    else
    {
        i = 0;
    }

    pControl->K_IA = i;
    pControl->K_IB = i;
    pControl->K_IC = i;
    pControl->K_IN = i;

    i = (config->ku) << GAIN_VI_Q; /* format Q22.10 */
    pControl->K_VA = i;
    pControl->K_VB = i;
    pControl->K_VC = i;

    /* Set CAL registers to default value before calibration */
    pControl->CAL_M_IA = 0x20000000;
    pControl->CAL_M_VA = 0x20000000;
    pControl->CAL_M_IB = 0x20000000;
    pControl->CAL_M_VB = 0x20000000;
    pControl->CAL_M_IC = 0x20000000;
    pControl->CAL_M_VC = 0x20000000;
    pControl->CAL_M_IN = 0x20000000;
    pControl->CAL_PH_IA = 0;
    pControl->CAL_PH_VA = 0;
    pControl->CAL_PH_IB = 0;
    pControl->CAL_PH_VB = 0;
    pControl->CAL_PH_IC = 0;
    pControl->CAL_PH_VC = 0;
    pControl->CAL_PH_IN = 0;

    gDrvMetObj.metRegisters->MET_CONTROL = *pControl;

}

void DRV_METROLOGY_GetEventsData(DRV_METROLOGY_AFE_EVENTS * events)
{
    *events = gDrvMetObj.metAFEData.afeEvents;
}

void DRV_METROLOGY_StartCalibration(void)
{
    DRV_METROLOGY_CALIBRATION * pCalibrationData = &gDrvMetObj.calibrationData;

    if (pCalibrationData->running == false)
    {
        DRV_METROLOGY_REGS_CONTROL * pMetControlRegs = &gDrvMetObj.metRegisters->MET_CONTROL;

        /* Set the number of integration periods to improve the accuracy of the calibration */
        pCalibrationData->numIntegrationPeriods = 4U;

        /* Increase accuracy of references for calibrating procedure */
        pCalibrationData->references.aimIA *= VI_ACCURACY_DOUBLE;
        pCalibrationData->references.aimVA *= VI_ACCURACY_DOUBLE;
        pCalibrationData->references.angleA *= ANGLE_ACCURACY_DOUBLE;
        pCalibrationData->references.aimIB *= VI_ACCURACY_DOUBLE;
        pCalibrationData->references.aimVB *= VI_ACCURACY_DOUBLE;
        pCalibrationData->references.angleB *= ANGLE_ACCURACY_DOUBLE;
        pCalibrationData->references.aimIC *= VI_ACCURACY_DOUBLE;
        pCalibrationData->references.aimVC *= VI_ACCURACY_DOUBLE;
        pCalibrationData->references.angleC *= ANGLE_ACCURACY_DOUBLE;
        pCalibrationData->references.aimIN *= VI_ACCURACY_DOUBLE;
        pCalibrationData->references.angleN *= ANGLE_ACCURACY_DOUBLE;

        /* Save FEATURE_CTRL register value, to be restored after calibration */
        pCalibrationData->featureCtrlBackup = pMetControlRegs->FEATURE_CTRL;

        /* Init Accumulators */
        pCalibrationData->dspAccIa = 0U;
        pCalibrationData->dspAccIb = 0U;
        pCalibrationData->dspAccIc = 0U;
        pCalibrationData->dspAccIn = 0U;
        pCalibrationData->dspAccUa = 0U;
        pCalibrationData->dspAccUb = 0U;
        pCalibrationData->dspAccUc = 0U;
        pCalibrationData->dspAccPa = 0;
        pCalibrationData->dspAccPb = 0;
        pCalibrationData->dspAccPc = 0;
        pCalibrationData->dspAccPn = 0;
        pCalibrationData->dspAccQa = 0;
        pCalibrationData->dspAccQb = 0;
        pCalibrationData->dspAccQc = 0;
        pCalibrationData->dspAccQn = 0;

        /* Initialize calibration registers to the default values */
        switch (pCalibrationData->references.lineId)
        {
            case PHASE_A:
                pMetControlRegs->FEATURE_CTRL =
                    (FEATURE_CTRL_SYNCH(FEATURE_CTRL_SYNCH_A_Val) | FEATURE_CTRL_PHASE_A_EN_Msk);
                break;

            case PHASE_B:
                pMetControlRegs->FEATURE_CTRL =
                    (FEATURE_CTRL_SYNCH(FEATURE_CTRL_SYNCH_B_Val) | FEATURE_CTRL_PHASE_B_EN_Msk);
                break;

            case PHASE_C:
                pMetControlRegs->FEATURE_CTRL =
                    (FEATURE_CTRL_SYNCH(FEATURE_CTRL_SYNCH_C_Val) | FEATURE_CTRL_PHASE_C_EN_Msk);
                break;

            case PHASE_N:
            case PHASE_T:
            case PHASE_TN:
                pMetControlRegs->FEATURE_CTRL =
                        FEATURE_CTRL_SYNCH(FEATURE_CTRL_SYNCH_ACTIVE_PHASE_Val) | FEATURE_CTRL_PHASE_A_EN_Msk |
                        FEATURE_CTRL_PHASE_B_EN_Msk | FEATURE_CTRL_PHASE_C_EN_Msk;
                break;

            default:
                pMetControlRegs->FEATURE_CTRL =
                    (FEATURE_CTRL_SYNCH(FEATURE_CTRL_SYNCH_A_Val) | FEATURE_CTRL_PHASE_A_EN_Msk);
                break;

        }

        pCalibrationData->running = true;
        pCalibrationData->result = false;
    }
}

bool DRV_METROLOGY_StartHarmonicAnalysis(uint32_t harmonicBitmap, DRV_METROLOGY_HARMONICS_RMS *pHarmonicResponse)
{
    if (harmonicBitmap > HARMONIC_CTRL_HARMONIC_m_REQ_Msk)
    {
        /* Requested harmonics out of bounds */
        return false;
    }
    if (gDrvMetObj.harmonicAnalysisData.running)
    {
        /* Harmonic Analysis already running */
        return false;
    }

    gDrvMetObj.harmonicAnalysisData.running = true;
    gDrvMetObj.harmonicAnalysisData.integrationPeriods = 2;
    gDrvMetObj.harmonicAnalysisData.harmonicBitmap = harmonicBitmap;

    /* Set Data pointer to store the Harmonic data result */
    gDrvMetObj.harmonicAnalysisData.pHarmonicAnalysisResponse = pHarmonicResponse;

    /* Set Number of Harmonic for Analysis */
    gDrvMetObj.metRegisters->MET_CONTROL.HARMONIC_CTRL = 0;
    gDrvMetObj.metRegisters->MET_CONTROL.HARMONIC_CTRL = HARMONIC_CTRL_HARMONIC_m_REQ(harmonicBitmap);
    /* Enable Harmonic Analysis */
    gDrvMetObj.metRegisters->MET_CONTROL.HARMONIC_CTRL |= HARMONIC_CTRL_HARMONIC_EN_Msk;

    return true;
}

void DRV_METROLOGY_StopHarmonicAnalysis(void)
{
    /* Clear flags and disable in registers */
    gDrvMetObj.harmonicAnalysisData.running = false;
    gDrvMetObj.metRegisters->MET_CONTROL.HARMONIC_CTRL &=
        (HARMONIC_CTRL_HARMONIC_EN(HARMONIC_CTRL_HARMONIC_EN_DISABLED_Val) |
        HARMONIC_CTRL_HARMONIC_m_REQ_Msk);
}
