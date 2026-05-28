/*******************************************************************************
  PRIME Reset Handler Service Interface Header File

  Company:
    Microchip Technology Inc.

  File Name:
    srv_firmware_upgrade.h

  Summary:
    PRIME Firmware Upgrade Service Interface Header File.

  Description:
    The Firmware Upgrade service provides the handling of the firmware upgrade
    and version swap for PRIME. This file provides the interface definition for
    this service.
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

#ifndef SRV_FIRMWARE_UPGRADE_H    // Guards against multiple inclusion
#define SRV_FIRMWARE_UPGRADE_H

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
// Section: Data Types
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Signature algorithms

 Summary:
    Signature algorithms defined for a PRIME firmware upgrade.

 Description:
    This enumeration lists the signature algorithms included in the PRIME
    specification for the firmware upgrade.

 Remarks:
    None.
*/
typedef enum {
	SRV_FU_SIGNATURE_ALGO_NO_SIGNATURE = 0,
	SRV_FU_SIGNATURE_ALGO_RSA3072_SHA256,
	SRV_FU_SIGNATURE_ALGO_ECDSA256_SHA256,
} SRV_FU_SIGNATURE_ALGO;

// *****************************************************************************
/* Firmware upgrade result

 Summary:
    Results defined for a PRIME firmware upgrade.

 Description:
    This enumeration lists the possible results of the firmware upgrade.

 Remarks:
    None.
*/
typedef enum {
	SRV_FU_RESULT_SUCCESS,          /* Request to restart with new image */
	SRV_FU_RESULT_CANCEL,           /* The FU has been killed */
	SRV_FU_RESULT_CRC_ERROR,        /* CRC error */
	SRV_FU_RESULT_FW_REVERT,        /* Request to restart with old image */
	SRV_FU_RESULT_FW_CONFIRM,       /* The FU has been confirmed */
	SRV_FU_RESULT_ERROR,            /* Error during FU */
	SRV_FU_RESULT_SIGNATURE_ERROR,  /* Signature error (only PRIME 1.4) */
	SRV_FU_RESULT_IMAGE_ERROR       /* Image verification (model/vendor) failed (only PRIME 1.4) */
} SRV_FU_RESULT;

// *****************************************************************************
/* Firmware upgrade verification result

 Summary:
    Verification result defined for a PRIME firmware upgrade.

 Description:
    This enumeration lists the possible results for the image verification during
    the firmware upgrade.

 Remarks:
    None.
*/
typedef enum {
	SRV_FU_VERIFY_RESULT_SUCCESS,
	SRV_FU_VERIFY_RESULT_SIGNATURE_FAIL,
	SRV_FU_VERIFY_RESULT_IMAGE_FAIL
} SRV_FU_VERIFY_RESULT;

// *****************************************************************************
/* Firmware upgrade information

 Summary:
    Data structure with the firmware upgrade information.

 Description:
    This structure contains the parameters for the firmware upgrade information.

 Remarks:
    None.
*/
typedef struct {
  uint32_t imageSize;
  uint16_t signLength;
  SRV_FU_SIGNATURE_ALGO signAlgorithm;
  uint8_t pageSize;
} SRV_FU_INFO;

// *****************************************************************************
/* CRC callback

  Summary:
    Callback function pointer to get the calculated CRC.

  Description:
    This callback is used to get the calculated CRC.

  Remarks:
    None.
*/
typedef void (*SRV_FU_CRC_CB)(uint32_t crc);

// *****************************************************************************
/* Image callback

  Summary:
    Callback function pointer to get the result of the image verification.

  Description:
    This callback is used to get the result of the image verification.

  Remarks:
    None.
*/
typedef void (*SRV_FU_IMAGE_VERIFY_CB)(SRV_FU_VERIFY_RESULT verifyResult);

// *****************************************************************************
/* Firmware upgrade callback

  Summary:
    Callback function pointer to get the result of the firmware upgrade process.

  Description:
    This callback is used to get the result of the firmware upgrade process.

  Remarks:
    None.
*/
typedef void (*SRV_FU_RESULT_CB)(SRV_FU_RESULT fuResult);

