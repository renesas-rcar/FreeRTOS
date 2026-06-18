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
#include "watchdog/r_swdt_api.h"
#include "pfc/r_pfc_api.h"
#include "device_tree_x5h.h"

#define main_SWDT_TASK_PRIORITY        ( tskIDLE_PRIORITY + 1 )
#define printf_delay(fmt, ...)      \
	vTaskDelay(1);		    \
printf(fmt, ##__VA_ARGS__);         \

#define SWDT_SWTCSRA_REG   (*(volatile uint32_t *)0x1C050004U)

/*-----------------------------------------------------------*/

/*
 * Configure the hardware as necessary to run this demo.
 */
static void prvSetupHardware( void );

static void prvSWDTTask( void *pvParameters );

/*-----------------------------------------------------------*/

int main( void )
{
	/* Configure the hardware ready to run the demo. */
	prvSetupHardware();


	xTaskCreate( prvSWDTTask, "prvSWDTTask", configMINIMAL_STACK_SIZE, NULL, main_SWDT_TASK_PRIORITY, NULL);
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

static void prvSWDTTask( void *pvParameters )
{
	/* Remove compiler warning about unused parameter. */
	( void ) pvParameters;
	uint8_t init_timeout = 59;
	uint8_t ping_rate = 5;
	uint8_t ping_count = 3;

	int timeout = 3000; // 3s

	if ((SWDT_SWTCSRA_REG & (1U << 7)) != 0U)
	{
		printf("Waiting for SWDT to be available...\n");

		while (SWDT_SWTCSRA_REG & (1U << 7))
		{
			vTaskDelay(1);
			timeout--;
			if (timeout <= 0)
			{
				printf("Timeout: SWDT still busy\n");
				for( ;; );
			}
		}
	}

	printf("\n=== TC1: Init Watchdog Timer ===\n");
	if (R_SWDT_Init(init_timeout) == 0) {
		printf("\n[INFO] Watchdog Timer initialized with timeout = %u seconds.\n", init_timeout);
	} else {
		printf("\n[ERROR] Failed to initialize Watchdog Timer.\n");
	}

	printf("\n=== TC2: Start Watchdog Timer ===\n");
	if (R_SWDT_Start() == 0) {
		printf("\n[INFO] Watchdog Timer started successfully.\n");
	} else {
		printf("\n[ERROR] Failed to start Watchdog Timer.\n");
	}

	printf("\n=== TC3: Ping Watchdog Timer ===\n");
	for (uint8_t i = 0; i < ping_count; i++) {
		printf("[INFO] Ping %u: Set pingrate to %u seconds.\n", i + 1, ping_rate);
		if (R_SWDT_Ping(ping_rate) != 0) {
			printf("[ERROR] Ping failed at count %u\n", i + 1);
		}
	}

	printf("\n=== TC4: Stop pinging to let system reset ===\n");
	printf("\n[INFO] Ping count reached %u. No further pings — system should reset after timeout (%u seconds).\n",
	ping_count, init_timeout);
	printf("\n[NOTE] Waiting for system reset...\n");

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
