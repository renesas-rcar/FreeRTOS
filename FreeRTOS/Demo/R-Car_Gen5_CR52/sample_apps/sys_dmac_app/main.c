/*
 * FreeRTOS Kernel V11.1.0
 * Copyright (C) 2021 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://github.com/FreeRTOS
 *
 */

/* Scheduler include files. */
#include "interrupts.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "dmac/dmac_common.h"
#include "dmac/rtdmac_ctrl.h"
#include "dmac/sysdmac_ctrl.h"
#include "pfc/r_pfc_api.h"
#include "device_tree.h"
#include "rcar_utils.h"

#include "stdio.h"
#include "stdbool.h"
#include <stdlib.h>
#define main_DMAC_TASK_PRIORITY        ( tskIDLE_PRIORITY + 1 )
#define DESTINATION_OFFSET 			   0x01000000

void dmacUserCallback(void *data);
void dmacUserCallback1(void *data);
void dmacUserCallback2(void *data);
void dmacUserCallback3(void *data);

/*-----------------------------------------------------------*/

/*
 * Configure the hardware as necessary to run this demo.
 */
static void prvSetupHardware( void );

static void prvSYSDMACTask( void *pvParameters );
static uint32_t get_osal_addr(uint32_t offset);

SemaphoreHandle_t xSemaphore = NULL;
#define DMA_WAIT_TIMEOUT_MS   (100U)

/*------------------------- Configure mem-to-mem with Normal mode ----------------------------------*/
/* Define configure DMA Controller */
rDmacCfg_t cfg0 =
{
    //Fill in the configuration details
    .mSrcAddr = 0,
    .mDestAddr = 0,
    .mTransferCount = 1,
    .mDMAMode = DRV_DMAC_DMA_NO_DESCRIPTOR, // Assuming DRV_DMAC_DMA_NO_DESCRIPTOR is defined
    .mSrcAddrMode = DRV_RTDMAC_ADDR_FIXED, // Assuming ADDR_MODE_FIXED is defined
    .mDestAddrMode = DRV_RTDMAC_ADDR_FIXED, // Assuming ADDR_MODE_FIXED is defined
    .mTransferUnit = DRV_RTDMAC_TRANS_UNIT_4BYTE, // Assuming DRV_RTDMAC_TRANS_UNIT_4BYTE is defined
    .mResource = DRV_RTDMAC_MEMORY, // Assuming DRV_RTDMAC_MEMORY is defined
    .mLowSpeed = DRV_RTDMAC_SPEED_NORMAL, // Assuming DRV_RTDMAC_SPEED_NORMAL is defined
    .mPrioLevel = 0
};

/*------------------------- Configure mem-to-mem with Descriptor Normal mode ----------------------------------*/
rDmacCfg_t cfg1 =
{
    //Fill in the configuration details
    .mSrcAddr = 0,
    .mDestAddr = 0,
    .mTransferCount = 3,
    .mDMAMode = DRV_DMAC_DMA_DESC_NORMAL, // Assuming DRV_DMAC_DMA_DESC_NORMAL is defined
    .mSrcAddrMode = DRV_RTDMAC_ADDR_FIXED, // Assuming ADDR_MODE_FIXED is defined
    .mDestAddrMode = DRV_RTDMAC_ADDR_INCREMENTED, // Assuming ADDR_MODE_INCREMENTED is defined
    .mTransferUnit = DRV_RTDMAC_TRANS_UNIT_4BYTE, // Assuming DRV_RTDMAC_TRANS_UNIT_4BYTE is defined
    .mResource = DRV_RTDMAC_MEMORY, // Assuming DRV_RTDMAC_MEMORY is defined
    .mLowSpeed = DRV_RTDMAC_SPEED_NORMAL, // Assuming DRV_RTDMAC_SPEED_NORMAL is defined
    .mPrioLevel = 0
};

/*
 * Offset table for TC4 descriptors.
 * Actual address = OSAL base + offset, resolved at runtime in prvDMACTask().
 * With OSAL base = 0x6B800000:
 *   [0] SAR=0x6C000000  DAR=0x7B800000
 *   [1] SAR=0x6C100000  DAR=0x7B900000
 *   [2] SAR=0x6C200000  DAR=0x7BA00000
 *   [3] SAR=0x6C300000  DAR=0x7BB00000
 */
