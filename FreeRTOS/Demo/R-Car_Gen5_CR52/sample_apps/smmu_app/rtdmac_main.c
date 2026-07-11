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
#include "dmac/dmac_common.h"
#include "dmac/rtdmac_ctrl.h"
#include "dmac/sysdmac_ctrl.h"
#include "pfc/r_pfc_api.h"

#include "device_tree.h"

#include "rcar_utils.h"
#include "smmu/smmu.h"

#include "stdio.h"
#include "stdbool.h"
#include "board.h"
#define main_DMAC_TASK_PRIORITY        ( tskIDLE_PRIORITY + 1 )
#define DESTINATION_OFFSET 			   0x01000000
#define SOURCE_OFFSET_MAPPING          0x10000000
#define DESTINATION_OFFSET_MAPPING     0x11000000

void dmacUserCallback(void *data);

/*-----------------------------------------------------------*/

/*
 * Configure the hardware as necessary to run this demo.
 */
static void prvSetupHardware( void );

static void prvDMACTask( void *pvParameters );

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

rDmacIrqCfg_t rDmacIrqHandler_t_irq =
{
	.Unit = RT_DMAC0,
	.SubCh = DMAC_CH1,
	.irq_channel = INTID_RTDMA0_CH1,
};

bool isr_flag = false;

/*-----------------------------------------------------------*/

int main( void )
{
        /* Configure the hardware ready to run the demo. */
        prvSetupHardware();


        xTaskCreate( prvDMACTask, "DMACTask", configMINIMAL_STACK_SIZE * 2, NULL, main_DMAC_TASK_PRIORITY, NULL );
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

static void prvDMACTask( void *pvParameters )
{
	/* Remove compiler warning about unused parameter. */
	( void ) pvParameters;
	int ret;
    bool is_secure = false;

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

    
    st_smmu_streamid_instance_ctrl_t smmu_ctrl = {
    #if (BOARD == MDP_AIACC_HIL || BOARD == MDP_AIACC_RFS2)
    .stream_id = 0x00E01, /*AIACC RTDMAC 0 CHANNEL 1*/
    #else
    .stream_id = 0xE0001, /*X5H RTDMAC 0 CHANNEL 1*/
    #endif    
    .smmu_domain = SMMU_RT,
    .is_secure = is_secure,
    };

    printf("**********************************************\r\n");

    printf("* Test case 1: Initializes and Enable SMMU. *\r\n");
    ret = R_SMMU_Init(smmu_ctrl.smmu_domain, smmu_ctrl.is_secure);
    if (ret == 0) {
        printf("Result: Passed\r\n");
    } else {
        printf("Result: Failed\r\n");
    }
    printf("**********************************************\r\n");

    printf("* Test case 2: Invalidate TLB *\r\n");
    ret = R_SMMU_InvalidateTLB(smmu_ctrl.smmu_domain, smmu_ctrl.is_secure);
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

    R_SMMU_Map(&smmu_ctrl, cfg0.mSrcAddr, cfg0.mSrcAddr + SOURCE_OFFSET_MAPPING, 0x5006000, ATTR_DEVICE_NGNRNE_EL1_RW_EL0_RW);

#if (BOARD == X5H_RFS2)
    smmu_ctrl.stream_id = 0xC00;
    R_SMMU_Attach(&smmu_ctrl);
    R_SMMU_Map(&smmu_ctrl, 0x10000000, 0x10000000, 0x10000000, ATTR_DEVICE_NGNRNE_EL1_RW_EL0_RW);
    R_SMMU_Map(&smmu_ctrl, cfg0.mSrcAddr, cfg0.mSrcAddr, DESTINATION_OFFSET + 0x1000, ATTR_DEVICE_NGNRNE_EL1_RW_EL0_RW);
    R_SMMU_Map(&smmu_ctrl, cfg0.mSrcAddr + SOURCE_OFFSET_MAPPING, cfg0.mSrcAddr + SOURCE_OFFSET_MAPPING,DESTINATION_OFFSET + 0x1000, ATTR_DEVICE_NGNRNE_EL1_RW_EL0_RW);
    R_SMMU_Map(&smmu_ctrl, 0xC0000000, 0xC0000000, 0x40000000, ATTR_DEVICE_NGNRNE_EL1_RW_EL0_RW);
#endif

    printf("**********************************************\r\n");

    printf("* Test case 4: Enable SMMU *\r\n");
    ret = R_SMMU_Enable(smmu_ctrl.smmu_domain, smmu_ctrl.is_secure);
    if (ret == 0) {
       printf("Result: Passed\r\n");
    } else {
        printf("Result: Failed \r\n");
    }
    printf("**********************************************\r\n");

	/* Device Driver Part */
    R_RTDMAC_RcarDmacCtrlInit(RT_DMAC0, DRV_RTDMAC_PRIO_FIX);

    volatile uint32_t *pa_src_ptr = (volatile uint32_t *)(cfg0.mSrcAddr + SOURCE_OFFSET_MAPPING);
    volatile uint32_t *pa_dst_ptr = (volatile uint32_t *)(cfg0.mSrcAddr + DESTINATION_OFFSET_MAPPING);

    *(volatile uint32_t *)pa_src_ptr = 0x7012;
    *(volatile uint32_t *)pa_dst_ptr = 0x234; // Value goes to cache; DMA may miss it if dont invalidate cache

    Context_t usr_context = 
    {
        .ctx = &rDmacIrqHandler_t_irq,
    };

    printf("* Test case 5: Test transaction data *\r\n");
    printf("Before DMA: pa dst address: 0x%lx, dst data: 0x%lx\n",pa_dst_ptr, *(volatile uint32_t *)pa_dst_ptr);
    ret = R_RTDMAC_RcarCallBackSet(&rDmacIrqHandler_t_irq, dmacUserCallback, &usr_context);

    int dmaStatus =R_RTDMAC_RcarDmacExec(RT_DMAC0, DMAC_CH1, &cfg0, 0);

    while(!isr_flag) {
        __asm__ volatile("nop");
    }

    // Verify destination data
    uint32_t total_transfer_size = 4;
	uint32_t destData = R_UTILS_ReadMemForDMA((void*)pa_dst_ptr, total_transfer_size);

    printf("After DMA: pa dst address: 0x%lx, dst data: 0x%lx\n",pa_dst_ptr, destData );
    printf("Source Info: pa src address: 0x%lx, src data: 0x%lx\n",pa_src_ptr, *(volatile uint32_t *)pa_src_ptr );
    if (destData == (*(volatile uint32_t *)pa_src_ptr)) {
        printf("Result: Passed\n");
    } else {
        printf("Result: Failed\n");
        if ((*(volatile uint32_t *)cfg0.mSrcAddr == *(volatile uint32_t *)cfg0.mDestAddr) && *(volatile uint32_t *)cfg0.mSrcAddr != 0) {
            printf("After DMA: va src address: 0x%lx, src data: 0x%lx\n", cfg0.mSrcAddr, *(volatile uint32_t *)cfg0.mSrcAddr );
            printf("After DMA: va dst address: 0x%lx, dst data: 0x%lx\n", cfg0.mDestAddr, *(volatile uint32_t *)cfg0.mDestAddr);
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

void dmacUserCallback(void *data) {
	rDmacIrqCfg_t * instance_ctrl = (rDmacIrqCfg_t *) data;
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
