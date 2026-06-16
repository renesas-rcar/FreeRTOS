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
#include "mfis/mfis.h"
/*-----------------------------------------------------------*/
#define INTERRUPT_FLAG_UNSET        0
#define INTERRUPT_FLAG_SET          1
#define COUNT_PER_TICK (GENERIC_TIMER_CLK / configTICK_RATE_HZ)

/*
 * Configure the hardware as necessary to run this demo.
 */
static void prvSetupHardware(void);
int printf_raw(const char *format, ...);
void SPI_handler(void *data);

/*-----------------------------------------------------------*/

static volatile int g_spi_irq_flag = INTERRUPT_FLAG_UNSET;
static struct mfis_channel mfis_tx = {
    .ch = 0,
    .type = MFIS_TYPE_SENDER,
    .cb_function = NULL,
    .arg = NULL,
};
static struct mfis_channel mfis_rx = {
    .ch = 0,
    .type = MFIS_TYPE_RECEVER,
    .cb_function = SPI_handler,
    .arg = (void *)&g_spi_irq_flag,
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
    volatile int *flag = (volatile int *)data;
    *flag = INTERRUPT_FLAG_SET;
}

void vConfigureSPIInterrupt(void)
{
    mfis_init(&mfis_tx);
    mfis_init(&mfis_rx);
    mfis_trigger_interrupt(&mfis_tx, 0x55);
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
    vConfigureSPIInterrupt();
    for (uint32_t i = 0; i < 30000000; i++)
    {
        if (g_spi_irq_flag == INTERRUPT_FLAG_SET)
        {
            break;
        }
    };
    
    if (g_spi_irq_flag == INTERRUPT_FLAG_SET)
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
