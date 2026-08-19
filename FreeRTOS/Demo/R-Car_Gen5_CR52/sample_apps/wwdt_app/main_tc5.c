/*
 * Copyright (c) 2026 Renesas Electronics Corporation
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

#include "device_tree.h"

#include "ecm/r_ecm.h"
#include "ecm/r_error_domain_id.h"
#include "../drivers/ecm/r_ecm_reg.h"
#include "watchdog/r_wwdt_api.h"

#include "board.h"

#if (BOARD == X5H_VDK) || (BOARD == X5H_IRONHIDE) || (BOARD == X5H_RFS2) || (BOARD == MDP_X5H_HIL)
#define WWDT_UNIT   R_WWDT20
#define WWDT_ECM_NMI_OUTPUT   WWDT20_DETECTS_ERROR_NMI_IS_OUTPUT
#elif (BOARD == MDP_AIACC_RFS2) || (BOARD == MDP_AIACC_HIL)
#define WWDT_UNIT   R_WWDT2
#define WWDT_ECM_NMI_OUTPUT   WWDT2_DETECTS_ERROR_NMI_IS_OUTPUT
#endif

#define STR(x) #x
#define XSTR(x) STR(x)

#define main_ECM_TASK_PRIORITY        ( tskIDLE_PRIORITY + 1 )

#define BUSECMSAFINTENCMN00                 (uint32_t)(0xCA4506C0U)
#define BUSECMSAFINTENMM00                  (uint32_t)(0xE9A206C0U)
#define BUSECMSAFINTENPERW00                (uint32_t)(0xC05906C0U)
#define BUSECMSAFINTENRT00                  (uint32_t)(0x1A8106C0U)
#define BUSECMSAFINTENSCP00                 (uint32_t)(0xC12906C0U)
#define BUSECMSAFINTENTOP00                 (uint32_t)(0xC68106C0U)
#define printf_delay(fmt, ...)      \
    vTaskDelay(1);		    \
printf(fmt, ##__VA_ARGS__);         \

/*-----------------------------------------------------------*/

/*
 * Configure the hardware as necessary to run this demo.
 */
static void prvSetupHardware( void );
static void IrqErrorHandler(e_ecm_error_id_t id);
static void prvEcmTask( void *pvParameters );

SemaphoreHandle_t xSemaphore = NULL;
/*-----------------------------------------------------------*/
int main( void )
{
    uint32_t irqID;

    /* Configure the hardware ready to run the demo. */
    prvSetupHardware();
 
    xSemaphore = xSemaphoreCreateBinary();

    xTaskCreate( prvEcmTask, "prvWWDTTask", configMINIMAL_STACK_SIZE, NULL, main_ECM_TASK_PRIORITY, NULL);
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

static void prvEcmTask( void *pvParameters )
{
    /* Remove compiler warning about unused parameter. */
    ( void ) pvParameters;
    uint32_t err;
    uint8_t ret;

#if (BOARD == MDP_X5H_HIL) || (BOARD == MDP_AIACC_HIL)
    vTaskDelay(6000);
#endif

    printf("---------- ECM SETTING START ----------- \r\n");
    printf("R_ECM_DisableAll\n");
    ret = R_ECM_DisableAll();
    if (ret)
    {
        printf("Result: Failed\n");
    }
    else
    {
        printf("Result: OK\n");
    }
    
    printf("R_ECM_SetInterruptCallback\n");
    ret = R_ECM_SetInterruptCallback(IrqErrorHandler);
    if (ret)
    {
        printf("Result: Failed\n");
    }
    else
    {
        printf("Result: OK\n");
    }

    printf("R_ECM_SetDetection\n");
    R_ECM_SetDetection(WWDT_ECM_NMI_OUTPUT, 1);
    if (ret)
    {
        printf("Result: Failed\n");
    }
    else
    {
        printf("Result: OK\n");
    }

    printf("R_ECM_SetInterruptNotification\n");
    R_ECM_SetInterruptNotification(WWDT_ECM_NMI_OUTPUT, 1);
    if (ret)
    {
        printf("Result: Failed\n");
    }
    else
    {
        printf("Result: OK\n");
    }

    printf("---------- ECM SETTING END ----------- \r\n");

    printf("---------- PROGRAM START ----------- \r\n");

    printf("=== Test Case 5: %s, 68ms, 100% window OPEN, NMI MODE ===\r\n", XSTR(WWDT_UNIT));
    /* Device driver part */
    R_WWDT_Init(WWDT_UNIT, WINDOW_100P, 68, false, ERM_NMI_MODE);
    err = R_WWDT_Refresh(WWDT_UNIT);

    if(xSemaphoreTake(xSemaphore, 2000) == pdTRUE)
    {
        printf("Result: Passed\n");
    }
    else
    {
        printf("Result: Failed\n");
    }

    printf("<APP_END>\n");
    
    for( ;; )
    {
        vTaskDelay(1);
    }

}

/*-----------------------------------------------------------*/
void IrqErrorHandler(e_ecm_error_id_t id)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    
    xSemaphoreGiveFromISR(xSemaphore, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
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
