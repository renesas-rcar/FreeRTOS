/*
 * FreeRTOS Kernel V11.3.0
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
#include "semphr.h"

#include "interrupts.h"
#include "stdio.h"
#define main_MFIS_TASK_PRIORITY        ( tskIDLE_PRIORITY + 1 )

#include "pfc/r_pfc_api.h"
#include "device_tree.h"
#include "rcar_utils.h"

#include "mfis/mfis.h"
#include "stdbool.h"
/*-----------------------------------------------------------*/
/* Interrupt receive timeout (milliseconds). */
#define TIME_OUT_INTERRUPT_MS       ( 8000 ) /* 8000 tick means 8000ms */
/*
 * Configure the hardware as necessary to run this demo.
 */
static void prvSetupHardware( void );

static void prvInitiatorTask( void *pvParameters );
static void prvResponderTask( void *pvParameters );
static void core0_mfis_cb( void *arg );
static void core1_mfis_cb( void *arg );

/*-----------------------------------------------------------*/

static struct mfis_channel core0_ch =
{
    .ch   = 0U,
    .type = MFIS_TYPE_RECEVER,
};

static struct mfis_channel core1_ch =
{
    .ch   = 0U,
    .type = MFIS_TYPE_SENDER,
};

static SemaphoreHandle_t xSem_mfis_cpu0 = NULL;
static SemaphoreHandle_t xSem_mfis_cpu1 = NULL;

int main( void )
{
    /* Configure the hardware ready to run the demo. */
    prvSetupHardware();
    uint32_t cpu_id = R_UTILS_GetCpuID();
    if (cpu_id == 0)
    {
        xTaskCreate( prvInitiatorTask, "prvInitiatorTask", configMINIMAL_STACK_SIZE * 2, NULL, main_MFIS_TASK_PRIORITY, NULL );
    }
    else if (cpu_id == 1)
    {
        xTaskCreate( prvResponderTask, "prvResponderTask", configMINIMAL_STACK_SIZE * 2, NULL, main_MFIS_TASK_PRIORITY, NULL );
    }
    else
    {
        return -1;
    }

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


static void core0_mfis_cb( void *arg )
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    ( void ) arg;

    if (xSem_mfis_cpu0 != NULL)
    {
        xSemaphoreGiveFromISR(xSem_mfis_cpu0, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

static void core1_mfis_cb( void *arg )
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    ( void ) arg;

    if (xSem_mfis_cpu1 != NULL)
    {
        xSemaphoreGiveFromISR(xSem_mfis_cpu1, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

static void prvInitiatorTask( void *pvParameters )
{
    /* Remove compiler warning about unused parameter. */
    ( void ) pvParameters;
    vTaskDelay(2000);
    uint32_t cpu_id = R_UTILS_GetCpuID();
    bool is_irq_received = false;
    printf("\n[CPU-%d] ============ ROLE: INITIATOR ============\n", cpu_id);

    xSem_mfis_cpu0 = xSemaphoreCreateBinary();

    core0_ch.cb_function = core0_mfis_cb;
    core0_ch.arg         = NULL;

    printf("[CPU-%d] TC1 | Initialize MFIS channel\n", cpu_id);
    if (mfis_init(&core0_ch) == 0)
    {
        printf("[CPU-%d] TC1 result PASS\n", cpu_id);
    }
    else
    {
        printf("[CPU-%d] TC1 result FAIL\n", cpu_id);
    }

    printf("[CPU-%d] TC2 | Trigger interrupt: send signal to peer core\n", cpu_id);
    if (mfis_trigger_interrupt(&core0_ch, 0U) == 0)
    {
        printf("[CPU-%d] TC2 result PASS\n", cpu_id);
    }
    else
    {
        printf("[CPU-%d] TC2 result FAIL\n", cpu_id);
    }

    printf("[CPU-%d] TC3 | Wait for interrupt signal from peer core\n\n", cpu_id);
    if (xSemaphoreTake(xSem_mfis_cpu0, TIME_OUT_INTERRUPT_MS) == pdTRUE)
    {
        is_irq_received = true;
        vTaskDelay(500);
        printf("[CPU-%d] TC3 |   Interrupt received from peer core\n", cpu_id);
    }

    if (is_irq_received == true)
    {
        printf("[CPU-%d] TC3 result PASS\n", cpu_id);
    }
    else
    {
        printf("[CPU-%d] TC3 result FAIL\n", cpu_id);
    }

    for( ;; )
    {
        vTaskDelay(1);
    }
}

static void prvResponderTask( void *pvParameters )
{
    /* Remove compiler warning about unused parameter. */
    ( void ) pvParameters;
    vTaskDelay(1000);
    uint32_t cpu_id = R_UTILS_GetCpuID();
    bool is_irq_received = false;
    printf("\n[CPU-%d] ============ ROLE: RESPONDER ============\n", cpu_id);

    xSem_mfis_cpu1 = xSemaphoreCreateBinary();

    core1_ch.cb_function = core1_mfis_cb;
    core1_ch.arg         = NULL;

    printf("[CPU-%d] TC1 | Initialize MFIS channel\n", cpu_id);
    if (mfis_init(&core1_ch) == 0)
    {
        printf("[CPU-%d] TC1 result PASS\n", cpu_id);
    }
    else
    {
        printf("[CPU-%d] TC1 result FAIL\n", cpu_id);
    }

    printf("[CPU-%d] TC2 | Wait for interrupt signal from peer core\n", cpu_id);
    if (xSemaphoreTake(xSem_mfis_cpu1, TIME_OUT_INTERRUPT_MS) == pdTRUE)
    {
        is_irq_received = true;
        vTaskDelay(500);
        printf("[CPU-%d] TC2 |   Interrupt received from peer core\n", cpu_id);
    }

    if (is_irq_received == true)
    {
        printf("[CPU-%d] TC2 result PASS\n", cpu_id);
    }
    else
    {
        printf("[CPU-%d] TC2 result FAIL\n", cpu_id);
    }

    vTaskDelay(1000);
    printf("[CPU-%d] TC3 | Trigger interrupt: send signal to peer core\n", cpu_id);
    if (mfis_trigger_interrupt(&core1_ch, 0U) == 0)
    {
        printf("[CPU-%d] TC3 result PASS\n\n", cpu_id);
    }
    else
    {
        printf("[CPU-%d] TC3 result FAIL\n\n", cpu_id);
    }

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