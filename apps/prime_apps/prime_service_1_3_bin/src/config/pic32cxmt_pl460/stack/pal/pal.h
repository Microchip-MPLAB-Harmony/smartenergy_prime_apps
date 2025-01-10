/*******************************************************************************
  Company:
    Microchip Technology Inc.

  File Name:
    pal.h

  Summary:
    Physical Abstraction Layer (PAL) header file.

  Description:
    This module provides the interface between the PRIME MAC layer and the
    different physical layers.
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

#ifndef PAL_H
#define PAL_H

// *****************************************************************************
// *****************************************************************************
// Section: Include Files
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include "system/system.h"
#include "pal_types.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    extern "C" {

#endif
// DOM-IGNORE-END

// ****************************************************************************
// ****************************************************************************
// Section: Interface Routines
// ****************************************************************************
// ****************************************************************************

// ****************************************************************************
/* Function:
    SYS_MODULE_OBJ PAL_Initialize(
        const SYS_MODULE_INDEX index
    )

  Summary:
    Initializes PRIME PAL module.

  Description:
    This routine initializes the PAL module, making it ready for clients to
    use it. The initialization data is specified by the init parameter. It is a
    single instance module, so this function should be called only once.

  Precondition:
    None.

  Parameters:
    index - Identifier for the instance to be initialized. Only one instance
            (index 0) supported.

  Returns:
    If successful, returns a valid handle to a module instance object.
    Otherwise, returns SYS_MODULE_OBJ_INVALID.

  Example:
    <code>
    int main()
    {
        PAL_Initialize(PRIME_PAL_INDEX);
    }
    </code>

  Remarks:
    None
*/
SYS_MODULE_OBJ PAL_Initialize(const SYS_MODULE_INDEX index);

// ****************************************************************************
/* Function:
    void PAL_Tasks(SYS_MODULE_OBJ object)

  Summary:
    Maintains the PAL state machine.

  Description:
    This function is used to maintain the PAL internal state machine and
    generate callbacks.

  Precondition:
    The PAL_Initialize function should have been called before calling this
    function.

  Parameters:
    object - Identifier for the object instance

  Returns:
    None.

  Example:
    <code>
    SYS_MODULE_OBJ sysObjPal;
    sysObjPal = PAL_Initialize(PRIME_PAL_INDEX);

    while (true)
    {
        PAL_Tasks(sysObjPal);
    }
    </code>

  Remarks:
    None.
*/
void PAL_Tasks(SYS_MODULE_OBJ object);

// *************************************************************************
/* Function:
    SYS_STATUS PAL_Status( SYS_MODULE_OBJ object )

  Summary:
    Gets the current status of the PAL module.

  Description:
    This routine provides the current status of the PAL module.

  Precondition:
    Function PAL_Initialize should have been called before calling
    this function.

  Parameters:
    object - Identifier for the object instance

  Returns:
    SYS_STATUS_READY: Indicates that the driver is ready and accept
    requests for new operations.

    SYS_STATUS_UNINITIALIZED: Indicates the driver is not initialized.

    SYS_STATUS_ERROR: Indicates the driver is not initialized correctly.

    SYS_STATUS_BUSY: Indicates the driver is initializing.

  Example:
    <code>
    SYS_MODULE_OBJ sysObjPal;
    sysObjPal = PAL_Initialize(PRIME_PAL_INDEX);
    SYS_STATUS status;

    status = PAL_Status(sysObjPal);
    </code>

  Remarks:
    None.
*/

SYS_STATUS PAL_Status(SYS_MODULE_OBJ object);

// ****************************************************************************
/* Function:
    void PAL_CallbackRegister(PAL_CALLBACKS *pCallbacks)

  Summary:
    Sets PAL layer callback functions

  Description:
    This routine links callback functions between upper layer and phy layer.

  Precondition:
    The PAL_Initialize function should have been called before calling this
    function.

  Parameters:
    pCallbacks         Callbacks structure

  Returns:
    None

  Example:
    <code>
    static void _data_confirm_handler(PAL_MSG_CONFIRM_DATA *dataConfirm)
    {
        ...
    }

    void main(void)
    {
        PAL_CALLBACKS pal_cbs;

        PAL_Initialize();

        memset(palCBs, NULL, sizeof(palCBs));
        palCBs.palDataConfirm = _data_confirm_handler;

        PAL_SetCallbacks(&palCBs);
    }
    </code>

  Remarks:
    None
*/
void PAL_CallbackRegister(PAL_CALLBACKS *pCallbacks);