static const uint32_t desc0_off[][2] = {
    /* {SAR_off, DAR_off} */
    {0x00800000, 0x10000000},
    {0x00900000, 0x10100000},
    {0x00A00000, 0x10200000},
    {0x00B00000, 0x10300000},
};

rDmacDescMemCfg_t desc_mem[] = { 
    {.SAR = 0x00000000, .DAR = 0x00000000, .TCR = 4, .CHCR = 0},
    {       0x00000000,        0x00000000,        4,         0},
    {       0x00000000,        0x00000000,        4,         0},
    {       0x00000000,        0x00000000,        4,         0},
};

/* Define configure the DMA descriptor */
rDmacDescCfg_t descCfg1 =
{
    .mDescBaseAddr = (uintptr_t)desc_mem,
    .mDescUpdate = {
        .mCHCRUpdate = false,
        .mDestAddrUpdate = true,
        .mSrcAddrUpdate = true,
        .mTransCountUpdate = true,
    },
    .mDescRead1st = true,
    .mStateEndEnable = false,
    .mDescCount = 4,
    .mDescIndex = 0
};

/*------------------------- Configure mem-to-mem with Descriptor Repeat mode ----------------------------------*/
/*
 * Offset table for TC5 descriptors (Repeat mode).
 * Actual address = OSAL base + offset, resolved at runtime in prvDMACTask().
 * SAR is the same for all descriptors (shared source); DAR increments.
 * With OSAL base = 0x6B800000:
 *   [0] SAR=0x6D000000  DAR=0x7D000000
 *   [1] SAR=0x6D000000  DAR=0x7D100000
 *   [2] SAR=0x6D000000  DAR=0x7D200000
 *   [3] SAR=0x6D000000  DAR=0x7D300000
 */
static const uint32_t desc2_off[][2] = {
    /* {SAR_off, DAR_off} */
    {0x01800000, 0x11800000},
    {0x01800000, 0x11900000},
    {0x01800000, 0x11A00000},
    {0x01800000, 0x11B00000},
};

rDmacDescMemCfg_t desc_mem2[] = {
    {.SAR = 0x00000000, .DAR = 0x00000000, .TCR = 4, .CHCR = 0},
    {       0x00000000,        0x00000000,        4,         0},
    {       0x00000000,        0x00000000,        4,         0},
    {       0x00000000,        0x00000000,        4,         0},
};

/* Define configure DMA Controller */
rDmacCfg_t cfg2 =
{
    .mSrcAddr = 0,                     // Source address
    .mDestAddr = 0,                    // Destination address
    .mTransferCount = 4,                        // Transfer count
    .mDMAMode = DRV_DMAC_DMA_DESC_REPEAT,       // Reapeat descriptor mode
    .mSrcAddrMode = DRV_RTDMAC_ADDR_FIXED,      // Source address fixed
    .mDestAddrMode = DRV_RTDMAC_ADDR_INCREMENTED, // Destination address increment
    .mTransferUnit = DRV_RTDMAC_TRANS_UNIT_4BYTE, // Transfer unit 4 bytes
    .mResource = DRV_RTDMAC_MEMORY,            // DMA resource
    .mLowSpeed = DRV_RTDMAC_SPEED_NORMAL,
    .mPrioLevel = 1
};

/* Define configure the DMA descriptor */
rDmacDescCfg_t descCfg2 =
{
    .mDescBaseAddr = (uintptr_t)desc_mem2,                // Descriptor base address
    .mDescUpdate = {                            // No descriptor update
        .mCHCRUpdate = false,
        .mDestAddrUpdate = true,
        .mSrcAddrUpdate = true,
        .mTransCountUpdate = true,
    },
    .mDescRead1st = true,                       // Read descriptor first
    .mStateEndEnable = false,                   // No Trigger state end
    .mDescCount = 4,                            // Descriptor count
    .mDescIndex = 0                             // Descriptor index
};

#define REPEAT_NUMBER           4

/*------------------------- Configure mem-to-mem with Descriptor Read-out mode ----------------------------------*/

