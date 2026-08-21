/*
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 */

/**
 * @brief This sample is for demonstrate rpmsg communicate via ucie
 *        between two CR core of chiplet. And this is for core run
 *        with UCIe EP mode.
 */

#include <stdio.h>
#include <errno.h>
#include <openamp/open_amp.h>
#include <openamp/version.h>
#include <metal/alloc.h>
#include <metal/version.h>
#include "FreeRTOS.h"
#include "task.h"
#include "interrupts.h"
#include "platform_info.h"
#include "platform_info_common.h"
#include "rsc_table.h"
#include "pfc/r_pfc_api.h"
#include "ucie/r_ucie.h"
#include "board.h"

#ifndef UCIE_CH
#define UCIE_CH UCIE_CH0
#endif

#define RPMSG_SERVICE_NAME         "rpmsg-client-sample"
#define SHUTDOWN_MSG    0xEF56A55A

static struct rpmsg_endpoint lept;
static int shutdown_req = 0;

#define hello_msg "Hello world from EP CR52/Free-RTOS!"
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
        printf("shutdown message is received.\r\n");
        shutdown_req = 1;
        return RPMSG_SUCCESS;
    }

    printf("Incoming msg: %s\r\n", payload);

    rpmsg_send(ept, hello_msg, strlen(hello_msg));

    return RPMSG_SUCCESS;
}

static void rpmsg_service_unbind(struct rpmsg_endpoint *ept)
{
    (void)ept;
    printf("unexpected Remote endpoint destroy\r\n");
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

#if (UCIE_CH != UCIE_CH0)
    R_UCIE_Config(UCIE_CH, UCIE_MODE_EP, LINKSPEED_4GTPS, true);
#endif
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

    void *rsc_table;
    int len;

    printf("openamp lib version: %s (", openamp_version());
    printf("Major: %d, ", openamp_version_major());
    printf("Minor: %d, ", openamp_version_minor());
    printf("Patch: %d)\r\n", openamp_version_patch());

    printf("libmetal lib version: %s (", metal_ver());
    printf("Major: %d, ", metal_ver_major());
    printf("Minor: %d, ", metal_ver_minor());
    printf("Patch: %d)\r\n", metal_ver_patch());

    rsc_table = get_resource_table(0, &len);

    /* Initialize platform */
    ret = platform_init(NO_USING_MFIS, &platform);
    if (ret) {
        printf("Failed to initialize platform.\r\n");
        goto err1;
    } else {
        rpdev = platform_create_rpmsg_vdev(platform, 0, VIRTIO_DEV_DEVICE, NULL, NULL);
        if (!rpdev) {
            printf("Failed to create rpmsg virtio device.\r\n");
            goto err2;
        }
    }

    /* Initialize RPMSG framework */
    printf("Try to create rpmsg endpoint.\r\n");

    ret = rpmsg_create_ept(&lept, rpdev, RPMSG_SERVICE_NAME,
                   RPMSG_ADDR_ANY, RPMSG_ADDR_ANY,
                   rpmsg_endpoint_cb,
                   rpmsg_service_unbind);
    if (ret) {
        printf("Failed to create endpoint.\r\n");
        goto err3;
    }

    printf("Successfully created rpmsg endpoint.\r\n");

    printf("RPMsg device TX buffer size: %#x\r\n", rpmsg_get_tx_buffer_size(&lept));
    printf("RPMsg device RX buffer size: %#x\r\n", rpmsg_get_rx_buffer_size(&lept));

    while(1) {
        platform_poll(platform);
        vTaskDelay(1);
        /* we got a shutdown request, exit */
        if (shutdown_req) {
            break;
        }
    }

    rpmsg_destroy_ept(&lept);
err3:
    platform_release_rpmsg_vdev(rpdev, platform);
err2:
    platform_cleanup(platform);
err1:
    printf("Stopping application...\r\n");
    while(1)
    {
        printf("Task loop\n");
        vTaskDelay(1000);
    }
}

/*-----------------------------------------------------------------------------*
 *  Application entry point
 *-----------------------------------------------------------------------------*/
int main(void)
{

    /* Configure the hardware ready to run the demo. */
    prvSetupHardware();

    xTaskCreate( echoTask, "echoTask", configMINIMAL_STACK_SIZE*100, NULL, ( tskIDLE_PRIORITY + 1 ), NULL );
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