// *****************************************************************************
/* Memory transaction result information

 Summary:
    Possible results of a memory transaction.

 Description:
    Possible results of a memory transaction.

 Remarks:
    None.
*/
typedef enum {
  SRV_FU_MEM_TRANSFER_OK,
  SRV_FU_MEM_TRANSFER_ERROR,
} SRV_FU_MEM_TRANSFER_RESULT;

// *****************************************************************************
/* Memory transaction commands

 Summary:
    Possible commands for a memory transaction.

 Description:
    This enumeration contains the commands for a memory transaction.

 Remarks:
    None.
*/
typedef enum {
  SRV_FU_MEM_TRANSFER_CMD_ERASE,
  SRV_FU_MEM_TRANSFER_CMD_READ,
  SRV_FU_MEM_TRANSFER_CMD_WRITE,
  SRV_FU_MEM_TRANSFER_CMD_BAD,
} SRV_FU_MEM_TRANSFER_CMD;

// *****************************************************************************
/* Memory transaction result callback

  Summary:
    Callback function pointer to get the result of a memory transaction.

  Description:
    This callback is used to get the result of a memory transaction.

  Remarks:
    None.
*/
typedef void (*SRV_FU_MEM_TRANSFER_CB)
    (SRV_FU_MEM_TRANSFER_CMD command, SRV_FU_MEM_TRANSFER_RESULT result);

// *****************************************************************************
/* Traffic versions

 Summary:
    Traffic versions.

 Description:
    This enumeration lists the traffic versions available in a PRIME network.

 Remarks:
    None.
*/
typedef enum {
	SRV_FU_TRAFFIC_VER_PRIME_1_3 = 1,
	SRV_FU_TRAFFIC_VER_PRIME_1_4 = 2,
} SRV_FU_TRAFFIC_VERSION;

// *****************************************************************************
/* Swap version callback

  Summary:
    Callback function pointer to trigger a swap of PRIME stack versions.

  Description:
    This callback is used to trigger a swap of PRIME stack versions.

  Remarks:
    None.
*/
typedef void (*SRV_FU_VERSION_SWAP_CB)(SRV_FU_TRAFFIC_VERSION trafficVersion);

// *****************************************************************************
// *****************************************************************************
// Section: Firmware Upgrade Service Interface Definition
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Function:
    void SRV_FU_Initialize(void)

  Summary:
    Initializes the Firmware Upgrade service.

  Description:
    This routine initializes the PRIME Firmware Upgrade service.

  Precondition:
    None.

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    int main()
    {
        SRV_FU_Initialize();
    }
    </code>

  Remarks:
    This routine must be called before any other PRIME Firmware Upgrade service
    routine. This function is normally not called directly by an application.
    It is called by the system's initialize routine (SYS_Initialize).
*/
void SRV_FU_Initialize(void);

// ****************************************************************************
/* Function:
    void SRV_FU_Tasks(void)

  Summary:
    Maintains the Firmware Upgrade state machine.

  Description:
    This function is used to maintain the Firmware Upgrade internal state
    machine.

  Precondition:
    The SRV_FU_Initialize function should have been called before calling this
    function.

  Parameters:
    None

  Returns:
    None

  Example:
    <code>
    while (true)
    {
        SRV_FU_Tasks();
    }
    </code>

  Remarks:
    This function is normally not called directly by an application.
    It is called by the system's tasks routine (SYS_Tasks).
*/
void SRV_FU_Tasks(void);

