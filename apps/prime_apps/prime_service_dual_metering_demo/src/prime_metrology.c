/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    prime_metrology.c

  Summary:
    This file contains the source code for the PRIME Service metrology 
    application.

  Description:
    This header file manages the exchange of metrology data through PRIME.
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "definitions.h"
#include "prime_metrology.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

const PRIME_API *gPrimeApi;

static SRV_STORAGE_PRIME_MODE_INFO_CONFIG boardInfo;

/* Meter parameters */
static APP_PRIME_METROLOGY_METER_PARAMS meterParams;

/* Connection status: one 4-32 connection per node */
static APP_PRIME_METROLOGY_432_CON_INFO con432Info;

/* Authentication enabled */
static uint8_t ae = 0;

/* Tx buffer */
DL_432_BUFFER buff432;

static bool isDataReceived;

APP_PRIME_METROLOGY_STATES app_prime_metrologyState;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

/* TODO:  Add any necessary callback functions.
*/

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************

static uint8_t _convertHEX2STR(uint8_t uc_num)
{
    if (uc_num > 9) 
    {
        return (uc_num + 0x37);
    } 
    else 
    {
        return (uc_num + 0x30);
    }
}

static void lAPP_PRIME_METROLOGY_GenerateSerial(uint8_t *serialBoard, 
                                                uint8_t *eui48)
{
    uint8_t index;
    uint8_t pos;
    uint8_t num, num1;
    
    memcpy(serialBoard, "ATM", 3);
    
    /* Convert hex to ascii */
    index = 1;
    pos = 3;
    while (pos < 13) 
    {
        num = ((*(eui48 + index) & 0xf0) >> 4);
        num1 = (*(eui48 + index) & 0x0f);
        index++;
        *(serialBoard + pos++) = _convertHEX2STR(num);
        *(serialBoard + pos++) = _convertHEX2STR(num1);
    }
    
    *(serialBoard + pos) = 0;
}

static void lAPP_PRIME_METROLOGY_MLME_GetConfirm(MLME_RESULT status, 
                                                 uint16_t pibAttrib,
                                                 void *pibValue, 
                                                 uint8_t pibSize)
{
    uint16_t temp16;
    uint8_t temp8;

    /* Check result */
    if (status != MLME_RESULT_DONE) {
            return;
    }

    switch (pibAttrib) {
    case PIB_MAC_APP_FW_VERSION:
            memcpy(meterParams.pibFwVersion, ((uint32_t *)pibValue), pibSize);
            break;

    case PIB_MAC_APP_VENDOR_ID:
            temp16 = *((uint16_t *)pibValue);
            meterParams.pibVendorId[0] = (uint8_t)(temp16 >> 8); /* high byte */
            meterParams.pibVendorId[1] = (uint8_t)(temp16 & 0x00FF); /* low byte */
            break;

    case PIB_MAC_APP_PRODUCT_ID:
            temp16 = *((uint16_t *)pibValue);
            meterParams.pibProductId[0] = (uint8_t)(temp16 >> 8); /* high byte */
            meterParams.pibProductId[1] = (uint8_t)(temp16 & 0x00FF); /* low byte */
            break;

    case PIB_MTP_MAC_EUI_48:
    case PIB_MAC_EUI_48:
            lAPP_PRIME_METROLOGY_GenerateSerial(meterParams.meterSerial, pibValue);
            break;

    case PIB_MAC_SEC_PROFILE_USED:
            temp8 =  *((uint8_t *)pibValue);
            /* Set encryption depending on security profile */
            ae = (temp8 == 0 ? 0 : 1);

    default:
            break;
    }
}

static void lAPP_PRIME_METROLOGY_MLME_RegisterIndication(uint8_t *sna, 
                                                         uint8_t sid)
{
    if (boardInfo.primeVersion == PRIME_VERSION_1_4)
    {
	/* Get current security profile */
	gPrimeApi->MlmeGetRequest(PIB_MAC_SEC_PROFILE_USED);
    } 
    else 
    {
        ae = 0;
    }

    /* Launch 432 connection */
    gPrimeApi->Cl432EstablishRequest((uint8_t *)meterParams.meterSerial, 
                                     strlen((const char *)meterParams.meterSerial), 
                                     ae);
}

static void lAPP_PRIME_METROLOGY_MLME_UnregisterIndication(void)
{
    /* Reset connection parameters */
    con432Info.isOpen = false;
    con432Info.baseAddr = CL_432_INVALID_ADDRESS;
    con432Info.nodeAddr = CL_432_INVALID_ADDRESS;
    memset(&con432Info.deviceId, 0, sizeof(con432Info.deviceId));
    con432Info.deviceIdLen = 0;
    con432Info.dstLsap = 0;
    con432Info.srcLsap = 0;
    con432Info.linkClass = 0;
}