/* Define configure DMA Controller */
rDmacCfg_t cfg3 =
{
    .mSrcAddr = 0,
    .mDestAddr = 0,
    .mTransferCount = 4,                          // Transfer count
    .mDMAMode = DRV_DMAC_DMA_DESC_READOUT,       // Infinite Reapeat descriptor mode
    .mSrcAddrMode = DRV_RTDMAC_ADDR_FIXED,        // Source address fixed
    .mDestAddrMode = DRV_RTDMAC_ADDR_INCREMENTED, // Destination address increment
    .mTransferUnit = DRV_RTDMAC_TRANS_UNIT_4BYTE, // Transfer unit 4 bytes
    .mResource = DRV_RTDMAC_MEMORY,               // DMA resource
    .mLowSpeed = DRV_RTDMAC_SPEED_NORMAL,
    .mPrioLevel = 1
};

/*
 * Offset table for TC6 descriptors (Read-out mode, 8 descriptors).
 * Actual address = OSAL base + offset, resolved at runtime in prvDMACTask().
 * SAR is the same for all descriptors (shared source); DAR increments.
 * With OSAL base = 0x6B800000:
 *   [0] SAR=0x6E000000  DAR=0x78000000
 *   [1] SAR=0x6E000000  DAR=0x78100000
 *   [2] SAR=0x6E000000  DAR=0x78200000
 *   [3] SAR=0x6E000000  DAR=0x78300000
 *   [4] SAR=0x6E000000  DAR=0x78400000
 *   [5] SAR=0x6E000000  DAR=0x78500000
 *   [6] SAR=0x6E000000  DAR=0x78600000
 *   [7] SAR=0x6E000000  DAR=0x78700000
 */
static const uint32_t desc3_off[][2] = {
    /* {SAR_off, DAR_off} */
    {0x02800000, 0x0C800000},
    {0x02800000, 0x0C900000},
    {0x02800000, 0x0CA00000},
    {0x02800000, 0x0CB00000},
    {0x02800000, 0x0CC00000},
    {0x02800000, 0x0CD00000},
    {0x02800000, 0x0CE00000},
    {0x02800000, 0x0CF00000},
};

rDmacDescMemCfg_t desc_mem3[] = {
    {.SAR = 0x00000000, .DAR = 0x00000000, .TCR = 4, .CHCR = 0},
    {       0x00000000,        0x00000000,        4,         0},
    {       0x00000000,        0x00000000,        4,         0},
    {       0x00000000,        0x00000000,        4,         0},
    {       0x00000000,        0x00000000,        4,         0},
    {       0x00000000,        0x00000000,        4,         0},
    {       0x00000000,        0x00000000,        4,         0},
    {       0x00000000,        0x00000000,        4,         0},
};

/* Define configure the DMA descriptor */
rDmacDescCfg_t desccfg3 =
{
    .mDescBaseAddr = (uintptr_t)desc_mem3,      // Descriptor base address
    .mDescUpdate = {                            // Descriptor update
        .mCHCRUpdate = false,
        .mDestAddrUpdate = true,
        .mSrcAddrUpdate = true,
        .mTransCountUpdate = true,
    },
    .mDescRead1st = true,        // Read descriptor first
    .mStateEndEnable = true,     // Trigger state end
    .mDescCount = 8,             // Descriptor count
    .mDescIndex = 2              // Descriptor index
};

/*------------------------- Configure mem-to-mem with Descriptor Infinite Repeat mode ----------------------------------*/

/* Define configure DMA Controller */
rDmacCfg_t cfg4 =
{
    .mSrcAddr = 0,
    .mDestAddr = 0,
    .mTransferCount = 4,                          // Transfer count
    .mDMAMode = DRV_DMAC_DMA_DESC_INFINITE,       // Infinite Reapeat descriptor mode
    .mSrcAddrMode = DRV_RTDMAC_ADDR_FIXED,        // Source address fixed
    .mDestAddrMode = DRV_RTDMAC_ADDR_INCREMENTED, // Destination address increment
    .mTransferUnit = DRV_RTDMAC_TRANS_UNIT_4BYTE, // Transfer unit 4 bytes
    .mResource = DRV_RTDMAC_MEMORY,               // DMA resource
    .mLowSpeed = DRV_RTDMAC_SPEED_NORMAL,
    .mPrioLevel = 1
};

