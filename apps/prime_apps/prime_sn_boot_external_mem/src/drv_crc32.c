/*******************************************************************************
  CRC32 Driver Implementation File

  Company:
    Microchip Technology Inc.

  File Name:
    drv_crc32.c

  Summary:
    Software CRC32 computation using a 16-entry nibble lookup table.

  Description:
    The 16 table entries below are the first 16 entries of the 256-entry
    byte-wide CRC32 table used by the PRIME stack (srv_pcrc.c). This is
    valid because, for a non-reflected CRC32 with polynomial 0x04C11DB7,
    processing a nibble N (0..15) placed at bit position 28 through four
    shift iterations produces the same value as processing byte N placed
    at bit position 24 through eight shift iterations: the first four
    iterations of the byte-wide form have no effect (the top nibble is
    zero for N < 16, so the MSB never triggers an XOR with the
    polynomial), which means the two forms converge to the same result.

    The consequence is that the 16 values below are bit-exact with the
    first 16 entries of pCrcTable32 in srv_pcrc.c. Keeping them verbatim
    makes it easy to see that the bootloader and the application agree
    on the CRC variant.
*******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "drv_crc32.h"

// *****************************************************************************
// *****************************************************************************
// Section: Local Constants
// *****************************************************************************
// *****************************************************************************

/* Nibble table: row i corresponds to CRC32 of the 4-bit value i. */
static const uint32_t crc32NibbleTable[16] =
{
    0x00000000UL, 0x04C11DB7UL, 0x09823B6EUL, 0x0D4326D9UL,
    0x130476DCUL, 0x17C56B6BUL, 0x1A864DB2UL, 0x1E475005UL,
    0x2608EDB8UL, 0x22C9F00FUL, 0x2F8AD6D6UL, 0x2B4BCB61UL,
    0x350C9B64UL, 0x31CD86D3UL, 0x3C8EA00AUL, 0x384FBDBDUL
};

// *****************************************************************************
// *****************************************************************************
// Section: Public Functions
// *****************************************************************************
// *****************************************************************************

uint32_t DRV_CRC32_Compute(uint32_t crc, const uint8_t *buf, uint32_t len)
{
    uint32_t i;
    uint32_t runningCrc;
    uint8_t  byte;
    uint8_t  idx;

    runningCrc = crc;

    for (i = 0U; i < len; i++)
    {
        byte = buf[i];

        /* Process the high nibble first (MSB-first byte order). */
        idx = (uint8_t) (((runningCrc >> 28) ^ ((uint32_t) byte >> 4)) & 0x0FU);
        runningCrc = (runningCrc << 4) ^ crc32NibbleTable[idx];

        /* Then the low nibble. */
        idx = (uint8_t) (((runningCrc >> 28) ^ (uint32_t) byte) & 0x0FU);
        runningCrc = (runningCrc << 4) ^ crc32NibbleTable[idx];
    }

    return runningCrc;
}

/*******************************************************************************
 End of File
*/