static void lAPP_PRIME_METROLOGY_CL432_EstablishConfirm(uint8_t *deviceId,
            uint8_t deviceIdLen, uint16_t dstAddress, uint16_t baseAddress,
            uint8_t ae)
{
    (void)ae;

    con432Info.isOpen = true;
    con432Info.baseAddr = baseAddress;
    con432Info.nodeAddr = dstAddress;

    if (deviceIdLen < sizeof(con432Info.deviceId)) {
        con432Info.deviceIdLen = deviceIdLen;
    } 
    else 
    {
        con432Info.deviceIdLen = sizeof(con432Info.deviceId);
    }

    memcpy(&con432Info.deviceId, deviceId, con432Info.deviceIdLen);
}

static void lAPP_PRIME_METROLOGY_CL432_ReleaseConfirm(uint16_t dstAddress,
                                                      DL_432_RESULT result)
{
    (void)dstAddress;
    (void)result;

    /* Reset connection parameters */
    con432Info.isOpen = false;
    con432Info.baseAddr = CL_432_INVALID_ADDRESS;
    con432Info.nodeAddr = CL_432_INVALID_ADDRESS;
    memset(&con432Info.deviceId, 0, sizeof(con432Info.deviceId));
    con432Info.deviceIdLen = 0;
    con432Info.dstLsap = 0;
    con432Info.srcLsap = 0;
    con432Info.linkClass = 0;
    
    /* Re-launch 432 connection */
    gPrimeApi->Cl432EstablishRequest((uint8_t *)meterParams.meterSerial, 
                                     strlen((const char *)meterParams.meterSerial), 
                                     ae);
}

static void lAPP_PRIME_METROLOGY_CL432_DlDataIndication(uint8_t dstLsap, 
            uint8_t srcLsap, uint16_t dstAddress, uint16_t srcAddress, 
            uint8_t *data, uint16_t lsduLen, uint8_t linkClass)
{
    (void)dstAddress;
    (void)srcAddress;
    (void)data;
    (void)lsduLen;
    
    con432Info.dstLsap = srcLsap;
    con432Info.srcLsap = dstLsap;
    con432Info.linkClass = linkClass;
    
    isDataReceived = true;
}

static void lAPP_PRIME_METROLOGY_CL432_DlDataConfirm(uint8_t dstLsap, 
            uint8_t srcLsap, uint16_t dstAddress, DL_432_TX_STATUS txStatus)
{
    (void)dstLsap;
    (void)srcLsap;
    (void)dstAddress;

    if (txStatus != CL_432_TX_STATUS_SUCCESS)
    {
        SRV_LOG_REPORT_Message(SRV_LOG_REPORT_INFO, 
                               "Error when sending: %d \n\r", (uint8_t)txStatus);
    }
}

