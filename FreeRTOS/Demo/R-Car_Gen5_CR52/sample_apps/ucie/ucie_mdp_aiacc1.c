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

#include <stdlib.h>
#include "stdio.h"
#include "string.h"

#include "pfc/r_pfc_api.h"
#include "device_tree_x5h.h"
#include "ucie/r_ucie.h"
#include "rcar_utils.h"
#define main_ucie_TASK_PRIORITY        (tskIDLE_PRIORITY + 1)
#define UCIE_EP_SIZE (configMINIMAL_STACK_SIZE * 20)

#define PIO_X5H_RC0_WRITE_DATA      0xFFCCFFCC
#define PIO_AIACC0_EP0_WRITE_DATA   0x68686868
#define PIO_X5H_RC1_WRITE_DATA      0x12345678
#define PIO_AIACC1_EP0_WRITE_DATA   0x87654321
#define PIO_AIACC0_RC1_WRITE_DATA   0x11111111
#define PIO_AIACC1_EP1_WRITE_DATA   0x22222222

/*-----------------------------------------------------------*/

/*
 * Configure the hardware as necessary to run this demo.
 */
static void prvSetupHardware( void );

static void ucie_comm_task( void *pvParameters );

int main( void )
{
    /* Configure the hardware ready to run the demo. */
    prvSetupHardware();

    xTaskCreate(ucie_comm_task, "UCIe", UCIE_EP_SIZE, NULL, main_ucie_TASK_PRIORITY, NULL );

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

/*-----------------------------------------------------------*/

/*-----------------------------------------------------------*/

static void ucie_comm_task(void *pvParameters)
{
    /* Remove compiler warning about unused parameter. */
    ( void ) pvParameters;

    uint32_t ret = 0;
    uint32_t timeout;
    uint32_t timer_freq = R_UTILS_GetTimerFrequency();
    uint64_t start;
    uint64_t x5h_rc0_addr    =  0x90000000;
    uint64_t aiacc0_ep0_addr =  0x63800000;
    uint64_t x5h_rc1_addr    =  0x90001000;
    uint64_t aiacc1_ep0_addr =  0x63801000;
    uint64_t aiacc0_rc1_addr =  0x90005000;
    uint64_t aiacc1_ep1_addr =  0x63805000;

    printf("<----- [AIACC1] TC1: UCIE0 INIT ----->\n");
    ret = R_UCIE_Setup(UCIE_CH0, UCIE_MODE_EP, LINKSPEED_16GTPS);
    if (ret != LINKUP_ERROR) {
        printf("Result: PASSED\n");
    }
    else {
        printf("Result: FAILED\n");
    }

    printf("<----- [AIACC1] TC2: UCIE0 LINKUP ----->\n");
    if (ret == LINKUP_TIMEOUT) {
        printf("The first time linkup timeout. Retry 30 times\n");
        ret = R_UCIE_Retry_Linkup(UCIE_CH0, UCIE_MODE_EP, LINKSPEED_16GTPS, 30);
    }
    if (ret) {
        printf("Result: FAILED\n");
    }
    else {
        printf("Result: PASSED\n");
    }

    printf("<----- [AIACC1] TC3: UCIE1 INIT ----->\n");
    ret = R_UCIE_Setup(UCIE_CH1, UCIE_MODE_EP, LINKSPEED_16GTPS);
    if (ret != LINKUP_ERROR) {
        printf("Result: PASSED\n");
    }
    else {
        printf("Result: FAILED\n");
    }

    printf("<----- [AIACC1] TC4: UCIE1 LINKUP ----->\n");
    if (ret == LINKUP_TIMEOUT) {
        printf("The first time linkup timeout. Retry 30 times\n");
        ret = R_UCIE_Retry_Linkup(UCIE_CH1, UCIE_MODE_EP, LINKSPEED_16GTPS, 30);
    }
    if(ret) {
        printf("Result: FAILED\n");
    }
    else {
        printf("Result: PASSED\n");
    }

    printf("<----- [AIACC1] TC5: UCIE0 HDMA Transfer ----->\n");
    printf("HDMA not suppport now\n");
    printf("Result: FAILED\n");

    printf("<----- [AIACC1] TC6: UCIE1 HDMA Transfer ----->\n");
    printf("HDMA not suppport now\n");
    printf("Result: FAILED\n");

    printf("<----- [AIACC1] TC7: UCIE0 PIO Transfer ----->\n");

    ret = 1;
    start = R_UTILS_GetTimerCounter();
    while ((R_UTILS_GetTimerCounter() - start)/timer_freq < 3) {
        if (*(volatile uint32_t*)(uintptr_t)aiacc1_ep0_addr == PIO_X5H_RC1_WRITE_DATA) {
            ret = 0;
            break;
        }
    }

    printf("Value at 0x%llX: 0x%X\n", aiacc1_ep0_addr, *(volatile uint32_t*)(uintptr_t)aiacc1_ep0_addr);
    if (ret) {
        printf("Result: FAILED\n");
    }
    else {
        printf("Result: PASSED\n");
    }

    printf("AIACC1-UCIE0 EP write 0x%X to PIO region\n", PIO_AIACC1_EP0_WRITE_DATA);
    *(volatile uint32_t*)(uintptr_t)aiacc1_ep0_addr = PIO_AIACC1_EP0_WRITE_DATA;

    printf("<----- [AIACC1] TC8: UCIE1 PIO Transfer ----->\n");

    ret = 1;
    start = R_UTILS_GetTimerCounter();
    while ((R_UTILS_GetTimerCounter() - start)/timer_freq < 3) {
        if (*(volatile uint32_t*)(uintptr_t)aiacc1_ep1_addr == PIO_AIACC0_RC1_WRITE_DATA) {
            ret = 0;
            break;
        }
    }

    printf("Value at 0x%llX: 0x%X\n", aiacc1_ep1_addr, *(volatile uint32_t*)(uintptr_t)aiacc1_ep1_addr);

    if (ret) {
        printf("Result: FAILED\n");
    }
    else {
        printf("Result: PASSED\n");
    }

    printf("AIACC1-UCIE1 EP write 0x%X to PIO region\n", PIO_AIACC1_EP1_WRITE_DATA);
    *(volatile uint32_t*)(uintptr_t)aiacc1_ep1_addr = PIO_AIACC1_EP1_WRITE_DATA;

    printf("<----- [AIACC1] TC9: UCIE0 interrupt ----->\n");
    printf("Result: FAILED\n");

    printf("<----- [AIACC1] TC10: UCIE1 interrupt ----->\n");
    printf("Result: FAILED\n");

    printf("<----- [AIACC1] END TEST ----->\n");

    for(;;) {
        __asm__ volatile("nop");
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