/*
 * Offset table for TC7 descriptors (Infinite Repeat mode).
 * Actual address = OSAL base + offset, resolved at runtime in prvDMACTask().
 * SAR is the same for all descriptors (shared source); DAR increments.
 * With OSAL base = 0x6B800000:
 *   [0] SAR=0x6F000000  DAR=0x79000000
 *   [1] SAR=0x6F000000  DAR=0x79100000
 *   [2] SAR=0x6F000000  DAR=0x79200000
 *   [3] SAR=0x6F000000  DAR=0x79300000
 */
static const uint32_t desc4_off[][2] = {
    /* {SAR_off, DAR_off} */
    {0x03800000, 0x0D800000},
    {0x03800000, 0x0D900000},
    {0x03800000, 0x0DA00000},
    {0x03800000, 0x0DB00000},
};

rDmacDescMemCfg_t desc_mem4[] = {
    {.SAR = 0x00000000, .DAR = 0x00000000, .TCR = 4, .CHCR = 0},
    {       0x00000000,        0x00000000,        4,         0},
    {       0x00000000,        0x00000000,        4,         0},
    {       0x00000000,        0x00000000,        4,         0},
};

/* Define configure the DMA descriptor */
rDmacDescCfg_t desccfg4 =
{
    .mDescBaseAddr = (uintptr_t)desc_mem4,      // Descriptor base address
    .mDescUpdate = {                            // Descriptor update
        .mCHCRUpdate = false,
        .mDestAddrUpdate = true,
        .mSrcAddrUpdate = true,
        .mTransCountUpdate = true,
    },
    .mDescRead1st = true,        // Read descriptor first
    .mStateEndEnable = false,    // No trigger state end
    .mDescCount = 4,             // Descriptor count
    .mDescIndex = 0              // Descriptor index
};

/*-----------------------------------------------------------*/

rDmacIrqCfg_t rDmacIrqHandler_t_irq =
{
    .Unit = SYS_DMAC3,
    .SubCh = DMAC_CH1,
    .irq_channel = INTID_SYSDMA3_CH1,
};

rDmacIrqCfg_t rDmacIrqHandler_t_irq1 =
{
    .Unit = SYS_DMAC2,
    .SubCh = DMAC_CH2,
    .irq_channel = INTID_SYSDMA2_CH2,
};

rDmacIrqCfg_t rDmacIrqHandler_t_irq2 =
{
    .Unit = SYS_DMAC1,
    .SubCh = DMAC_CH1,
    .irq_channel = INTID_SYSDMA1_CH1,
};

rDmacIrqCfg_t rDmacIrqHandler_t_irq3 =
{
    .Unit = SYS_DMAC0,
    .SubCh = DMAC_CH0,
    .irq_channel = INTID_SYSDMA0_CH0,
};

/*-----------------------------------------------------------*/

int main( void )
{
        /* Configure the hardware ready to run the demo. */
        prvSetupHardware();

        xSemaphore = xSemaphoreCreateBinary();
        xTaskCreate( prvSYSDMACTask, "SYSDMACTask", configMINIMAL_STACK_SIZE * 2, NULL, main_DMAC_TASK_PRIORITY, NULL );
        /* Start the tasks and timer running. */
        vTaskStartScheduler();
        for( ;; )
        {
        }
        /* Don't expect to reach here. */
        return 0;
}
/*-----------------------------------------------------------*/

static void prvSetupHardware( void )
{
    /* Ensure no interrupts execute while the scheduler is in an inconsistent
    state.  Interrupts are automatically enabled when the scheduler is
    started. */
    portDISABLE_INTERRUPTS();
    Irq_Setup();
    (void)pfcInitModules(getModuleConfigs());
}

static uint32_t get_osal_addr(uint32_t offset)
{
    static st_memory_t region = {0};

    if (region.size == 0)
    {
        region = R_UTILS_GetMemoryRegionInfo(OSAL, 0);
    }

    return region.base_address + offset;
}

