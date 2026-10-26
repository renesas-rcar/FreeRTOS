/*
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 */

#include "FreeRTOS.h"
#include "task.h"
#include "interrupts.h"

#include "pfc/r_pfc_api.h"
#include <stdio.h>
#include <string.h>

#include "rcar-rpmsg/rcar_rpmsg.h"

#include "board.h"

/*
 * FE talking to rpmsg-echo.c on the other core (vdev 0, MFIS 0).
 * echo.c replies with a raw string, not an rcar_msg_t frame: the received
 * text is shifted.
 */

/* Logging Function include. */
#include "logging_stack.h"

#define RPMSG_SERVICE_NAME     "rpmsg-client-sample" /* must match echo.c */
#define FE_DRIVER_ID            RCAR_RPMSG_ID_0  /* vdev 0, as echo.c */

#define FE_STARTUP_DELAY_MS      1000U    /* let echo.c write the rsc table first */

#define SEND_PERIOD_MS           2000U
#define RECEIVE_TIMEOUT_MS       1000U

#define main_TASK_PRIORITY      (tskIDLE_PRIORITY + 1)
#define MAIN_TASK_STACK_SIZE    (configMINIMAL_STACK_SIZE * 10)

static void prvSetupHardware(void);
static void echo_client_task(void *pvParameters);

int main(void)
{
    prvSetupHardware();

    xTaskCreate(echo_client_task, "ECHO_FE", MAIN_TASK_STACK_SIZE, NULL,
                main_TASK_PRIORITY, NULL);

    vTaskStartScheduler();
    for (;;) {
    }
    return 0;
}

static void prvSetupHardware(void)
{
    portDISABLE_INTERRUPTS();
    Irq_Setup();
    (void)pfcInitModules(getModuleConfigs());
}

static void echo_client_task(void *pvParameters)
{
    (void)pvParameters;

    vTaskDelay(pdMS_TO_TICKS(FE_STARTUP_DELAY_MS));

    static rcar_rpmsg_channel_t ch;
    rcar_rpmsg_open_cfg_t cfg = {
        .driver_id = FE_DRIVER_ID,
        .side      = RCAR_RPMSG_SIDE_FE,
        .name      = RPMSG_SERVICE_NAME,
    };
    if (rcar_rpmsg_open(&ch, cfg) != RCAR_OK) {
        LogDebug(("[FE] rcar_rpmsg_open FAILED\n"));
        vTaskDelete(NULL);
        return;
    }
    LogDebug(("[FE] endpoint ready\n"));

    const char *msg = "hello";
    for (;;) {
        e_rcar_status_t rc = rcar_rpmsg_send(&ch, msg, (uint16_t)strlen(msg));
        if (rc == RCAR_OK) {
            LogDebug(("[FE] sent: %s\n", msg));
        } else if (rc == RCAR_ERR_NOT_CONNECTED) {
            LogDebug(("[FE] echo not bound yet, retry\n"));
        } else {
            LogDebug(("[FE] send FAILED (%d)\n", (int)rc));
        }

        char payload[RCAR_RPMSG_MAX_PAYLOAD + 1];
        uint16_t n = 0U;
        rc = rcar_rpmsg_receive(&ch, payload, RCAR_RPMSG_MAX_PAYLOAD, &n, RECEIVE_TIMEOUT_MS);
        if ((rc == RCAR_OK) || (rc == RCAR_ERR_PAYLOAD_TOO_LARGE)) {
            payload[n] = '\0';
            LogDebug(("[FE] received: %s\n", payload));
        } else if (rc == RCAR_ERR_TIMEOUT) {
            LogDebug(("[FE] no reply within %ums\n", RECEIVE_TIMEOUT_MS));
        } else {
            LogDebug(("[FE] receive FAILED (%d)\n", (int)rc));
        }

        vTaskDelay(pdMS_TO_TICKS(SEND_PERIOD_MS));
    }
}

int printf_raw(const char *format, ...);

void vMainAssertCalled(const char *pcFileName, uint32_t ulLineNumber)
{
    printf_raw("ASSERT!  Line %d of file %s\n", ulLineNumber, pcFileName);
    taskENTER_CRITICAL();
    for (;;) {
    }
}

void vDeleteCallingTask(void)
{
    vTaskDelete(NULL);
}