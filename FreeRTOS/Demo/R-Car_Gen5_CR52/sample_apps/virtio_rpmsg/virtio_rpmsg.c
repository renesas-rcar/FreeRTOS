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
#include "ucie/r_ucie.h"
#include "rcar_utils.h"

#include <openamp/open_amp.h>
#include <openamp/version.h>
#include <metal/alloc.h>
#include <metal/version.h>
#include <openamp/remoteproc.h>
#include <openamp/rpmsg_virtio.h>
#include "virtio/r_virtio.h"
#include "platform_rcar.h"
#include "rsc_table.h"

#include "device_tree.h"
#include "board.h"

#define RPMSG_SERV_NAME "rpmsg-client-sample"
#define VIRTIO_SERVICE_NAME         "rpmsg-client-sample"

#define RSC_MEM_PA_OFFSET           0x0000UL
#define RSC_MEM_SIZE                0x1000UL
#define VRING_MEM_PA_OFFSET         0x1000UL
#define VRING_MEM_OFFSET            0x4000UL
#define SHARED_BUF_PA_OFFSET        0x9000UL
#define SHARED_BUF_SIZE             0x40000UL

#define main_ucie_TASK_PRIORITY        (tskIDLE_PRIORITY + 1)
#define UCIE_RC_SIZE (configMINIMAL_STACK_SIZE * 20)

#define PIO_X5H_RC0_WRITE_DATA      0xFFCCFFCC
#define PIO_AIACCL_EP0_WRITE_DATA   0x68686868
#define PIO_X5H_RC1_WRITE_DATA      0x12345678
#define PIO_AIACCR_EP0_WRITE_DATA   0x87654321
#define PIO_AIACCL_RC1_WRITE_DATA   0x11111111
#define PIO_AIACCR_EP1_WRITE_DATA   0x22222222

static int rpmsg_endpoint_cb(struct rpmsg_endpoint *ept, void *data, size_t len,
                 uint32_t src, void *priv);
static void rpmsg_service_unbind(struct rpmsg_endpoint *ept);
static int rpmsg_endpoint_cb1(struct rpmsg_endpoint *ept, void *data, size_t len,
                 uint32_t src, void *priv);
/*-----------------------------------------------------------*/

/*
 * Configure the hardware as necessary to run this demo.
 */
static void prvSetupHardware( void );

static void ucie_comm_task( void *pvParameters );
static void ucie_comm_task1( void *pvParameters );

int main( void )
{
    /* Configure the hardware ready to run the demo. */
    prvSetupHardware();

    xTaskCreate(ucie_comm_task, "UCIe", UCIE_RC_SIZE, NULL, main_ucie_TASK_PRIORITY, NULL );
    xTaskCreate(ucie_comm_task1, "UCIe", UCIE_RC_SIZE, NULL, main_ucie_TASK_PRIORITY, NULL );
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

    printf("RPMSG FRONTEND\n");
    vTaskDelay(5000);

    uintptr_t rsc_table_address;
    int rsc_size = 0;
    st_virtio_endpoint_t *lept;
    e_mfis_channel_t ch = 0;
    rsc_table_address = (uintptr_t)get_resource_table(ch, &rsc_size);

    st_rsc_table_info_t rsc_table = {
        .rsc_mem_pa         = rsc_table_address + RSC_MEM_PA_OFFSET,
        .rsc_mem_size       = RSC_MEM_SIZE,
        .vring_mem_pa       = rsc_table_address + VRING_MEM_PA_OFFSET,
        .vring_mem_offset   = VRING_MEM_OFFSET,
        .shared_buf_pa      = rsc_table_address + SHARED_BUF_PA_OFFSET,
        .shared_buf_size    = SHARED_BUF_SIZE,
    };

    st_virtio_instance_ctrl_t *virtio_inst = NULL;
    lept = ( struct rpmsg_endpoint * ) pvPortMalloc( sizeof( struct rpmsg_endpoint ) );
    printf("R_VIRTIO_FE_Create\n");
    virtio_inst = R_VIRTIO_FE_Create(ch, &rsc_table);
    if(virtio_inst != NULL)
    {
        printf("R_VIRTIO_CreateEP\n");
        int ret = R_VIRTIO_CreateEP(virtio_inst, lept, RPMSG_SERV_NAME, rpmsg_endpoint_cb, rpmsg_service_unbind, NULL);
        if( ret == 0)
        {
            printf("R_VIRTIO_CreateEP success\n");
        }
    }
    else{
        printf("R_VIRTIO_FE_Create fail\n");
    }
    vTaskDelay(1000);
    printf("rpmsg_send\n");
    char msg0[16];
    for (int i = 0; i <100; i++)
    {
        snprintf(msg0, sizeof(msg0), "hello fe0 %d", i);
        rpmsg_send(lept, msg0, strlen(msg0));
        // vTaskDelay(100);
    }
    

    for(;;) {
        __asm__ volatile("nop");
    }
}

static int rpmsg_endpoint_cb(struct rpmsg_endpoint *ept, void *data, size_t len,
			     uint32_t src, void *priv)
{
	char payload[RPMSG_BUFFER_SIZE];
    (void)priv;
    (void)src;

    memset(payload, 0, RPMSG_BUFFER_SIZE);
    memcpy(payload, data, len);

    printf("Incoming msg: %s\r\n", payload);

    // rpmsg_send(ept, "fe mess", strlen("fe mess"));
    return RPMSG_SUCCESS;
}

static void rpmsg_service_unbind(struct rpmsg_endpoint *ept)
{
	(void)ept;
}

static void ucie_comm_task1(void *pvParameters)
{
    /* Remove compiler warning about unused parameter. */
    ( void ) pvParameters;

    uint32_t ret = 0;
    
    printf("RPMSG BECKEND\n");
    vTaskDelay(1000);

    // st_virtio_instance_ctrl_t *result = NULL;
    st_virtio_instance_ctrl_t *virtio_inst = NULL;
    st_virtio_endpoint_t *lept = ( struct rpmsg_endpoint * ) pvPortMalloc( sizeof( struct rpmsg_endpoint ) );
    e_mfis_channel_t ch = 0;
  
    printf("R_VIRTIO_BE_Create\n");
    virtio_inst = R_VIRTIO_BE_Create(ch);
    if(virtio_inst != NULL)
    {

        printf("R_VIRTIO_CreateEP\n");
        int ret = R_VIRTIO_CreateEP(virtio_inst, lept, VIRTIO_SERVICE_NAME, rpmsg_endpoint_cb1, rpmsg_service_unbind, NULL);
        if( ret == 0)
        {
            printf("R_VIRTIO_CreateEP success\n");
        }
        else
        {
            printf("R_VIRTIO_CreateEP failed\n");
        }
    }
    else{
        printf("R_VIRTIO_BE_Create fail\n");
    }

    for(;;) {
        vTaskDelay(5000);
        __asm__ volatile("nop");
    }
}

static int rpmsg_endpoint_cb1(struct rpmsg_endpoint *ept, void *data, size_t len,
			     uint32_t src, void *priv)
{
	char payload[RPMSG_BUFFER_SIZE];
    (void)priv;
    (void)src;
    static int i = 0;
    memset(payload, 0, RPMSG_BUFFER_SIZE);
    memcpy(payload, data, len);

    printf("Incoming msg0: %s\r\n", payload);

    return RPMSG_SUCCESS;
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