static void prvSYSDMACTask( void *pvParameters )
{
    /* Remove compiler warning about unused parameter. */
    ( void ) pvParameters;
    int ret;

    Context_t usr_context =
    {
        .ctx = &rDmacIrqHandler_t_irq,
    };

    Context_t usr_context1 =
    {
        .ctx = &rDmacIrqHandler_t_irq1,
    };

    Context_t usr_context2 =
    {
        .ctx = &rDmacIrqHandler_t_irq2,
    };

    Context_t usr_context3 =
    {
        .ctx = &rDmacIrqHandler_t_irq3,
    };

    /* Get memory region first */
    st_memory_t region = R_UTILS_GetMemoryRegionInfo(OSAL, 0);
    cfg0.mSrcAddr = region.base_address;
    cfg0.mDestAddr = region.base_address + DESTINATION_OFFSET;
    if (cfg0.mDestAddr > region.base_address + region.size) 
    {
        printf("Failed: Destination address 0x%08X exceeds memory region (end at 0x%08X)\n", cfg0.mDestAddr, region.base_address + region.size);
        for( ;; )
        {
            vTaskDelay(3000);
        }
    }
    printf("***TC1: Initialize SYS-DMAC Control***\r\n");

    /* Device Driver Part */
    ret = R_SYSDMAC_RcarDmacCtrlInit( rDmacIrqHandler_t_irq.Unit, DRV_RTDMAC_PRIO_FIX);

    ret = R_SYSDMAC_RcarCallBackSet(&rDmacIrqHandler_t_irq, dmacUserCallback, &usr_context);

    if (ret == 0)
    {
        printf("TC1 Result: Passed\n");
    }
    else
    {
        printf("TC1 Result: Failed\n");
    }
    printf("**********************************************************\r\n");
    *(volatile uint32_t *)cfg0.mDestAddr = 0x9;
    *(volatile uint32_t *)cfg0.mSrcAddr = 0x3;

    int dmaStatus =R_SYSDMAC_RcarDmacExec(rDmacIrqHandler_t_irq.Unit, rDmacIrqHandler_t_irq.SubCh, &cfg0, NULL);

    // Check DMA execution status
    if (dmaStatus != 0)
        printf("DMA execution failed with status: %d\n", dmaStatus);

    printf("***TC2: SYS-DMAC Interrupt Callback***\r\n");

    // Wait DMA to transfer data.
    if (xSemaphoreTake(xSemaphore, pdMS_TO_TICKS(DMA_WAIT_TIMEOUT_MS)) == pdTRUE)
    {
        printf("TC2 Result: Passed\n");
    }
    else
    {
        printf("TC2 Result: Failed\n");
    }
    printf("**********************************************************\r\n");
    printf("***TC3: SYS-DMAC mem-to-mem transfer in Normal mode***\r\n");

    printf("Value at DestAddr before DMA: 0x%x \n", *(volatile uint32_t*)cfg0.mDestAddr);
    printf("Value at SrcAddr: 0x%x \n",*(volatile uint32_t*)cfg0.mSrcAddr);

    // Verify destination data
    uint32_t total_transfer_size = 4;
    uint32_t destData = R_UTILS_ReadMemForDMA((void*)cfg0.mDestAddr, total_transfer_size);
    printf("Value at DestAddr after DMA: 0x%x \n", destData);

    if( destData == (*(volatile uint32_t*)cfg0.mSrcAddr) )
    {
        printf("TC3 Result: Passed\n");
    }
    else
    {
        printf("TC3 Result: Failed\n");
    }

    R_SYSDMAC_RcarDmacStop(rDmacIrqHandler_t_irq.Unit, rDmacIrqHandler_t_irq.SubCh);

    printf("*************************************************************\r\n");
    printf("***TC4: SYS-DMAC mem-to-mem transfer in Descriptor Normal mode***\r\n");

    for (int i = 0; i < descCfg1.mDescCount; i++)
    {
        desc_mem[i].SAR = get_osal_addr(desc0_off[i][0]);
        desc_mem[i].DAR = get_osal_addr(desc0_off[i][1]);
    }

    /* Device Driver Part */
    R_SYSDMAC_RcarDmacCtrlInit(rDmacIrqHandler_t_irq1.Unit, DRV_RTDMAC_PRIO_FIX);

    /* Convert descriptor offsets to absolute addresses */
    for(int i = 0; i < descCfg1.mDescCount ; i++)
    {
        *(volatile uint32_t*)desc_mem[i].DAR = 0x33333333;
        *(volatile uint32_t*)desc_mem[i].SAR = 0x11111111;

        printf("Before dma: Value src desc %d at %x : 0x%x \n", i, desc_mem[i].SAR, *(volatile uint32_t*)desc_mem[i].SAR);
        printf("Before dma: Value dst desc %d at %x : 0x%x \n", i, desc_mem[i].DAR, *(volatile uint32_t*)desc_mem[i].DAR);
    }

    
    ret = R_SYSDMAC_RcarCallBackSet(&rDmacIrqHandler_t_irq1, dmacUserCallback1, &usr_context1);
    if (ret)
        printf("CallbackSet Failed: ret = %d\n", ret);

    dmaStatus =R_SYSDMAC_RcarDmacExec(rDmacIrqHandler_t_irq1.Unit, rDmacIrqHandler_t_irq1.SubCh, &cfg1, &descCfg1);


    // Check DMA execution status
    if (dmaStatus != 0)
        printf("DMA execution failed with status: %d\n", dmaStatus);

    // Wait DMA to transfer data.
    if(xSemaphoreTake(xSemaphore, portMAX_DELAY) == pdTRUE)
    {

    }

    // Verify destination data
    total_transfer_size = 4;
    printf("Verify destination data\n");
    ret = 0;
    for(int i = 0; i < descCfg1.mDescCount ; i++)
    {
        for(int j = 0; j < desc_mem[i].TCR ; j++)
        {
            destData = R_UTILS_ReadMemForDMA((void*)(desc_mem[i].DAR + 4*j), total_transfer_size);
            printf("After dma: Value dst desc %d at %x : 0x%x \n", i, (desc_mem[i].DAR + 4*j), destData);
            if(destData != (*(volatile uint32_t*)(desc_mem[i].SAR)))
            {
                ret = -1;
            }
        }
        
    }

    if(ret == 0)
    {
        printf("TC4 Result: Passed\n");
    }
    else
    {
        printf("TC4 Result: Failed\n");
    }

    R_SYSDMAC_RcarDmacStop(rDmacIrqHandler_t_irq1.Unit, rDmacIrqHandler_t_irq1.SubCh);
    printf("*************************************************************\r\n");
    printf("***TC5: SYS-DMAC mem-to-mem transfer in Descriptor Repeat mode***\r\n");

    /* Convert descriptor offsets to absolute addresses */
    for (int i = 0; i < descCfg2.mDescCount; i++)
    {
        desc_mem2[i].SAR = get_osal_addr(desc2_off[i][0]);
        desc_mem2[i].DAR = get_osal_addr(desc2_off[i][1]);
    }

    /* Device Driver Part */
    R_SYSDMAC_RcarDmacCtrlInit(rDmacIrqHandler_t_irq2.Unit, DRV_RTDMAC_PRIO_FIX);

    *(volatile uint32_t*)desc_mem2[0].SAR = 0x11111111;
    printf("Before dma: Value src desc at %x : 0x%x \n", desc_mem2[0].SAR, *(volatile uint32_t*)desc_mem2[0].SAR);

    ret = R_SYSDMAC_RcarCallBackSet(&rDmacIrqHandler_t_irq2, dmacUserCallback2, &usr_context2);
    if (ret)
        printf("CallbackSet Failed: ret = %d\n", ret);

    dmaStatus =R_SYSDMAC_RcarDmacExec(rDmacIrqHandler_t_irq2.Unit, rDmacIrqHandler_t_irq2.SubCh, &cfg2, &descCfg2);


    // Check DMA execution status
    if (dmaStatus != 0)
        printf("DMA execution failed with status: %d\n", dmaStatus);

    // Wait DMA to transfer data.
    if (xSemaphoreTake(xSemaphore, pdMS_TO_TICKS(DMA_WAIT_TIMEOUT_MS)) != pdTRUE)
    {
        printf("Timeout: no TE interrupt received\r\n");
        R_RTDMAC_RcarDmacStop(rDmacIrqHandler_t_irq2.Unit, rDmacIrqHandler_t_irq2.SubCh);
    }

    total_transfer_size = 4;
    uint32_t descadd;
    printf("Verify destination data\n");
    ret = 0;
    for (int  repeat = 0; repeat < REPEAT_NUMBER; repeat++)
    {
        for(int i = 0; i < descCfg1.mDescCount ; i++)
        {
            for(int j = 0; j < desc_mem2[0].TCR ; j++)
            {
                descadd = (desc_mem2[0].SAR + 0x10000000 + 0x01000000*repeat + 0x00100000*i + 4*j);
                destData = R_UTILS_ReadMemForDMA((void*)descadd, total_transfer_size);
                printf("After dma: Value dst desc %d at %x : 0x%x \n", i, descadd, destData);
                if(destData != (*(volatile uint32_t*)(desc_mem2[0].SAR)))
                {
                    ret = -1;
                }
            }
            
        }
    }
    
    if(ret == 0)
    {
        printf("TC5 Result: Passed\n");
    }
    else
    {
        printf("TC5 Result: Failed\n");
    }

    R_SYSDMAC_RcarDmacStop(rDmacIrqHandler_t_irq2.Unit, rDmacIrqHandler_t_irq2.SubCh);

    printf("*************************************************************\r\n");
    printf("***TC6: SYS-DMAC mem-to-mem transfer in Descriptor Read-out mode***\r\n");

    /* Convert descriptor offsets to absolute addresses */
    for (int i = 0; i < desccfg3.mDescCount; i++)
    {
        desc_mem3[i].SAR = get_osal_addr(desc3_off[i][0]);
        desc_mem3[i].DAR = get_osal_addr(desc3_off[i][1]);
    }

    /* Device Driver Part */
    R_SYSDMAC_RcarDmacCtrlInit(rDmacIrqHandler_t_irq3.Unit, DRV_RTDMAC_PRIO_FIX);

    for (int i = 0; i < desccfg3.mDescCount; i++)
    {
        for(int j = 0; j < desc_mem3[i].TCR; j++)
        {
            *(volatile uint32_t *)(desc_mem3[i].DAR + j*4) = 0x33333333;
            *(volatile uint32_t *)(desc_mem3[i].SAR + j*4) = 0x99999999;
            printf("Before dma: Value src desc %d at %x : 0x%x \n", i, (desc_mem3[i].SAR + j*4), *(volatile uint32_t *)(desc_mem3[i].SAR + j*4));
            printf("Before dma: Value dst desc %d at %x : 0x%x \n", i, (desc_mem3[i].DAR + j*4), *(volatile uint32_t *)(desc_mem3[i].DAR + j*4));
        }
    }

    ret = R_SYSDMAC_RcarCallBackSet(&rDmacIrqHandler_t_irq3, dmacUserCallback3, &usr_context3);
    if (ret)
        printf("CallbackSet Failed: ret = %d\n", ret);

    dmaStatus = R_SYSDMAC_RcarDmacExec(rDmacIrqHandler_t_irq3.Unit, rDmacIrqHandler_t_irq3.SubCh, &cfg3, &desccfg3);

    // Check DMA execution status
    if (dmaStatus != 0)
        printf("DMA execution failed with status: %d\n", dmaStatus);

    // Wait DMA to transfer data.
    if(xSemaphoreTake(xSemaphore, portMAX_DELAY) == pdTRUE)
    {

    }

    total_transfer_size = 4;
    printf("Verify destination data!\n");
    ret = 0;
    for(int i = 0; i < desccfg3.mDescCount ; i++)
    {
        for(int j = 0; j < desc_mem3[i].TCR ; j++)
        {
            destData = R_UTILS_ReadMemForDMA((void *)(desc_mem3[i].DAR + j*4), total_transfer_size);
            printf("After dma: Value dst desc %d at %x : 0x%x \n", i, (volatile uint32_t *)(desc_mem3[i].DAR + j*4), destData);
            if(destData != (*(volatile uint32_t*)(desc_mem3[i].SAR + j*4)))
            {
                ret = -1;
            }
        }
    }

    if(ret == 0)
    {
        printf("TC6 Result: Passed\n");
    }
    else
    {
        printf("TC6 Result: Failed\n");
    }

    printf("*************************************************************\r\n");
    printf("***TC7: SYS-DMAC mem-to-mem transfer in Descriptor Infinite Repeat mode***\r\n");

    /* Convert descriptor offsets to absolute addresses */
    for (int i = 0; i < desccfg4.mDescCount; i++)
    {
        desc_mem4[i].SAR = get_osal_addr(desc4_off[i][0]);
        desc_mem4[i].DAR = get_osal_addr(desc4_off[i][1]);
    }

    /* Device Driver Part */
    R_SYSDMAC_RcarDmacCtrlInit(SYS_DMAC0, DRV_RTDMAC_PRIO_FIX);

    for (int i = 0; i < desccfg4.mDescCount; i++)
    {
        for(int j = 0; j < desc_mem4[i].TCR; j++)
        {
            *(volatile uint32_t *)(desc_mem4[i].DAR + j*4) = 0x22222222;
            *(volatile uint32_t *)(desc_mem4[i].SAR + j*4) = 0x88888888;
            printf("Before dma: Value src desc %d at %x : 0x%x \n", i, (desc_mem4[i].SAR + j*4), *(volatile uint32_t *)(desc_mem4[i].SAR + j*4));
            printf("Before dma: Value dst desc %d at %x : 0x%x \n", i, (desc_mem4[i].DAR + j*4), *(volatile uint32_t *)(desc_mem4[i].DAR + j*4));
        }
    }

    dmaStatus =R_SYSDMAC_RcarDmacExec(SYS_DMAC0, DMAC_CH1, &cfg4, &desccfg4);

    // Check DMA execution status
    if (dmaStatus != 0)
        printf("DMA execution failed with status: %d\n", dmaStatus);

    vTaskDelay(1000);

    total_transfer_size = 4;
    printf("Verify destination data!\n");
    ret = 0;
    for (int repeat = 0; repeat < 10; repeat++)
    {
        for(int i = 0; i < desccfg4.mDescCount ; i++)
        {
            for(int j = 0; j < desc_mem4[i].TCR ; j++)
            {
                destData = R_UTILS_ReadMemForDMA((void *)(desc_mem4[i].DAR + j*4), total_transfer_size);
                printf("After dma: Value dst desc %d at %x : 0x%x \n", i, (volatile uint32_t *)(desc_mem4[i].DAR + j*4), destData);
                if(destData != (*(volatile uint32_t*)(desc_mem4[i].SAR + j*4)))
                {
                    ret = -1;
                }
            }
        }
    }

    if(ret == 0)
    {
        printf("TC7 Result: Passed\n");
    }
    else
    {
        printf("TC7 Result: Failed\n");
    }
    R_SYSDMAC_RcarDmacStop(SYS_DMAC0, DMAC_CH1);
    printf("*************************************************************\r\n");
    printf("<APP_END>\n");

    for( ;; )
    {
    }
}