// ****************************************************************************
/* Function:
    void SRV_FU_Start(SRV_FU_INFO *fuInfo)

  Summary:
    Starts the firmware upgrade process.

  Description:
    This function is used to start the firmware upgrade process by initializing
    and unlocking the memory.

  Precondition:
    The SRV_FU_Initialize function should have been called before calling this
    function.

  Parameters:
    fuInfo    - Pointer to the firmware upgrade information

  Returns:
    None

  Example:
    <code>
    SRV_FU_INFO fuInfo;

    fuInfo.imageSize = 0x1000;
    fuInfo.pageSize = 192;
    fuInfo.signAlgorithm = SRV_FU_SIGNATURE_ALGO_NO_SIGNATURE;
    fuInfo.signLength = 0;

    SRV_FU_Start(&fuInfo);
    </code>

  Remarks:
    This function is called by the PRIME stack.
*/
void SRV_FU_Start(SRV_FU_INFO *fuInfo);

// ****************************************************************************
/* Function:
    void SRV_FU_RegisterCallbackFuResult(SRV_FU_RESULT_CB callback)

  Summary:
    Registers a function to be called back when a firmware upgrade process has
    finished.

  Description:
    This function allows the application to register a function to be called
    back when a firmware upgrade process has finished. Depending on the result,
    the application will trigger the execution of the new firmware.

  Precondition:
    The SRV_FU_Initialize function should have been called before calling this
    function.

  Parameters:
    callback       - Pointer to the callback function

  Returns:
    None.

  Example:
    <code>
    void _fu_result(SRV_FU_RESULT fuResult)
    {
        ...
    }

    void main(void)
    {
        SRV_FU_Initialize();

        SRV_FU_RegisterCallbackFuResult(_fu_result);
    }
    </code>

  Remarks:
    This function is called by the application.
*/
void SRV_FU_RegisterCallbackFuResult(SRV_FU_RESULT_CB callback);

// ****************************************************************************
/* Function:
   void SRV_FU_End(SRV_FU_RESULT fuResult)

  Summary:
    Ends the firmware upgrade process.

  Description:
    This function is used to finish the firmware upgrade process and to trigger the
    execution of the new firmware.

  Precondition:
    The SRV_FU_Initialize function should have been called before calling this
    function.

  Parameters:
    fuResult    - Result of the firmware upgrade process

  Returns:
    None

  Example:
    <code>
    SRV_FU_End(SRV_FU_RESULT_SUCCESS);
    </code>

  Remarks:
    This function is called by the PRIME stack.
*/
void SRV_FU_End(SRV_FU_RESULT fuResult);

// ****************************************************************************
/* Function:
   bool SRV_FU_SwapFirmware(void)

  Summary:
    Swaps the firmware.

  Description:
    This function is used to swap the firmware, if needed, and update data for
    the bootloader.

  Precondition:
    The SRV_FU_Initialize function should have been called before calling this
    function.

  Parameters:
    None.

  Returns:
    True if firmware must be swapped. Otherwise, false.

  Example:
    <code>
    if (SRV_FU_SwapFirmware() == true)
    {
        ... Invoke bootloader
    }
    </code>

  Remarks:
    This function is called by the application.
*/
bool SRV_FU_SwapFirmware(void);

// ****************************************************************************
/* Function:
   void SRV_FU_SetECDSAPublicKey(uint8_t *pubKey, uint32_t pubKeyLen)

  Summary:
    Sets the ECDSA public key used to verify firmware images.

  Description:
    This function passes the ECDSA P-256 public key (uncompressed SEC1
    format: 0x04 prefix + 32 B X + 32 B Y = 65 B) to the FU service so
    that SRV_FU_VerifyImage can verify the image signature appended at
    the end of the FU image.

  Precondition:
    The SRV_FU_Initialize function must have been called before.

  Parameters:
    pubKey    - Pointer to the public key bytes (must outlive the FU service)
    pubKeyLen - Length of the public key in bytes (65 for P-256 uncompressed)

  Returns:
    None.

  Remarks:
    Called by the application during init. The pointer is stored, not
    copied, so the buffer must remain valid for the lifetime of the FU
    service.
*/
void SRV_FU_SetECDSAPublicKey(uint8_t *pubKey, uint32_t pubKeyLen);

