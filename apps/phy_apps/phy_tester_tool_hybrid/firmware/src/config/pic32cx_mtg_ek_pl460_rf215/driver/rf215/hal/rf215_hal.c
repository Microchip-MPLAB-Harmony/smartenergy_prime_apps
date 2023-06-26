/******************************************************************************
  RF215 Driver Hardware Abstraction Layer Implementation.

  Company:
    Microchip Technology Inc.

  File Name:
    rf215_hal.c

  Summary:
    Source code for the RF215 Driver Hardware Abstraction Layer implementation.

  Description:
    The RF215 driver HAL (Hardware Abstraction Layer) manages the hardware
    peripherals used by the RF215 driver. This file contains the source code
    for the implementation of the RF215 driver HAL.
*******************************************************************************/

//DOM-IGNORE-BEGIN
/*******************************************************************************
* Copyright (C) 2022 Microchip Technology Inc. and its subsidiaries.
*
* Subject to your compliance with these terms, you may use Microchip software
* and any derivatives exclusively with Microchip products. It is your
* responsibility to comply with third party license terms applicable to your
* use of third party software (including open source software) that may
* accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
* EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
* WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
* PARTICULAR PURPOSE.
*
* IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
* INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
* WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
* BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
* FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
* ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
*******************************************************************************/
//DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Include Files
// *****************************************************************************
// *****************************************************************************
#include <string.h>
#include "configuration.h"
#include "system/ports/sys_ports.h"
#include "driver/rf215/drv_rf215_local.h"
#include "driver/rf215/hal/rf215_hal.h"

// *****************************************************************************
// *****************************************************************************
// Section: Macro Definitions
// *****************************************************************************
// *****************************************************************************

/* Reset pin pulse width.
 * From RF215 datasheet (Table 10-3): Min tRST = 625 ns */
#define RF215_RST_PULSE_US            7

/* SPI DMA buffer size */
#define RF215_SPI_BUF_SIZE            (RF215_SPI_CMD_SIZE + DRV_RF215_MAX_PSDU_LEN)

/* SPI transfer pool size: Maximum number of SPI transfers that can be queued */
#define RF215_SPI_TRANSFER_POOL_SIZE  35

// *****************************************************************************
// *****************************************************************************
// Section: File Scope Data
// *****************************************************************************
// *****************************************************************************

/* RF215 HAL object */
static RF215_HAL_OBJ rf215HalObj = {0};

/* SPI transfer pool */
static RF215_SPI_TRANSFER_OBJ halSpiTransferPool[RF215_SPI_TRANSFER_POOL_SIZE] = {0};

/* DMA buffers for SPI transmit and receive */
static uint8_t halSpiTxData[RF215_SPI_BUF_SIZE];
static uint8_t halSpiRxData[RF215_SPI_BUF_SIZE];

// *****************************************************************************
// *****************************************************************************
// Section: File Scope Functions
// *****************************************************************************
// *****************************************************************************

static inline void _RF215_HAL_ExtIntDisable()
{
    PIO_PinInterruptDisable((PIO_PIN) DRV_RF215_EXT_INT_PIN);
    rf215HalObj.extIntDisableCount++;
}

static inline void _RF215_HAL_ExtIntEnable()
{
    if (rf215HalObj.extIntDisableCount > 0)
    {
        rf215HalObj.extIntDisableCount--;
    }

    if (rf215HalObj.extIntDisableCount == 0)
    {
        PIO_PinInterruptEnable((PIO_PIN) DRV_RF215_EXT_INT_PIN);
    }
}

static inline bool _RF215_HAL_DisableIntSources(bool* timeIntStatus, bool* plcExtIntStatus)
{
    *plcExtIntStatus = SYS_INT_SourceDisable(rf215HalObj.plcExtIntSource);
    *timeIntStatus = SYS_INT_SourceDisable(rf215HalObj.sysTimeIntSource);
    return SYS_INT_SourceDisable(rf215HalObj.dmaIntSource);
}

static inline void _RF215_HAL_RestoreIntSources(bool dmaIntStatus, bool timeIntStatus, bool plcExtIntStatus)
{
    SYS_INT_SourceRestore(rf215HalObj.dmaIntSource, dmaIntStatus);
    SYS_INT_SourceRestore(rf215HalObj.sysTimeIntSource, timeIntStatus);
    SYS_INT_SourceRestore(rf215HalObj.plcExtIntSource, plcExtIntStatus);
}

