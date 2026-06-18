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
#include "semphr.h"

#include "scif.h"
#include "interrupts.h"
#include "stdio.h"
#include "serial/r_serial.h"
#include "pfc/r_pfc_api.h"
#include "device_tree_x5h.h"
#define main_LOG_TASK_PRIORITY        ( tskIDLE_PRIORITY + 1 )

/*-----------------------------------------------------------*/

/*
 * Configure the hardware as necessary to run this demo.
 */
static void prvSetupHardware( void );

static void prvLogTask( void *pvParameters );

static void UartIrqTriggerTask(void *pvParameters);
static void UARTInterruptHandler(void *data);
SemaphoreHandle_t xSemaphore = NULL;
unsigned char p_char;
/*-----------------------------------------------------------*/
uint32_t getIrqID(uint8_t uart_id)
{
    uint32_t irqID;
    switch(uart_id) {
        case 0:
            irqID = SCIF0_INT_ID;
            break;
        case 1:
            irqID = SCIF1_INT_ID;
            break;
        case 3:
            irqID = SCIF3_INT_ID;
            break;
        case 4:
            irqID = SCIF4_INT_ID;
            break;
        case 5:
            irqID = HSCIF0_INT_ID;
            break;
        case 6:
            irqID = HSCIF1_INT_ID;
            break;
        case 7:
            irqID = HSCIF2_INT_ID;
            break;
        case 8:
            irqID = HSCIF3_INT_ID;
            break;
        default:
            return 0;
    }

    return irqID;
}

int main( void )
{
    uint32_t irqID;

    /* Configure the hardware ready to run the demo. */
    prvSetupHardware();

    irqID = getIrqID(UART_ID);

    if (irqID == 0) {
        printf("Not support UART_ID = %d\n", UART_ID);
    } else {
        /* Set Handler for Irq */
        Irq_SetupEntry(irqID, UARTInterruptHandler, NULL);

        /* Set priority for Irq */
        Irq_SetPriority(irqID, IPRIORITY(2));

        /* Enable Irq */
        Irq_Enable(irqID);
    }
 
    xSemaphore = xSemaphoreCreateBinary();

    if (xSemaphore == NULL) {
        printf("Semaphore creation failed!\n");
    }
    else {
        xTaskCreate(UartIrqTriggerTask, "UartIrqTriggerTask", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES - 1, NULL);
    }

    xTaskCreate( prvLogTask, "Log", configMINIMAL_STACK_SIZE, NULL, main_LOG_TASK_PRIORITY, NULL );
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

    R_SERIAL_PortInit(UART_ID);

	Irq_Setup();

    (void)pfcInitModules(getModuleConfigs());
}

static void prvLogTask( void *pvParameters )
{

    /* Remove compiler warning about unused parameter. */
    ( void ) pvParameters;
    unsigned char buffer[24] = "prvLogTask ...\n";

    // --- TC 1: Support Log Test ---
    printf("**********************************************\r\n");
    printf("TEST CASE 1: Support log test\n");
    printf(">>> TC1 PASS if you see this logs. <<<\r\n\n");

    // --- TC 2: Disable Log Test ---
    printf("**********************************************\r\n");
    printf("TEST CASE 2: Silent console test\n");
    printf("MSG: Unexpect log out below (Silent for 8s)\n");
    
    R_SERIAL_SetLogState(LOG_OFF);
    for (int i = 0; i < 4; i++)
    {
        printf("This message should NOT appear. Test FAILED if you see this logs!\n");
        vTaskDelay(2000);
    }
    R_SERIAL_SetLogState(LOG_ON);
    printf(">>> Logs re-enabled. TC2 PASS if no messages leaked during silence time. <<<\n\n");

    // --- TC 3: Multi-channel Test ---
    printf("**********************************************\r\n");
    printf("TEST CASE 3: Test multi channel\n");
    printf("Switching to HSCIF0. Please update Terminal to 115200 baudrate on NEW COM\n");
    
    if (R_SERIAL_ReConfigure(HSCIF0) == 0) 
    {
        for (int i = 0; i < 12; i++) /* Loop provides sufficient time for user to switch COM port connection */
        {
            printf(">>> TC3 PASS if you see this logs. <<<\n");
            vTaskDelay(1000);
        }
    }
    else
    {
        for (int i = 0; i < 10; i++) /* Loop provides sufficient time for user to switch COM port connection */
        {
            printf(">>> TC3 failed. <<<\n");
            vTaskDelay(1000);
        }
    }

    R_SERIAL_ReConfigure(UART_ID); /* Restore console to default port for the main loop (SCIF1) */
    printf("=== All Test Cases Completed. Returning to Default Console (SCIF1) ===\n");
    printf("<APP_END>\n");
    for( ;; )
    {
        printf("\nUART Interrupt is ready - Please type to RX terminal for testing\n");
        R_SERIAL_PutString(buffer, sizeof(buffer));
        vTaskDelay(3000);
    }
}

/*-----------------------------------------------------------*/

void UARTInterruptHandler(void *data) {
    (void)data;

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    R_SERIAL_GetChar(&p_char);
    xSemaphoreGiveFromISR(xSemaphore, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void UartIrqTriggerTask(void *pvParameters) {
    ( void ) pvParameters;

    for(;;) {
        if (xSemaphoreTake(xSemaphore, portMAX_DELAY) == pdTRUE) {
            printf("Task has been triggered by interrupt! Receive char: %c\n", p_char);
        }
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