// ****************************************************************************
/* Function:
   void SRV_FU_CfgRead(void *dst, uint16_t size)

  Summary:
    Reads the firmware upgrade information.

  Description:
    This function is used to read the firmware upgrade information, which is
    stored out of the PRIME stack.

  Precondition:
    The SRV_FU_Initialize function should have been called before calling this
    function.

  Parameters:
    dst    - Pointer to the buffer to store the information
    size   - Number of bytes to read

  Returns:
    None

  Example:
    <code>
    uint32_t fuData[3];
    SRV_FU_CfgRead(&fuData, sizeof(fuData));
    </code>

  Remarks:
    This function is called by the PRIME stack.
*/
void SRV_FU_CfgRead(void *dst, uint16_t size);

// ****************************************************************************
/* Function:
   void SRV_FU_CfgWrite(void *src, uint16_t size)

  Summary:
    Writes the firmware upgrade information.

  Description:
    This function is used to write the firmware upgrade information, which is
    stored out of the PRIME stack.

  Precondition:
    The SRV_FU_Initialize function should have been called before calling this
    function.

  Parameters:
    src    - Pointer to the buffer with the information to write
    size   - Number of bytes to write

  Returns:
    None

  Example:
    <code>
    uint32_t fuData[3];
    SRV_FU_CfgWrite(&fuData, sizeof(fuData));
    </code>

  Remarks:
    This function is called by the PRIME stack.
*/
void SRV_FU_CfgWrite(void *src, uint16_t size);

// ****************************************************************************
/* Function:
  void SRV_FU_RegisterCallbackMemTransfer(SRV_FU_MEM_TRANSFER_CB callback);

  Summary:
    Registers a function to be called back when a memory operation finishes.

  Description:
    This function allows the PRIME stack to register a function to be called
    back when when a memory operation finishes.

  Precondition:
    The SRV_FU_Initialize function should have been called before calling this
    function.

  Parameters:
    callback       - Pointer to the callback function

  Returns:
    None.

  Example:
    <code>

    void _endMemoryTransaction(SRV_FU_MEM_TRANSFER_CMD command,
                   SRV_FU_MEM_TRANSFER_RESULT result)
    {
        ...
    }

    void main(void)
    {
        SRV_FU_Initialize();

        SRV_FU_RegisterCallbackMemTransfer(_endMemoryTransaction);
    }
    </code>

  Remarks:
    This function is called by the PRIME stack.
*/
void SRV_FU_RegisterCallbackMemTransfer(SRV_FU_MEM_TRANSFER_CB callback);

// ****************************************************************************
/* Function:
   void SRV_FU_DataRead(uint32_t address, uint8_t *buffer, uint16_t size)

  Summary:
    Reads image from memory.

  Description:
    This function is used to read the image from memory.

  Precondition:
    The SRV_FU_Initialize function should have been called before calling this
    function.

  Parameters:
    address - Image address to read
    buffer  - Pointer to the buffer to store the information
    size    - Number of bytes to read

  Returns:
    None.

  Example:
    <code>
    uint32_t image[100];
    SRV_FU_DataRead(0x100, &image, sizeof(image));
    </code>

  Remarks:
    This function is called by the PRIME stack.
*/
void SRV_FU_DataRead(uint32_t address, uint8_t *buffer, uint16_t size);

// ****************************************************************************
/* Function:
   void SRV_FU_DataWrite(uint32_t address, uint8_t *buffer, uint16_t size)

  Summary:
    Writes image in memory.

  Description:
    This function is used to write the image in memory.

  Precondition:
    The SRV_FU_Initialize function should have been called before calling this
    function.

  Parameters:
    address - Image address to write
    buffer  - Pointer to the buffer with the information
    size    - Number of bytes to write

  Returns:
    None

  Example:
    <code>
    uint32_t image[100];
    SRV_FU_DataWrite(0x100, &image, sizeof(image));
    </code>

  Remarks:
    This function is called by the PRIME stack.
*/
void SRV_FU_DataWrite(uint32_t address, uint8_t *buffer, uint16_t size);

