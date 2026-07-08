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
#include "pfc/r_pfc_api.h"
#include "device_tree.h"
#include "rcar_utils.h"
/*-----------------------------------------------------------*/
/*
 * Configure the hardware as necessary to run this demo.
 */
static void prvSetupHardware(void);
int printf_raw(const char *format, ...);

/*-----------------------------------------------------------*/

int main(void)
{
    /* Configure the hardware ready to run the demo. */
    prvSetupHardware();
    printf_raw(">> TC1: Testing get timer counter <<\n");
    uint64_t timer_counter_before = R_UTILS_GetTimerCounter();
    printf_raw("Counter before: %llu\nDelay a little time...\n", timer_counter_before);
    for (uint32_t i = 0; i < 300000000; i++)
    {};
    uint64_t timer_counter_after = R_UTILS_GetTimerCounter();
    printf_raw("Counter after: %llu\n", timer_counter_after);
    if (timer_counter_after > timer_counter_before)
    {
        printf_raw("TC 1 result: PASS\n");
    }
    else
    {
        printf_raw("TC 1 result: FAIL\n");
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