// ****************************************************************************
/* Function:
    uint8_t PAL_DataRequestTransmission(PAL_MSG_REQUEST_DATA *requestMsg)

  Summary:
    Request to transmit a message

  Description:
    This functions is used to initiate the transmission process of a PPDU
    (PHY Protocol Data Unit) to the medium indicated in the transmission
    information structure.

  Precondition:
    The PAL_Initialize function should have been called before calling this
    function.

  Parameters:
    requestMsg         MPDU data transmission structure

  Returns:
    PAL Transmission results.

  Example:
    <code>
    uint8_t result=PAL_TX_RESULT_SUCCESS;
    PAL_MSG_REQUEST_DATA requestMsg;
    uint8_t msg[30];

    requestMsg.dataBuf = &msg;
    requestMsg.timeDelay = 10000;
    requestMsg.dataLength = sizeof(msg);
    requestMsg.pch = 16;
    requestMsg.buffIdentifier = 2;
    requestMsg.attLevel = 0;
    requestMsg.scheme = PAL_PLC_DBPSK_R;
    requestMsg.disableRx = 0;
    requestMsg.mode = PAL_MODE_TYPE_B;
    requestMsg.timeMode = PAL_TX_MODE_ABSOLUTE;
    requestMsg.numSenses = 3;
    requestMsg.senseDelayMs = 3;

    result = PAL_DataRequestTransmission(&requestMsg);
    </code>

  Remarks:
    None
*/
uint8_t PAL_DataRequest(PAL_MSG_REQUEST_DATA *pData);

// ****************************************************************************
/* Function:
    uint8_t PAL_GetSNR(uint16_t pch, uint8_t *snr, uint8_t qt)

  Summary:
    Convert QT value to Signal Noise Ratio (SNR).

  Description:
    This function is used to get the value of the Signal to Noise Ratio,
    defined as the ratio of measured received signal level to noise level of
    last received PPDU (PHY Protocol Data Unit).

  Precondition:
    The PAL_Initialize function should have been called before calling this
    function.

  Parameters:
    pch         Physical channel
    snr         Signal to noise ratio output parameter
    qt          QT input parameter to get SNR level

  Returns:
    PAL_CFG_SUCCESS         - If successful
    PAL_CFG_INVALID_INPUT   - If unsuccessful

  Example:
    <code>
    uint8_t result=PAL_CFG_SUCCESS;
    uint8_t snr=0;
    uint8_t qt=5;
    uint16_t pch=1;

    result = PAL_GetSNR(pch, &snr, qt);
    </code>

  Remarks:
    Not available in PHY Serial medium
*/
uint8_t PAL_GetSNR(uint16_t pch, uint8_t *snr, uint8_t qt);

// ****************************************************************************
/* Function:
    uint8_t PAL_GetZCT(uint16_t pch, uint32_t *zct)

  Summary:
    Get zero-cross time (ZCT).

  Description:
    This function is used to get the value of the zero-cross time of the mains
    and the time between the last transmission or reception and the zero cross
    of the mains.

  Precondition:
    The PAL_Initialize function should have been called before calling this
    function.

  Parameters:
    pch         Physical channel
    zct         Zero time output parameter

  Returns:
    PAL_CFG_SUCCESS         - If successful
    PAL_CFG_INVALID_INPUT   - If unsuccessful

  Example:
    <code>
    uint8_t result=PAL_CFG_SUCCESS;
    uint8_t zct=0;
    uint16_t pch=1;

    result = PAL_GetZCT(pch, &zct);
    </code>

  Remarks:
    Not available for both PHY Serial and RF medium.
*/
uint8_t PAL_GetZCT(uint16_t pch, uint32_t *zct);

