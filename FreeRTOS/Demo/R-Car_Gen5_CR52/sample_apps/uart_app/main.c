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
#include "string.h"
#include "serial/r_serial.h"
#include "pfc/r_pfc_api.h"
#include "device_tree.h"
#define main_LOG_TASK_PRIORITY        ( tskIDLE_PRIORITY + 1 )
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#if (BOARD == X5H_RFS2)
    uint8_t serialChannelArr[] = {
        SCIF1,
        SCIF0,
        HSCIF0,
        SCIF3,
        HSCIF1,
        HSCIF2
    };
#elif (BOARD == AI_ACC)
    uint8_t serialChannelArr[] = {
        SCIF1,
        SCIF0,
        HSCIF0,
        HSCIF1
    };
#elif (BOARD == X5H_IRONHIDE)
    uint8_t serialChannelArr[] = {
        SCIF1,
        HSCIF0
    };
#else //MDP board
    uint8_t serialChannelArr[] = {
        HSCIF0
    };
#endif
/*-----------------------------------------------------------*/

/*
 * Configure the hardware as necessary to run this demo.
 */
static void prvSetupHardware( void );

static void prvLogTask( void *pvParameters );

static void UartIrqTriggerTask(void *pvParameters);
static void UARTInterruptHandler(void *data);
static void uartIDtoString(uint8_t uartID,unsigned char *uartName);
static void uartAppExample(uint8_t *channelArr, uint8_t arrLength);
SemaphoreHandle_t xSemaphore = NULL;
unsigned char p_char;
uint8_t tcNum = 1;
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

static void uartIDtoString(uint8_t uartID, unsigned char *uartName)
{
    memset(uartName,0,strlen(uartName));

    switch(uartID) {
            case SCIF1:
                strcpy(uartName, "SCIF1");
                break;
            case SCIF0:
                strcpy(uartName, "SCIF0");
                break;
            case HSCIF0:
                strcpy(uartName, "HSCIF0");
                break;
            case SCIF2_UNSUPPORTED:
                strcpy(uartName, "SCIF2");
                break;
            case SCIF3:
                strcpy(uartName, "SCIF3");
                break;
            case SCIF4:
                strcpy(uartName, "SCIF4");
                break;
            case HSCIF1:
                strcpy(uartName, "HSCIF1");
                break;
            case HSCIF2:
                strcpy(uartName, "HSCIF2");
                break;
            case HSCIF3:
                strcpy(uartName, "HSCIF3");
                break;
            default:
                break;
    }
}

static void uartAppExample(uint8_t *channelArr, uint8_t arrLength)
{
    uint8_t loopIndex = 0;
    unsigned char uartNameBuf[24];
    uint8_t allChannel[] = {
        SCIF0,
        SCIF1,
        SCIF2_UNSUPPORTED,
        SCIF3,
        SCIF4,
        HSCIF0,
        HSCIF1,
        HSCIF2,
        HSCIF3
    };

    // --- TC 1: Support Log Test ---
    printf("**********************************************\r\n");
    printf("TEST CASE %d: Support log test\n", tcNum);
    printf(">>> TC%d result: PASS. <<<\r\n", tcNum);
    // --- TC 2: Disable Log Test ---
    printf("**********************************************\r\n");
    printf("TEST CASE %d: Silent console test\n", ++tcNum);
    printf("MSG: Unexpect log out below (Silent for 8s)\n");

    R_SERIAL_SetLogState(LOG_OFF);
    for (loopIndex = 0; loopIndex < 4; loopIndex++)
    {
        printf("This message should NOT appear. TC%d result: FAIL.\n", tcNum);
        vTaskDelay(2000);
    }
    R_SERIAL_SetLogState(LOG_ON);
    printf(">>> Logs re-enabled. TC%d result: PASS. <<<\n\n",tcNum);

    // --- TC: Multi-channel Test ---
    for (uint8_t setIndex = 0; setIndex < ARRAY_SIZE(allChannel); setIndex++) {

        if(allChannel[setIndex] == serialChannelArr[0]) {continue;}

        uint8_t flag = 0;

        for (uint8_t chnIndex = 0; chnIndex < arrLength; chnIndex++) {
            if (allChannel[setIndex] == serialChannelArr[chnIndex]) {
                flag = 1;
                break;
            }
        }

        ++tcNum;

        R_SERIAL_ReConfigure(serialChannelArr[0]);
        uartIDtoString(allChannel[setIndex],uartNameBuf);
        printf("\n**********************************************\r\n");
        printf("TEST CASE %d: Test %s\n", tcNum, uartNameBuf);
        printf("\n=== Please manually switch to %s and update Terminal to 115200 baudrate on NEW COM ===\n", uartNameBuf);
        vTaskDelay(500);

        #if (BOARD == X5H_RFS2)
        if (flag == 0 || allChannel[setIndex] == SCIF3)
        #else
        if (flag == 0)
        #endif
        {
            /* NA TC*/
            R_SERIAL_ReConfigure(serialChannelArr[0]);
            uartIDtoString(serialChannelArr[0],uartNameBuf);
            printf(">>> TC%d result: NA. <<<\r\n", tcNum, uartNameBuf);
            vTaskDelay(500);
        } else {
            /* Normal TC */
            if (R_SERIAL_ReConfigure(allChannel[setIndex]) == 0) {
                uartIDtoString(allChannel[setIndex],uartNameBuf);
                printf("\n**********************************************\r\n");
                printf("TEST CASE %d: Test %s\n\n", tcNum, uartNameBuf);
                for (loopIndex = 0; loopIndex < 12; loopIndex++)   /* Loop provides sufficient time for user to switch COM port connection */
                {
                    printf(">>> TC%d result: PASS. <<<\n", tcNum);
                    vTaskDelay(1000);
                }
            } else {
                for (loopIndex = 0; loopIndex < 10; loopIndex++) {
                    printf(">>> TC%d result: FAIL. <<<\n", tcNum);
                    vTaskDelay(1000);
                }
            }
        }
    }

    tcNum += 1;
    uartIDtoString(serialChannelArr[0],uartNameBuf);
    R_SERIAL_ReConfigure(serialChannelArr[0]); /* Restore console to default port for the main loop */
    printf("\n=== Returning to Default Console (%s) ===\n", uartNameBuf);
}

int main( void )
{
    uint32_t irqID;

    /* Configure the hardware ready to run the demo. */
    prvSetupHardware();

    irqID = getIrqID(serialChannelArr[0]);

    if (irqID == 0) {
        printf("Not support UART_ID = %d\n", serialChannelArr[0]);
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

    R_SERIAL_PortInit(serialChannelArr[0]);

	Irq_Setup();

    (void)pfcInitModules(getModuleConfigs());
}

static void prvLogTask( void *pvParameters )
{

    /* Remove compiler warning about unused parameter. */
    ( void ) pvParameters;
    unsigned char buffer[24] = "prvLogTask ...\n";

    uartAppExample(serialChannelArr,ARRAY_SIZE(serialChannelArr));

    printf("\nTEST CASE %d: UART Interrupt is ready - Please type to RX terminal for testing\n",tcNum);
    for( ;; )
    {
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

    if (xSemaphoreTake(xSemaphore, portMAX_DELAY) == pdTRUE) {
        printf("Task has been triggered by interrupt! Receive char: %c\n", p_char);
        printf(">>> TC%d result: PASS. UART Irq triggered successfully. <<<\n\n", tcNum);
        printf("\n<APP_END>\n");
    }

    for(;;) {
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
