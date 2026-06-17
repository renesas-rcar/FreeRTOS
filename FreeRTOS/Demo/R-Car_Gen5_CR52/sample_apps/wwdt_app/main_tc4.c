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
#include "watchdog/r_wwdt_api.h"
#include "pfc/r_pfc_api.h"

#include "device_tree.h"

#define main_WWDT_TASK_PRIORITY        ( tskIDLE_PRIORITY + 1 )
#define printf_delay(fmt, ...)      \
	vTaskDelay(1);		    \
printf(fmt, ##__VA_ARGS__);         \
/*-----------------------------------------------------------*/

/*
 * Configure the hardware as necessary to run this demo.
 */
static void prvSetupHardware( void );

static void prvWWDTTask( void *pvParameters );

/*-----------------------------------------------------------*/

int main( void )
{
	/* Configure the hardware ready to run the demo. */
	prvSetupHardware();


	xTaskCreate( prvWWDTTask, "prvWWDTTask", configMINIMAL_STACK_SIZE, NULL, main_WWDT_TASK_PRIORITY, NULL);
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

static void prvWWDTTask( void *pvParameters )
{
	/* Remove compiler warning about unused parameter. */
	( void ) pvParameters;
	uint32_t err;

	/* Device driver part */
	printf("---------- PROGRAM START ----------- \r\n");

	printf_delay("=== Test Case 4: WWDT20, 136ms, 25% window ===\r\n");
	R_WWDT_Init(R_WWDT20, WINDOW_25P, 136, false, ERM_RESET_MODE);
	printf_delay("R_WWDT_Init: done\r\n");

	err = R_WWDT_Refresh(R_WWDT20);
	printf("\n[INFO]: After 12s, the system will reset\n");
	for (int i = 0; i < 100; i++) {
		vTaskDelay(120);  // adjust so refresh stays in allowed window
		err = R_WWDT_Refresh(R_WWDT20);
		if (err != 0) break;
	}

    printf("<APP_END>\n");
	for( ;; )
	{
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