// ****************************************************************************
/* Function:
    uint8_t PAL_GetTimer(uint16_t pch, uint32_t *timer)

  Summary:
    Get the current PHY time in us.

  Description:
    This routine is used to get current PHY time.

  Precondition:
    The PAL_Initialize function should have been called before calling this
    function.

  Parameters:
    pch             Physical channel
    timer           Current output time of PHY

  Returns:
    PAL_CFG_SUCCESS         - If successful
    PAL_CFG_INVALID_INPUT   - If unsuccessful

  Example:
    <code>
    uint8_t result=PAL_CFG_SUCCESS;
    uint32_t timer=0;
    uint16_t pch=1;

    result = PAL_GetTimer(pch, &timer);
    </code>

  Remarks:
    None
*/
uint8_t PAL_GetTimer(uint16_t pch, uint32_t *timer);

// ****************************************************************************
/* Function:
    uint8_t PAL_GetTimerExtended(uint16_t pch, uint64_t *timer)

  Summary:
    Get the extended PHY time in us.

  Description:
    This routine is used to get the extended PHY time.

  Precondition:
    The PAL_Initialize function should have been called before calling this
    function.

  Parameters:
    pch             Physical channel
    timer           Extended output time  of PHY

  Returns:
    PAL_CFG_SUCCESS         - If successful
    PAL_CFG_INVALID_INPUT   - If unsuccessful

  Example:
    <code>
    uint8_t  result=PAL_CFG_SUCCESS;
    uint64_t timer=0;
    uint16_t pch=1;

    result = PAL_GetTimerExtended(pch, &timer);
    </code>

  Remarks:
    None
*/
uint8_t PAL_GetTimerExtended(uint16_t pch, uint64_t *timer);

// ****************************************************************************
/* Function:
    uint8_t PAL_GetCD(
        uint16_t pch,
        uint8_t *cd,
        uint8_t *rssi,
        uint32_t *time,
        uint8_t *header)

  Summary:
    Get the carrier detect signal.

  Description:
    This routine is used to get the value of carrier detect signal.

  Precondition:
    The PAL_Initialize function should have been called before calling this
    function.

  Parameters:
    pch             Physical channel
    cd              Carrier detect signal output parameter
    rssi            Received signal strength indicator
    time            Current time in us
    header          Header type

  Returns:
    PAL_CFG_SUCCESS         - If successful
    PAL_CFG_INVALID_INPUT   - If unsuccessful

  Example:
    <code>
    uint8_t result=PAL_CFG_SUCCESS;
    uint8_t cd=0;
    uint8_t rssi=0;
    uint32_t time=0;
    uint8_t header=0;
    uint16_t pch=1;

    result = PAL_GetCD(pch, &cd, &rssi, &time, &header);
    </code>

  Remarks:
    Not available for both PHY Serial and PHY RF.
*/
uint8_t PAL_GetCD(uint16_t pch, uint8_t *cd, uint8_t *rssi, uint32_t *time,
    uint8_t *header);

// ****************************************************************************
/* Function:
    uint8_t PAL_GetNL(uint16_t pch, uint8_t *noise)

  Summary:
    Get the noise floor level value.

  Description:
    This routine is used to know the noise level present in the powerline.

  Precondition:
    The PAL_Initialize function should have been called before calling this
    function.

  Parameters:
    pch            Physical channel
    noise          Noise floor level output parameter

  Returns:
    PAL_CFG_SUCCESS         - If successful
    PAL_CFG_INVALID_INPUT   - If unsuccessful

  Example:
    <code>
    uint8_t result=PAL_CFG_SUCCESS;
    uint8_t nl=0;
    uint16_t pch=1;

    result = PAL_GetNL(pch, &nl);
    </code>

  Remarks:
    Not available for PHY Serial.
*/
uint8_t PAL_GetNL(uint16_t pch, uint8_t *noise);