// ****************************************************************************
/* Function:
    void SRV_FU_RegisterCallbackCrc(SRV_FU_CRC_CB callback)

  Summary:
    Registers a function to be called back when the CRC of the received image
    has been calculated.

  Description:
    This function allows the PRIME stack to register a function to be called
    back when the CRC of the received image has been calculated.

  Precondition:
    The SRV_FU_Initialize function should have been called before calling this
    function.

  Parameters:
    callback       - Pointer to the callback function

  Returns:
    None.

  Example:
    <code>
    void _check_crc(uint32_t crc)
    {
        ...
    }

    void main(void)
    {
        SRV_FU_Initialize();

        SRV_FU_RegisterCallbackCrc(_check_crc);
    }
    </code>

  Remarks:
    This function is called by the PRIME stack.
*/
void SRV_FU_RegisterCallbackCrc(SRV_FU_CRC_CB callback);

// ****************************************************************************
/* Function:
   void SRV_FU_CalculateCrc(void)

  Summary:
    Calculates the CRC of the received image.

  Description:
    This function is used to calculate the CRC of the received image.

  Precondition:
    The SRV_FU_Initialize function should have been called before calling this
    function.

  Parameters:
    None.

  Returns:
    None

  Example:
    <code>
    SRV_FU_CalculateCrc();
    </code>

  Remarks:
    This function is called by the PRIME stack.
*/
void SRV_FU_CalculateCrc(void);

// ****************************************************************************
/* Function:
    void SRV_FU_RegisterCallbackVerify(SRV_FU_IMAGE_VERIFY_CB callback)

  Summary:
    Registers a function to be called back when the received image
    has been verified.

  Description:
    This routine allows the PRIME stack to register a function to be
    called back when the received image has been verified.

  Precondition:
    The SRV_FU_Initialize function should have been called before
    calling this function.

  Parameters:
    callback       - Pointer to the callback function

  Returns:
    None.

  Example:
    <code>
    void _image_verification(SRV_FU_VERIFY_RESULT verifyResult)
    {
        ...
    }

    void main(void)
    {
        SRV_FU_Initialize();

        SRV_FU_RegisterCallbackVerify(_image_verification);
    }
    </code>

  Remarks:
    This function is called by the PRIME stack.
*/
void SRV_FU_RegisterCallbackVerify(SRV_FU_IMAGE_VERIFY_CB callback);

// ****************************************************************************
/* Function:
   void SRV_FU_VerifyImage(void)

  Summary:
    Verifies the received image.

  Description:
    This function is used to verify the received image. Metadata and signature,
    if available, are checked.

  Precondition:
    The SRV_FU_Initialize function should have been called before calling this
    function.

  Parameters:
    None.

  Returns:
    None

  Example:
    <code>
    SRV_FU_VerifyImage();
    </code>

  Remarks:
    This function is called by the PRIME stack.
*/
void SRV_FU_VerifyImage(void);

// ****************************************************************************
/* Function:
   uint16_t SRV_FU_GetBitmap(uint8_t *bitmap, uint32_t *numRxPages)

  Summary:
    Gets the bitmap with the information about the status of each page of the
    image.

  Description:
    This function is used to gets the bitmap with the information about the
    status of each page of the image.

  Precondition:
    The SRV_FU_Initialize function should have been called before calling this
    function.

  Parameters:
    bitmap        - Pointer to the bitmap information
    numRxPages    - Pointer to the number of pages received

  Returns:
    Size of bitmap. Maximum value is 1024 bytes. In case of returning 0, the
    bitmap buffer will be initialized internally in the PRIME stack.

  Example:
    <code>
    uint8_t bitmap[1024];
    uint32_t numPages = 0;

    SRV_FU_GetBitmap(&bitmap, &numPages);
    </code>

  Remarks:
    This function is called by the PRIME stack.
*/
uint16_t SRV_FU_GetBitmap(uint8_t *bitmap, uint32_t *numRxPages);

