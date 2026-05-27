/*
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */
/*
 * FreeRTOS Kernel V10.4.1
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
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
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

/******************************************************************************
 * NOTE 1:  This project provides two demo applications.  A simple blinky
 * style project, and a more comprehensive test and demo application.  The
 * mainSELECTED_APPLICATION setting in main.c is used to select between the two.
 * See the notes on using mainSELECTED_APPLICATION in main.c.  This file
 * implements the comprehensive version.
 *
 * NOTE 2:  This file only contains the source code that is specific to the
 * full demo.  Generic functions, such FreeRTOS hook functions, and functions
 * required to configure the hardware, are defined in main.c.
 *
 * NOTE 3:  The full demo includes a test that checks the floating point context
 * is maintained correctly across task switches.  The standard GCC libraries can
 * use floating point registers and made this test fail (unless the tasks that
 * use the library are given a floating point context as described on the
 * documentation page for this demo).
 *
 ******************************************************************************
 *
 * main_full() creates all the demo application tasks and software timers, then
 * starts the scheduler.  The web documentation provides more details of the
 * standard demo application tasks, which provide no particular functionality,
 * but do provide a good example of how to use the FreeRTOS API.
 *
 * "Check" task - The check task period is set to five seconds.  Each time it
 * executes it checks all the standard demo tasks, and the register check tasks,
 * are not only still executing, but are executing without reporting any errors,
 * then outputs the system status to the UART.
 */

/* Standard includes. */
#include <stdio.h>
#include <string.h>
/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"

/* Standard demo application includes. */
#include "flop.h"
#include "semtest.h"
#include "dynamic.h"
#include "blocktim.h"
#include "countsem.h"
#include "GenQTest.h"
#include "recmutex.h"
//#include "IntQueue.h"
#include "EventGroupsDemo.h"
#include "TaskNotify.h"
#include "IntSemTest.h"
#include "StaticAllocation.h"
#include "AbortDelay.h"
#include "QueueOverwrite.h"
#include "TimerDemo.h"


/* CORTEX M3 QEMU include */
#include "QPeek.h"
#include "QueueSet.h"
#include "MessageBufferDemo.h"
#include "StreamBufferDemo.h"
#include "death.h"

#ifdef configSUPPORT_POSIX
#include "posix_demo.h"
#endif

/* Logging Function include. */
#ifdef LOGGING_ENABLE
#include "logging_stack.h"
#endif

#include "scif.h"
#include "interrupts.h"
/*------------------------*/

/* Priorities for the demo application tasks. */
#define mainSEM_TEST_PRIORITY				( tskIDLE_PRIORITY + ( UBaseType_t ) 1 )
#define mainFLOP_TASK_PRIORITY				( tskIDLE_PRIORITY )
#define mainCHECK_TASK_PRIORITY				( configMAX_PRIORITIES - ( UBaseType_t ) 1 )
#define mainQUEUE_OVERWRITE_PRIORITY		( tskIDLE_PRIORITY )

/* The period of the check task, in ms. */
#define mainNO_ERROR_CHECK_TASK_PERIOD		pdMS_TO_TICKS( ( TickType_t ) 5000)

/* The base period used by the timer test tasks. */
#define mainTIMER_TEST_PERIOD				( 50 )

/*CORTEX M3 DEFINE*/
#define mainMESSAGE_BUFFER_TASKS_STACK_SIZE	( 200 )
#define mainCREATOR_TASK_PRIORITY  ( configMAX_PRIORITIES - ( UBaseType_t ) 2 )

#define mainPOSIX_DEMO_PRIORITY    ( tskIDLE_PRIORITY + 4 )
/*-----------------------------------------------------------*/

/*
 * The check task, as described at the top of this file.
 */
static void prvCheckTask( void *pvParameters );
TaskHandle_t prvCheckTaskHandle;

/*
 * A high priority task that does nothing other than execute at a pseudo random
 * time to ensure the other test tasks don't just execute in a repeating
 * pattern.
 */
static void prvPseudoRandomiser( void *pvParameters );

