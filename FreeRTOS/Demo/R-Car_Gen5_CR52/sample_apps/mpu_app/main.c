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

#define main_MPU_TASK_PRIORITY      ( tskIDLE_PRIORITY + 1 )
extern uint32_t _RAM_START;
#define REGION_SRAM           ((uint32_t)&_RAM_START + 0x2BF000)
//#define REGION_RO             0x50000000
#define REGION_DEVICE         0xc11d0000
/*-----------------------------------------------------------*/

/*
 * Configure the hardware as necessary to run this demo.
 */
static void prvSetupHardware( void );

static void prvTestMPURegions( void *pvParameters );
/*-----------------------------------------------------------*/

int main( void )
{
	/* Configure the hardware ready to run the demo. */
	prvSetupHardware();
   
    xTaskCreate(prvTestMPURegions, "TestMPU", configMINIMAL_STACK_SIZE, NULL, main_MPU_TASK_PRIORITY, NULL);

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
}

static void prvTestMPURegions( void *pvParameters )
{
    
     /* Remove compiler warning about unused parameter. */
    ( void ) pvParameters;

    volatile uint32_t *sram_ptr = (uint32_t *)REGION_SRAM;
    //volatile uint32_t *ro_ptr = (uint32_t *)REGION_RO;
    volatile uint32_t *dev_ptr = (uint32_t *)REGION_DEVICE;

    printf("----- Test OSAL MPU api\n");
    vTaskDelay(100);
    printf("TC1: Total OSAL region: %d\n", R_UTILS_GetTotalRegionOfMemory(OSAL));

    vTaskDelay(100);
    st_memory_t region = R_UTILS_GetMemoryRegionInfo(OSAL, 0);
    printf("TC2: TEST OSAL MPU api get OSAL region base address and size\n");
    vTaskDelay(100);
    printf("Case osal idx 0. Base address: 0x%x, Size: 0x%x\n", region.base_address, region.size);
    vTaskDelay(100);
    region = R_UTILS_GetMemoryRegionInfo(OSAL, 1);
    printf("Case osal idx 1, out of range so expect base: 0, size: 0. Base address: 0x%x, Size: 0x%x\n", region.base_address, region.size);
    vTaskDelay(100);

    /* Test Region SRAM (Read/Write)*/ 
    printf("TC3: Testing Region SRAM ...\n");
    vTaskDelay(100);
    *sram_ptr = 0xAAAAAAAA;
    printf("Writing to region SRAM ...\n");
    uint32_t sram_value = *sram_ptr;
    printf("Read Value = 0x%X\n\n", sram_value);
    vTaskDelay(100);

    /* Test Region DEVICE (Read/Write)*/
    printf("TC4: Testing Region DEVICE ...\n");
    vTaskDelay(100);
    *dev_ptr = 0xBBBBBBBB;
    printf("Writing to region DEVICE ...\n");
    uint32_t dev_value = *dev_ptr;
    printf("Read Value = 0x%X\n\n", dev_value);
    vTaskDelay(100);
    printf("<APP_END>\n");

    /* Test Region RO (Read-Only) */ 
    /*printf("Testing Region Read-Only ...\n");
    vTaskDelay(100);
    uint32_t ro_value = *ro_ptr;
    printf("Read Value = 0x%X\n\n", ro_value);
    vTaskDelay(100);*/

    /* Attempt to Write to Region RO */
    /*printf("Attempting to write to Region RO...\n");
    *ro_ptr = 0xCCCCCCCC;*/

    vTaskDelete(NULL);
 
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
