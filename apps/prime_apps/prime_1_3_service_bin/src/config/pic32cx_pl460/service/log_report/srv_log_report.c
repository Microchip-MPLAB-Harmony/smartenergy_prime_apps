/*******************************************************************************
  Source for the log report service

  Company:
    Microchip Technology Inc.

  File Name:
    srv_log_report.c

  Summary:
    Interface implementation for the log report service.

  Description:
    This file implements the interface for the debug report service.
    Debug messages and log information is printed on the console.
    If a display is available, debug code errors will be shown.
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

#include <stdio.h>
#include <stdarg.h>
#include "configuration.h"
#include "srv_log_report.h"

// *****************************************************************************
// *****************************************************************************
// Section: File scope functions
// *****************************************************************************
// *****************************************************************************

/* MISRA C-2012 deviation block start */
/* MISRA C-2012 Rule 17.1 deviated 6 times. Deviation record ID - H3_MISRAC_2012_R_17_1_DR_1 */
/* MISRA C-2012 Rule 21.6 deviated 4 times. Deviation record ID - H3_MISRAC_2012_R_21_1_DR_6 */

static va_list srvLogReportArgs;


void SRV_LOG_REPORT_Message_With_Code(SRV_LOG_REPORT_LEVEL logLevel,
                                      SRV_LOG_REPORT_CODE code,
                                      const char *info, ...)
{
    (void)logLevel;
    (void)info;


}

void SRV_LOG_REPORT_Message(SRV_LOG_REPORT_LEVEL logLevel,
                            const char *info, ...)
{
    (void)logLevel;
    (void)info;
}

void SRV_LOG_REPORT_Buffer(SRV_LOG_REPORT_LEVEL logLevel,
                           const uint8_t *buffer, uint32_t bufferLength,
                           const char *info, ...)
{
    (void)logLevel;
    (void)buffer;
    (void)bufferLength;
    (void)info;
}

/* MISRA C-2012 deviation block end */