/*-----------------------------------------------------------*/

void dmacUserCallback(void *data) {
	rDmacIrqCfg_t * instance_ctrl = (rDmacIrqCfg_t *) data;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(xSemaphore, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void dmacUserCallback1(void *data) {
    rDmacIrqCfg_t * instance_ctrl = (rDmacIrqCfg_t *) data;

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(xSemaphore, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void dmacUserCallback2(void *data) {
    rDmacIrqCfg_t * instance_ctrl = (rDmacIrqCfg_t *) data;
    static uint32_t repeat = REPEAT_NUMBER;

    if(repeat == 0)
    {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xSemaphoreGiveFromISR(xSemaphore, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }

    for(int i = 0; i < descCfg2.mDescCount; i++)
    {
        desc_mem2[i].DAR += 0x00100000;
    }
    repeat -= 1;
}

volatile int count_irq_sysdmac_read_out = 0;
void dmacUserCallback3(void *data) {
    rDmacIrqCfg_t * instance_ctrl = (rDmacIrqCfg_t *) data;
    count_irq_sysdmac_read_out++;
    if(count_irq_sysdmac_read_out == 1)
    {
        desccfg3.mDescIndex = 6;
    }
    else if (count_irq_sysdmac_read_out == 2)
    {
        desccfg3.mDescIndex = 1;
    }
    else
    {
        R_SYSDMAC_RcarDmacStop(rDmacIrqHandler_t_irq3.Unit, rDmacIrqHandler_t_irq3.SubCh);
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xSemaphoreGiveFromISR(xSemaphore, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

/*-----------------------------------------------------------*/

int printf_raw(const char *format, ...);

void vMainAssertCalled( const char *pcFileName, uint32_t ulLineNumber )
{
    /* Don't use printf as it uses FreeRTOS resources */
    printf_raw("ASSERT!  Line %d of file %s\n", ulLineNumber, pcFileName);
    taskENTER_CRITICAL();
    for( ;; );
}

void vDeleteCallingTask( void )
{
    vTaskDelete( NULL );
}
