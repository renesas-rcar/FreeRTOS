/*
 * FreeRTOS Kernel V11.1.0
 * Copyright (C) 2021 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 * Copyright (c) 2026 Renesas Electronics Corporation
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
#include "stdio.h"

#include "interrupts.h"
#include "../drivers/timer/arm_generic_timer.h"
#include "pfc/r_pfc_api.h"
#include "device_tree_x5h.h"
#include "gic.h"
#include "cmsis_cp15.h"
#include "dmac/dmac_common.h"
#include "dmac/rtdmac_ctrl.h"
#include "rcar_utils.h"
/*-----------------------------------------------------------*/
#define INTERRUPT_FLAG_UNSET        0
#define INTERRUPT_FLAG_SET          1
#define COUNT_PER_TICK (GENERIC_TIMER_CLK / configTICK_RATE_HZ)
#define DMA_WAIT_TIMEOUT_MS   (1000U)
/*
 * Configure the hardware as necessary to run this demo.
 */
static void prvSetupHardware(void);
int printf_raw(const char *format, ...);
void SPI_handler(void *data);

/*-----------------------------------------------------------*/

static volatile int g_spi_irq_flag = INTERRUPT_FLAG_UNSET;
/*------------------------- Configure mem-to-mem with Normal mode ----------------------------------*/
/* Define configure DMA Controller */
rDmacCfg_t cfg0 =
{
    // Fill in the configuration details
    .mSrcAddr = 0,
    .mDestAddr = 0,
    .mTransferCount = 1,
    .mDMAMode = DRV_DMAC_DMA_NO_DESCRIPTOR,       // Assuming DRV_DMAC_DMA_NO_DESCRIPTOR is defined
    .mSrcAddrMode = DRV_RTDMAC_ADDR_FIXED,        // Assuming ADDR_MODE_FIXED is defined
    .mDestAddrMode = DRV_RTDMAC_ADDR_FIXED,       // Assuming ADDR_MODE_FIXED is defined
    .mTransferUnit = DRV_RTDMAC_TRANS_UNIT_4BYTE, // Assuming DRV_RTDMAC_TRANS_UNIT_4BYTE is defined
    .mResource = DRV_RTDMAC_MEMORY,               // Assuming DRV_RTDMAC_MEMORY is defined
    .mLowSpeed = DRV_RTDMAC_SPEED_NORMAL,         // Assuming DRV_RTDMAC_SPEED_NORMAL is defined
    .mPrioLevel = 0
};

rDmacIrqCfg_t rDmacIrqHandler_t_irq =
{
	.Unit = RT_DMAC0,
	.SubCh = DMAC_CH1,
	.irq_channel = INTID_RTDMA0_CH1,
};

void PPI_handler(void *data)
{
    volatile int *flag = (volatile int *)data;
    *flag = INTERRUPT_FLAG_SET;
    CNTP_CTL_WRITE(0); // stop this interrupt for preventing trigger multiple times
}

void vConfigurePPIInterrupt(volatile int *irq_flag)
{
    Irq_SetupEntry(R_OS_BSP_GENERIC_ARM_TIMER_IRQNUM, (IrqHandlerFn)PPI_handler, (Context_t *)irq_flag);

    Irq_SetPriority(R_OS_BSP_GENERIC_ARM_TIMER_IRQNUM, IPRIORITY(24));

    /* set timer expiration from current counter value */
    CNTP_CVAL_WRITE(CNTPCT_READ() + COUNT_PER_TICK);

    /* configure CNTP_CTL to enable timer interrupts */
    CNTP_CTL_WRITE(1);

    Irq_Enable(R_OS_BSP_GENERIC_ARM_TIMER_IRQNUM);
}

void SPI_handler(void *data)
{
    (void)data;
    g_spi_irq_flag = INTERRUPT_FLAG_SET;
}

