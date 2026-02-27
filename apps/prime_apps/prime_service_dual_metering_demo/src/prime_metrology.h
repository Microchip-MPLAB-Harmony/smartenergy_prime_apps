/*******************************************************************************
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    prime_metrology.h

  Summary:
    This header file provides prototypes and definitions for the PRIME Service
    metrology application.

  Description:
    This header file defines the exchange of metrology data through PRIME.
*******************************************************************************/

#ifndef _PRIME_METROLOGY_H
#define _PRIME_METROLOGY_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

#include "definitions.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

extern "C" {

#endif
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Type Definitions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Application states

  Summary:
    Application states enumeration

  Description:
    This enumeration defines the valid application states.  These states
    determine the behavior of the application at various times.
*/

typedef enum
{
    /* Application's state machine's initial state. */
    APP_PRIME_METROLOGY_STATE_PRIME_OPEN=0,
    APP_PRIME_METROLOGY_STATE_SERVICE_CONFIGURE,
    APP_PRIME_METROLOGY_STATE_SERVICE_TASKS,

} APP_PRIME_METROLOGY_STATES;

typedef struct {
    bool isOpen;
    uint8_t deviceId[32];
    uint8_t deviceIdLen;
    uint16_t baseAddr;
    uint16_t nodeAddr;
    uint8_t dstLsap; 
    uint8_t srcLsap;
    uint8_t linkClass;
} APP_PRIME_METROLOGY_432_CON_INFO;

typedef struct {
	uint8_t meterSerial[13 + 1];
	uint8_t pibFwVersion[16];
	uint8_t pibVendorId[2];
	uint16_t pibProductId[2];
} APP_PRIME_METROLOGY_METER_PARAMS;

// *****************************************************************************
/* Metrology RMS Data

  Summary:
    Holds metrology data.

  Description:
    This structure holds the metrology data to be sent through UDP
    (RMS instantaneous values).

  Remarks:
    None.
 */

typedef struct
{
    /* RMS voltage for phase A */
    float rmsUA;

    /* RMS voltage for phase B */
    float rmsUB;

    /* RMS voltage for phase C */
    float rmsUC;

    /* RMS current for phase A */
    float rmsIA;

    /* RMS current for phase B */
    float rmsIB;

    /* RMS current for phase C */
    float rmsIC;

    /* RMS current for neutral */
    float rmsINI;

    /* RMS current for neutral */
    float rmsINM;

    /* RMS current for neutral */
    float rmsINMI;

    /* RMS active power total */
    float rmsPT;

    /* RMS active power for phase A */
    float rmsPA;

    /* RMS active power for phase B */
    float rmsPB;

    /* RMS active power for phase C */
    float rmsPC;

    /* RMS reactive power total */
    float rmsQT;

    /* RMS reactive power for phase A */
    float rmsQA;

    /* RMS reactive power for phase B */
    float rmsQB;

    /* RMS reactive power for phase C */
    float rmsQC;

    /* RMS aparent power total */
    float rmsST;

    /* RMS aparent power for phase A */
    float rmsSA;

    /* RMS aparent power for phase B */
    float rmsSB;

    /* RMS aparent power for phase C */
    float rmsSC;

    /* Frequency of the line voltage fundamental harmonic component determined
     * by the Metrology library using the dominant phase */
    float freq;

    /* Angle between the voltage and current vectors for phase A */
    float angleA;

    /* Angle between the voltage and current vectors for phase B */
    float angleB;

    /* Angle between the voltage and current vectors for phase C */
    float angleC;

    /* Angle between the voltage and current vectors for neutral */
    float angleN;

} APP_PRIME_METROLOGY_RESPONSE_DATA;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Routines
// *****************************************************************************
// *****************************************************************************
/* These routines are called by drivers when certain events occur.
*/

// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_PRIME_METROLOGY_Initialize ( void )

  Summary:
     MPLAB Harmony application initialization routine.

  Description:
    This function initializes the Harmony application.  It places the
    application in its initial state and prepares it to run so that its
    APP_PRIME_METROLOGY_Tasks function can be called.

  Precondition:
    All other system initialization routines should be called before calling
    this routine (in "SYS_Initialize").

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    APP_PRIME_METROLOGY_Initialize();
    </code>

  Remarks:
    This routine must be called from the SYS_Initialize function.
*/

void APP_PRIME_METROLOGY_Initialize ( void );


/*******************************************************************************
  Function:
    void APP_PRIME_METROLOGY_Tasks ( void )

  Summary:
    MPLAB Harmony Demo application tasks function

  Description:
    This routine is the Harmony Demo application's tasks function.  It
    defines the application's state machine and core logic.

  Precondition:
    The system and application initialization ("SYS_Initialize") should be
    called before calling this.

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    APP_PRIME_METROLOGY_Tasks();
    </code>

  Remarks:
    This routine must be called from SYS_Tasks() routine.
 */

void APP_PRIME_METROLOGY_Tasks( void );

//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif /* _APP_PRIME_METROLOGY_H */

/*******************************************************************************
 End of File
 */