static void _RF215_HAL_SpiTransferStart (
    RF215_SPI_TRANSFER_MODE mode,
    uint16_t regAddr,
    void* pData,
    size_t size
)
{
    size_t transferSize;
    uint16_t cmd;
    bool intStatus;
    uint8_t* pTxData = halSpiTxData;
    RF215_HAL_OBJ* hObj = &rf215HalObj;

    /* Build 16 bits corresponding to COMMAND
     * COMMAND[15:14] = MODE[1:0]; COMMAND[13:0] = ADDRESS[13:0] */
    cmd = regAddr | (uint16_t)mode;
    transferSize = size + RF215_SPI_CMD_SIZE;

    /* Write COMMAND to SPI transmit buffer (MSB first) */
    *pTxData++ = (uint8_t)(cmd >> 8);
    *pTxData++ = (uint8_t)(cmd & 0xFF);

    if (mode == RF215_SPI_WRITE)
    {
        /* Copy data to SPI transmit buffer */
        memcpy(pTxData, pData, size);
    }

    /* Disable all interrupts for a while to avoid delays between SPI transfer
     * and SYS_TIME counter read */
    intStatus = SYS_INT_Disable();

    /* Write and read through SPI. This triggers the SPI transfer */
    hObj->spiPlibWriteRead(halSpiTxData, transferSize, halSpiRxData, transferSize);

    /* Read SYS_TIME counter just after SPI transfer is launched */
    hObj->sysTimeTransfer = SYS_TIME_Counter64Get();
    SYS_INT_Restore(intStatus);
}

static void _RF215_HAL_SpiTransfer (
    RF215_SPI_TRANSFER_MODE mode,
    uint16_t regAddr,
    void* pData,
    size_t size,
    bool fromTasks,
    RF215_SPI_TRANSFER_CALLBACK callback,
    uintptr_t context
)
{
    bool dmaIntStatus, timeIntStatus, plcExtIntStatus;
    RF215_SPI_TRANSFER_OBJ* transfer;
    RF215_SPI_TRANSFER_OBJ* transferPoolEdge;

    /* Critical region to avoid conflict in SPI transfer queue */
    dmaIntStatus = _RF215_HAL_DisableIntSources(&timeIntStatus, &plcExtIntStatus);
    _RF215_HAL_ExtIntDisable();

    /* Look for a free transfer object in the pool */
    transferPoolEdge = &halSpiTransferPool[RF215_SPI_TRANSFER_POOL_SIZE];
    for (transfer = halSpiTransferPool; transfer < transferPoolEdge; transfer++)
    {
        if (transfer->inUse == false)
        {
            /* Copy transfer parameters */
            transfer->next = NULL;
            transfer->pData = pData;
            transfer->callback = callback;
            transfer->context = context;
            transfer->size = size;
            transfer->mode = mode;
            transfer->regAddr = regAddr;
            transfer->inUse = true;
            transfer->fromTasks = fromTasks;

            if (rf215HalObj.spiQueueFirst == NULL)
            {
                /* No SPI transfers in the queue */
                rf215HalObj.spiQueueFirst = transfer;
                rf215HalObj.spiQueueLast = transfer;
            }
            else
            {
                /* Add SPI transfer to the queue */
                rf215HalObj.spiQueueLast->next = transfer;
                rf215HalObj.spiQueueLast = transfer;
            }

            break;
        }
    }

    /* External interrupt kept disabled until SPI transfer finishes */
    if (rf215HalObj.spiQueueFirst == transfer)
    {
        /* This transfer is the first in the queue so it can be started */
        if (fromTasks == false)
        {
            _RF215_HAL_SpiTransferStart(mode, regAddr, pData, size);
        }
        else
        {
            rf215HalObj.spiTransferFromTasks = true;
        }
    }

    /* Leave critical region */
    _RF215_HAL_RestoreIntSources(dmaIntStatus, timeIntStatus, plcExtIntStatus);
}