/*
 *  The full demo uses the tick hook function to include test code in the tick
 *  interrupt.  vFullDemoTickHook() is called by vApplicationTickHook(), which
 *  is defined in main.c.
 */
void vFullDemoTickHook( void );

/*
 * UART Rx task test 
 */
void UARTInterruptHandler(void *data);
SemaphoreHandle_t xSemaphore = NULL;
void UartIrqTriggerTask(void *pvParameters);
unsigned char p_char;
/*-----------------------------------------------------------*/

void main_full( void )
{	
	printf( "%s", "This call from full main.\n" );

#if 0	
	Irq_SetupEntry(623, UARTInterruptHandler, NULL);
	Irq_SetPriority(623, IPRIORITY(2));
	Irq_Enable(623);
	
	xSemaphore = xSemaphoreCreateBinary();

	if (xSemaphore == NULL) {
        printf("Semaphore creation failed!\n");
    }
    else {
		printf("UART Interrupt is ready - Please type to RX terminal for testing\n");
        xTaskCreate(UartIrqTriggerTask, "UartIrqTriggerTask", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
    }
#endif
       /* Start all the other standard demo/test tasks.  They have no particular
	functionality, but do demonstrate how to use the FreeRTOS API and test the
	kernel port. */
	//vStartInterruptQueueTasks();
	vStartDynamicPriorityTasks();
	vCreateBlockTimeTasks();
	vStartCountingSemaphoreTasks();
	vStartGenericQueueTasks( tskIDLE_PRIORITY );
	vStartRecursiveMutexTasks();
	vStartSemaphoreTasks( mainSEM_TEST_PRIORITY );
	#if 0
	vStartMathTasks( mainFLOP_TASK_PRIORITY );
	#endif
	vStartEventGroupTasks();
	vStartTaskNotifyTask();
	vStartInterruptSemaphoreTasks();
	vStartStaticallyAllocatedTasks();
	vCreateAbortDelayTasks();
	vStartQueueOverwriteTask( mainQUEUE_OVERWRITE_PRIORITY );
	vStartTimerDemoTask( mainTIMER_TEST_PERIOD );
  
  /* The suicide tasks must be created last as they need to know how many
  tasks were running prior to their creation in order to ascertain whether
  or not the correct/expected number of tasks are running at any given time. */
  vCreateSuicidalTasks( mainCREATOR_TASK_PRIORITY );

#if 1
	/* CORTEX M3 QEMU */
	vStartQueuePeekTasks();
	vStartQueueSetTasks();
	vStartMessageBufferTasks( mainMESSAGE_BUFFER_TASKS_STACK_SIZE );
	vStartStreamBufferTasks();
	/*----------------*/
#endif

#ifdef configSUPPORT_POSIX
    xTaskCreate( vStartPOSIXDemo, "posix", configMINIMAL_STACK_SIZE, NULL, mainPOSIX_DEMO_PRIORITY, NULL );
#endif 

	/* Create the task that just adds a little random behaviour. */
	xTaskCreate( prvPseudoRandomiser, "Rnd", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 4, NULL );

	/* Create the task that performs the 'check' functionality,	as described at
	the top of this file. */
	xTaskCreate( prvCheckTask, "Check", configMINIMAL_STACK_SIZE, NULL, mainCHECK_TASK_PRIORITY, &prvCheckTaskHandle);

	/* Start the scheduler. */
	vTaskStartScheduler();

	/* If all is well, the scheduler will now be running, and the following
	line will never be reached.  If the following line does execute, then
	there was either insufficient FreeRTOS heap memory available for the idle
	and/or timer tasks to be created, or vTaskStartScheduler() was called from
	User mode.  See the memory management section on the FreeRTOS web site for
	more details on the FreeRTOS heap http://www.freertos.org/a00111.html.  The
	mode from which main() is called is set in the C start up code and must be
	a privileged mode (not user mode). */
	for( ;; );
}
/*-----------------------------------------------------------*/

static void prvCheckTask( void *pvParameters )
{
TickType_t xDelayPeriod = mainNO_ERROR_CHECK_TASK_PERIOD;
TickType_t xLastExecutionTime;
uint32_t ulErrorFound = pdFALSE;
const char *pcStatusString = "Pass";

	/* Just to stop compiler warnings. */
	( void ) pvParameters;

	/* Initialise xLastExecutionTime so the first call to vTaskDelayUntil()
	works correctly. */
	xLastExecutionTime = xTaskGetTickCount();

	/* Cycle for ever, delaying then checking all the other tasks are still
	operating without error.  The system status is written to the UART on each
	iteration. */
	for( ;; )
	{
		/* Delay until it is time to execute again. */
		vTaskDelayUntil( &xLastExecutionTime, xDelayPeriod );

		/* Check all the demo tasks (other than the flash tasks) to ensure
		that they are all still running, and that none have detected an error. */
		/*if( xAreIntQueueTasksStillRunning() != pdTRUE )
		{
			ulErrorFound |= 1UL << 0UL;
			pcStatusString = "Error: IntQ";
		}*/
		#if 0
		if( xAreMathsTaskStillRunning() != pdTRUE )
		{
			ulErrorFound |= 1UL << 1UL;
			pcStatusString = "Error: Math";
		}
		#endif
		if( xAreDynamicPriorityTasksStillRunning() != pdTRUE )
		{
			ulErrorFound |= 1UL << 2UL;
			pcStatusString = "Error: Dynamic";
		}

		if ( xAreBlockTimeTestTasksStillRunning() != pdTRUE )
		{
			ulErrorFound |= 1UL << 4UL;
			pcStatusString = "Error: Block Time";
		}

		if ( xAreGenericQueueTasksStillRunning() != pdTRUE )
		{
			ulErrorFound |= 1UL << 5UL;
			pcStatusString = "Error: Generic Queue";
		}

		if ( xAreRecursiveMutexTasksStillRunning() != pdTRUE )
		{
			ulErrorFound |= 1UL << 6UL;
			pcStatusString = "Error: Recursive Mutex";
		}

		if( xAreSemaphoreTasksStillRunning() != pdTRUE )
		{
			ulErrorFound |= 1UL << 8UL;
			pcStatusString = "Error: Semaphore";
		}

		if( xAreCountingSemaphoreTasksStillRunning() != pdTRUE )
		{
			ulErrorFound |= 1UL << 10UL;
			pcStatusString = "Error: Counting Semaphore";
		}

		if( xAreEventGroupTasksStillRunning() != pdPASS )
		{
			ulErrorFound |= 1UL << 12UL;
			pcStatusString = "Error: Event Group";
		}

		if( xAreTaskNotificationTasksStillRunning() != pdTRUE )
		{
			ulErrorFound |= 1UL << 13UL;
			pcStatusString = "Error: Task Notifications";
		}

		if( xAreInterruptSemaphoreTasksStillRunning() != pdTRUE )
		{
			ulErrorFound |= 1UL << 14UL;
			pcStatusString = "Error: Interrupt Semaphore";
		}

		if( xAreStaticAllocationTasksStillRunning() != pdTRUE )
		{
			ulErrorFound |= 1UL << 15UL;
			pcStatusString = "Error: Static Allocation";
		}

		if( xAreAbortDelayTestTasksStillRunning() != pdTRUE )
		{
			ulErrorFound |= 1UL << 16UL;
			pcStatusString = "Error: Abort Delay";
		}

		if( xIsQueueOverwriteTaskStillRunning() != pdTRUE )
		{
			ulErrorFound |= 1UL << 17UL;
			pcStatusString = "Error: Queue Overwrite";
		}

		if( xAreTimerDemoTasksStillRunning( xDelayPeriod ) != pdTRUE )
		{
			ulErrorFound |= 1UL << 18UL;
			pcStatusString = "Error: Timer Demo";
		}

		if( xAreStreamBufferTasksStillRunning() != pdTRUE ) 
		{
			ulErrorFound |= 1UL << 19UL;
			pcStatusString = "Error: Stream Buf";
		}

		if( xAreMessageBufferTasksStillRunning() != pdTRUE ) 
		{
			ulErrorFound |= 1UL << 20UL;
			pcStatusString = "Error: Msg Buf";
		}

		if( xIsCreateTaskStillRunning() != pdTRUE ) 
		{
			ulErrorFound |= 1UL << 21UL;
			pcStatusString = "Error: Create Task";
		}

		if( xAreQueuePeekTasksStillRunning() != pdTRUE )
		{
			ulErrorFound |= 1UL << 22UL;
			pcStatusString = "Error: Q Peek";
		}

		if( xAreQueueSetTasksStillRunning() != pdPASS ) 
		{
			ulErrorFound |= 1UL << 23UL;
			pcStatusString = "Error: Q Set";
		}

		/* Output the system status string. */
		/* Change your output method */
		#ifdef LOGGING_ENABLE
		  LogDebug( ( "%s, status code = %lu, tick count = %lu", pcStatusString, ulErrorFound, xTaskGetTickCount() ) );
		#else
		  printf( "%s, status code = %lu, tick count = %lu\r\n", pcStatusString, ulErrorFound, xTaskGetTickCount() );
		#endif

		//configASSERT( ulErrorFound == pdFALSE );
	}
}
/*-----------------------------------------------------------*/

static void prvPseudoRandomiser( void *pvParameters )
{
const uint32_t ulMultiplier = 0x015a4e35UL, ulIncrement = 1UL, ulMinDelay = pdMS_TO_TICKS( 95 );
volatile uint32_t ulNextRand = ( uint32_t ) &pvParameters, ulValue;

	/* This task does nothing other than ensure there is a little bit of
	disruption in the scheduling pattern of the other tasks.  Normally this is
	done by generating interrupts at pseudo random times. */
	for( ;; )
	{
		ulNextRand = ( ulMultiplier * ulNextRand ) + ulIncrement;
		ulValue = ( ulNextRand >> 16UL ) & 0xffUL;

		if( ulValue < ulMinDelay )
		{
			ulValue = ulMinDelay;
		}

		vTaskDelay( ulValue );

		while( ulValue > 0 )
		{
			__asm volatile( "NOP" );
			__asm volatile( "NOP" );
			__asm volatile( "NOP" );
			__asm volatile( "NOP" );
			ulValue--;
		}
	}
}
/*-----------------------------------------------------------*/

void vFullDemoTickHook( void )
{
	/* CORTEX M3 QEMU */

	/* Write to a queue that is in use as part of the queue set demo to
	demonstrate using queue sets from an ISR. */
	vQueueSetAccessQueueSetFromISR();

	/* Exercise stream buffers from interrupts. */
	vPeriodicStreamBufferProcessing();

	/*----------END CORTEX M3 QEMU----------*/

	/* The full demo includes a software timer demo/test that requires
	prodding periodically from the tick interrupt. */
	vTimerPeriodicISRTests();

	/* Call the periodic queue overwrite from ISR demo. */
	vQueueOverwritePeriodicISRDemo();

	/* Call the periodic event group from ISR demo. */
	vPeriodicEventGroupsProcessing();

	/* Call the ISR component of the interrupt semaphore test. */
	vInterruptSemaphorePeriodicTest();

	/* Call the code that 'gives' a task notification from an ISR. */
	xNotifyTaskFromISR();

	/* Test flop alignment in interrupts - calling printf from an interrupt
	is BAD! */
	#if( configASSERT_DEFINED == 0 )
	{
	char cBuf[ 20 ];
	UBaseType_t uxSavedInterruptStatus;

		uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();
		{
			sprintf( cBuf, "%1.3f", 1.234 );
		}
		portCLEAR_INTERRUPT_MASK_FROM_ISR( uxSavedInterruptStatus );

		configASSERT( strcmp( cBuf, "1.234" ) == 0 );
	}
	#endif /* configASSERT_DEFINED */
}

void UARTInterruptHandler(void *data) {
    (void)data;

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	console_getc(&p_char);
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