int vConfigureSPIInterrupt(void)
{
    Context_t usr_context =
    {
		.ctx = &rDmacIrqHandler_t_irq,
    };
    /* Get memory region first */
    st_memory_t region = R_UTILS_GetMemoryRegionInfo(OSAL, 0);
    cfg0.mSrcAddr = region.base_address;
    cfg0.mDestAddr = region.base_address + 0x01000000;
    if (cfg0.mDestAddr > region.base_address + region.size)
    {
        printf_raw("Failed: Destination address 0x%08X exceeds memory region (end at 0x%08X)\n", cfg0.mDestAddr, region.base_address + region.size);
        return -1;
    }
    /* Device Driver Part */
    R_RTDMAC_RcarDmacCtrlInit(rDmacIrqHandler_t_irq.Unit, DRV_RTDMAC_PRIO_FIX);
    R_RTDMAC_RcarCallBackSet(&rDmacIrqHandler_t_irq, SPI_handler, &usr_context);
    *(volatile uint32_t *)cfg0.mDestAddr = 0x9;
    *(volatile uint32_t *)cfg0.mSrcAddr  = 0x3;
    int dmaStatus = R_RTDMAC_RcarDmacExec(rDmacIrqHandler_t_irq.Unit, rDmacIrqHandler_t_irq.SubCh, &cfg0, NULL);

    // Check DMA execution status
    if (dmaStatus != 0)
    {
        return -1;
    }
    /* Wait DMA done */
    for (uint32_t i = 0; i < 30000000; i++)
    {
        if (g_spi_irq_flag == INTERRUPT_FLAG_SET)
        {
            break;
        }
    };
    R_RTDMAC_RcarDmacStop(rDmacIrqHandler_t_irq.Unit, rDmacIrqHandler_t_irq.SubCh);
    return 0;
}

void SGI_handler(void *data)
{
    volatile int *flag = (volatile int *)data;
    *flag = INTERRUPT_FLAG_SET;
}

void vConfigureSGIInterrupt(volatile int *irq_flag)
{
    uint32_t sgi_id = 1;
    Irq_SetupEntry(sgi_id, SGI_handler, (Context_t *)irq_flag);
    Irq_SetPriority(sgi_id, IPRIORITY(10));
    Irq_Enable(sgi_id);
    uint32_t rd = R_GIC_GetRedistID(__get_MPIDR() & 0xFFFF); // instead of use Irq_GetAffinity()

    // Trigger SGI
    R_GIC_SetIntPending(sgi_id, rd);
}

int main(void)
{
    /* Configure the hardware ready to run the demo. */
    prvSetupHardware();

    volatile int irq_flag = INTERRUPT_FLAG_UNSET;
    printf_raw(">> TC1: Software Generated Interrupt (SGI) testing <<\n");
    vConfigureSGIInterrupt(&irq_flag);

    /* Wait TC1 done */
    for (uint32_t i = 0; i < 30000000; i++)
    {
        if (irq_flag == INTERRUPT_FLAG_SET)
        {
            break;
        }
    };
    
    if (irq_flag == INTERRUPT_FLAG_SET)
    {
        printf_raw("TC 1 result: PASS\n");
    }
    else
    {
        printf_raw("TC 1 result: FAIL\n");
    }

    printf_raw(">> TC2: Private Peripheral Interrupt (PPI) testing <<\n");
    irq_flag = INTERRUPT_FLAG_UNSET;
    vConfigurePPIInterrupt(&irq_flag);

    /* Wait TC2 done */
    for (uint32_t i = 0; i < 30000000; i++)
    {
        if (irq_flag == INTERRUPT_FLAG_SET)
        {
            break;
        }
    };

    if (irq_flag == INTERRUPT_FLAG_SET)
    {
        irq_flag = INTERRUPT_FLAG_UNSET;
        printf_raw("TC 2 result: PASS\n");
    }
    else
    {
        printf_raw("TC 2 result: FAIL\n");
    }

    printf_raw(">> TC3: Shared Peripheral Interrupt (SPI) testing <<\n");
    int ret = vConfigureSPIInterrupt();
    if ((ret == 0) && (g_spi_irq_flag == INTERRUPT_FLAG_SET))
    {
        printf_raw("TC 3 result: PASS\n");
    }
    else
    {
        printf_raw("TC 3 result: FAIL\n");
    }

    printf_raw("<APP_END>");
    for (;;)
    {
    }
    /* Don't expect to reach here. */
    return 0;
}
/*-----------------------------------------------------------*/

static void prvSetupHardware(void)
{
    /* Ensure no interrupts execute while the scheduler is in an inconsistent
    state.  Interrupts are automatically enabled when the scheduler is
    started. */
    portDISABLE_INTERRUPTS();

    Irq_Setup();

    (void)pfcInitModules(getModuleConfigs());
}

/*-----------------------------------------------------------*/

void vMainAssertCalled(const char *pcFileName, uint32_t ulLineNumber)
{
    /* Don't use printf as it uses FreeRTOS resources */
    printf_raw("ASSERT!  Line %d of file %s\n", ulLineNumber, pcFileName);
    taskENTER_CRITICAL();
    for (;;)
        ;
}

void vDeleteCallingTask(void)
{
    vTaskDelete(NULL);
}
