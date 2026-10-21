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

#include "pfc/r_pfc_api.h"
#include "virtio-iommu-backend/r_virtio_iommu_backend.h"
#include <stdio.h>

/*----------------------------------------------------------------------------*/
static void prvSetupHardware( void )
{
    /* Ensure no interrupts execute while the scheduler is in an inconsistent
    state.  Interrupts are automatically enabled when the scheduler is
    started. */
    portDISABLE_INTERRUPTS();

    Irq_Setup();

    (void)pfcInitModules(getModuleConfigs());
}

/*-----------------------------------------------------------------------------*
 *  Application
 *-----------------------------------------------------------------------------*/
void echoTask( void *pvParameters )
{
    /* Remove compiler warning about unused parameter. */
    ( void ) pvParameters;
    int ret;
    printf("VIRTIO IOMMU Backend:  Starting Virtio sample\r\n");
    virtio_iommu_instance_ctrl_t *virtio_iommu_inst;
    e_mfis_channel_t mfis_ch;
    if (MFIS_CHAN == 0)
    {
        mfis_ch = MFIS_CR_TO_CA_CH0;
    }
    else if(MFIS_CHAN == 1)
    {
        mfis_ch = MFIS_CR_TO_CA_CH1;
    }
    else
    {
        printf("VIRTIO IOMMU Backend:  MFIS Channel not support\r\n");
    }
    

    printf("VIRTIO IOMMU Backend:  TC1: Virtio IOMMU Create. Waiting for connection ...\r\n");
    virtio_iommu_inst = R_VIRTIO_IOMMU_Backend_Init(mfis_ch);
    if(virtio_iommu_inst == NULL)
    {
        printf("VIRTIO IOMMU Backend: TC1 Result: Failed\r\n");
    }
    else
    {
        printf("VIRTIO IOMMU Backend: TC1 Result: Passed\r\n");
    };

    for( ;; )
    {
        vTaskDelay(1);
    }
}


/*-----------------------------------------------------------------------------*
 *  Application entry point
 *-----------------------------------------------------------------------------*/
int main(void)
{

    /* Configure the hardware ready to run the demo. */
    prvSetupHardware();

    xTaskCreate( echoTask, "echoTask", configMINIMAL_STACK_SIZE, NULL, ( tskIDLE_PRIORITY + 1 ), NULL );
    /* Start the tasks and timer running. */
    vTaskStartScheduler();
    for( ;; )
    {
    }
    /* Don't expect to reach here. */

    return 0;
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
