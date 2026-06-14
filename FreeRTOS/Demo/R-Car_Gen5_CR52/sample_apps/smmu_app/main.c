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
#include "FreeRTOS.h"
#include "task.h"
#include "interrupts.h"

#include "interrupts.h"
#include <stdio.h>
#include <string.h>
#include "smmu/smmu.h"
#include "dmac/dmac_common.h"
#include "dmac/rtdmac_ctrl.h"
#include "dmac/sysdmac_ctrl.h"
#include "pfc/r_pfc_api.h"
#include "device_tree.h"
#include "rcar_utils.h"

#define main_SMMU_TASK_PRIORITY        ( tskIDLE_PRIORITY + 1 )
#define DESTINATION_OFFSET 			   0x01000000
#define SOURCE_OFFSET_MAPPING          0x10000000
#define DESTINATION_OFFSET_MAPPING     0x11000000
/*-----------------------------------------------------------*/

/*
 * Configure the hardware as necessary to run this demo.
 */
static void prvSetupHardware( void );

static void prvSMMUTask( void *pvParameters );
void dmacUserCallback(void *data);
/* Define configure DMA Controller */
rDmacCfg_t cfg =
{
	//Fill in the configuration details
	// .mSrcAddr = 0x189E7000,
	// .mDestAddr = 0x189E7100,
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

rDmacIrqCfg_t rDmacIrqHandler_t_irq =
{
	.Unit = SYS_DMAC3,
	.SubCh = DMAC_CH1,
	.irq_channel = INTID_SYSDMA3_CH1
};

bool isr_flag = false;
/*-----------------------------------------------------------*/

int main( void )
{
	/* Configure the hardware ready to run the demo. */
    
    prvSetupHardware();

    xTaskCreate(prvSMMUTask, "SMMUTask", configMINIMAL_STACK_SIZE * 2, NULL, main_SMMU_TASK_PRIORITY, NULL );
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

static void prvSMMUTask( void *pvParameters )
{
    /* Remove compiler warning about unused parameter. */
    (void)pvParameters;
    int ret,i;
    bool is_secure = false;

    /* Get memory region first */
	st_memory_t region = R_UTILS_GetMemoryRegionInfo(OSAL, 0);
	cfg.mSrcAddr = region.base_address;
	cfg.mDestAddr = region.base_address + DESTINATION_OFFSET;
	if (cfg.mDestAddr > region.base_address + region.size) 
	{
		printf("Failed: Destination address 0x%08X exceeds memory region (end at 0x%08X)\n", cfg.mDestAddr, region.base_address + region.size);
		for( ;; )
		{
			vTaskDelay(3000);
		}
	}

    uint64_t data_size = 0x5000000;

    st_smmu_streamid_instance_ctrl_t smmu_ctrl = {
        .stream_id = 0x50001,
        .smmu_domain = SMMU_PERW,
	.is_secure = is_secure,
    };

    printf("**********************************************\r\n");

    printf("* Test case 1: Initializes and Enable SMMU. *\r\n");
    ret = R_SMMU_Init(SMMU_PERW, is_secure);
    if (ret == 0) {
        printf("Result: Passed\r\n");
    } else {
        printf("Result: Failed\r\n");
    }
    printf("**********************************************\r\n");

    printf("* Test case 2: Invalidate TLB *\r\n");
    ret = R_SMMU_InvalidateTLB(SMMU_PERW, is_secure);
    if (ret == 0) {
        printf("Result: Passed\r\n");
    } else {
        printf("Result: Failed\r\n");
    }
    printf("**********************************************\r\n");
    
    printf("* Test case 3: Attach stream id. *\r\n");

    ret = R_SMMU_Attach(&smmu_ctrl);
    if (ret == 0) {
        printf("Result: Passed\r\n");
    } else {
        printf("Result: Failed\r\n");
    }

    R_SMMU_Map(&smmu_ctrl, cfg.mSrcAddr, cfg.mSrcAddr + SOURCE_OFFSET_MAPPING, data_size, ATTR_DEVICE_NGNRNE_EL1_RW_EL0_RW);

    printf("**********************************************\r\n");

    printf("* Test case 4: Enable SMMU *\r\n");
    ret = R_SMMU_Enable(SMMU_PERW, is_secure);
    if (ret == 0) {
       printf("Result: Passed\r\n");
    } else {
        printf("Result: Failed \r\n");
    }
    printf("**********************************************\r\n");

    /* Device Driver Part */
    R_SYSDMAC_RcarDmacCtrlInit(SYS_DMAC3, DRV_RTDMAC_PRIO_FIX);

    volatile uint32_t *pa_src_ptr = (volatile uint32_t *)(cfg.mSrcAddr + SOURCE_OFFSET_MAPPING);
    volatile uint32_t *pa_dst_ptr = (volatile uint32_t *)(cfg.mSrcAddr + DESTINATION_OFFSET_MAPPING);

    *(volatile uint32_t *)pa_src_ptr = 0x7012;
    *(volatile uint32_t *)cfg.mSrcAddr = 0x123;
    *(volatile uint32_t *)pa_dst_ptr = 0x123; // Value goes to cache; DMA may miss it if dont invalidate cache

    Context_t usr_context = 
    {
        .ctx = &rDmacIrqHandler_t_irq,
    };

    printf("* Test case 5: Test transaction data *\r\n");
    printf("Before DMA: pa dst address: 0x%lx, dst data: 0x%lx\n",pa_dst_ptr, *(volatile uint32_t *)pa_dst_ptr);
    ret = R_SYSDMAC_RcarCallBackSet(&rDmacIrqHandler_t_irq, dmacUserCallback, &usr_context);

    int dmaStatus = R_SYSDMAC_RcarDmacExec(SYS_DMAC3, DMAC_CH1, &cfg, 0);

    while(!isr_flag) {
        __asm__ volatile("nop");
    }
    isr_flag = false;

    // Verify destination data
    uint32_t total_transfer_size = 4;
	uint32_t destData = R_UTILS_ReadMemForDMA((void*)pa_dst_ptr, total_transfer_size);

    printf("After DMA: pa dst address: 0x%lx, dst data: 0x%lx\n",pa_dst_ptr, destData );
    printf("Source Info: pa src address: 0x%lx, src data: 0x%lx\n",pa_src_ptr, *(volatile uint32_t *)pa_src_ptr );
    if (destData == (*(volatile uint32_t *)pa_src_ptr)) {
        printf("Result: Passed\n");
    } else {
        printf("Result: Failed\n");
        if ((*(volatile uint32_t *)cfg.mSrcAddr == *(volatile uint32_t *)cfg.mDestAddr) && *(volatile uint32_t *)cfg.mSrcAddr != 0) {
            printf("After DMA: va src address: 0x%lx, src data: 0x%lx\n", cfg.mSrcAddr, *(volatile uint32_t *)cfg.mSrcAddr );
            printf("After DMA: va dst address: 0x%lx, dst data: 0x%lx\n", cfg.mDestAddr, *(volatile uint32_t *)cfg.mDestAddr);
            printf ("DMAC worked without SMMU.\n");
        }
    }

    printf("**********************************************\r\n");

    printf("* Test case 6: R_SMMU_Unmap. *\r\n");
    
    R_SMMU_Unmap(&smmu_ctrl, cfg.mSrcAddr, cfg.mSrcAddr + SOURCE_OFFSET_MAPPING, data_size);

    R_SYSDMAC_RcarDmacStop(SYS_DMAC3, DMAC_CH1);

    *(volatile uint32_t *)pa_src_ptr = 0x111;
    *(volatile uint32_t *)cfg.mSrcAddr = 0x222;
    *(volatile uint32_t *)pa_dst_ptr = 0x555; // Value goes to cache; DMA may miss it if dont invalidate cache
    printf("Before DMA: pa dst address: 0x%lx, dst data: 0x%lx\n",pa_dst_ptr, *(volatile uint32_t *)pa_dst_ptr);
    dmaStatus = R_SYSDMAC_RcarDmacExec(SYS_DMAC3, DMAC_CH1, &cfg, 0);

    while(!isr_flag) {
        __asm__ volatile("nop");
    }
    isr_flag = false;

    // Verify destination data
    total_transfer_size = 4;
	destData = R_UTILS_ReadMemForDMA((void*)pa_dst_ptr, total_transfer_size);

    printf("After DMA: pa dst address: 0x%lx, dst data: 0x%lx\n",pa_dst_ptr, destData );
    printf("Source Info: pa src address: 0x%lx, src data: 0x%lx\n",pa_src_ptr, *(volatile uint32_t *)pa_src_ptr );
    if (destData == (*(volatile uint32_t *)pa_src_ptr)) {
        printf("Result: Failed\n");
    } else {
        if ((*(volatile uint32_t *)cfg.mSrcAddr == *(volatile uint32_t *)cfg.mDestAddr) && *(volatile uint32_t *)cfg.mSrcAddr != 0) {
            printf("After DMA: va src address: 0x%lx, src data: 0x%lx\n", cfg.mSrcAddr, *(volatile uint32_t *)cfg.mSrcAddr );
            printf("After DMA: va dst address: 0x%lx, dst data: 0x%lx\n", cfg.mDestAddr, *(volatile uint32_t *)cfg.mDestAddr);
            printf("Result: Failed\n");
        }
        else
        {
            printf("Result: Passed\n");
        }
    }
    printf("**********************************************\r\n");
    printf("* Test case 7: R_SMMU_Map(Re-map after unmap for verification). *\r\n");
    
    R_SMMU_Map(&smmu_ctrl, cfg.mSrcAddr, cfg.mSrcAddr + SOURCE_OFFSET_MAPPING, data_size, ATTR_DEVICE_NGNRNE_EL1_RW_EL0_RW);
    R_SYSDMAC_RcarDmacStop(SYS_DMAC3, DMAC_CH1);

    *(volatile uint32_t *)pa_src_ptr = 0x123;
    *(volatile uint32_t *)cfg.mSrcAddr = 0x456;
    *(volatile uint32_t *)pa_dst_ptr = 0x777; // Value goes to cache; DMA may miss it if dont invalidate cache
    printf("Before DMA: pa dst address: 0x%lx, dst data: 0x%lx\n",pa_dst_ptr, *(volatile uint32_t *)pa_dst_ptr);
    dmaStatus = R_SYSDMAC_RcarDmacExec(SYS_DMAC3, DMAC_CH1, &cfg, 0);

    while(!isr_flag) {
        __asm__ volatile("nop");
    }
    isr_flag = false;

    // Verify destination data
    total_transfer_size = 4;
	destData = R_UTILS_ReadMemForDMA((void*)pa_dst_ptr, total_transfer_size);

    printf("After DMA: pa dst address: 0x%lx, dst data: 0x%lx\n",pa_dst_ptr, destData );
    printf("Source Info: pa src address: 0x%lx, src data: 0x%lx\n",pa_src_ptr, *(volatile uint32_t *)pa_src_ptr );
    if (destData == (*(volatile uint32_t *)pa_src_ptr)) {
        printf("Result: Passed\n");
    } else {
        printf("Result: Failed\n");
        if ((*(volatile uint32_t *)cfg.mSrcAddr == *(volatile uint32_t *)cfg.mDestAddr) && *(volatile uint32_t *)cfg.mSrcAddr != 0) {
            printf("After DMA: va src address: 0x%lx, src data: 0x%lx\n", cfg.mSrcAddr, *(volatile uint32_t *)cfg.mSrcAddr );
            printf("After DMA: va dst address: 0x%lx, dst data: 0x%lx\n", cfg.mDestAddr, *(volatile uint32_t *)cfg.mDestAddr);
            printf ("DMAC worked without SMMU.\n");
        }
    }

    printf("**********************************************\r\n");
    printf("<APP_END>\n");

    for (;;)
    {
    }
}

/*-----------------------------------------------------------*/
void dmacUserCallback(void *data)
{
    rDmacIrqCfg_t *instance_ctrl = (rDmacIrqCfg_t *)data;
    isr_flag = true;
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
