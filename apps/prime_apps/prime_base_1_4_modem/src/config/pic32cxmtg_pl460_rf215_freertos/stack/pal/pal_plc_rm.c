/*******************************************************************************
  Company:
    Microchip Technology Inc.

  File Name:
    pal_plc_rm.c

  Summary:
    Platform Abstraction Layer (PAL) PLC Robust Management.

  Description:
    Platform Abstraction Layer (PAL) PLC Robust Management source file.
*******************************************************************************/

//DOM-IGNORE-BEGIN
/*
Copyright (C) 2024, Microchip Technology Inc., and its subsidiaries. All rights reserved.

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

/* System includes */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "driver/plc/phy/drv_plc_phy_comm.h"
#include "pal_types.h"
#include "pal_plc_rm.h"

#define CINR_CONV(x)                     (((((int16_t)x) + 1000) * 4) / 100)
#define EVM_INV_CONV(x)                  ((((uint32_t)x) * 512U) / 10U)
#define EVM_INV_ACC_CONV(x)              ((((uint64_t)x) * 131072U * 256U) / 100U)

typedef struct {
	uint16_t evm;
	uint16_t evmAcc;
	PAL_SCHEME modulation;
	uint8_t narBandPercentMin;
	uint8_t narBandPercentMax;
	uint8_t impNoisePercentMin;
	uint8_t impNoisePercentMax;
	uint8_t cinrAvg;
	uint8_t cinrMin;
	uint8_t berSoftAvg;
	uint8_t berSoftMax;
} PAL_PLC_RM_CONDITIONS_DATA;

typedef struct {
	PAL_PLC_RM_CONDITIONS_DATA *pData;
	uint8_t numConditions;
} PAL_PLC_RM_CONDITIONS;

#define NUM_CONDITIONS_R_DBPSK       23
static const PAL_PLC_RM_CONDITIONS_DATA palPlcRmData_R_DBPSK[NUM_CONDITIONS_R_DBPSK] = {
		{1000, 2000, PAL_SCHEME_D8PSK_C,   0,   0,   0,   0, CINR_CONV(  900), CINR_CONV(  450), 133, 136},
		{1000, 2100, PAL_SCHEME_D8PSK_C,   1, 117,   0,   0, CINR_CONV(  425), CINR_CONV(  325), 162, 170},
		{1000, 3250, PAL_SCHEME_DQPSK_C,   0,   0,   0,   0, CINR_CONV(  525), CINR_CONV(  375), 161, 165},
		{1000, 3960, PAL_SCHEME_DQPSK_C,   1,  66,   0,   0, CINR_CONV( -150), CINR_CONV( -250), 166, 173},
		{1000, 2790, PAL_SCHEME_DQPSK_C,  67, 255,   0,   0, CINR_CONV(   50), CINR_CONV(   50), 170, 173},
		{1000, 4710, PAL_SCHEME_DBPSK_C,   0,   0,   0,   0, CINR_CONV(  300), CINR_CONV(   75), 177, 180},
		{1000, 4310, PAL_SCHEME_DBPSK_C,   1,  43,   0,   0, CINR_CONV( -150), CINR_CONV( -600), 176, 177},
		{1000, 4420, PAL_SCHEME_DBPSK_C,  44, 133,   0,   0, CINR_CONV( -175), CINR_CONV( -175), 178, 186},
		{1000, 2960, PAL_SCHEME_DBPSK_C, 134, 146,   0,   0, CINR_CONV(   75), CINR_CONV(  -50), 173, 179},
		{1000, 2810, PAL_SCHEME_DBPSK_C, 147, 255,   0,   0, CINR_CONV(  575), CINR_CONV(  375), 180, 180},
		{1000, 5470, PAL_SCHEME_R_DQPSK,   0,   0,   0,   0, CINR_CONV(  225), CINR_CONV(  150), 182, 182},
		{1000, 5360, PAL_SCHEME_R_DQPSK,   0,   0,   1, 255, CINR_CONV(  100), CINR_CONV(    0), 187, 191},
		{1000, 4630, PAL_SCHEME_R_DQPSK,   1,  53,   0,   0, CINR_CONV( -175), CINR_CONV( -650), 176, 177},
		{1000, 4420, PAL_SCHEME_R_DQPSK,  54, 139,   0,   0, CINR_CONV( -200), CINR_CONV( -200), 178, 186},
		{1000, 3420, PAL_SCHEME_R_DQPSK, 140, 149,   0,   0, CINR_CONV(  125), CINR_CONV(  -50), 178, 178},
		{1000, 3230, PAL_SCHEME_R_DQPSK, 150, 255,   0,   0, CINR_CONV(  575), CINR_CONV(  375), 179, 179},
		{1000, 5950, PAL_SCHEME_R_DBPSK,   0,   0,   0,   0, CINR_CONV(  100), CINR_CONV(  -75), 187, 191},
		{1000, 6890, PAL_SCHEME_R_DBPSK,   0,   0,   1, 255, CINR_CONV(  100), CINR_CONV(    0), 190, 190},
		{1000, 4630, PAL_SCHEME_R_DBPSK,   1,  53,   0,   0, CINR_CONV( -175), CINR_CONV( -650), 176, 177},
		{1000, 4420, PAL_SCHEME_R_DBPSK,  54,  75,   0,   0, CINR_CONV( -200), CINR_CONV( -200), 185, 186},
		{1000, 3560, PAL_SCHEME_R_DBPSK,  76, 139,   0,   0, CINR_CONV( -200), CINR_CONV( -200), 173, 173},
		{1000, 2940, PAL_SCHEME_R_DBPSK, 140, 149,   0,   0, CINR_CONV(  125), CINR_CONV(  -50), 177, 180},
		{1000, 2760, PAL_SCHEME_R_DBPSK, 149, 255,   0,   0, CINR_CONV(  575), CINR_CONV(  375), 180, 180},
};

