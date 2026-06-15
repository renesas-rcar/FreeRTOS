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
#include "stdio.h"

#include "pfc/r_pfc_api.h"
#include "device_tree_x5h.h"
#include "rcar_utils.h"
#include "taud/r_taud.h"

#define main_taud_app_TASK_PRIORITY        ( tskIDLE_PRIORITY + 1 )
#define TAUD_APP_SIZE (configMINIMAL_STACK_SIZE * 2)
/*-----------------------------------------------------------*/

/*
 * Configure the hardware as necessary to run this demo.
 */
static void prvSetupHardware( void );

static void prvTaudPwmTask( void *pvParameters );
extern char _RAM_START;

static void taud_end_cycle(void * p_context);
static void taud_end_duty(void * p_context);

/*-----------------------------------------------------------*/

int main( void )
{
	/* Configure the hardware ready to run the demo. */
	prvSetupHardware();
    
    
    xTaskCreate( prvTaudPwmTask, "TaudPwm", TAUD_APP_SIZE, NULL, main_taud_app_TASK_PRIORITY, NULL );
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

static void prvTaudPwmTask( void *pvParameters )
{

    /* Remove compiler warning about unused parameter. */
    ( void ) pvParameters;
    e_taud_err_t ret = TAUD_SUCCESS;
    uint32_t cpu_id = R_UTILS_GetCpuID();

    printf("*** TAUD: PWM output functions ***\n");
    st_taud_instance_ctrl_t  g_taud_device_ctr0;
    st_taud_slave_ch_cfg_t   g_taud_slave_cfg0[] = {
        {.ch = 1, .duty = 80, .phase = 0, .duty_end_irq = {.p_callback = taud_end_duty, .p_context = &g_taud_slave_cfg0[0],},},
        {.ch = 2, .duty = 50, .phase = 0, .duty_end_irq = {.p_callback = taud_end_duty, .p_context = &g_taud_slave_cfg0[1],},},
        {.ch = 3, .duty = 10, .phase = 0, .duty_end_irq = {.p_callback = taud_end_duty, .p_context = &g_taud_slave_cfg0[2],},},
    };
    st_taud_cfg_t     g_taud_cfg = {
        .unit = 0,
        .func = TAUD_PWM_OUTPUT_FUNCTION,
        .func_cfg.pwm = {
            .master_ch = 0,
            .freq_hz = 1,
            .slave_num = 3,
            .p_slave = g_taud_slave_cfg0,
            .cycle_end_irq = {
                .p_callback = taud_end_cycle,
                .p_context = &g_taud_device_ctr0.func_cfg.pwm,
            },
        },
    };

    ret = R_TAUD_PWM_Open(&g_taud_device_ctr0, &g_taud_cfg);
    if(ret != TAUD_SUCCESS)
    {
        printf("TC1: Open: Failed: %d\n", ret);
    }
    else
    {
        printf("TC1: Open: Passed\n");
    }

    vTaskDelay(1000);

    ret = R_TAUD_PWM_Start(&g_taud_device_ctr0);
    if(ret != TAUD_SUCCESS)
    {
        printf("TC2: Start: Failed\n");
    }
    else
    {
        printf("TC2: Start: Passed\n");
    }

    vTaskDelay(1000);

    R_TAUD_PWM_UpdateFreq(&g_taud_device_ctr0, 2);
    R_TAUD_PWM_UpdateDuty(&g_taud_device_ctr0, 1, 10);
    R_TAUD_PWM_UpdateDuty(&g_taud_device_ctr0, 2, 20);
    R_TAUD_PWM_UpdateDuty(&g_taud_device_ctr0, 3, 50);

    vTaskDelay(1000);

    ret = R_TAUD_PWM_Stop(&g_taud_device_ctr0);
    if(ret != TAUD_SUCCESS)
    {
        printf("TC3: Stop: Failed\n");
    }
    else
    {
        printf("TC3: Stop: Passed\n");
    }
    printf("<APP_END>\n");

    for( ;; )
    {
        vTaskDelay(1);
    }
}

static void taud_end_cycle(void * p_context)
{
    st_taud_pwm_cfg_t *p_pwm = (st_taud_pwm_cfg_t *)p_context;
}

static void taud_end_duty(void * p_context)
{
    st_taud_slave_ch_cfg_t * p_slave = (st_taud_slave_ch_cfg_t *)p_context;
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
