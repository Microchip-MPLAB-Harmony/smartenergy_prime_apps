/*******************************************************************************
  Header for the Time Management in PRIME stack

  Company:
    Microchip Technology Inc.

  File Name:
    srv_time_management.h

  Summary:
    Interface definition of the Time Management in PRIME.

  Description:
    This file defines the interface for the time management in PRIME.
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
#ifndef TIME_MANAGEMENT_H
#define TIME_MANAGEMENT_H

//DOM-IGNORE-BEGIN
#ifdef __cplusplus // Provide C++ Compatibility
 extern "C" {
#endif
//DOM-IGNORE-END

#include "system/time/sys_time.h"

// *****************************************************************************
// *****************************************************************************
// Section: Time Management Service Interface Definition
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Function:
    uint64_t SRV_TIME_MANAGEMENT_GetTimeUS64(void)

  Summary:
    The SRV_TIME_MANAGEMENT_GetTimeUS64 primitive gets the value of a counter
    and converts it in a 64 bit variable in microseconds.

  Description:
    This primitive makes use of SYS_TIME service to get the value of the
    microseconds counter in order to be able to set timeouts and perform delays.
    This function returns the current value of such counter.

  Precondition:
    SYS_TIME_Initialize primitive has to be called before.

  Parameters:
    None.

  Returns:
    Value of microseconds in 64 bits.

  Example:
    <code>
    uint64_t previousTimeUS = SRV_TIME_MANAGEMENT_GetTimeUS64();

    uint64_t newTimeUS = SRV_TIME_MANAGEMENT_GetTimeUS64();

    if ((newTimeUS - previousTimeUS) > 1000000)
    {
    }
    </code>

  Remarks:
    None.
*/
uint64_t SRV_TIME_MANAGEMENT_GetTimeUS64(void);

// *****************************************************************************
/* Function:
    uint32_t SRV_TIME_MANAGEMENT_GetTimeUS(void)

  Summary:
    The SRV_TIME_MANAGEMENT_GetTimeUS primitive gets the value of a counter
    and converts it in a 32 bit variable in microseconds.

  Description:
    This primitive makes use of SYS_TIME service to get the value of the
    microseconds counter in order to be able to set timeouts and perform delays.
    This function returns the current value of such counter.

  Precondition:
    SYS_TIME_Initialize primitive has to be called before.

  Parameters:
    None.

  Returns:
    Value of microseconds in 32 bits.

  Example:
    <code>
    uint32_t previousTimeUS = SRV_TIME_MANAGEMENT_GetTimeUS();

    uint32_t newTimeUS = SRV_TIME_MANAGEMENT_GetTimeUS();

    if ((newTimeUS - previousTimeUS) > 1000000)
    {
    }
    </code>

  Remarks:
    None.
*/
uint32_t SRV_TIME_MANAGEMENT_GetTimeUS(void);

// *****************************************************************************
/* Function:
    uint64_t SRV_TIME_MANAGEMENT_USToCount(uint32_t timeUs)

  Summary:
    The SRV_TIME_MANAGEMENT_USToCount primitive converts a given time in
    microseconds and returns the equivalent value in cycles.

  Description:
    This primitive makes use of SYS_TIME service to convert a value in
    microseconds and returns a value in cyles.

  Precondition:
    SYS_TIME_Initialize primitive has to be called before.

  Parameters:
    timeUs - Time in microseconds

  Returns:
    Value of cycles counter.

  Example:
    <code>
    uint64_t nextTimeCounter;
    uint32_t nextTimeUS;
    uint32_t currentTimeUS;

    currentTimeUS= SRV_TIME_MANAGEMENT_GetTimeUS();

    nextTimeUS = currentTimeUS + 1000;

    nextTimeCounter = SRV_TIME_MANAGEMENT_USToCount(nextTimeUS);
    </code>

  Remarks:
    Time to convert should be as close as possible to the current time
    to avoid an overflow.
*/
uint64_t SRV_TIME_MANAGEMENT_USToCount(uint32_t timeUs);

// *****************************************************************************
/* Function:
    uint32_t SRV_TIME_MANAGEMENT_CountToUS(uint64_t counter)

  Summary:
    The SRV_TIME_MANAGEMENT_CountToUS primitive converts a given time in cycles
    and returns the equivalent value in microseconds.

  Description:
    This primitive makes use of SYS_TIME service to convert a value
    in cycles and returns a value in microseconds.

  Precondition:
    SYS_TIME_Initialize primitive has to be called before.

  Parameters:
    counter - Cycles of the counter

  Returns:
    Converted time in microseconds.

  Example:
    <code>
    uint64_t timeReceptionCounter;
    uint32_t timeUs;

    timeUs = SRV_TIME_MANAGEMENT_CountToUS(timeReceptionCounter);

    printf("Transmission Reception time:%u", timeUs);
    </code>

  Remarks:
    Cycles to convert should be as close as possible to the current value
    to avoid an overflow.
*/
uint32_t SRV_TIME_MANAGEMENT_CountToUS(uint64_t counter);

