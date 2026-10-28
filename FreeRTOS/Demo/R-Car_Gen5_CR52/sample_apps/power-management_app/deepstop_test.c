/*
 *
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include "FreeRTOS.h"
#include "task.h"
#include "interrupts.h"
#include "state-manager/r_state_manager.h"
#include "state-manager/r_power_domain_id.h"
#include "state-manager/r_clock_domain_id.h"
#include "state-manager/r_reset_domain_id.h"
#include "pfc/r_pfc_api.h"
#include "cmsis_rcar_gen5.h"
#include "serial/r_serial.h"

#define CPUID (__get_MPIDR() & 0xFF)
#define UARTID UART_ID

#define pmApp_TASK_PRIORITY ( tskIDLE_PRIORITY + 1 )
#define PM_LOG(format, ...) \
    {\
        printf("PM [%s:%d] ", __func__, __LINE__);\
        printf(format "\r\n", ##__VA_ARGS__);\
    }

unsigned char p_char;

static void prvPmAppTask(void *pvParameters);
static void pmAppExample(void);

void UARTInterruptHandler(void *data);

static void prvSetupHardware(void)
{
    portDISABLE_INTERRUPTS();
    if(1 == CPUID)
    {
        R_SERIAL_PortInit(UARTID);
    }
    
    Irq_Setup();
    (void)pfcInitModules(getModuleConfigs());
}

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

void main()
{

    prvSetupHardware();

    if(1 == CPUID)
    {
        uint32_t irqID;
        irqID = getIrqID(UARTID);

        if (irqID == 0) {
            printf("Not support UARTID = %d\n", UARTID);
        } else {
            /* Set Handler for Irq */
            Irq_SetupEntry(irqID, UARTInterruptHandler, NULL);

            /* Set priority for Irq */
            Irq_SetPriority(irqID, IPRIORITY(2));

            /* Enable Irq */
            Irq_Enable(irqID);
        }
    }

	xTaskCreate(prvPmAppTask, "powerManagementApp", configMINIMAL_STACK_SIZE * 10,
				NULL, pmApp_TASK_PRIORITY, NULL);

    /* Start the tasks and timer running. */
    vTaskStartScheduler();
    for( ;; )
    {
    }
	/* Don't expect to reach here. */
	return;
}

static void prvPmAppTask(void *pvParameters )
{
    /* Remove compiler warning about unused parameter. */
    (void) pvParameters;

    vTaskDelay(10000);
    if (1 == CPUID) {
        // This is to avoid logs mixing between 2 freertos
        vTaskDelay(10000);
    }
	PM_LOG("[CPU%d] PowerManagement FreeRTOS starting...\n", CPUID);
	pmAppExample();

    for( ;; )
    {
        PM_LOG("[CPU%d] prvPmAppTask...\n", CPUID);
        vTaskDelay(3000);
    }
}

static void pmAppExample(void)
{
	int ret;
	int domain_id;
    int tc_number = 0;
    uint32_t rates[2] = {0};

    PM_LOG("*******TC%d: SCMI protocols information starting*******\r\n",
            ++tc_number);
	ret = R_StateManager_SCMI_Info_Show();
	if (ret) {
		PM_LOG("Error: Failed to show SCMI information.\r\n");
		return;
	}
    PM_LOG("*******TC%d: SCMI protocols information  end!*******\r\n\r\n",
            tc_number);

    if (0 == CPUID) {
#define RAM_EVENT_BASE  (*(volatile uint32_t *)(0xC131F8C0U))
#define DEEPSTOP_START ((uint32_t)0x1)
        uint32_t val = 0;

        // Clear ram value first
        RAM_EVENT_BASE = 0x0;
        PM_LOG("Main FreeRTOS is waiting signal from 2nd FRTOS to start DeepStop...\r\n");
        for (;;) {
            val = RAM_EVENT_BASE;
            if (DEEPSTOP_START == val) {
                PM_LOG("Got signal from 2nd FRTOS!\r\n");
                break;
            }
            vTaskDelay(1000);
        }
        PM_LOG("******* SCMI System DeepStop starting!*******\r\n");
        ret = R_StateManager_RequestDeepStop();
        if (ret) {
            PM_LOG("Error: Failed to request DeepStop.\r\n");
            return;
        }
    } else if (1 == CPUID) {
#define RAM_EVENT_BASE  (*(volatile uint32_t *)(0xC131F8C0U))
#define DEEPSTOP_START ((uint32_t)0x1)

        PM_LOG("2nd FRTOS Please enter \"a\" to trigger deepstop from Main FRTOS\r\n");
        for (;;) {
            if(p_char == 'a')
            {
                RAM_EVENT_BASE = DEEPSTOP_START;
                p_char = 0;
                PM_LOG("2nd FRTOS is trigger event success...\r\n");
                break;
            }

            vTaskDelay(1000);
        }
        
    }
}

void UARTInterruptHandler(void *data) {
    (void)data;
    R_SERIAL_GetChar(&p_char);
    printf("UARTInterruptHandler: %c\n", p_char);
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

