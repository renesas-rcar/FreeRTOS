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

#include "interrupts.h"
#include "stdio.h"
#include "gpio/r_gpio.h"
#define main_GPIO_TASK_PRIORITY        ( tskIDLE_PRIORITY + 1 )

#include "pfc/r_pfc_api.h"

#include "device_tree.h"

/*-----------------------------------------------------------*/

/*
 * Configure the hardware as necessary to run this demo.
 */
static void prvSetupHardware( void );

static void prvGPIOTask( void *pvParameters );

static void gpioUserCallback(void *data);
/*-----------------------------------------------------------*/
/* Setup MDP_X5H pins to test AIACC 0 pins */
#define MDP_X5H_PIN_INPUT0      GPIO_PORT_05_PIN_7
#define MDP_X5H_PIN_OUTPUT0     GPIO_PORT_05_PIN_6
/* Setup MDP_X5H pins to test AIACC 1 pins */
#define MDP_X5H_PIN_INPUT1      GPIO_PORT_08_PIN_5
#define MDP_X5H_PIN_OUTPUT1     GPIO_PORT_08_PIN_4

/*
 * Declare some structs used for GPIO API.
 */
/**** Config GPIO output/input general mode ****/
gpio_pin_cfg_t g_gpio_pin_cfg0[2] =
{
    {
        .pin_cfg = GPIO_DIRECTION_INPUT,
        .pin = MDP_X5H_PIN_INPUT0
    },
    {
        .pin_cfg = GPIO_DIRECTION_OUTPUT,
        .pin = MDP_X5H_PIN_OUTPUT0
    }
};

gpio_pin_cfg_t g_gpio_pin_cfg1[2] =
{
    {
        .pin_cfg = GPIO_DIRECTION_INPUT,
        .pin = MDP_X5H_PIN_INPUT1
    },
    {
        .pin_cfg = GPIO_DIRECTION_OUTPUT,
        .pin = MDP_X5H_PIN_OUTPUT1
    }
};

gpio_cfg_t g_gpio_cfg[2] =
{
    {
        .number_of_pins = sizeof(g_gpio_pin_cfg0)/sizeof(g_gpio_pin_cfg0[0]),
        .p_pin_cfg_data = &g_gpio_pin_cfg0[0],
        .p_extend = NULL
    },
    {
        .number_of_pins = sizeof(g_gpio_pin_cfg1)/sizeof(g_gpio_pin_cfg1[0]),
        .p_pin_cfg_data = &g_gpio_pin_cfg1[0],
        .p_extend = NULL
    }
};

gpio_instance_ctrl_t g_gpio_instance_ctrl;

/*-----------------------------------------------------------*/
int main( void )
{
    /* Configure the hardware ready to run the demo. */
    prvSetupHardware();

    xTaskCreate( prvGPIOTask, "GPIO", configMINIMAL_STACK_SIZE, NULL, main_GPIO_TASK_PRIORITY, NULL );
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

static void prvGPIOTask( void *pvParameters )
{
    uint8_t ret;
    int i;
    gpio_level_t lv = GPIO_LEVEL_HIGH;
    gpio_level_t readLevel;

    /* Remove compiler warning about unused parameter. */
    ( void ) pvParameters;

    vTaskDelay(500);

    for(uint8_t inst = 0; inst < 2; inst++)
    {
        printf("\n********** MDP_X5H: Setup Output/Input pins to test AIACC %d  GPIO **********\n", inst);

        ret = R_GPIO_Open(&g_gpio_instance_ctrl, &g_gpio_cfg[inst]);
        printf("Open : ret = %d\n", ret);

        gpio_port_pin_t x5h_output_pin = g_gpio_cfg[inst].p_pin_cfg_data[1].pin;
        gpio_port_pin_t x5h_input_pin  = g_gpio_cfg[inst].p_pin_cfg_data[0].pin;

        ret  = R_GPIO_PinRead(&g_gpio_instance_ctrl, x5h_output_pin, &readLevel);
        printf("PinRead: Before MDP_X5H_PIN_OUTPUT = %d\n", readLevel);

        vTaskDelay(1000);
        printf("MDP_X5H: Send signal to AIACC %d\n", inst);

        ret = R_GPIO_PinWrite(&g_gpio_instance_ctrl, x5h_output_pin, GPIO_LEVEL_HIGH);
        ret = R_GPIO_PinRead(&g_gpio_instance_ctrl, x5h_output_pin, &readLevel);
        printf("PinRead: After MDP_X5H_PIN_OUTPUT = %d\n", readLevel);

        ret = R_GPIO_PinSetPull(&g_gpio_instance_ctrl, x5h_input_pin, GPIO_REQ_PULL_DOWN);
        printf("\nR_GPIO_PinSetPull: Down to MDP_X5H_PIN_INPUT ret = %d\n", ret);

        ret |= R_GPIO_PinRead(&g_gpio_instance_ctrl, x5h_input_pin, &readLevel);
        printf("PinRead: Before MDP_X5H_PIN_INPUT  = %d\n", readLevel);

        printf("MDP_X5H: Wait signal from AIACC %d\n", inst);

        while(readLevel != GPIO_LEVEL_HIGH)
            ret |= R_GPIO_PinRead(&g_gpio_instance_ctrl, x5h_input_pin, &readLevel);
        printf("PinRead: After MDP_X5H_PIN_INPUT  = %d\n", readLevel);

        vTaskDelay(500);

        printf("\nMDP_X5H: Send signal to AIACC %d\n", inst);
        for (int i = 0; i < 4; i++)
        {
            lv = !lv;
            printf("MDP_X5H_PIN_OUTPUT: %d\n", lv);
            ret = R_GPIO_PinWrite(&g_gpio_instance_ctrl, x5h_output_pin, lv);
            vTaskDelay(100);
        }

        ret = R_GPIO_Close(&g_gpio_instance_ctrl);
        printf("Close: ret = %d\n", ret);

    }

    for( ;; )
    {
    }
}

/*-----------------------------------------------------------*/

void gpioUserCallback(void *data) {
    gpio_instance_ctrl_t * p_instance_ctrl = (gpio_instance_ctrl_t *) data;
    printf("Handle GPIO interrupt\n");
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