// *****************************************************************************
/* Function:
    SYS_TIME_HANDLE SRV_TIME_MANAGEMENT_CbRegisterUS ( SYS_TIME_CALLBACK callback,
                        uintptr_t context, uint32_t us, SYS_TIME_CALLBACK_TYPE type )

   Summary:
        Registers a function with the time system service to be called back when the
        requested number of microseconds have expired (either once or repeatedly).

   Description:
        Creates a timer object and registers a function with it to be called back
        when the requested delay (specified in microseconds) has completed.  The
        caller must identify if the timer should call the function once or repeatedly
        every time the given delay period expires.

   Parameters:
    callback    - Pointer to the function to be called.
                  For single shot timers, the callback cannot be NULL.
                  For periodic timers, if the callback pointer is given as NULL,
                  no callback will occur, but SYS_TIME_TimerPeriodHasExpired can
                  still be polled to determine if the period has expired for a
                  periodic timer.

    context     - A client-defined value that is passed to the callback function.

    us          - Time period in microseconds.

    type        - Type of callback requested. If type is SYS_TIME_SINGLE, the
                  Callback function will be called once when the time period expires.
                  After the time period expires, the timer object will be freed.
                  If type is SYS_TIME_PERIODIC Callback function will be called
                  repeatedly, every time the time period expires until the timer
                  object is stopped or deleted.


   Returns:
        SYS_TIME_HANDLE - A valid timer object handle if the call succeeds.
                      SYS_TIME_HANDLE_INVALID if it fails.

   Example:
      Given a callback function implementation matching the following prototype:
      <code>
      void MyCallback ( uintptr_t context);
      </code>

      The following example call will register it, requesting a 500 microsecond
      periodic callback.
      <code>

      SYS_TIME_HANDLE handle = SRV_TIME_MANAGEMENT_CbRegisterUS(MyCallback, (uintptr_t)0,
                500, SYS_TIME_PERIODIC);
      if (handle != SYS_TIME_HANDLE_INVALID)
      {

      }
      </code>

   Remarks:
       Will give a callback after the requested number of microseconds or longer
       have elapsed, depending on system performance. In tick-based mode, the requested
       delay will be ceiled to the next timer tick. For example, if the
       timer tick is set to 1 msec and the requested delay is 1500 usec, a
       delay of 2 msec will be generated.

       Delay values of 0 will return SYS_TIME_ERROR.
*/

SYS_TIME_HANDLE SRV_TIME_MANAGEMENT_CbRegisterUS ( SYS_TIME_CALLBACK callback,
                        uintptr_t context, uint32_t us, SYS_TIME_CALLBACK_TYPE type );

// *****************************************************************************
/* Function:
    SYS_TIME_HANDLE SRV_TIME_MANAGEMENT_CbRegisterMS ( SYS_TIME_CALLBACK callback,
                        uintptr_t context, uint32_t ms, SYS_TIME_CALLBACK_TYPE type )

   Summary:
        Registers a function with the time system service to be called back when the
        requested number of microseconds have expired (either once or repeatedly).

   Description:
        Creates a timer object and registers a function with it to be called back
        when the requested delay (specified in microseconds) has completed.  The
        caller must identify if the timer should call the function once or repeatedly
        every time the given delay period expires.

   Parameters:
    callback    - Pointer to the function to be called.
                  For single shot timers, the callback cannot be NULL.
                  For periodic timers, if the callback pointer is given as NULL,
                  no callback will occur, but SYS_TIME_TimerPeriodHasExpired can
                  still be polled to determine if the period has expired for a
                  periodic timer.

    context     - A client-defined value that is passed to the callback function.

    ms          - Time period in miliseconds.

    type        - Type of callback requested. If type is SYS_TIME_SINGLE, the
                  Callback function will be called once when the time period expires.
                  After the time period expires, the timer object will be freed.
                  If type is SYS_TIME_PERIODIC Callback function will be called
                  repeatedly, every time the time period expires until the timer
                  object is stopped or deleted.


   Returns:
        SYS_TIME_HANDLE - A valid timer object handle if the call succeeds.
                      SYS_TIME_HANDLE_INVALID if it fails.

   Example:
      Given a callback function implementation matching the following prototype:
      <code>
      void MyCallback ( uintptr_t context);
      </code>

      The following example call will register it, requesting a 1 second
      periodic callback.
      <code>

      SYS_TIME_HANDLE handle = SRV_TIME_MANAGEMENT_CbRegisterMS(MyCallback, (uintptr_t)0,
                1000, SYS_TIME_PERIODIC);
      if (handle != SYS_TIME_HANDLE_INVALID)
      {

      }
      </code>

   Remarks:
       Will give a callback after the requested number of microseconds or longer
       have elapsed, depending on system performance. In tick-based mode, the requested
       delay will be ceiled to the next timer tick. For example, if the
       timer tick is set to 1 msec and the requested delay is 1500 usec, a
       delay of 2 msec will be generated.

       Delay values of 0 will return SYS_TIME_ERROR.
*/

SYS_TIME_HANDLE SRV_TIME_MANAGEMENT_CbRegisterMS ( SYS_TIME_CALLBACK callback,
                        uintptr_t context, uint32_t ms, SYS_TIME_CALLBACK_TYPE type );

//DOM-IGNORE-BEGIN
#ifdef __cplusplus // Provide C++ Compatibility
 }
#endif
//DOM-IGNORE-END

#endif /* TIME_MANAGEMENT_H */