#define NUM_CONDITIONS_R_DQPSK       15
static const PAL_PLC_RM_CONDITIONS_DATA palPlcRmData_R_DQPSK[NUM_CONDITIONS_R_DQPSK] = {
		{1000, 1670, PAL_SCHEME_D8PSK_C,   0,   0,   0,   0, CINR_CONV(  900), CINR_CONV(  675), 149, 152},
		{1000, 1620, PAL_SCHEME_D8PSK_C,   1, 123,   0,   0, CINR_CONV(  550), CINR_CONV(  200), 164, 165},
		{1000, 2840, PAL_SCHEME_DQPSK_C,   0,   0,   0,   0, CINR_CONV(  550), CINR_CONV(  475), 175, 177},
		{1000, 2570, PAL_SCHEME_DQPSK_C,   1, 141,   0,   0, CINR_CONV( -150), CINR_CONV( -150), 173, 181},
		{1000, 1800, PAL_SCHEME_DQPSK_C, 142, 255,   0,   0, CINR_CONV(  525), CINR_CONV(  150), 173, 173},
		{1000, 4590, PAL_SCHEME_DBPSK_C,   0,   0,   0,   0, CINR_CONV(  400), CINR_CONV(  100), 184, 184},
		{1000, 3740, PAL_SCHEME_DBPSK_C,   1,  75,   0,   0, CINR_CONV( -175), CINR_CONV( -575), 180, 182},
		{1000, 2740, PAL_SCHEME_DBPSK_C,  76, 139,   0,   0, CINR_CONV( -200), CINR_CONV( -200), 177, 185},
		{1000, 2960, PAL_SCHEME_DBPSK_C, 140, 144,   0,   0, CINR_CONV(  900), CINR_CONV(  650), 179, 181},
		{1000, 1800, PAL_SCHEME_DBPSK_C, 144, 146,   0,   0, CINR_CONV(  525), CINR_CONV(  150), 173, 173},
		{1000, 2420, PAL_SCHEME_DBPSK_C, 147, 255,   0,   0, CINR_CONV(  700), CINR_CONV(  400), 181, 181},
		{1000, 5380, PAL_SCHEME_R_DQPSK,   0,   0,   0,   0, CINR_CONV(  200), CINR_CONV(    0), 187, 187},
		{1000, 6900, PAL_SCHEME_R_DQPSK,   0,   0,   1, 255, CINR_CONV(  100), CINR_CONV(   25), 190, 190},
		{1000, 4160, PAL_SCHEME_R_DQPSK,   1, 139,   0,   0, CINR_CONV( -200), CINR_CONV( -625), 178, 182},
		{1000, 2780, PAL_SCHEME_R_DQPSK, 140, 255,   0,   0, CINR_CONV(  250), CINR_CONV(    0), 178, 178},
};

