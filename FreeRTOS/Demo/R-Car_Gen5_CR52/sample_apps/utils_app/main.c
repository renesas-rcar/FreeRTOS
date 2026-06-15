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
#include "rcar_utils.h"

#define main_UTILS_TASK_PRIORITY        ( tskIDLE_PRIORITY + 1 )
#define WRITE_VALUE                123456789

#include "pfc/r_pfc_api.h"

#if (BOARD == X5H_VDK || BOARD == X5H_IRONHIDE || BOARD == X5H_RFS2)

#include "device_tree_x5h.h"
#else   // (BOARD == MDP_AIACC_HIL || BOARD == MDP_AIACC_RFS2)

#include "device_tree_mdp_aiacc.h"
#endif  // (BOARD == X5H_VDK || BOARD == X5H_IRONHIDE || BOARD == X5H_RFS2)

/*-----------------------------------------------------------*/

/*
 * Configure the hardware as necessary to run this demo.
 */
static void prvSetupHardware( void );

static void prvUtilsTask( void *pvParameters );

/*-----------------------------------------------------------*/

int main( void )
{
	/* Configure the hardware ready to run the demo. */
	prvSetupHardware();
    
    
    xTaskCreate( prvUtilsTask, "Utils_Task", configMINIMAL_STACK_SIZE, NULL, main_UTILS_TASK_PRIORITY, NULL );
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

static void prvUtilsTask( void *pvParameters )
{
    /* Remove compiler warning about unused parameter. */
    ( void ) pvParameters;
    
    uint8_t total_osal;
    st_memory_t info;

    printf("<---------- BEGIN TEST OSAL UTILS API ---------->\n");
    printf("TC1: Get total OSAL Region\n");
    total_osal = R_UTILS_GetTotalRegionOfMemory(OSAL);
    printf("Total Osal region: %d\n", total_osal);
    if(total_osal == 0) {
        printf("Failed\n");
    }
    else {
        printf("True\n");
    }
    printf("<---------- END TC1 ---------->\n");

    printf("TC2: Get OSAL Region 0 Infomation\n");
    info = R_UTILS_GetMemoryRegionInfo(OSAL, 0);
    printf("[Base : 0x%x] - [Size: 0x%x]\n", info.base_address, info.size);
    if (info.size == 0) {
        printf("Failed\n");
        printf("<---------- END TC2 ---------->\n");
    }
    else {
        uint32_t read_value;
        printf("True\n");
        printf("<---------- END TC2 ---------->\n");
        
        printf("TC3: Test Read/Write in OSAL Region 0\n");
        *(uint32_t *)(info.base_address + 0x8) = WRITE_VALUE;
        read_value = *(uint32_t *)(info.base_address + 0x8); 
        printf("Write at addr 0x%x, Value: %d\n",info.base_address + 0x8, WRITE_VALUE);
        printf("Read at addr 0x%x, Value: %d\n", info.base_address + 0x8, read_value);
        if(read_value == WRITE_VALUE) {
            printf("True\n");
        }
        else {
            printf("Failed\n");
        }
        printf("<---------- END TC3 ---------->\n");
    }

    printf("TC4: Get CPU cycles\n");
    uint32_t cycle_cnt = R_UTILS_GetCPUCycles();
    printf("CPU cycles: %u\n", cycle_cnt);
    printf("<---------- END TC4 ---------->\n");

    printf("<APP_END>\n");
    
    for(;;) {}
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
