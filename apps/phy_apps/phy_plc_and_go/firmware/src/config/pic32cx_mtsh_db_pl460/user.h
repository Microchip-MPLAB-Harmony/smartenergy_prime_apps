/*******************************************************************************
  User Configuration Header

  File Name:
    user.h

  Summary:
    Build-time configuration header for the user defined by this project.

  Description:
    An MPLAB Project may have multiple configurations.  This file defines the
    build-time options for a single configuration.

  Remarks:
    It only provides macro definitions for build-time configuration options

*******************************************************************************/

#ifndef USER_H
#define USER_H

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

extern "C" {

#endif
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: User Configuration macros
// *****************************************************************************
// *****************************************************************************

#define CLEAR_WATCHDOG()              DWDT_WDT0_Clear()

#define USER_BLINK_LED_On()           LED_RED_On()  
#define USER_BLINK_LED_Off()          LED_RED_Off()   
#define USER_BLINK_LED_Toggle()       LED_RED_Toggle()  
    
#define USER_PLC_IND_LED_On()         LED_IR_On()
#define USER_PLC_IND_LED_Off()        LED_IR_Off()
#define USER_PLC_IND_LED_Toggle()     LED_IR_Toggle()

//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif // USER_H
/*******************************************************************************
 End of File
*/