// ****************************************************************************
/* Function:
    void SRV_FU_RegisterCallbackSwapVersion(SRV_FU_VERSION_SWAP_CB callback)

  Summary:
    Registers a function to be called back when the PRIME stack requests to
    trigger a PRIME stack version swap.

  Description:
    This routine allows the application to register a function to be called
    back when the PRIME stack requests to trigger a PRIME stack version swap.

  Precondition:
    The SRV_FU_Initialize function should have been called before calling this function.

  Parameters:
    callback       - Pointer to the callback function

  Returns:
    None.

  Example:
    <code>
    void _swap(SRV_FU_TRAFFIC_VERSION trafficVersion)
    {
        ...
    }

    void main(void)
    {
        SRV_FU_Initialize();

        SRV_FU_RegisterCallbackSwapVersion(_swap);
    }
    </code>

  Remarks:
    This function is called by the application.
*/
void SRV_FU_RegisterCallbackSwapVersion(SRV_FU_VERSION_SWAP_CB callback);

// ****************************************************************************
/* Function:
   void SRV_FU_RequestSwapVersion(SRV_FU_TRAFFIC_VERSION trafficVersion)

  Summary:
    Requests to swap the PRIME stack version.

  Description:
    This function is used to request to swap the PRIME stack version.

  Precondition:
    The SRV_FU_Initialize function should have been called before calling this function.

  Parameters:
    trafficVersion  - Type of traffic PRIME 1.3 or 1.4 detected

  Returns:
    None

  Example:
    <code>
    SRV_FU_RequestSwapVersion(SRV_FU_TRAFFIC_VERSION_PRIME_1_3);
    </code>

  Remarks:
    This function is called by the PRIME stack.
*/
void SRV_FU_RequestSwapVersion(SRV_FU_TRAFFIC_VERSION trafficVersion);

// *****************************************************************************
// *****************************************************************************
// Section: External-Memory BOOT_MODE_INFO Handshake (SST26 BOOT_FLAG sector)
//
// The bootloader and the modem application share a 12-byte structure persisted
// at APP_BOOTLOADER_SST26_BOOT_FLAG_OFFSET (0x140000) of the SST26. The
// application writes it before requesting a reset to tell the bootloader what
// to do on the next boot (NORMAL, INSTALL_PENDING, REVERT_PENDING, UART_PENDING).
//
// The R/W path runs on the FU service's existing DRV_MEMORY client, so all
// callsites must be in FU-idle state (state == CMD_WAIT). The Set/Get kicks
// are non-blocking; the caller polls SRV_FU_ExtMemBootModeStatus() until it
// transitions out of BUSY, then reads SRV_FU_ExtMemBootModeResult() (Get only).
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Boot Mode operation status

  Summary:
    Status of the last SRV_FU_ExtMemBootModeSet/Get request.

  Description:
    IDLE     - No operation pending or last result already consumed.
    BUSY     - Operation in flight (erase, write or read).
    OK       - Last operation completed successfully.
    ERROR    - Last operation failed (DRV_MEMORY error, invalid magic
               on read, or service not idle when kicked).
*/
typedef enum
{
    SRV_FU_EXT_MEM_BOOT_MODE_STATUS_IDLE  = 0,
    SRV_FU_EXT_MEM_BOOT_MODE_STATUS_BUSY  = 1,
    SRV_FU_EXT_MEM_BOOT_MODE_STATUS_OK    = 2,
    SRV_FU_EXT_MEM_BOOT_MODE_STATUS_ERROR = 3,
} SRV_FU_EXT_MEM_BOOT_MODE_STATUS;

// *****************************************************************************
/* Boot Mode values written to BOOT_MODE_INFO.mode

  Summary:
    Mirror of APP_BOOTLOADER_BOOT_MODE in the bootloader's app_bootloader.h.

  Description:
    Pass one of these to SRV_FU_ExtMemBootModeSet to ask the bootloader
    to take a specific action on the next reset.
*/
#define SRV_FU_EXT_MEM_BOOT_MODE_NORMAL          (0U)
#define SRV_FU_EXT_MEM_BOOT_MODE_INSTALL_PENDING (1U)
#define SRV_FU_EXT_MEM_BOOT_MODE_REVERT_PENDING  (2U)
#define SRV_FU_EXT_MEM_BOOT_MODE_UART_PENDING    (3U)

