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
#include "../drivers/audio/ak4619_module.h"
#define main_AK4619_TASK_PRIORITY        ( tskIDLE_PRIORITY + 1 )
/*-----------------------------------------------------------*/

/*
 * Configure the hardware as necessary to run this demo.
 */
static void prvSetupHardware( void );

static void prvAk4619Task( void *pvParameters );
extern char _RAM_START;

#include "pfc/r_pfc_api.h"
#include "device_tree_x5h.h"
/*-----------------------------------------------------------*/

int main( void )
{
	/* Configure the hardware ready to run the demo. */
	prvSetupHardware();
    
    
    xTaskCreate( prvAk4619Task, "AK4619", configMINIMAL_STACK_SIZE, NULL, main_AK4619_TASK_PRIORITY, NULL );
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

static void prvAk4619Task( void *pvParameters )
{
    /* Remove compiler warning about unused parameter. */
    ( void ) pvParameters;
    uint8_t ret = 0;
    ak4619_instance_set_t instance_set =
    {
        .format = STEREO_I2S_COMPATIBLE,
        .mclk_multiplier = MCLK_256_FS,
        .fs = FS_48K,
        .didl_set = WORD_16BIT,
        .dac_1 = DAC_INPUT_FROM_SDIN1,
    };

    ret = ak4619_power_on();
    if (ret != 0)
    {
        printf("Can not setup AK4619\n");
    }
    vTaskDelay(pdMS_TO_TICKS(10)); // delay 10ms
    // After this, PMDA1 bit = RSTN bit = 0, at reset state -> let config register after this state
    ret = ak4619_module_init(&instance_set, 2, 16);
    if (ret != 0)
    {
        printf("Can not setup AK4619 module\n");
    }
    else 
    {
        printf("Set up AK4619 module done\n");
    }

    for( ;; )
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
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
