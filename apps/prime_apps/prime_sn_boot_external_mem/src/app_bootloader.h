/*******************************************************************************
  PRIME Service Node Bootloader Header

  Company:
    Microchip Technology Inc.

  File Name:
    app_bootloader.h

  Summary:
    Constants, types and prototypes for the bare-metal SAMD20J18 bootloader.

  Description:
    The bootloader reads the BOOT_MODE_INFO handshake from the SST26
    BOOT_FLAG sector, optionally installs a firmware bundle from the SST26
    DOWNLOAD zone or restores the previous image from the REVERT zones, and
    finally jumps to the application at APP_START. No Harmony, no C library:
    all peripherals are accessed through direct register writes.
*******************************************************************************/

//DOM-IGNORE-BEGIN
/*
Copyright (C) 2026 Microchip Technology Inc., and its subsidiaries. All rights reserved.

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

#ifndef APP_BOOTLOADER_H
#define APP_BOOTLOADER_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include <stdbool.h>

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

extern "C" {

#endif
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Macro Definitions - Internal flash layout (SAMD20J18, 256 KB)
// *****************************************************************************
// *****************************************************************************

#define APP_BOOTLOADER_APP_START            (0x00002000U)
#define APP_BOOTLOADER_APP_END              (0x0003FF00U)   /* exclusive; EEPROM row untouched */
#define APP_BOOTLOADER_MAX_APP_SIZE         (APP_BOOTLOADER_APP_END - APP_BOOTLOADER_APP_START)

#define APP_BOOTLOADER_FLASH_ROW_SIZE       (256U)          /* NVMCTRL erase granularity */

// *****************************************************************************
// *****************************************************************************
// Section: Macro Definitions - SST26 geometry constants
// *****************************************************************************
// *****************************************************************************

#define APP_BOOTLOADER_SST26_SECTOR_SIZE        (4096U)
#define APP_BOOTLOADER_SST26_BLOCK_64K_SIZE     (65536U)
#define APP_BOOTLOADER_SST26_PAGE_SIZE          (256U)

// *****************************************************************************
// *****************************************************************************
// Section: SST26 Layout v3 - zone offsets and sizes
// *****************************************************************************
// *****************************************************************************

/* External serial flash (SST26VF064B, 8 MB) is partitioned into named zones
 * shared between this bootloader and the modem application. */

#define APP_BOOTLOADER_SST26_DOWNLOAD_OFFSET    (0x000000U)
#define APP_BOOTLOADER_SST26_DOWNLOAD_SIZE      (0x080000U)   /* 512 KB */

#define APP_BOOTLOADER_SST26_APP_CURRENT_OFFSET (0x080000U)
#define APP_BOOTLOADER_SST26_APP_CURRENT_SIZE   (0x040000U)   /* 256 KB */

#define APP_BOOTLOADER_SST26_APP_REVERT_OFFSET  (0x0C0000U)
#define APP_BOOTLOADER_SST26_APP_REVERT_SIZE    (0x040000U)   /* 256 KB */

#define APP_BOOTLOADER_SST26_PL360_CURRENT_OFFSET (0x100000U)
#define APP_BOOTLOADER_SST26_PL360_CURRENT_SIZE   (0x020000U) /* 128 KB */

#define APP_BOOTLOADER_SST26_PL360_REVERT_OFFSET  (0x120000U)
#define APP_BOOTLOADER_SST26_PL360_REVERT_SIZE    (0x020000U) /* 128 KB */

#define APP_BOOTLOADER_SST26_BOOT_FLAG_OFFSET     (0x140000U)
#define APP_BOOTLOADER_SST26_BOOT_FLAG_SIZE       (0x001000U) /* 4 KB sector */

// *****************************************************************************
// *****************************************************************************
// Section: BOOT_MODE_INFO - bootloader handshake (v3, in SST26 BOOT_FLAG zone)
// *****************************************************************************
// *****************************************************************************

/* 12-byte structure persisted at the start of the BOOT_FLAG sector. The
 * modem application writes it to request an action at the next reset; the
 * bootloader reads it on every boot and dispatches accordingly. */

#define APP_BOOTLOADER_BOOT_MODE_MAGIC          (0x444F4D42UL)  /* 'BMOD' little-endian */

typedef enum
{
    APP_BOOTLOADER_BOOT_MODE_NORMAL          = 0x00,    /* Jump to app */
    APP_BOOTLOADER_BOOT_MODE_INSTALL_PENDING = 0x01,    /* Install bundle from DOWNLOAD */
    APP_BOOTLOADER_BOOT_MODE_REVERT_PENDING  = 0x02,    /* Restore from REVERT zones */
    APP_BOOTLOADER_BOOT_MODE_UART_PENDING    = 0x03,    /* Enter UART recovery mode */
} APP_BOOTLOADER_BOOT_MODE;

