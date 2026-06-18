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

#include <stdio.h>
#include <errno.h>
#include "pfc/r_pfc_api.h"
#include "virtio-iommu-frontend/r_virtio_iommu_frontend.h"
#include "virtio-iommu-backend/r_virtio_iommu_backend.h"
#include "serial/r_serial.h"
#include "rcar_utils.h"
#include "dmac/dmac_common.h"
#include "dmac/rtdmac_ctrl.h"
#include "dmac/sysdmac_ctrl.h"

#define main_VIOMMUFE_TASK_PRIORITY        ( tskIDLE_PRIORITY + 1 )

/*-----------------------------------------------------------*/

/* smmu app config */
#define main_SMMU_TASK_PRIORITY        ( tskIDLE_PRIORITY + 1 )
#define SOURCE_OFFSET                  0x1000000
#define DESTINATION_OFFSET             0x01000000
#define SOURCE_OFFSET_MAPPING          0x10000000
#define DESTINATION_OFFSET_MAPPING     0x11000000

void dmacUserCallback(void *data);

rDmacCfg_t cfg =
{
    //Fill in the configuration details
    .mSrcAddr = 0x0,
    .mDestAddr = 0x0,
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
    .SubCh = DMAC_CH2,
    .irq_channel = INTID_SYSDMA3_CH2
};

bool isr_flag = false;

void dmacUserCallback(void *data)
{
    rDmacIrqCfg_t *instance_ctrl = (rDmacIrqCfg_t *)data;
    isr_flag = true;
}

/*
 * Configure the hardware as necessary to run this demo.
 */
static void prvSetupHardware( void );

static void prvVIOMMUBEMgrTask( void *pvParameters );
static void prvVIOMMUBETask0( void *pvParameters );
static void prvVIOMMUBETask1( void *pvParameters );
static void prvVIOMMUFETask( void *pvParameters );

TaskHandle_t xTaskHandle[2] = {NULL};
/*-----------------------------------------------------------*/