static void lAPP_PRIME_METROLOGY_SendData(void)
{   
    /* Metrology data request (RMS instantaneous values) */
    APP_PRIME_METROLOGY_RESPONSE_DATA metData;
    DRV_METROLOGY_MEASURE_SIGN rmsSign;

    /* Get RMS voltage values (without sign) */
    APP_METROLOGY_GetMeasure(MEASURE_UA_RMS, &metData.rmsUA, NULL);
    APP_METROLOGY_GetMeasure(MEASURE_UB_RMS, &metData.rmsUB, NULL);
    APP_METROLOGY_GetMeasure(MEASURE_UC_RMS, &metData.rmsUC, NULL);

    /* Get RMS current values (without sign) */
    APP_METROLOGY_GetMeasure(MEASURE_IA_RMS, &metData.rmsIA, NULL);
    APP_METROLOGY_GetMeasure(MEASURE_IB_RMS, &metData.rmsIB, NULL);
    APP_METROLOGY_GetMeasure(MEASURE_IC_RMS, &metData.rmsIC, NULL);
    APP_METROLOGY_GetMeasure(MEASURE_INI_RMS, &metData.rmsINI, NULL);
    APP_METROLOGY_GetMeasure(MEASURE_INM_RMS, &metData.rmsINM, NULL);
    APP_METROLOGY_GetMeasure(MEASURE_INMI_RMS, &metData.rmsINMI, NULL);

    /* Get RMS active power values (with sign) */
    APP_METROLOGY_GetMeasure(MEASURE_PT, (uint32_t*) &metData.rmsPT, &rmsSign);
    if (rmsSign == MEASURE_SIGN_NEGATIVE)
    {
        metData.rmsPT = -metData.rmsPT;
    }

    APP_METROLOGY_GetMeasure(MEASURE_PA, (uint32_t*) &metData.rmsPA, &rmsSign);
    if (rmsSign == MEASURE_SIGN_NEGATIVE)
    {
        metData.rmsPA = -metData.rmsPA;
    }

    APP_METROLOGY_GetMeasure(MEASURE_PB, (uint32_t*) &metData.rmsPB, &rmsSign);
    if (rmsSign == MEASURE_SIGN_NEGATIVE)
    {
        metData.rmsPB = -metData.rmsPB;
    }

    APP_METROLOGY_GetMeasure(MEASURE_PC, (uint32_t*) &metData.rmsPC, &rmsSign);
    if (rmsSign == MEASURE_SIGN_NEGATIVE)
    {
        metData.rmsPC = -metData.rmsPC;
    }

    /* Get RMS reactive power values (with sign) */
    APP_METROLOGY_GetMeasure(MEASURE_QT, (uint32_t*) &metData.rmsQT, &rmsSign);
    if (rmsSign == MEASURE_SIGN_NEGATIVE)
    {
        metData.rmsQT = -metData.rmsQT;
    }

    APP_METROLOGY_GetMeasure(MEASURE_QA, (uint32_t*) &metData.rmsQA, &rmsSign);
    if (rmsSign == MEASURE_SIGN_NEGATIVE)
    {
        metData.rmsQA = -metData.rmsQA;
    }

    APP_METROLOGY_GetMeasure(MEASURE_QB, (uint32_t*) &metData.rmsQB, &rmsSign);
    if (rmsSign == MEASURE_SIGN_NEGATIVE)
    {
        metData.rmsQB = -metData.rmsQB;
    }

    APP_METROLOGY_GetMeasure(MEASURE_QC, (uint32_t*) &metData.rmsQC, &rmsSign);
    if (rmsSign == MEASURE_SIGN_NEGATIVE)
    {
        metData.rmsQC = -metData.rmsQC;
    }

    /* Get RMS aparent power values (without sign) */
    APP_METROLOGY_GetMeasure(MEASURE_ST, &metData.rmsST, NULL);
    APP_METROLOGY_GetMeasure(MEASURE_SA, &metData.rmsSA, NULL);
    APP_METROLOGY_GetMeasure(MEASURE_SB, &metData.rmsSB, NULL);
    APP_METROLOGY_GetMeasure(MEASURE_SC, &metData.rmsSC, NULL);

    /* Get frequency of the line voltage fundamental harmonic
     * component determined by the Metrology library using the
     * dominant phase */
    APP_METROLOGY_GetMeasure(MEASURE_FREQ, &metData.freq, NULL);

    /* Get angles between the voltage and current vectors
     * (with sign) */
    APP_METROLOGY_GetMeasure(MEASURE_ANGLEA, (uint32_t*) &metData.angleA, &rmsSign);
    if (rmsSign == MEASURE_SIGN_NEGATIVE)
    {
        metData.angleA = -metData.angleA;
    }

    APP_METROLOGY_GetMeasure(MEASURE_ANGLEB, (uint32_t*) &metData.angleB, &rmsSign);
    if (rmsSign == MEASURE_SIGN_NEGATIVE)
    {
        metData.angleB = -metData.angleB;
    }

    APP_METROLOGY_GetMeasure(MEASURE_ANGLEC, (uint32_t*) &metData.angleC, &rmsSign);
    if (rmsSign == MEASURE_SIGN_NEGATIVE)
    {
        metData.angleC = -metData.angleC;
    }

    APP_METROLOGY_GetMeasure(MEASURE_ANGLEN, (uint32_t*) &metData.angleN, &rmsSign);
    if (rmsSign == MEASURE_SIGN_NEGATIVE)
    {
        metData.angleN = -metData.angleN;
    }

    /* Insert metrology data in reply */
    memcpy(&buff432.dl.buff, &metData, sizeof(metData));
    gPrimeApi->Cl432DlDataRequest(con432Info.dstLsap, con432Info.srcLsap, 
                                  con432Info.baseAddr, &buff432, sizeof(metData), 
                                  con432Info.linkClass);
}