#define NUM_CONDITIONS_DBPSK_C       18
static const PAL_PLC_RM_CONDITIONS_DATA palPlcRmData_DBPSK_C[NUM_CONDITIONS_DBPSK_C] = {
		{1000, 1770, PAL_SCHEME_D8PSK_C,   0,   0,   0,   0, CINR_CONV(  950), CINR_CONV(  450), 148, 148},
		{1000, 2530, PAL_SCHEME_D8PSK_C,   1, 136,   0,   0, CINR_CONV(  400), CINR_CONV(  300), 163, 163},
		{1000, 1710, PAL_SCHEME_D8PSK_C, 137, 255,   0,   0, CINR_CONV(  400), CINR_CONV(  275), 169, 169},
		{1000, 2800, PAL_SCHEME_DQPSK_C,   0,   0,   0,   0, CINR_CONV(  550), CINR_CONV(  425), 159, 159},
		{1000, 2560, PAL_SCHEME_DQPSK_C,   1, 152,   0,   0, CINR_CONV(  175), CINR_CONV(   25), 171, 171},
		{1000, 2020, PAL_SCHEME_DQPSK_C, 153, 255,   0,   0, CINR_CONV( 1050), CINR_CONV( 1025), 171, 171},
		{1000, 4530, PAL_SCHEME_DBPSK_C,   0,   0,   0,   0, CINR_CONV(  350), CINR_CONV(  225), 178, 178},
		{1000, 4590, PAL_SCHEME_DBPSK_C,   1,  59,   0,   0, CINR_CONV( -200), CINR_CONV( -550), 177, 177},
		{1000, 3890, PAL_SCHEME_DBPSK_C,  60, 139,   0,   0, CINR_CONV( -175), CINR_CONV( -175), 172, 172},
		{1000, 2650, PAL_SCHEME_DBPSK_C, 140, 156,   0,   0, CINR_CONV(  -50), CINR_CONV(  -75), 176, 176},
		{1000, 2710, PAL_SCHEME_DBPSK_C, 157, 255,   0,   0, CINR_CONV(  575), CINR_CONV(  400), 180, 180},
		{1000, 5120, PAL_SCHEME_R_DQPSK,   0,   0,   0,   0, CINR_CONV(  175), CINR_CONV(   75), 186, 186},
		{1000, 5220, PAL_SCHEME_R_DQPSK,   0,   0,   1,  67, CINR_CONV(  400), CINR_CONV(    0), 190, 190},
		{1000, 5200, PAL_SCHEME_R_DQPSK,   0,   0,  68, 255, CINR_CONV(  250), CINR_CONV(    0), 177, 177},
		{1000, 4680, PAL_SCHEME_R_DQPSK,   1,  34,   0,   0, CINR_CONV( -350), CINR_CONV( -600), 177, 177},
		{1000, 4030, PAL_SCHEME_R_DQPSK,  35, 139,   0,   0, CINR_CONV( -200), CINR_CONV( -625), 174, 174},
		{1000, 3790, PAL_SCHEME_R_DQPSK, 140, 157,   0,   0, CINR_CONV(  -50), CINR_CONV(  -75), 173, 173},
		{1000, 2710, PAL_SCHEME_R_DQPSK, 158, 255,   0,   0, CINR_CONV(  575), CINR_CONV(  400), 180, 180},
};