static void _RF215_HAL_SpiTransferFinished(RF215_SPI_TRANSFER_OBJ* transfer)
{
    uint64_t callbackTime;
    uintptr_t callbackContext;
    void* callbackData;
    RF215_SPI_TRANSFER_OBJ* next;
    RF215_SPI_TRANSFER_CALLBACK callback;

    if (transfer->mode == RF215_SPI_READ)
    {
        /* Copy SPI received data to buffer from upper layer */
        memcpy(transfer->pData, &halSpiRxData[RF215_SPI_CMD_SIZE], transfer->size);
    }

    /* Copy needed data to local variables */
    callback = transfer->callback;
    callbackContext = transfer->context;
    callbackData = transfer->pData;
    callbackTime = rf215HalObj.sysTimeTransfer;

    /* The transfer object can now be freed */
    transfer->inUse = false;

    /* Check next transfer */
    next = transfer->next;
    if (next != NULL)
    {
        /* Move queue start to next transfer */
        rf215HalObj.spiQueueFirst = next;
    }
    else
    {
        /* No more SPI transfers in the queue */
        rf215HalObj.spiQueueFirst = NULL;
        rf215HalObj.spiQueueLast = NULL;
    }

    if (next != NULL)
    {
        /* Start next SPI transfer */
        if (next->fromTasks == false)
        {
            _RF215_HAL_SpiTransferStart(next->mode, next->regAddr, next->pData, next->size);
        }
        else
        {
            rf215HalObj.spiTransferFromTasks = true;
        }
    }

    if (callback != NULL)
    {
        /* Notify upper layer via callback */
        callback(callbackContext, callbackData, callbackTime);
    }

    /* External interrupt can now be enabled */
    _RF215_HAL_ExtIntEnable();
}

static void _RF215_HAL_SpiDmaHandler(uintptr_t ctxt)
{
    bool dmaIntStatus, timeIntStatus, plcExtIntStatus;
    RF215_SPI_TRANSFER_OBJ* transfer = rf215HalObj.spiQueueFirst;

    if (transfer == NULL)
    {
        /* Empty SPI transfer queue, probably because of RF215_HAL_Reset */
        return;
    }

    /* Critical region to avoid conflicts */
    dmaIntStatus = _RF215_HAL_DisableIntSources(&timeIntStatus, &plcExtIntStatus);
    _RF215_HAL_ExtIntDisable();

    /* SPI transfer finished successfully */
    _RF215_HAL_SpiTransferFinished(transfer);

    /* Leave critical region */
    _RF215_HAL_ExtIntEnable();
    _RF215_HAL_RestoreIntSources(dmaIntStatus, timeIntStatus, plcExtIntStatus);
}

static void _RF215_HAL_ExtIntHandler(PIO_PIN pin, uintptr_t context)
{
    /* Check if external interrupt is still active */
    if (SYS_PORT_PinRead((SYS_PORT_PIN)pin) == true)
    {
        DRV_RF215_ExtIntHandler();
    }
}

// *****************************************************************************
// *****************************************************************************
// Section: RF215 Driver HAL Interface Implementation
// *****************************************************************************
// *****************************************************************************

void RF215_HAL_Initialize(const DRV_RF215_INIT * const init)
{
    /* Store interrupt sources */
    rf215HalObj.sysTimeIntSource = init->sysTimeIntSource;
    rf215HalObj.dmaIntSource = init->dmaIntSource;
    rf215HalObj.plcExtIntSource = init->plcExtIntSource;

    /* Store pointers to SPI PLIB functions */
    rf215HalObj.spiPlibIsBusy = init->spiPlibIsBusy;
    rf215HalObj.spiPlibWriteRead = init->spiPlibWriteRead;

    /* Register callback for SPI transfer finished */
    init->spiPlibSetCallback(_RF215_HAL_SpiDmaHandler, 0);

    /* Register callback for external interrupt pin */
    PIO_PinInterruptCallbackRegister((PIO_PIN) DRV_RF215_EXT_INT_PIN,
            _RF215_HAL_ExtIntHandler, 0);

    /* Pin interrupt is disabled at initialization */
    rf215HalObj.extIntDisableCount = 1;
    rf215HalObj.firstReset = true;

    /* Zero initialization */
    rf215HalObj.spiQueueFirst = NULL;
    rf215HalObj.spiQueueLast = NULL;
    rf215HalObj.spiTransferFromTasks = false;
    rf215HalObj.ledRxOnCount = 0;
    rf215HalObj.ledTxOnCount = 0;
    for (uint8_t idx = 0; idx < RF215_SPI_TRANSFER_POOL_SIZE; idx++)
    {
        halSpiTransferPool[idx].inUse = false;
    }
}

void RF215_HAL_Deinitialize()
{
    bool dmaIntStatus, timeIntStatus, plcExtIntStatus;

    /* Critical region to avoid conflict in SPI transfer queue */
    dmaIntStatus = _RF215_HAL_DisableIntSources(&timeIntStatus, &plcExtIntStatus);
    _RF215_HAL_ExtIntDisable();

    /* Push reset pin */
    SYS_PORT_PinClear(DRV_RF215_RESET_PIN);

    /* Clear SPI transfer pool */
    rf215HalObj.spiQueueFirst = NULL;
    rf215HalObj.spiQueueLast = NULL;
    for (uint8_t idx = 0; idx < RF215_SPI_TRANSFER_POOL_SIZE; idx++)
    {
        halSpiTransferPool[idx].inUse = false;
    }

    /* Leave critical region. External interrupt disabled */
    _RF215_HAL_RestoreIntSources(dmaIntStatus, timeIntStatus, plcExtIntStatus);
}