static void lAPP_PRIME_METROLOGY_SetCallbacks(void)
{
    MAC_CALLBACKS macCallbacks;
    CL_432_CALLBACKS cl432_callbacks;
    
    memset(&macCallbacks, 0, sizeof(macCallbacks));
    memset(&cl432_callbacks, 0, sizeof(cl432_callbacks));
    
    macCallbacks.mlme_get_cfm = lAPP_PRIME_METROLOGY_MLME_GetConfirm;
    macCallbacks.mlme_register_ind = lAPP_PRIME_METROLOGY_MLME_RegisterIndication;
    macCallbacks.mlme_unregister_ind = lAPP_PRIME_METROLOGY_MLME_UnregisterIndication;

    gPrimeApi->MacSetCallbacks(&macCallbacks);
    
    cl432_callbacks.cl_432_establish_cfm = lAPP_PRIME_METROLOGY_CL432_EstablishConfirm;
    cl432_callbacks.cl_432_release_cfm = lAPP_PRIME_METROLOGY_CL432_ReleaseConfirm;
    cl432_callbacks.cl_432_dl_data_ind = lAPP_PRIME_METROLOGY_CL432_DlDataIndication;
    cl432_callbacks.cl_432_dl_data_cfm = lAPP_PRIME_METROLOGY_CL432_DlDataConfirm;
  
    gPrimeApi->Cl432SetCallbacks(&cl432_callbacks);    
}

// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_PRIME_METROLOGY_Initialize ( void )

  Remarks:
    See prototype in prime_metrology.h.
 */

void APP_PRIME_METROLOGY_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    app_prime_metrologyState = APP_PRIME_METROLOGY_STATE_INIT;

    (void) memset(&boardInfo, 0, sizeof(boardInfo));

    /* Get the PRIME version */
    SRV_STORAGE_GetConfigInfo(SRV_STORAGE_TYPE_MODE_PRIME, 
                              (uint8_t)sizeof(boardInfo),
                              (void *)&boardInfo);

    /* Get PRIME API pointer */
    switch (boardInfo.primeVersion)
    {
        case PRIME_VERSION_1_3:
            PRIME_API_GetPrime13API(&gPrimeApi);
            break;

        case PRIME_VERSION_1_4:
        default:
            PRIME_API_GetPrime14API(&gPrimeApi);
            break;
    }

    isDataReceived = false;
    
    /* Set state */
    app_prime_metrologyState = APP_PRIME_METROLOGY_STATE_SERVICE_CONFIGURE;
}


/******************************************************************************
  Function:
    void APP_PRIME_METROLOGY_Tasks ( void )

  Remarks:
    See prototype in prime_metrology.h.
 */

void APP_PRIME_METROLOGY_Tasks ( void )
{
    /* Check the application's current state. */
    switch ( app_prime_metrologyState )
    {
        /* Application's initial state. */
        case APP_PRIME_METROLOGY_STATE_SERVICE_CONFIGURE:
        {
            /* Check if PRIME stack is ready */
            if (gPrimeApi->Status() == SYS_STATUS_READY)
            {
                /* Set callback functions */
                lAPP_PRIME_METROLOGY_SetCallbacks();
                
                /* Reset connection parameters */
                con432Info.isOpen = false;
                con432Info.baseAddr = CL_432_INVALID_ADDRESS;
                con432Info.nodeAddr = CL_432_INVALID_ADDRESS;
                memset(&con432Info.deviceId, 0, sizeof(con432Info.deviceId));
                con432Info.deviceIdLen = 0;
                
                /* Read parameters for serial number */
                if (boardInfo.primeVersion == PRIME_VERSION_1_3)
                {
                    gPrimeApi->MlmeGetRequest(PIB_MTP_MAC_EUI_48);
                } 
                else 
                {
                    gPrimeApi->MlmeGetRequest(PIB_MAC_EUI_48);
                }

                gPrimeApi->MlmeGetRequest(PIB_MAC_APP_FW_VERSION);
                gPrimeApi->MlmeGetRequest(PIB_MAC_APP_VENDOR_ID);
                gPrimeApi->MlmeGetRequest(PIB_MAC_APP_PRODUCT_ID);

                app_prime_metrologyState = APP_PRIME_METROLOGY_STATE_SERVICE_TASKS;
            }
            
            break;
        }

        case APP_PRIME_METROLOGY_STATE_SERVICE_TASKS:
        {
            if (isDataReceived == true)
            {
                /* Send metrology data */
                lAPP_PRIME_METROLOGY_SendData();
                
                isDataReceived = false;
            }
            
            break;
        }

        /* TODO: implement your application state machine.*/


        /* The default state should never be executed. */
        default:
        {
            /* TODO: Handle error in application's state machine. */
            break;
        }
    }
}


/*******************************************************************************
 End of File
 */