int main( void )
{
    /* Configure the hardware ready to run the demo. */
    prvSetupHardware();
    xTaskCreate( prvVIOMMUFETask, "VIRTIO_MMU_FE", configMINIMAL_STACK_SIZE*10, NULL, main_VIOMMUFE_TASK_PRIORITY + 2, NULL );
    xTaskCreate( prvVIOMMUBEMgrTask, "prvVIOMMUBEMgrTask", configMINIMAL_STACK_SIZE*10, NULL, main_VIOMMUFE_TASK_PRIORITY + 2, NULL );
    xTaskCreate( prvVIOMMUBETask0, "VIRTIO_MMU_BE0", configMINIMAL_STACK_SIZE*10, NULL, main_VIOMMUFE_TASK_PRIORITY , &xTaskHandle[0] );
    xTaskCreate( prvVIOMMUBETask1, "VIRTIO_MMU_BE1", configMINIMAL_STACK_SIZE*10, NULL, main_VIOMMUFE_TASK_PRIORITY , &xTaskHandle[1] );
    
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

static void prvVIOMMUBEMgrTask( void *pvParameters )
{
    int num_task = 2;
    for (int i = 0; ; i++)
    {
        for (int j = 0; j < num_task; j++)
        {
            vTaskSuspend(xTaskHandle[j]);
        }
        vTaskResume(xTaskHandle[i%num_task]);
        vTaskDelay(100);
    }
    
}

void prvVIOMMUBETask0( void *pvParameters )
{
    /* Remove compiler warning about unused parameter. */
    ( void ) pvParameters;
    // vTaskPrioritySet(NULL, main_VIOMMUFE_TASK_PRIORITY);
    uint32_t cpu_id = R_UTILS_GetCpuID();
    int ret;
    printf("VIRTIO IOMMU Backend:  Starting Virtio 0 sample\r\n");
    virtio_iommu_instance_ctrl_t *virtio_iommu_inst;
    e_mfis_channel_t mfis_ch;
    if (cpu_id == 0)
    {
        mfis_ch = MFIS_CR_TO_CA_CH0;
    }
    else if(cpu_id == 1)
    {
        mfis_ch = MFIS_CR_TO_CA_CH1;
    }
    else
    {
        printf("VIRTIO IOMMU Frontend:  MFIS Channel not support\r\n");
    }

    printf("VIRTIO IOMMU Backend:  TC1: Virtio IOMMU Create. Waiting for connection ...\r\n");
    virtio_iommu_inst = R_VIRTIO_IOMMU_Backend_Init(mfis_ch);
    if(virtio_iommu_inst == NULL)
    {
        printf("VIRTIO IOMMU Backend:  Result: Failed\r\n");
    }
    else
    {
        printf("VIRTIO IOMMU Backend:  Result: Passed\r\n");
    };

    for( ;; )
    {
        vTaskDelay(1);
    }
}

void prvVIOMMUBETask1( void *pvParameters )
{
    /* Remove compiler warning about unused parameter. */
    ( void ) pvParameters;
    // vTaskPrioritySet(NULL, main_VIOMMUFE_TASK_PRIORITY);
    uint32_t cpu_id = R_UTILS_GetCpuID();
    int ret;
    printf("VIRTIO IOMMU Backend:  Starting Virtio 1 sample\r\n");
    virtio_iommu_instance_ctrl_t *virtio_iommu_inst;
    e_mfis_channel_t mfis_ch;
    if (cpu_id == 0)
    {
        mfis_ch = MFIS_CR_TO_CA_CH1;
    }
    else if(cpu_id == 1)
    {
        mfis_ch = MFIS_CR_TO_CA_CH0;
    }
    else
    {
        printf("VIRTIO IOMMU Frontend:  MFIS Channel not support\r\n");
    }

    printf("VIRTIO IOMMU Backend:  TC1: Virtio IOMMU Create. Waiting for connection ...\r\n");
    virtio_iommu_inst = R_VIRTIO_IOMMU_Backend_Init(mfis_ch);
    if(virtio_iommu_inst == NULL)
    {
        printf("VIRTIO IOMMU Backend:  Result: Failed\r\n");
    }
    else
    {
        printf("VIRTIO IOMMU Backend:  Result: Passed\r\n");
    };

    for( ;; )
    {
        vTaskDelay(1);
    }
}

static void prvVIOMMUFETask( void *pvParameters )
{
    uint32_t cpu_id = R_UTILS_GetCpuID();
    st_memory_t region = R_UTILS_GetMemoryRegionInfo(OSAL, 0);
    cfg.mSrcAddr = region.base_address + SOURCE_OFFSET;
    cfg.mDestAddr = cfg.mSrcAddr + DESTINATION_OFFSET;
    if (cfg.mDestAddr > region.base_address + region.size)
    {
        printf("VIRTIO IOMMU Frontend:  Failed: Destination address 0x%08X exceeds memory region (end at 0x%08X)\n", cfg.mDestAddr, region.base_address + region.size);
        for( ;; )
        {
            vTaskDelay(1000);
        }
    }
    /* Remove compiler warning about unused parameter. */
    ( void ) pvParameters;
    int ret = 0;
    st_smmu_streamid_instance_ctrl_t smmu_ctrl = {
        .stream_id = 0x50002,
        .smmu_domain = SMMU_PERW,
        .is_secure = false,
    };

    virtio_iommu_frontend_instance_ctrl_t *virtio_iommu_inst;
    e_mfis_channel_t mfis_ch;
    if (cpu_id == 0)
    {
        mfis_ch = MFIS_CR_TO_CA_CH1;
    }
    else if(cpu_id == 1)
    {
        mfis_ch = MFIS_CR_TO_CA_CH0;
    }
    else
    {
        printf("VIRTIO IOMMU Frontend:  MFIS Channel not support\r\n");
    }

    vTaskDelay(3000);
    printf("VIRTIO IOMMU Frontend:  * Test multi Frontend \n");
    printf("VIRTIO IOMMU Frontend:  * Test case 1: Test R_VIRTIO_IOMMU_Init\n");
    virtio_iommu_inst = R_VIRTIO_IOMMU_Init(mfis_ch);
    smmu_ctrl.p_context = virtio_iommu_inst;
    vTaskDelay(1000);
    if(virtio_iommu_inst == NULL)
    {
        printf("VIRTIO IOMMU Frontend:  Result: Failed\r\n");
    }
    else
    {
        printf("VIRTIO IOMMU Frontend:  Result: Passed\r\n");
    };
    
    printf("VIRTIO IOMMU Frontend:  * Test case 2: Test R_VIRTIO_IOMMU_Attach\n");
    ret = R_VIRTIO_IOMMU_Attach(&smmu_ctrl);
    if (ret == 0) {
        printf("VIRTIO IOMMU Frontend:  Result: Passed\r\n");
    } else {
        printf("VIRTIO IOMMU Frontend:  Result: Failed\r\n");
    }

    printf("VIRTIO IOMMU Frontend:  * Test case 3: Test R_VIRTIO_IOMMU_Map\n");
    ret = R_VIRTIO_IOMMU_Map(&smmu_ctrl, cfg.mSrcAddr, cfg.mSrcAddr + SOURCE_OFFSET_MAPPING, 0x5000000, ATTR_DEVICE_NGNRNE_EL1_RW_EL0_RW);
    if (ret == 0) {
        printf("VIRTIO IOMMU Frontend:  Result: Passed\r\n");
    } else {
        printf("VIRTIO IOMMU Frontend:  Result: Failed\r\n");
    }
    /*----------------------------------------------------------*/

    /*SYSDMA setup*/
    R_SYSDMAC_RcarDmacCtrlInit(rDmacIrqHandler_t_irq.Unit, DRV_RTDMAC_PRIO_FIX);
    volatile uint32_t *pa_src_ptr = (volatile uint32_t *)(cfg.mSrcAddr + SOURCE_OFFSET_MAPPING);
    volatile uint32_t *pa_dst_ptr = (volatile uint32_t *)(cfg.mSrcAddr + DESTINATION_OFFSET_MAPPING);
    *(volatile uint32_t *)pa_src_ptr = 0x7012;
    *(volatile uint32_t *)cfg.mSrcAddr = 0x123;
    *(volatile uint32_t *)pa_dst_ptr = 0x123; // Value goes to cache; DMA may miss it if dont invalidate cache
    Context_t usr_context = 
    {
        .ctx = &rDmacIrqHandler_t_irq,
    };

    printf("VIRTIO IOMMU Frontend:  * Test case 4: Test transaction data *\r\n");
    printf("VIRTIO IOMMU Frontend:  Before DMA: pa dst address: 0x%lx, dst data: 0x%lx\n",pa_dst_ptr, *(volatile uint32_t *)pa_dst_ptr);
    ret = R_SYSDMAC_RcarCallBackSet(&rDmacIrqHandler_t_irq, dmacUserCallback, &usr_context);

    int dmaStatus = R_SYSDMAC_RcarDmacExec(rDmacIrqHandler_t_irq.Unit, rDmacIrqHandler_t_irq.SubCh, &cfg, 0);

    while(!isr_flag) {
        __asm__ volatile("nop");
    }
    isr_flag = false;

    // Verify destination data
    uint32_t total_transfer_size = 4;
    uint32_t destData = R_UTILS_ReadMemForDMA((void*)pa_dst_ptr, total_transfer_size);

    printf("VIRTIO IOMMU Frontend:  After DMA: pa dst address: 0x%lx, dst data: 0x%lx\n",pa_dst_ptr, destData );
    printf("VIRTIO IOMMU Frontend:  Source Info: pa src address: 0x%lx, src data: 0x%lx\n",pa_src_ptr, *(volatile uint32_t *)pa_src_ptr );
    if (destData == (*(volatile uint32_t *)pa_src_ptr)) {
        printf("VIRTIO IOMMU Frontend:  Result: Passed\n");
    } else {
        printf("VIRTIO IOMMU Frontend:  Result: Failed\n");
        if ((*(volatile uint32_t *)cfg.mSrcAddr == *(volatile uint32_t *)cfg.mDestAddr) && *(volatile uint32_t *)cfg.mSrcAddr != 0) {
            printf("VIRTIO IOMMU Frontend:  After DMA: va src address: 0x%lx, src data: 0x%lx\n", cfg.mSrcAddr, *(volatile uint32_t *)cfg.mSrcAddr );
            printf("VIRTIO IOMMU Frontend:  After DMA: va dst address: 0x%lx, dst data: 0x%lx\n", cfg.mDestAddr, *(volatile uint32_t *)cfg.mDestAddr);
            printf("VIRTIO IOMMU Frontend:  DMAC worked without VIRTIO IOMMU.\n");
        }
    }
    
    printf("VIRTIO IOMMU Frontend:  * Test case 5: Test R_VIRTIO_IOMMU_UnMap\n");
    ret = R_VIRTIO_IOMMU_UnMap(&smmu_ctrl, cfg.mSrcAddr, cfg.mSrcAddr + SOURCE_OFFSET_MAPPING, 0x5000000);
    if (ret == 0) {
        R_SYSDMAC_RcarDmacStop(rDmacIrqHandler_t_irq.Unit, rDmacIrqHandler_t_irq.SubCh);

        *(volatile uint32_t *)pa_src_ptr = 0x111;
        *(volatile uint32_t *)cfg.mSrcAddr = 0x222;
        *(volatile uint32_t *)pa_dst_ptr = 0x555; // Value goes to cache; DMA may miss it if dont invalidate cache
        printf("VIRTIO IOMMU Frontend:  Before DMA: pa dst address: 0x%lx, dst data: 0x%lx\n",pa_dst_ptr, *(volatile uint32_t *)pa_dst_ptr);
        dmaStatus = R_SYSDMAC_RcarDmacExec(rDmacIrqHandler_t_irq.Unit, rDmacIrqHandler_t_irq.SubCh, &cfg, 0);

        while(!isr_flag) {
            __asm__ volatile("nop");
        }
        isr_flag = false;

        // Verify destination data
        total_transfer_size = 4;
        destData = R_UTILS_ReadMemForDMA((void*)pa_dst_ptr, total_transfer_size);

        printf("VIRTIO IOMMU Frontend:  After DMA: pa dst address: 0x%lx, dst data: 0x%lx\n",pa_dst_ptr, destData );
        printf("VIRTIO IOMMU Frontend:  Source Info: pa src address: 0x%lx, src data: 0x%lx\n",pa_src_ptr, *(volatile uint32_t *)pa_src_ptr );
        if (destData == (*(volatile uint32_t *)pa_src_ptr)) {
            printf("VIRTIO IOMMU Frontend:  Result: Failed\n");
        } else {
            if ((*(volatile uint32_t *)cfg.mSrcAddr == *(volatile uint32_t *)cfg.mDestAddr) && *(volatile uint32_t *)cfg.mSrcAddr != 0) {
                printf("VIRTIO IOMMU Frontend:  After DMA: va src address: 0x%lx, src data: 0x%lx\n", cfg.mSrcAddr, *(volatile uint32_t *)cfg.mSrcAddr );
                printf("VIRTIO IOMMU Frontend:  After DMA: va dst address: 0x%lx, dst data: 0x%lx\n", cfg.mDestAddr, *(volatile uint32_t *)cfg.mDestAddr);
                printf("VIRTIO IOMMU Frontend:  DMAC worked without SMMU.\n");
            }
            printf("VIRTIO IOMMU Frontend:  Result: Passed\n");
        }
    } else {
        printf("VIRTIO IOMMU Frontend:  Result: Failed\r\n");
    }
    
    printf("VIRTIO IOMMU Frontend:  * Test case 6: R_SMMU_Map(Re-map after unmap for verification). *\r\n");
    
    ret = R_VIRTIO_IOMMU_Map(&smmu_ctrl, cfg.mSrcAddr, cfg.mSrcAddr + SOURCE_OFFSET_MAPPING, 0x5000000, ATTR_DEVICE_NGNRNE_EL1_RW_EL0_RW);
    if (ret == 0) {
        R_SYSDMAC_RcarDmacStop(rDmacIrqHandler_t_irq.Unit, rDmacIrqHandler_t_irq.SubCh);

        *(volatile uint32_t *)pa_src_ptr = 0x123;
        *(volatile uint32_t *)cfg.mSrcAddr = 0x456;
        *(volatile uint32_t *)pa_dst_ptr = 0x777; // Value goes to cache; DMA may miss it if dont invalidate cache
        printf("VIRTIO IOMMU Frontend:  Before DMA: pa dst address: 0x%lx, dst data: 0x%lx\n",pa_dst_ptr, *(volatile uint32_t *)pa_dst_ptr);
        dmaStatus = R_SYSDMAC_RcarDmacExec(rDmacIrqHandler_t_irq.Unit, rDmacIrqHandler_t_irq.SubCh, &cfg, 0);

        while(!isr_flag) {
            __asm__ volatile("nop");
        }
        isr_flag = false;

        // Verify destination data
        total_transfer_size = 4;
        destData = R_UTILS_ReadMemForDMA((void*)pa_dst_ptr, total_transfer_size);

        printf("VIRTIO IOMMU Frontend:  After DMA: pa dst address: 0x%lx, dst data: 0x%lx\n",pa_dst_ptr, destData );
        printf("VIRTIO IOMMU Frontend:  Source Info: pa src address: 0x%lx, src data: 0x%lx\n",pa_src_ptr, *(volatile uint32_t *)pa_src_ptr );
        if (destData == (*(volatile uint32_t *)pa_src_ptr)) {
            printf("VIRTIO IOMMU Frontend:  Result: Passed\n");
        } else {
            if ((*(volatile uint32_t *)cfg.mSrcAddr == *(volatile uint32_t *)cfg.mDestAddr) && *(volatile uint32_t *)cfg.mSrcAddr != 0) {
                printf("VIRTIO IOMMU Frontend:  After DMA: va src address: 0x%lx, src data: 0x%lx\n", cfg.mSrcAddr, *(volatile uint32_t *)cfg.mSrcAddr );
                printf("VIRTIO IOMMU Frontend:  After DMA: va dst address: 0x%lx, dst data: 0x%lx\n", cfg.mDestAddr, *(volatile uint32_t *)cfg.mDestAddr);
                printf("VIRTIO IOMMU Frontend:  DMAC worked without SMMU.\n");
            }
            printf("VIRTIO IOMMU Frontend:  Result: Failed\n");
        }
    } else {
        printf("VIRTIO IOMMU Frontend:  Result: Failed\r\n");
    }

    
    printf("VIRTIO IOMMU Frontend:  **********************************************\r\n");
    printf("<APP_END>\n");

    for( ;; )
    {
        vTaskDelay(1);
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

