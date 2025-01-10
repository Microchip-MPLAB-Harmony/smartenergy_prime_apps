/*******************************************************************************
  PRIME Non-Volatile Storage Service Interface Header File

  Company:
    Microchip Technology Inc.

  File Name:
    srv_storage.h

  Summary:
    PRIME Non-Volatile Storage Service Interface Header File.

  Description:
    The non-volatile Storage service provides a simple interface to read and
    write non-volatile data used by the PRIME stack. This file provides the
    interface definition for the Storage service.

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

#ifndef SRV_STORAGE_H    // Guards against multiple inclusion
#define SRV_STORAGE_H

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
// Section: Macros
// *****************************************************************************
// *****************************************************************************

/* PRIME versions */
#define PRIME_VERSION_1_3           1U
#define PRIME_VERSION_1_4           2U

/* PRIME mode */
#define PRIME_MODE_BN               1U
#define PRIME_MODE_SN               2U

// *****************************************************************************
// *****************************************************************************
// Section: Data Types
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Non-Volatile Storage Data Types

  Summary:
    List of data types used in PRIME non-volatile storage service.

  Description:
    This data type defines the list of the different information stored in
    non-volatile memory used by the PRIME stack.

  Remarks:
    None
*/

typedef enum
{
    SRV_STORAGE_TYPE_MAC_INFO = 0,
    SRV_STORAGE_TYPE_PHY_INFO = 1,
    SRV_STORAGE_TYPE_BN_INFO = 2,
    SRV_STORAGE_TYPE_MODE_PRIME = 3,
    SRV_STORAGE_TYPE_SECURITY = 4,
    SRV_STORAGE_TYPE_BOOT_INFO = 5,
    SRV_STORAGE_TYPE_END_LIST

} SRV_STORAGE_TYPE;

// *****************************************************************************
/* MAC configuration information

  Summary:
    Structure and key to define the MAC configuration information that needs to
    be kept in non-volatile storage.

  Description:
    This data type defines the structure and key to define the MAC configuration
    information used by the PRIME stack that needs to be kept in non-volatile
    storage.

  Remarks:
    None
*/

#define SRV_STORAGE_MAC_CFG_KEY      0xAA55

typedef struct {
	uint16_t cfgKey;
	uint8_t mac[6];
} SRV_STORAGE_MAC_CONFIG;

// *****************************************************************************
/* PHY configuration information

  Summary:
    Structure and key to define the PHY configuration information that needs to
    be kept in non-volatile storage.

  Description:
    This data type defines the structure and key to define the PHY configuration
    information used by the PRIME stack that needs to be kept in non-volatile
    storage.

  Remarks:
    None
*/

#define SRV_STORAGE_PHY_CONFIG_KEY      0xAA99

typedef struct {
	uint16_t cfgKey;
	uint16_t rfChannel;
	uint8_t txrxChannel;
	uint8_t txrxChannelList;
	uint8_t txrxDoubleChannelList;
} SRV_STORAGE_PHY_CONFIG;

// *****************************************************************************
/* Base Node configuration information

  Summary:
    Structure and key to define the Base Node configuration information that
    needs to be kept in non-volatile storage.

  Description:
    This data type defines the structure and key to define the Base Node
    configuration information used by the PRIME stack that needs to be kept in
    non-volatile storage.

  Remarks:
    None
*/

#define SRV_STORAGE_BN_INFO_CFG_KEY_13    0xAA55
#define SRV_STORAGE_BN_INFO_CFG_KEY_14    0xAA66

typedef struct {
	uint16_t cfgKey;
	uint16_t macLnidOffset;
	uint8_t confSar;
	uint8_t confModBcnAuto;
	uint8_t confRmForced;
	uint8_t confBcnSwitchRate;
	uint8_t confSecProf;
} SRV_STORAGE_BN_INFO_CONFIG;

// *****************************************************************************
/* PRIME board configuration information

  Summary:
    Structure and key to define the PRIME board configuration information that
    needs to be kept in non-volatile storage.

  Description:
    This data type defines the structure and key to define the PRIME board
    configuration information used by the PRIME stack as well as the user
    application that needs to be kept in non-volatile storage.
    PRIME version indicates the PRIME version protocol (PRIME_VERSION_1_3 or
    PRIME_VERSION_1_4).
    The board mode indicates the board function (PRIME_MODE_BN or PRIME_MODE_SN).

  Remarks:
    None
*/

