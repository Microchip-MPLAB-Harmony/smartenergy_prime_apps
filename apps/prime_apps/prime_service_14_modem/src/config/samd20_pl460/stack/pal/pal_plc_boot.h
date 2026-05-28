/*******************************************************************************
  PRIME PAL PLC PL360 Boot Streamer Header

  Company:
    Microchip Technology Inc.

  File Name:
    pal_plc_boot.h

  Summary:
    Feeds the PL360 firmware image to the PLC PHY driver from the
    external SST26 PL360_CURRENT zone.

  Description:
    The bootloader (v3) stages the PL360 binary in the SST26
    PL360_CURRENT zone (offset 0x100000, 256-byte ZONE_HEADER at
    offset 0, payload from offset 256). The modem application no
    longer carries a copy of the same binary in its own flash; it
    passes the callback declared here to DRV_PLC_PHY_Open so the PLC
    boot sequence streams the image fragment by fragment from SST26.

    Internals:

      - First invocation opens a dedicated DRV_MEMORY client, reads
        the ZONE_HEADER, validates magic ('PLCC') and the payload size
        against the zone capacity, and prepares the streaming cursor.

      - Subsequent invocations issue one synchronous AsyncRead +
        polling DRV_MEMORY_Tasks loop per fragment.

      - When the payload runs out (or anything fails — virgin zone,
        bad magic, transfer error), the callback sets *length = 0 and
        closes the DRV_MEMORY client. The PLC boot driver treats that
        as "boot finished" and the firmware-check that follows will
        fail if no PL360 was actually loaded; that failure is the
        application's cue to enter UART recovery.
*******************************************************************************/

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
    DRV_PLC_BOOT_DATA_CALLBACK implementation that streams PL360
    firmware from SST26 PL360_CURRENT.

  Description:
    Pass this function to DRV_PLC_PHY_Open as the second argument
    instead of NULL. The PLC boot driver invokes it once per fragment
    until *length is set to 0.

    The function is synchronous on the calling thread: each fragment
    triggers an SST26 read that blocks until the SPI transfer
    completes (the function pumps DRV_MEMORY_Tasks while waiting).
    Safe to call from a cooperative scheduler.
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
