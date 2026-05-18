/*******************************************************************************
  System Configuration Header

  File Name:
    configuration.h

  Summary:
    Build-time configuration header for the SAMD20 bare-metal bootloader.

  Description:
    Minimal configuration for the prime_sn_boot_external_mem bootloader.
    No Harmony drivers or PLIBs. All peripheral access is done via direct
    register writes from app_bootloader.c.
*******************************************************************************/

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include "user.h"
#include "device.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Nothing else here — bootloader-specific constants live in app_bootloader.h */

#ifdef __cplusplus
}
#endif

#endif /* CONFIGURATION_H */
