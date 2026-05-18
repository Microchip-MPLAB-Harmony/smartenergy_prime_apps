/*******************************************************************************
  CRC32 Driver Header File

  Company:
    Microchip Technology Inc.

  File Name:
    drv_crc32.h

  Summary:
    Software CRC32 matching the one used by the PRIME firmware upgrade
    service (srv_pcrc / srv_firmware_upgrade).

  Description:
    Polynomial 0x04C11DB7 (IEEE 802.3), MSB-first, no reflection on
    input or output, initial value 0, no final XOR. This is the exact
    variant that srv_pcrc_Get32 computes when the FU service validates
    the downloaded image.

    The implementation uses a 16-entry (nibble) lookup table to keep
    the bootloader footprint small: 64 bytes of ROM for the table
    instead of the 1024 bytes the modem's byte-wide table would need.
    The trade-off is ~2x throughput vs the byte-wide table, which at
    the bootloader's 8 MHz clock still processes a 256 KB image in
    well under a second.

    Typical usage, for an image split across multiple SST26 reads:

        uint32_t crc = 0U;
        while (remaining > 0U) {
            DRV_SST26_Read(addr, buf, chunk);
            crc = DRV_CRC32_Compute(crc, buf, chunk);
            addr += chunk;
            remaining -= chunk;
        }
        if (crc == expectedCrc) { ... }
*******************************************************************************/

#ifndef DRV_CRC32_H
#define DRV_CRC32_H

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
    uint32_t DRV_CRC32_Compute ( uint32_t crc,
                                 const uint8_t *buf,
                                 uint32_t len )

  Summary:
    Updates a running CRC32 value with len bytes from buf.

  Description:
    Pass 0 as crc on the first call. The return value is the new
    running CRC; feed it straight back in for subsequent calls. When
    the last chunk has been consumed the return value is the final
    CRC that can be compared against the expected one.
*/

uint32_t DRV_CRC32_Compute(uint32_t crc, const uint8_t *buf, uint32_t len);

#ifdef __cplusplus
}
#endif

#endif /* DRV_CRC32_H */

/*******************************************************************************
 End of File
*/
