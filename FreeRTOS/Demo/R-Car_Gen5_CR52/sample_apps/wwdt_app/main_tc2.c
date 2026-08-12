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

#if (BOARD == X5H_VDK) || (BOARD == X5H_IRONHIDE) || (BOARD == X5H_RFS2) || (BOARD == MDP_X5H_HIL)
#define WWDT_UNIT   R_WWDT20
#elif (BOARD == MDP_AIACC_RFS2) || (BOARD == MDP_AIACC_HIL)
#define WWDT_UNIT   R_WWDT2
#endif

#define STR(x) #x
#define XSTR(x) STR(x)

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

#if (BOARD == MDP_X5H_HIL) || (BOARD == MDP_AIACC_HIL)
    vTaskDelay(6000);
#endif

    /* Device driver part */
    printf("---------- PROGRAM START ----------- \r\n");

    printf("=== Test Case 2: %s, 273ms, 75% window OPEN ===\r\n", XSTR(WWDT_UNIT));
    R_WWDT_Init(WWDT_UNIT, WINDOW_75P, 273, false, ERM_RESET_MODE);

    printf("R_WWDT_Init: done\r\n");
    err = R_WWDT_Refresh(WWDT_UNIT);

    // After (150 * 100) ms, the system will reset
    printf("\n[INFO]: After 15s, the system will reset\n");
    for (int i = 0; i < 150; i++)
    {
        // Need to remove this function in actual situation
        // Choose a value that refresh the WDT in remaining of 75% OPEN window
        vTaskDelay(100);

        // Refresh before the counter underflows to prevent Reset
        err = R_WWDT_Refresh(WWDT_UNIT);
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