#define NUM_CONDITIONS_DQPSK_C       12
static const PAL_PLC_RM_CONDITIONS_DATA palPlcRmData_DQPSK_C[NUM_CONDITIONS_DQPSK_C] = {
		{ 439,  960, PAL_SCHEME_DQPSK,     0,   0,   0,   0, CINR_CONV( 1275), CINR_CONV(  700), 124, 124},
		{1000, 1400, PAL_SCHEME_D8PSK_C,   0,   0,   0,   0, CINR_CONV(  900), CINR_CONV(  725), 154, 154},
		{1000, 2530, PAL_SCHEME_D8PSK_C,   1, 136,   0,   0, CINR_CONV(  500), CINR_CONV(  175), 166, 166},
		{1000, 1910, PAL_SCHEME_D8PSK_C, 137, 255,   0,   0, CINR_CONV(  775), CINR_CONV(  700), 173, 173},
		{1000, 2820, PAL_SCHEME_DQPSK_C,   0,   0,   0,   0, CINR_CONV(  575), CINR_CONV(  425), 172, 172},
		{1000, 3310, PAL_SCHEME_DQPSK_C,   1,  27,   0,   0, CINR_CONV(  -50), CINR_CONV( -175), 170, 170},
		{1000, 2430, PAL_SCHEME_DQPSK_C,  28, 255,   0,   0, CINR_CONV(   75), CINR_CONV(   50), 176, 176},
		{1000, 3960, PAL_SCHEME_DBPSK_C,   0,   0,   0,   0, CINR_CONV(  325), CINR_CONV(  200), 183, 183},
		{1000, 4100, PAL_SCHEME_DBPSK_C,   1, 139,   0,   0, CINR_CONV( -300), CINR_CONV( -550), 177, 177},
		{1000, 2960, PAL_SCHEME_DBPSK_C, 140, 160,   0,   0, CINR_CONV(  250), CINR_CONV(   25), 177, 177},
		{1000, 3530, PAL_SCHEME_DBPSK_C, 144, 157,   0,   0, CINR_CONV(  975), CINR_CONV(  525), 180, 180},
		{1000, 2490, PAL_SCHEME_DBPSK_C, 158, 255,   0,   0, CINR_CONV(  625), CINR_CONV(  400), 180, 180},
};

#define NUM_CONDITIONS_D8PSK_C       11
static const PAL_PLC_RM_CONDITIONS_DATA palPlcRmData_D8PSK_C[NUM_CONDITIONS_D8PSK_C] = {
		{ 402,  760, PAL_SCHEME_DQPSK,     0,   0,   0,   0, CINR_CONV( 1000), CINR_CONV(  650), 160, 160},
		{1000, 1290, PAL_SCHEME_D8PSK_C,   0,   0,   0,   0, CINR_CONV(  950), CINR_CONV(  550), 168, 168},
		{1000, 2390, PAL_SCHEME_D8PSK_C,   1, 101,   0,   0, CINR_CONV(  -25), CINR_CONV(  -25), 170, 170},
		{1000, 3980, PAL_SCHEME_D8PSK_C,  91, 101,   0,   0, CINR_CONV(  450), CINR_CONV(  200), 175, 175},
		{1000, 1720, PAL_SCHEME_D8PSK_C, 102, 255,   0,   0, CINR_CONV(  575), CINR_CONV(  300), 171, 171},
		{1000, 2760, PAL_SCHEME_DQPSK_C,   0,   0,   0,   0, CINR_CONV(  550), CINR_CONV(  475), 178, 178},
		{1000, 2540, PAL_SCHEME_DQPSK_C,   1,  91,   0,   0, CINR_CONV( -150), CINR_CONV( -975), 178, 178},
		{1000, 4790, PAL_SCHEME_DQPSK_C,  91, 139,   0,   0, CINR_CONV( -200), CINR_CONV( -200), 178, 178},
		{1000, 2270, PAL_SCHEME_DQPSK_C, 140, 149,   0,   0, CINR_CONV(  425), CINR_CONV(  225), 183, 183},
		{1000, 1460, PAL_SCHEME_DQPSK_C, 144, 149,   0,   0, CINR_CONV( -100), CINR_CONV( -100), 172, 172},
		{1000, 2240, PAL_SCHEME_DQPSK_C, 150, 255,   0,   0, CINR_CONV(  850), CINR_CONV(  600), 183, 183},
};