void RF215_HAL_Tasks()
{
    if (rf215HalObj.spiTransferFromTasks == true)
    {
        RF215_SPI_TRANSFER_OBJ* transfer = rf215HalObj.spiQueueFirst;

        if (transfer != NULL)
        {
            bool dmaIntStatus, timeIntStatus, plcExtIntStatus;

            /* Critical region to avoid new SPI transfers */
            dmaIntStatus = _RF215_HAL_DisableIntSources(&timeIntStatus, &plcExtIntStatus);

            /* Start SPI transfer from tasks */
            _RF215_HAL_SpiTransferStart(transfer->mode, transfer->regAddr, transfer->pData, transfer->size);

            /* Leave critical region */
            _RF215_HAL_RestoreIntSources(dmaIntStatus, timeIntStatus, plcExtIntStatus);
        }

        rf215HalObj.spiTransferFromTasks = false;
    }
}

void RF215_HAL_Reset()
{
    SYS_TIME_HANDLE timeHandle;
    bool dmaIntStatus, timeIntStatus, plcExtIntStatus;

    /* Critical region to avoid interrupts during reset */
    dmaIntStatus = _RF215_HAL_DisableIntSources(&timeIntStatus, &plcExtIntStatus);
    if (rf215HalObj.firstReset == false)
    {
        _RF215_HAL_ExtIntDisable();
    }

    if (rf215HalObj.spiQueueFirst != NULL)
    {
        /* Wait to SPI transfer to finish */
        while(rf215HalObj.spiPlibIsBusy() == true);
    }

    /* Push reset pin */
    SYS_PORT_PinClear(DRV_RF215_RESET_PIN);

    /* Clear SPI transfer pool. Pending SPI transfers aborted */
    rf215HalObj.spiQueueFirst = NULL;
    rf215HalObj.spiQueueLast = NULL;
    rf215HalObj.spiTransferFromTasks = false;
    rf215HalObj.firstReset = false;
    for (uint8_t idx = 0; idx < RF215_SPI_TRANSFER_POOL_SIZE; idx++)
    {
        halSpiTransferPool[idx].inUse = false;
    }

    /* Perform reset pulse delay (SYS_TIME interrupt has to be enabled) */
    SYS_INT_SourceEnable(rf215HalObj.sysTimeIntSource);
    if (SYS_TIME_DelayUS(RF215_RST_PULSE_US, &timeHandle) == SYS_TIME_SUCCESS)
    {
        while (SYS_TIME_DelayIsComplete(timeHandle) == false);
    }

    /* Disable again SYS_TIME interrupt */
    SYS_INT_SourceDisable(rf215HalObj.sysTimeIntSource);

    /* Release reset pin and enable/restore interrupts */
    SYS_PORT_PinSet(DRV_RF215_RESET_PIN);
    _RF215_HAL_ExtIntEnable();
    _RF215_HAL_RestoreIntSources(dmaIntStatus, timeIntStatus, plcExtIntStatus);
}

bool RF215_HAL_SpiLock()
{
    /* Disable interrupts to avoid new SPI transfers */
    RF215_HAL_OBJ* hObj = &rf215HalObj;
    hObj->dmaIntStatus = _RF215_HAL_DisableIntSources(&hObj->sysTimeIntStatus, &hObj->plcExtIntStatus);
    _RF215_HAL_ExtIntDisable();

    if (hObj->spiQueueFirst == NULL)
    {
        /* SPI is free */
        return true;
    }
    else
    {
        /* SPI is busy */
        return false;
    }
}

void RF215_HAL_SpiUnlock()
{
    /* Restore interrupts */
    RF215_HAL_OBJ* hObj = &rf215HalObj;
    _RF215_HAL_ExtIntEnable();
    _RF215_HAL_RestoreIntSources(hObj->dmaIntStatus, hObj->sysTimeIntStatus, hObj->plcExtIntStatus);
}

void RF215_HAL_EnterCritical()
{
    bool intStatus = SYS_INT_Disable();

    /* Critical region: Disable interrupts to avoid conflicts
     * External interrupt not disabled because it just makes one SPI transfer
     * and there is no other static variable update */
    RF215_HAL_OBJ* hObj = &rf215HalObj;
    hObj->dmaIntStatus = _RF215_HAL_DisableIntSources(&hObj->sysTimeIntStatus, &hObj->plcExtIntStatus);

    SYS_INT_Restore(intStatus);
}

