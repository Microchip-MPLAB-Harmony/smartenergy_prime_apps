/*******************************************************************************
  Source for the Time Management in PRIME stack

  Company:
    Microchip Technology Inc.

  File Name:
    srv_time_management.c

  Summary:
    Implementation of the Time Management for PRIME.

  Description:
    This file implements the conversion between cycles and base time
    in microseconds.
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

// *****************************************************************************
// *****************************************************************************
// Section: File includes
// *****************************************************************************
// *****************************************************************************

#include <stdbool.h>
#include <stdint.h>
#include "system/time/sys_time.h"
#include "srv_time_management.h"

// *****************************************************************************
// *****************************************************************************
// Section: Static Data
// *****************************************************************************
// *****************************************************************************

/* Time control variables */
static uint64_t srvTimeMngPreviousCounter = 0;
static uint64_t srvTimeMngCurrentTimeUs = 0;

// *****************************************************************************
// *****************************************************************************
// Section: File scope functions
// *****************************************************************************
// *****************************************************************************

uint64_t SRV_TIME_MANAGEMENT_GetTimeUS64(void)
{
    uint64_t counter;
    uint64_t currentTimeUs;
    uint64_t diffCounter;
    uint32_t elapsedUs;

    // Get current cycle counter
    counter = SYS_TIME_Counter64Get();
    // Diff with previous
    diffCounter = (counter - srvTimeMngPreviousCounter);
    // Convert to Us
    elapsedUs = SYS_TIME_CountToUS((uint32_t)diffCounter);
    // Current time in Us
    currentTimeUs = srvTimeMngCurrentTimeUs + elapsedUs;

    // Every time we update counters, there may be up to 1 us error
    // Only update if at least 10 seconds elapsed
    if (elapsedUs >= 10000000U)
    {
        // Update counter Us
        srvTimeMngCurrentTimeUs = currentTimeUs;
        // Update previous counter for next computation
        srvTimeMngPreviousCounter += SYS_TIME_USToCount(elapsedUs);
    }

    return currentTimeUs;
}


uint32_t SRV_TIME_MANAGEMENT_GetTimeUS(void)
{
    uint32_t currentTimeUs;

    // Get the low part of the current timer counter
    currentTimeUs = (uint32_t)SRV_TIME_MANAGEMENT_GetTimeUS64();

    return currentTimeUs;
}


uint64_t SRV_TIME_MANAGEMENT_USToCount(uint32_t timeUs)
{
    uint64_t counter;
    uint32_t currentUs;
    int32_t diffUs;

    // Update reference counters, just in case  is not called periodically
    (void) SRV_TIME_MANAGEMENT_GetTimeUS64();

    // Get the low part of the current timer counter
    currentUs = (uint32_t)srvTimeMngCurrentTimeUs;

    diffUs = (int32_t)timeUs - (int32_t)currentUs;

    if (diffUs < 0)
    {
      counter = srvTimeMngPreviousCounter - SYS_TIME_USToCount((uint32_t)(-diffUs));
    }
    else
    {
      counter = srvTimeMngPreviousCounter + SYS_TIME_USToCount((uint32_t)diffUs);
    }

    return counter;
}


uint32_t SRV_TIME_MANAGEMENT_CountToUS(uint64_t counter)
{
    uint32_t timeUs;
    int64_t diffCount;

    // Update reference counters, just in case it is not called periodically
    (void) SRV_TIME_MANAGEMENT_GetTimeUS64();

    // Get the low part of the current timer counter
    timeUs = (uint32_t)srvTimeMngCurrentTimeUs;

    diffCount = (int64_t)counter - (int64_t)srvTimeMngPreviousCounter;

    if (diffCount < 0)
    {
      timeUs -= SYS_TIME_CountToUS((uint32_t)(-diffCount));
    }
    else
    {
      timeUs += SYS_TIME_CountToUS((uint32_t)diffCount);
    }

    return timeUs;
}

SYS_TIME_HANDLE SRV_TIME_MANAGEMENT_CbRegisterUS ( SYS_TIME_CALLBACK callback,
                        uintptr_t context, uint32_t us, SYS_TIME_CALLBACK_TYPE type )
{
    return SYS_TIME_CallbackRegisterUS(callback, context, us, type);
}

SYS_TIME_HANDLE SRV_TIME_MANAGEMENT_CbRegisterMS ( SYS_TIME_CALLBACK callback,
                        uintptr_t context, uint32_t ms, SYS_TIME_CALLBACK_TYPE type )
{
    return SYS_TIME_CallbackRegisterMS(callback, context, ms, type);
}