#define NUM_CONDITIONS_DBPSK         7
static const PAL_PLC_RM_CONDITIONS_DATA palPlcRmData_DBPSK[NUM_CONDITIONS_DBPSK] = {
		{1000, 1320, PAL_SCHEME_D8PSK_C,   0,   0,   0,   0, CINR_CONV(  650), CINR_CONV(  525), 143, 143},
		{1000, 3840, PAL_SCHEME_D8PSK_C,   1, 107,   0,   0, CINR_CONV(  150), CINR_CONV(    0), 165, 165},
		{1000, 1500, PAL_SCHEME_D8PSK_C, 108, 255,   0,   0, CINR_CONV(  975), CINR_CONV(  525), 170, 170},
		{1000, 2470, PAL_SCHEME_DQPSK_C,   0,   0,   0,   0, CINR_CONV(  550), CINR_CONV(  450), 162, 162},
		{1000, 4090, PAL_SCHEME_DQPSK_C,   1, 107,   0,   0, CINR_CONV( -125), CINR_CONV( -125), 170, 170},
		{1000, 2180, PAL_SCHEME_DQPSK_C, 108, 133,   0,   0, CINR_CONV(  475), CINR_CONV(  325), 170, 170},
		{1000, 1420, PAL_SCHEME_DQPSK_C, 134, 255,   0,   0, CINR_CONV( 1075), CINR_CONV(  650), 170, 170},
};

#define NUM_CONDITIONS_DQPSK         7
static const PAL_PLC_RM_CONDITIONS_DATA palPlcRmData_DQPSK[NUM_CONDITIONS_DQPSK] = {
		{  306,  400, PAL_SCHEME_D8PSK,     0,   0,   0,   0,CINR_CONV( 1900), CINR_CONV(  950), 102, 102},
		{  417,  790, PAL_SCHEME_DQPSK,     0,   0,   0,   0,CINR_CONV( 1250), CINR_CONV(  725), 124, 124},
		{  648, 1510, PAL_SCHEME_D8PSK_C,   0,   0,   0,   0,CINR_CONV(  875), CINR_CONV(  575), 150, 150},
		{ 1000, 1730, PAL_SCHEME_D8PSK_C,   1, 108,   0,   0,CINR_CONV(  175), CINR_CONV(    0), 172, 172},
		{ 1000, 1690, PAL_SCHEME_D8PSK_C, 109, 117,   0,   0,CINR_CONV(  300), CINR_CONV(  175), 170, 170},
		{ 1000, 1340, PAL_SCHEME_D8PSK_C, 118, 125,   0,   0,CINR_CONV(  150), CINR_CONV(    0), 163, 163},
		{ 1000, 1910, PAL_SCHEME_D8PSK_C, 126, 255,   0,   0,CINR_CONV(  275), CINR_CONV(  200), 175, 175},
};

#define NUM_CONDITIONS_D8PSK         2
static const PAL_PLC_RM_CONDITIONS_DATA palPlcRmData_D8PSK[NUM_CONDITIONS_D8PSK] = {
		{238,  400, PAL_SCHEME_D8PSK,      0,   0,   0,   0,  CINR_CONV( 1525), CINR_CONV(  850), 139, 139},
		{429,  920, PAL_SCHEME_DQPSK,      0,   0,   0,   0,  CINR_CONV( 1150), CINR_CONV(  625), 151, 151},
};

static const PAL_PLC_RM_CONDITIONS palPlcRmConditions[] =
{
	{(PAL_PLC_RM_CONDITIONS_DATA *)&palPlcRmData_DBPSK, NUM_CONDITIONS_DBPSK},
	{(PAL_PLC_RM_CONDITIONS_DATA *)&palPlcRmData_DQPSK, NUM_CONDITIONS_DQPSK},
	{(PAL_PLC_RM_CONDITIONS_DATA *)&palPlcRmData_D8PSK, NUM_CONDITIONS_D8PSK},
	{NULL, 0},
	{(PAL_PLC_RM_CONDITIONS_DATA *)&palPlcRmData_DBPSK_C, NUM_CONDITIONS_DBPSK_C},
	{(PAL_PLC_RM_CONDITIONS_DATA *)&palPlcRmData_DQPSK_C, NUM_CONDITIONS_DQPSK_C},
	{(PAL_PLC_RM_CONDITIONS_DATA *)&palPlcRmData_D8PSK_C, NUM_CONDITIONS_D8PSK_C},
	{NULL, 0},
	{NULL, 0},
	{NULL, 0},
	{NULL, 0},
	{NULL, 0},
	{(PAL_PLC_RM_CONDITIONS_DATA *)&palPlcRmData_R_DBPSK, NUM_CONDITIONS_R_DBPSK},
	{(PAL_PLC_RM_CONDITIONS_DATA *)&palPlcRmData_R_DQPSK, NUM_CONDITIONS_R_DQPSK},
};

