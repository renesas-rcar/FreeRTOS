/*
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * This is a sample demonstration application that showcases usage of rpmsg
 * This application is meant to run on the remote CPU running baremetal code.
 * This application echoes back data that was sent to it by the host core.
 */

#include <stdio.h>
#include <errno.h>
#include <openamp/open_amp.h>
#include <openamp/version.h>
#include <metal/alloc.h>
#include <metal/version.h>
#include "FreeRTOS.h"
#include "interrupts.h"
#include "platform_info.h"
#include "rsc_table.h"
#include "pfc/r_pfc_api.h"

#define RPMSG_SERVICE_NAME         "rpmsg-client-sample"
#define SHUTDOWN_MSG    0xEF56A55A

#define LPRINTF(format, ...) printf(format, ##__VA_ARGS__); vTaskDelay(10);
//#define LPRINTF(format, ...)
#define LPERROR(format, ...) LPRINTF("ERROR: " format, ##__VA_ARGS__)

static struct rpmsg_endpoint lept;
static int shutdown_req = 0;
int is_print_result = 1;

#define hello_msg "Hello world from CR52/Free-RTOS!"
#define goodbye_msg "Good bye!"

/*-----------------------------------------------------------------------------*
 *  RPMSG endpoint callbacks
 *-----------------------------------------------------------------------------*/
static int rpmsg_endpoint_cb(struct rpmsg_endpoint *ept, void *data, size_t len,
                 uint32_t src, void *priv)
{
    char payload[RPMSG_BUFFER_SIZE];
    (void)priv;
    (void)src;

    memset(payload, 0, RPMSG_BUFFER_SIZE);
    memcpy(payload, data, len);

    /* On reception of a shutdown we signal the application to terminate */
    if ((*(unsigned int *)data) == SHUTDOWN_MSG) {
        LPRINTF("shutdown message is received.\r\n");
        shutdown_req = 1;
        return RPMSG_SUCCESS;
    }

    if(is_print_result == 1)
    {
        LPRINTF("TC1 result: PASS\r\n");
        LPRINTF("TC2: If the TC1 OK log is found on both Core 0 and Core 1, judge as OK. Otherwise, judge as NG.\r\n");
        LPRINTF("<APP_END>\r\n");
        is_print_result = 0;
    }
    LPRINTF("Incoming msg: %s\r\n", payload);

    rpmsg_send(ept, hello_msg, strlen(hello_msg));

    return RPMSG_SUCCESS;
}

static void rpmsg_service_unbind(struct rpmsg_endpoint *ept)
{
    (void)ept;
    LPRINTF("unexpected Remote endpoint destroy\r\n");
    shutdown_req = 1;
}

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
    void *platform;
    struct rpmsg_device *rpdev;

    LPRINTF("openamp lib version: %s (", openamp_version());
    LPRINTF("Major: %d, ", openamp_version_major());
    LPRINTF("Minor: %d, ", openamp_version_minor());
    LPRINTF("Patch: %d)\r\n", openamp_version_patch());

    LPRINTF("libmetal lib version: %s (", metal_ver());
    LPRINTF("Major: %d, ", metal_ver_major());
    LPRINTF("Minor: %d, ", metal_ver_minor());
    LPRINTF("Patch: %d)\r\n", metal_ver_patch());

    LPRINTF("Starting application...\r\n");

    /* Initialize platform */
    ret = platform_init(MFIS_CHAN, &platform);
    if (ret) {
        LPERROR("Failed to initialize platform.\r\n");
        ret = -1;
    } else {
        rpdev = platform_create_rpmsg_vdev(platform, 0,
                           VIRTIO_DEV_DEVICE,
                           NULL, NULL);
        if (!rpdev) {
            LPERROR("Failed to create rpmsg virtio device.\r\n");
            ret = -1;
        }
    }

    /* Initialize RPMSG framework */
    LPRINTF("Try to create rpmsg endpoint.\r\n");

    ret = rpmsg_create_ept(&lept, rpdev, RPMSG_SERVICE_NAME,
                   RPMSG_ADDR_ANY, RPMSG_ADDR_ANY,
                   rpmsg_endpoint_cb,
                   rpmsg_service_unbind);
    if (ret) {
        LPERROR("Failed to create endpoint.\r\n");
        goto task_end;
    }

    LPRINTF("Successfully created rpmsg endpoint.\r\n");

    LPRINTF("RPMsg device TX buffer size: %#x\r\n", rpmsg_get_tx_buffer_size(&lept));
    LPRINTF("RPMsg device RX buffer size: %#x\r\n", rpmsg_get_rx_buffer_size(&lept));

    LPRINTF("TC1: Check data tranfer.\r\n");
    int time_out_test = 10000;
    while(1) {
        platform_poll(platform);
        vTaskDelay(1);
        time_out_test--;
        if(time_out_test == 0)
        {
            if(is_print_result == 1)
            {
                LPRINTF("TC1 result: FAIL\r\n");
            }     
        }
        /* we got a shutdown request, exit */
        if (shutdown_req) {
            break;
        }
    }

    LPRINTF("Stopping application...\r\n");
    rpmsg_destroy_ept(&lept);
    platform_release_rpmsg_vdev(rpdev, platform);
    platform_cleanup(platform);

    goto task_end;
task_end:
    while(1)
    {
        vTaskDelay(10);
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