// ****************************************************************************
/* Function:
    uint8_t PAL_GetAGC(uint16_t pch, uint8_t *mode, uint8_t *gain)

  Summary:
    Get the automatic gain mode of the PHY PLC layer.

  Description:
    This routine is used to get Automatic Gain Mode (AGC) of the PHY PLC layer.

  Precondition:
    The PAL_Initialize function should have been called before calling this
    function.

  Parameters:
    pch            Physical channel
    mode           Auto/Manual mode
    gain           Initial receiving gain in auto mode

  Returns:
    PAL_CFG_SUCCESS         - If successful
    PAL_CFG_INVALID_INPUT   - If unsuccessful

  Example:
    <code>
    uint8_t result=PAL_CFG_SUCCESS;
    uint8_t mode=0;
    uint8_t gain=0;
    uint16_t pch=1;

    result = PAL_GetAGC(&mode, &gain);
    </code>

  Remarks:
    Not available for PHY Serial and PHY RF.
*/
uint8_t PAL_GetAGC(uint16_t pch, uint8_t *mode, uint8_t *gain);

// ****************************************************************************
/* Function:
    uint8_t PAL_SetAGC(uint16_t pch, uint8_t mode, uint8_t gain)

  Summary:
    Set the automatic gain mode of the PHY PLC layer.

  Description:
    This routine is used to set Automatic Gain Mode (AGC) of the PHY PLC layer.

  Precondition:
    The PAL_Initialize function should have been called before calling this
    function.

  Parameters:
    pch             Physical channel
    mode            Auto/Manual mode (auto mode(0), manual mode(1))
    gain            Initial receiving gain in auto mode

  Returns:
    PAL_CFG_SUCCESS         - If successful
    PAL_CFG_INVALID_INPUT   - If unsuccessful

  Example:
    <code>
    uint8_t result=PAL_CFG_SUCCESS;
    uint8_t mode=0;
    uint8_t gain=0;
    uint16_t pch=1;

    result = PAL_SetAGC(pch, mode, gain);
    </code>

  Remarks:
    Not available for PHY Serial and PHY RF.
*/
uint8_t PAL_SetAGC(uint16_t pch, uint8_t mode, uint8_t gain);

// ****************************************************************************
/* Function:
    uint8_t PAL_GetCCA(uint16_t pch, uint8_t *pState)

  Summary:
    Get clear pch assessment mode value.

  Description:
    This routine is used to get the clear pch assesment mode.
    The pch state helps to know whether or not the RF physical medium is
    free.

  Precondition:
    The PAL_Initialize function should have been called before calling this
    function.

  Parameters:
    pch                    Physical channel
    pState                 Channel state (0: busy, 1: free) of RF module

  Returns:
    PAL_CFG_SUCCESS         - If successful
    PAL_CFG_INVALID_INPUT   - If unsuccessful

  Example:
    <code>
    uint8_t result=PAL_CFG_SUCCESS;
    uint8_t pState=0;
    uint16 pch = 512;

    result = PAL_GetCCA(pch, &pState);
    </code>

  Remarks:
    Only implemented in PHY RF interface.
*/
uint8_t PAL_GetCCA(uint16_t pch, uint8_t *pState);

// ****************************************************************************
/* Function:
    uint8_t PAL_GetChannel(uint16_t *pch, uint16_t channelReference)

  Summary:
    Get the band (PLC) or the pch (RF).

  Description:
    This routine is used to get the pch or band used for the communication.

  Precondition:
    The PAL_Initialize function should have been called before calling this
    function.

  Parameters:
    pch               Pointer to store the Physical channel in use
    channelReference  Physical channel in the same channels range

  Returns:
    PAL_CFG_SUCCESS         - If successful
    PAL_CFG_INVALID_INPUT   - If unsuccessful

  Example:
    <code>
    uint8_t result=PAL_CFG_SUCCESS;
    uint16_t pch=0;
    uint16_t channelRef=1;

    result = PAL_GetChannel(&pch, channelRef);
    </code>

  Remarks:
    Not available for PHY Serial.
*/
uint8_t PAL_GetChannel(uint16_t *pch, uint16_t channelReference);

// ****************************************************************************
/* Function:
    uint8_t PAL_SetChannel(uint16_t pch)

  Summary:
    Set the band (PLC) or the pch (RF).

  Description:
    This routine is used to set the pch or band used for the communication.

  Precondition:
    The PAL_Initialize function should have been called before calling this
    function.

  Parameters:
    pch       Physical channel

  Returns:
    PAL_CFG_SUCCESS         - If successful
    PAL_CFG_INVALID_INPUT   - If unsuccessful

  Example:
    <code>
    uint8_t result=PAL_CFG_SUCCESS;
    uint16_t pch=1; This mask belongs to PLC channels

    result = PAL_SetChannel(pch);
    </code>

  Remarks:
    Not available for PHY Serial.
*/
uint8_t PAL_SetChannel(uint16_t pch);