/* Bandwidth of every modulation */
static const uint8_t palPlcRmBandwidth[] = {
		40,  /* SCHEME_DBPSK */
		80,  /* SCHEME_DQPSK */
		120, /* SCHEME_D8PSK */
		0,
		20,  /* SCHEME_DBPSK_C */
		40,  /* SCHEME_DQPSK_C */
		60,  /* SCHEME_D8PSK_C */
		0,
		0,
		0,
		0,
		0,
		5,   /* SCHEME_R_DBPSK */
		10,  /* SCHEME_R_DQPSK */
		0,
		0    /* PAL_OUTDATED_INF */
};

uint8_t PAL_PLC_RM_GetLqi(uint8_t cinr)
{
	return ((cinr + 12U) / 4U);
}

uint8_t PAL_PLC_RM_GetLessRobustModulation(PAL_SCHEME mod1, PAL_SCHEME mod2)
{
	if (palPlcRmBandwidth[mod1] > palPlcRmBandwidth[mod2])
	{
		return (uint8_t)(mod1);
	}
	else
	{
		return (uint8_t)(mod2);
	}
}

bool PAL_PLC_RM_CheckMinimumQuality(PAL_SCHEME reference, PAL_SCHEME modulation)
{
	if ((palPlcRmBandwidth[modulation] >= palPlcRmBandwidth[reference]) && (palPlcRmBandwidth[modulation] > 0U))
	{
		return true;
	}
	else
	{
		return false;
	}
}

void PAL_PLC_RM_GetRobustModulation(void *indObj, uint16_t *pBitRate, PAL_SCHEME *pModulation, uint16_t pch)
{
	uint64_t evmAccumulated;
	uint32_t evm;
	const PAL_PLC_RM_CONDITIONS_DATA *pConditionData;
	DRV_PLC_PHY_RECEPTION_OBJ *pIndObj;
	uint8_t index;
	uint8_t numConditions;
	uint8_t bestModulation;

	pIndObj = (DRV_PLC_PHY_RECEPTION_OBJ *)indObj;

	/* Get conditions for the given modulation */
	numConditions = palPlcRmConditions[pIndObj->scheme].numConditions;
	pConditionData = palPlcRmConditions[pIndObj->scheme].pData;

	bestModulation = PAL_OUTDATED_INF;
	for (index = 0; index < numConditions; index++)
	{
		evm = EVM_INV_CONV(pConditionData->evm);
		evmAccumulated = EVM_INV_ACC_CONV(pConditionData->evmAcc);

		if ((pIndObj->narBandPercent >= pConditionData->narBandPercentMin) &&
			(pIndObj->narBandPercent <= pConditionData->narBandPercentMax) &&
			(pIndObj->impNoisePercent >= pConditionData->impNoisePercentMin) &&
			(pIndObj->impNoisePercent <= pConditionData->impNoisePercentMax) &&
			(pIndObj->evmPayload <= evm) &&
			(pIndObj->evmHeader <= evm) &&
			(pIndObj->evmHeaderAcum <= (uint32_t)evmAccumulated) &&
			(pIndObj->evmPayloadAcum <= (uint32_t)evmAccumulated) &&
			(pIndObj->cinrAvg >= pConditionData->cinrAvg) &&
			(pIndObj->cinrMin >= pConditionData->cinrMin) &&
			(pIndObj->berSoftAvg <= pConditionData->berSoftAvg) &&
			(pIndObj->berSoftMax <= pConditionData->berSoftMax))
		{
				bestModulation = (uint8_t)pConditionData->modulation;
				break;
		}

		pConditionData++;
	}

	*pModulation = (PAL_SCHEME)(bestModulation);
	if (pch >= (uint16_t)CHN1_CHN2)
	{
		*pBitRate = (uint16_t)palPlcRmBandwidth[bestModulation] << 1;
	}
	else
	{
		*pBitRate = (uint16_t)palPlcRmBandwidth[bestModulation];
	}
}
