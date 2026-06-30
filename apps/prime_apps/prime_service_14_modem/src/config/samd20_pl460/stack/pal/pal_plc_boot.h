/*******************************************************************************
  PRIME PAL PLC PL360 Boot Streamer Header

  Company:
    Microchip Technology Inc.

  File Name:
    pal_plc_boot.h

  Summary:
    Streams the PL360 firmware image to the PLC PHY driver from external
    memory.

  Description:
    Declares the boot-data callback used by the PLC PHY driver to load the
    PL360 firmware image from external memory instead of from internal flash.
    Refer to the PRIME PAL documentation for the external-memory layout and the
    bootloader handshake.
*******************************************************************************/
//DOM-IGNORE-BEGIN
/*
Copyright (C) 2026, Microchip Technology Inc., and its subsidiaries. All rights reserved.

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


#ifndef PAL_PLC_BOOT_H
#define PAL_PLC_BOOT_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// *****************************************************************************
// *****************************************************************************
// Section: Public Function Prototypes
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void PAL_PLC_BOOT_DataCallback (
        uint32_t *address,
        uint16_t *length,
        uintptr_t context )

  Summary:
    Boot-data callback (DRV_PLC_BOOT_DATA_CALLBACK) that supplies the
    PL360 firmware image to the PLC PHY driver.

  Description:
    The PLC PHY driver invokes this callback repeatedly while it boots
    the PL360 device, each call returning the next fragment of the
    firmware image read from the external-memory PL360_CURRENT zone. A
    zero-length fragment signals the end of the image. It is registered
    by passing it to DRV_PLC_PHY_Open() in place of NULL.

    Each call is synchronous: it reads one fragment from external memory
    and blocks (pumping DRV_MEMORY_Tasks) until the SPI transfer
    completes, so it is safe to call from a cooperative scheduler.

  Precondition:
    Registered as the data callback when DRV_PLC_PHY_Open() is called;
    invoked by the PLC boot sequence, not directly by the application.

  Parameters:
    address - (output) Receives the address of the buffer holding the next
              firmware fragment.
    length  - (output) Receives the fragment size in bytes; set to 0 to signal
              the end of the stream (or on any error).
    context - User context supplied by the PLC boot driver (unused).

  Returns:
    None.

  Example:
    <code>
    // Stream the PL360 image from external memory instead of internal flash.
    DRV_PLC_PHY_Open(DRV_PLC_PHY_INDEX, PAL_PLC_BOOT_DataCallback);
    </code>

  Remarks:
    None.
*/

void PAL_PLC_BOOT_DataCallback(uint32_t *address, uint16_t *length,
                               uintptr_t context);

#ifdef __cplusplus
}
#endif

#endif /* PAL_PLC_BOOT_H */

/*******************************************************************************
 End of File
*/