typedef struct
{
    uint32_t magic;          /* APP_BOOTLOADER_BOOT_MODE_MAGIC when valid */
    uint8_t  mode;           /* APP_BOOTLOADER_BOOT_MODE */
    uint8_t  imageIdx;       /* 0..numImages-1, current image being processed */
    uint8_t  imageStep;      /* 0=pristine, 1=backup_done, 2=install_done */
    uint8_t  reserved;
    uint32_t modeXor;        /* mode XOR low byte of magic, sanity */
} APP_BOOTLOADER_BOOT_MODE_INFO;        /* 12 B */

// *****************************************************************************
// *****************************************************************************
// Section: BUNDLE_HEADER - multi-image staging in SST26 DOWNLOAD zone
// *****************************************************************************
// *****************************************************************************

#define APP_BOOTLOADER_BUNDLE_MAGIC_START       (0x4C444E42UL)  /* 'BNDL' little-endian */
#define APP_BOOTLOADER_BUNDLE_MAGIC_END         (0x444E4245UL)  /* 'EBND' little-endian */
#define APP_BOOTLOADER_BUNDLE_FORMAT_VERSION    (1U)
#define APP_BOOTLOADER_BUNDLE_MAX_IMAGES        (4U)

#define APP_BOOTLOADER_TYPE_MAGIC_APP           (0x43505041UL)  /* 'APPC' little-endian */
#define APP_BOOTLOADER_TYPE_MAGIC_PL360         (0x43434C50UL)  /* 'PLCC' little-endian */

typedef struct
{
    uint32_t typeMagic;      /* 'APPC' = APP, 'PLCC' = PL360 */
    uint32_t offset;         /* offset from start of bundle to payload */
    uint32_t size;           /* payload size in bytes */
} APP_BOOTLOADER_BUNDLE_IMAGE;          /* 12 B */

typedef struct
{
    uint32_t magicStart;     /* APP_BOOTLOADER_BUNDLE_MAGIC_START */
    uint32_t formatVersion;  /* APP_BOOTLOADER_BUNDLE_FORMAT_VERSION */
    uint32_t totalSize;      /* bytes from magicStart up to (but not including) magicEnd */
    uint32_t numImages;      /* 1..APP_BOOTLOADER_BUNDLE_MAX_IMAGES */
    /* APP_BOOTLOADER_BUNDLE_IMAGE images[numImages] follows here */
    /* payloads at images[i].offset (relative to magicStart) */
    /* uint32_t magicEnd at offset totalSize */
} APP_BOOTLOADER_BUNDLE_HEADER_FIXED;   /* 16 B fixed prefix */

// *****************************************************************************
// *****************************************************************************
// Section: ZONE_HEADER - per-image header at the start of each CURRENT/REVERT
// *****************************************************************************
// *****************************************************************************

/* 256 B aligned to a SST26 page. */

#define APP_BOOTLOADER_ZONE_HEADER_SIZE         (256U)

#define APP_BOOTLOADER_ZONE_MAGIC_APP_CURRENT   (APP_BOOTLOADER_TYPE_MAGIC_APP)
#define APP_BOOTLOADER_ZONE_MAGIC_APP_REVERT    (0x52505041UL)  /* 'APPR' little-endian */
#define APP_BOOTLOADER_ZONE_MAGIC_PL360_CURRENT (APP_BOOTLOADER_TYPE_MAGIC_PL360)
#define APP_BOOTLOADER_ZONE_MAGIC_PL360_REVERT  (0x52434C50UL)  /* 'PLCR' little-endian */

/* The zone header occupies the first APP_BOOTLOADER_ZONE_HEADER_SIZE bytes of
 * each CURRENT/REVERT zone. Only the first two 32-bit words (magic and size)
 * are interpreted; the remainder is 0xFF padding. The bootloader reads and
 * writes those words directly through the page buffer, so no dedicated struct
 * type is declared for the header. */

// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and Entry Point
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_BOOTLOADER_Main ( void )

  Summary:
    Bootloader main entry point. Called from main().

  Description:
    Reads the boot configuration, dispatches to the appropriate operation
    (APPLY_TELECARGA, APPLY_REVERT, or jump-to-app), and never returns.

  Parameters:
    None.

  Returns:
    Does not return. Either branches to the application via
    APP_BOOTLOADER_JumpToApp or triggers NVIC_SystemReset().
*/

void APP_BOOTLOADER_Main(void);

/*******************************************************************************
  Function:
    void APP_BOOTLOADER_JumpToApp ( void )

  Summary:
    Transfers control to the application at APP_BOOTLOADER_APP_START.

  Description:
    Relocates the vector table via SCB->VTOR, loads the stack pointer from
    the application vector table, and branches to its reset handler.

  Parameters:
    None.

  Returns:
    Does not return.
*/

void APP_BOOTLOADER_JumpToApp(void);

//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif /* APP_BOOTLOADER_H */

/*******************************************************************************
 End of File
*/