// ****************************************************************************
/* Function:
    void PAL_ProgramChannelSwitch
    (
        uint16_t pch,
        uint32_t timeSync,
        uint8_t timeMode
    )

  Summary:
    Program a pch switch in the given time.

  Description:
    This routine is used to program a pch switch in the given time.

  Precondition:
    The PAL_Initialize function should have been called before calling this
    function.

  Parameters:
    pch               Physical Channel to be updated
    timeSync          Initial pch switch time in us
    timeMode          Channel switch time mode

  Returns:
    None

  Example:
    <code>
    uint32_t timeSync = 10000
    uint16_t pch = 600;
    uint8_t timeMode = PAL_TX_MODE_ABSOLUTE;

    PAL_ProgramChannelSwitch(pch, timeSync, timeMode);
    </code>

  Remarks:
    Only available for PHY RF.
*/
void PAL_ProgramChannelSwitch(uint16_t pch, uint32_t timeSync,
    uint8_t timeMode);

// ****************************************************************************
/* Function:
    uint8_t PAL_GetConfiguration(
        uint16_t pch,
        uint16_t id,
        void *val,
        uint16_t len)

  Summary:
    Get a PHY attribute.

  Description:
    This function is used to get a PHY attribute from the selected medium.

  Precondition:
    The PAL_Initialize function should have been called before calling this
    function.

  Parameters:
    pch             Physical channel
    id              Identifiers requested from the MAC layer
    val             Output parameter value
    length          Length of the parameter

  Returns:
    PAL_CFG_SUCCESS         - If successful
    PAL_CFG_INVALID_INPUT   - If unsuccessful

  Example:
    <code>
    uint8_t result=PAL_CFG_SUCCESS;
    uint16_t id=PAL_ID_CFG_ATTENUATION;
    void val=0;
    uint16_t len=1;
    uint16_t pch = 16;

    result = PAL_GetConfiguration(pch, id, &val, len);
    </code>

  Remarks:
    None
*/
uint8_t PAL_GetConfiguration(uint16_t pch, uint16_t id, void *val,
    uint16_t length);

// ****************************************************************************
/* Function:
    uint8_t PAL_SetConfiguration(
        uint16_t pch,
        uint16_t id,
        void *val,
        uint16_t len)

  Summary:
    Set PHY attribute.

  Description:
    This function is used to set a PHY attribute in the selected medium.

  Precondition:
    The PAL_Initialize function should have been called before calling this
    function.

  Parameters:
    pch             Physical channel
    id              PHY attribute identifier
    val             Input parameter value
    len             Length of the parameter

  Returns:
    PAL_CFG_SUCCESS         - If successful
    PAL_CFG_INVALID_INPUT   - If unsuccessful

  Example:
    <code>
    uint8_t result=PAL_CFG_SUCCESS;
    uint16_t id=PAL_ID_CFG_ATTENUATION;
    void val=2;
    uint16_t len=1;
    uint16_t pch = 16;

    result = PAL_SetConfiguration(pch, id, &val, len);
    </code>

  Remarks:
    Not available for PHY Serial.
*/
uint8_t PAL_SetConfiguration(uint16_t pch, uint16_t id, void *val,
    uint16_t length);

// ****************************************************************************
/* Function:
    uint16_t PAL_GetSignalCapture(
        uint16_t pch,
        uint8_t *noiseCapture,
        uint8_t mode,
        uint32_t timeStart,
        uint32_t duration)

  Summary:
    Get Capture Noise Data

  Description:
    This routine is used to read noise data for PLC medium communication.

  Precondition:
    The PAL_Initialize function should have been called before calling this
    function.

  Parameters:
    pch             Physical channel
    noiseCapture    Pointer to destination buffer to store data
    mode            Capture mode
    timeStart       Start time in us based on PL360 timer reference
    duration        Duration time in us

  Returns:
    Size in bytes of data capture.

  Example:
    <code>
    uint32_t timeStart = 10000;
    uint32_t duration = 5000;
    uint8_t noiseCapture[300];
    uint16_t noiseSize;
    uint8_t mode = PAL_MODE_TYPE_B;
    uint8_t pch = 1;

    noiseSize = PAL_GetSignalCapture(pch, &noiseCapture, mode, timeStart, duration);
    </code>

  Remarks:
    Only available for PHY PLC.
*/
uint16_t PAL_GetSignalCapture(uint16_t pch, uint8_t *noiseCapture, uint8_t mode,
                              uint32_t timeStart, uint32_t duration);