void RF215_HAL_LeaveCritical()
{
    bool intStatus = SYS_INT_Disable();

    /* Leave critical region: Restore interrupts */
    RF215_HAL_OBJ* hObj = &rf215HalObj;
    _RF215_HAL_RestoreIntSources(hObj->dmaIntStatus, hObj->sysTimeIntStatus, hObj->plcExtIntStatus);

    SYS_INT_Restore(intStatus);
}

void RF215_HAL_SpiRead (
    uint16_t addr,
    void* pData,
    size_t size,
    RF215_SPI_TRANSFER_CALLBACK cb,
    uintptr_t context
)
{
    _RF215_HAL_SpiTransfer(RF215_SPI_READ, addr, pData, size, false, cb, context);
}

void RF215_HAL_SpiReadFromTasks (
    uint16_t addr,
    void* pData,
    size_t size,
    RF215_SPI_TRANSFER_CALLBACK cb,
    uintptr_t context
)
{
    _RF215_HAL_SpiTransfer(RF215_SPI_READ, addr, pData, size, true, cb, context);
}

void RF215_HAL_SpiWrite (
    uint16_t addr,
    void* pData,
    size_t size
)
{
    _RF215_HAL_SpiTransfer(RF215_SPI_WRITE, addr, pData, size, false, NULL, 0);
}

void RF215_HAL_SpiWriteUpdate (
    uint16_t addr,
    uint8_t* pDataNew,
    uint8_t* pDataOld,
    size_t size
)
{
    uint8_t* pDataWrite = pDataOld;
    size_t sizeWrite = 0;
    size_t sizeSame = 0;
    uint16_t addrWrite = addr;

    for (uint8_t idx = 0; idx < size; idx++)
    {
        if (pDataNew[idx] != pDataOld[idx])
        {
            /* Different byte, needs to be written. Update static array to store
             * register values */
            pDataOld[idx] = pDataNew[idx];

            /* Split the SPI transfer only if it is worth: 3 consecutive bytes
             * not updated (2 SPI header bytes) */
            if (sizeSame > RF215_SPI_CMD_SIZE)
            {
                RF215_HAL_SpiWrite(addrWrite, pDataWrite, sizeWrite);
                sizeWrite = 0;
                sizeSame = 0;
            }

            /* Check start of SPI transfer block */
            if (sizeWrite == 0)
            {
                addrWrite = addr + idx;
                pDataWrite = &pDataOld[idx];
            }

            /* Update write counters */
            sizeWrite += (sizeSame + 1);
            sizeSame = 0;
        }
        else if (sizeWrite != 0)
        {
            /* Same byte and SPI transfer block started */
            sizeSame += 1;
        }
    }

    if (sizeWrite != 0)
    {
        /* Send the last SPI transfer */
        RF215_HAL_SpiWrite(addrWrite, pDataWrite, sizeWrite);
    }
}

size_t RF215_HAL_GetSpiQueueSize()
{
    size_t queueSize = 0;
    RF215_SPI_TRANSFER_OBJ* transfer = rf215HalObj.spiQueueFirst;

    while (transfer != NULL)
    {
        queueSize += transfer->size + RF215_SPI_CMD_SIZE;
        transfer = transfer->next;
    }

    return queueSize;
}

void RF215_HAL_LedRx(bool on)
{
    if (on == true)
    {
        SYS_PORT_PinSet(DRV_RF215_LED_RX_PIN);
        rf215HalObj.ledRxOnCount++;
    }
    else
    {
        if (rf215HalObj.ledRxOnCount > 0)
        {
            rf215HalObj.ledRxOnCount--;
        }

        if (rf215HalObj.ledRxOnCount == 0)
        {
            SYS_PORT_PinClear(DRV_RF215_LED_RX_PIN);
        }
    }
}

void RF215_HAL_LedTx(bool on)
{
    if (on == true)
    {
        SYS_PORT_PinSet(DRV_RF215_LED_TX_PIN);
        rf215HalObj.ledTxOnCount++;
    }
    else
    {
        if (rf215HalObj.ledTxOnCount > 0)
        {
            rf215HalObj.ledTxOnCount--;
        }

        if (rf215HalObj.ledTxOnCount == 0)
        {
            SYS_PORT_PinClear(DRV_RF215_LED_TX_PIN);
        }
    }
}