// *****************************************************************************
/* Boot Mode operation result

  Summary:
    12-byte structure read from the SST26 BOOT_FLAG sector.

  Description:
    Mirror of APP_BOOTLOADER_BOOT_MODE_INFO defined in the bootloader's
    app_bootloader.h. Layouts must stay binary-compatible (packed).
*/
typedef struct __attribute__((packed))
{
    uint32_t magic;
    uint8_t  mode;
    uint8_t  imageIdx;
    uint8_t  imageStep;
    uint8_t  reserved;
    uint32_t modeXor;
} SRV_FU_EXT_MEM_BOOT_MODE_INFO;

/*******************************************************************************
  Function:
    bool SRV_FU_ExtMemBootModeSet(uint8_t mode, uint8_t imageIdx,
                                  uint8_t imageStep)

  Summary:
    Kicks an asynchronous erase + write of the BOOT_MODE_INFO structure.

  Description:
    Builds a 12-byte structure with magic = 'BMOD' and modeXor =
    mode XOR (magic & 0xFF), and schedules a 4 KB sector erase followed
    by a 256-byte page write at the start of the BOOT_FLAG sector.
    Non-blocking: the caller must poll SRV_FU_ExtMemBootModeStatus().

  Precondition:
    SRV_FU_Initialize() and SRV_FU_Tasks() must have completed driver
    open and geometry probe (state == CMD_WAIT).

  Parameters:
    mode      - One of APP_BOOTLOADER_BOOT_MODE values (0..3).
    imageIdx  - Image index inside the bundle (0..numImages-1).
    imageStep - 0 pristine, 1 backup_done, 2 install_done.

  Returns:
    true  - Request accepted, status moves to BUSY.
    false - Service is not idle or another boot-mode op is pending.
*/
bool SRV_FU_ExtMemBootModeSet(uint8_t mode, uint8_t imageIdx, uint8_t imageStep);

/*******************************************************************************
  Function:
    bool SRV_FU_ExtMemBootModeGet(void)

  Summary:
    Kicks an asynchronous read of the BOOT_MODE_INFO structure.

  Description:
    Schedules a 256-byte page read at the start of the BOOT_FLAG sector.
    On completion the first 12 bytes are validated against magic and
    modeXor; bad magic returns the structure with mode = NORMAL and
    status = OK so the caller treats unknown contents as a safe default.

  Returns:
    true  - Request accepted, status moves to BUSY.
    false - Service is not idle or another boot-mode op is pending.
*/
bool SRV_FU_ExtMemBootModeGet(void);

/*******************************************************************************
  Function:
    SRV_FU_EXT_MEM_BOOT_MODE_STATUS SRV_FU_ExtMemBootModeStatus(void)

  Summary:
    Returns the status of the last SRV_FU_ExtMemBootModeSet/Get request.
*/
SRV_FU_EXT_MEM_BOOT_MODE_STATUS SRV_FU_ExtMemBootModeStatus(void);

/*******************************************************************************
  Function:
    void SRV_FU_ExtMemBootModeResult(SRV_FU_EXT_MEM_BOOT_MODE_INFO *info)

  Summary:
    Copies the result of the last SRV_FU_ExtMemBootModeGet into *info.

  Description:
    Only meaningful when SRV_FU_ExtMemBootModeStatus() returned OK after
    a Get. After a successful Set the structure mirrors what was written.
*/
void SRV_FU_ExtMemBootModeResult(SRV_FU_EXT_MEM_BOOT_MODE_INFO *info);

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    }

#endif
// DOM-IGNORE-END

#endif // SRV_FIRMWARE_UPGRADE_H