// ****************************************************************************
/* Function:
    uint8_t PAL_GetMsgDuration(
        uint16_t pch,
        uint16_t msgLen,
        PAL_SCHEME scheme,
        uint8_t mode,
        uint32_t *duration)

  Summary:
    Get message duration

  Description:
    This function is used to calculate the message duration.

  Precondition:
    The PAL_Initialize function should have been called before calling this
    function.

  Parameters:
    pch             Physical channel used
    msgLen          Message length
    scheme          Modulation scheme of message
    mode            Indicates if the message to transmit is type A, type B or
                    type BC
    duration        Pointer to message duration in us (output)

  Returns:
    PAL_CFG_SUCCESS         - If successful
    PAL_CFG_INVALID_INPUT   - If unsuccessful

  Example:
    <code>
    uint32_t duration = 0;
    uint16_t pch = 16;
    uint16_t msgLen = 30;
    PAL_SCHEME scheme = PAL_PLC_DBPSK_R;
    uint8_t mode = PAL_MODE_TYPE_B;
    uint8_t result=PAL_CFG_SUCCESS;

    result = PAL_GetMsgDuration(pch, msgLen, scheme, mode, &duration);
    </code>

  Remarks:
    Not available for PHY serial.
*/
uint8_t PAL_GetMsgDuration(uint16_t pch, uint16_t length, PAL_SCHEME scheme,
    uint8_t mode, uint32_t *duration);

// ****************************************************************************
/* Function:
   bool PAL_HasMinimumQuality(
    uint16_t pch,
    PAL_SCHEME scheme,
    uint8_t lessRobustMode)

  Summary:
    Check minimum quality for modulation scheme

  Description:
    This routine is used to check if the modulation is good enough for a low FER
    (Frame Error rate) for the given scheme.

  Precondition:
    The PAL_Initialize function should have been called before calling this
    function.

  Parameters:
    pch                 Physical channel used
    scheme              Modulation scheme of message
    lessRobustMode      Less robust modulation

  Returns:
    true         - If successful
    false        - If unsuccessful

  Example:
    <code>
    uint16_t pch = 16;
    PAL_SCHEME scheme = PAL_PLC_DBPSK_R;
    uint8_t lessRobustMode = PAL_PLC_DQPSK;
    bool result=true;

    result = PAL_HasMinimumQuality(pch, scheme, lessRobustMode);
    </code>

  Remarks:
    Not available for PHY Serial.
*/
bool PAL_CheckMinimumQuality(uint16_t pch, uint8_t reference, uint8_t modulation);

// ****************************************************************************
/* Function:
    uint8_t PAL_GetLessRobustModulation(uint16_t pch, uint8_t mod1, uint8_t mod2)

  Summary:
    Get less robust modulation scheme.

  Description:
    This routine is used to get less robust modulation scheme for a selected
    pch.

  Precondition:
    The PAL_Initialize function should have been called before calling this
    function.

  Parameters:
    pch             Physical channel used
    mod1            Modulation 1 to compare
    mod2            Modulation 2 to compare

  Returns:
   mod1 or mod2 scheme

  Example:
    <code>
    uint16_t pch=1;
    uint8_t mod;
    uint8_t mod1=PAL_PLC_DBPSK_R;
    uint8_t mod2=PAL_PLC_DQPSK;

    mod = PAL_GetLessRobustModulation(pch, mod1, mod2);
    </code>

  Remarks:
    Not available for PHY Serial.
*/
uint8_t PAL_GetLessRobustModulation(uint16_t pch, uint8_t mod1, uint8_t mod2);

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    }

#endif
// DOM-IGNORE-END

#endif //PAL_H