#define SRV_STORAGE_PRIME_MODE_INFO_CFG_KEY    0xA55A

typedef struct {
	uint16_t cfgKey;
	uint8_t primeVersion;
	uint8_t boardMode;
} SRV_STORAGE_PRIME_MODE_INFO_CONFIG;


// *****************************************************************************
/* Security configuration information

  Summary:
    Structure and key to define the security configuration information that
    needs to be kept in non-volatile storage.

  Description:
    This data type defines the structure and key to define the security
    configuration information used by the PRIME stack that needs to be kept in
    non-volatile storage.

  Remarks:
    None
*/

#define SRV_STORAGE_SEC_CFG_KEY      0x5AA5

typedef struct {
	uint16_t cfgKey;
	uint8_t duk[16];
} SRV_STORAGE_SEC_CONFIG;

// *****************************************************************************
/* Boot configuration information

  Summary:
    Structure and key to define the boot configuration information that needs to
    be kept in non-volatile storage.

  Description:
    This data type defines the structure and key to define the boot configuration
    information used by the Firmware Upgrade service and the PRIME Bootloader
    application that needs to be kept in non-volatile storage.

  Remarks:
    None
*/

#define SRV_STORAGE_BOOT_CFG_KEY     0x55AA55AA

typedef struct {
	uint32_t cfgKey;
	uint32_t imgSize;
	uint32_t origAddr;
	uint32_t destAddr;
	uint8_t pagesCounter;
	uint8_t bootState;
} SRV_STORAGE_BOOT_CONFIG;

// *****************************************************************************
// *****************************************************************************
// Section: Storage Service Interface Definition
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Function:
    void SRV_STORAGE_Initialize(void);

  Summary:
    Initializes the PRIME non-volatile Storage service.

  Description:
    This routine initializes the PRIME non-volatile Storage service.

  Precondition:
    None.

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    SRV_STORAGE_Initialize();
    </code>

  Remarks:
    This routine must be called before any other PRIME non-volatile Storage
    service routine. This function is normally not called directly by an
    application. It is called by the system's initialize routine
    (SYS_Initialize).
*/

void SRV_STORAGE_Initialize(void);

// *****************************************************************************
/* Function:
    bool SRV_STORAGE_GetConfigInfo(SRV_STORAGE_TYPE infoType, uint8_t size, void* pData);

  Summary:
    Reads configuration information from non-volatile memory.

  Description:
    This routine reads configuration information from non-volatile memory.

  Precondition:
    The SRV_STORAGE_Initialize routine must have been called before.

  Parameters:
    infoType - Configuration information type to read.
    size     - Size in bytes of the configuration information to read.
    pData    - Pointer to store the read configuration information.

  Returns:
    The result of the read operation (true if success, false if error).

  Example:
    <code>
    uint8_t macInfo[8];

    if (SRV_STORAGE_GetConfigInfo(SRV_STORAGE_TYPE_MAC_INFO, 8, macInfo))
    {

    }
    </code>

  Remarks:
    None.
*/

bool SRV_STORAGE_GetConfigInfo(SRV_STORAGE_TYPE infoType, uint8_t size, void *pData);

// *****************************************************************************
/* Function:
    bool SRV_STORAGE_SetConfigInfo(SRV_STORAGE_TYPE infoType, uint8_t size, void* pData);

  Summary:
    Writes configuration information in non-volatile memory.

  Description:
    This routine writes configuration information in non-volatile memory.

  Precondition:
    The SRV_STORAGE_Initialize routine must have been called before.

  Parameters:
    infoType - Configuration information type to write.
    size     - Size in bytes of the configuration information to write.
    pData    - Pointer to configuration information data to write.

  Returns:
    The result of the write operation (true if success, false if error).

  Example:
    <code>
    uint8_t macInfo[8] = {0x55, 0xAA, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05};

    SRV_STORAGE_SetConfigInfo(SRV_STORAGE_TYPE_MAC_INFO, 8, macInfo);
    </code>

  Remarks:
    None.
*/

bool SRV_STORAGE_SetConfigInfo(SRV_STORAGE_TYPE infoType, uint8_t size, void *pData);

#endif //SRV_STORAGE_H
