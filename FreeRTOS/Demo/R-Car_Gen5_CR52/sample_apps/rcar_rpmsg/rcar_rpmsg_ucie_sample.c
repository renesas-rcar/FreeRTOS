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
#include "ucie/r_ucie.h"

#include "board.h"

/*
 * Same source on both chips, role by BOARD:
 *   X5H   (UCIe RC): FE, sends "hello ucie N" (N = 0..ECHO_COUNT-1),
 *                    checks each echo
 *   AIACC (UCIe EP): BE, echoes every message back; the shared memory is
 *                    in this chip's DRAM
 * Test case TC5: result printed by the X5H; the AIACC prints "Failed" only
 * if its open/receive/echo fails.
 * RCAR_RPMSG_ID_6 goes over UCIe CH0: rcar_rpmsg does the SMMU and the iATU
 * on the X5H. Link-up: ucie0_setup_task of each chip (X5H: R_UCIE_Config()
 * below; AIACC: default config), or the X5H CA core.
 */

/* Logging Function include. */
#include "logging_stack.h"

#if ((BOARD == MDP_X5H_HIL) || (BOARD == X5H_RFS2))
#define UCIE_SIDE_FE            1
#else
#define UCIE_SIDE_FE            0
#endif

#define ECHO_DRIVER_ID          RCAR_RPMSG_ID_6
#define ECHO_COUNT              20U
#define TC_UCIE                 5

#define FE_STARTUP_DELAY_MS     1000U   /* let the UART flush the start-up log */
#define OPEN_RETRY_MS           1000U
#define SEND_PERIOD_MS          1000U
#define RECEIVE_TIMEOUT_MS      1000U
#define SEND_RETRY_MS           1U

#define main_TASK_PRIORITY      (tskIDLE_PRIORITY + 1)
#define MAIN_TASK_STACK_SIZE    (configMINIMAL_STACK_SIZE * 10)

static void prvSetupHardware(void);
static void print_result(int tc, bool passed);
#if UCIE_SIDE_FE
static void echo_fe_task(void *pvParameters);
#else
static void echo_be_task(void *pvParameters);
#endif

int main(void)
{
    prvSetupHardware();

#if UCIE_SIDE_FE
    LogDebug(("=== X5H : FRONTEND (UCIe RC) ===\n"));
    xTaskCreate(echo_fe_task, "ECHO_FE", MAIN_TASK_STACK_SIZE, NULL,
                main_TASK_PRIORITY, NULL);
#else
    LogDebug(("=== AIACC : BACKEND (UCIe EP) ===\n"));
    xTaskCreate(echo_be_task, "ECHO_BE", MAIN_TASK_STACK_SIZE, NULL,
                main_TASK_PRIORITY, NULL);
#endif

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
#if UCIE_SIDE_FE
    /* CR runs without the CA core: ucie0_setup_task links CH0 up. It reads
     * this once when it starts, so before the scheduler. */
    (void)R_UCIE_Config(UCIE_CH0, UCIE_MODE_RC, LINKSPEED_4GTPS, true);
#endif
}

#if UCIE_SIDE_FE

static void echo_fe_task(void *pvParameters)
{
    (void)pvParameters;

    LogDebug(("[RC] task started, open in %ums\n", FE_STARTUP_DELAY_MS));
    vTaskDelay(pdMS_TO_TICKS(FE_STARTUP_DELAY_MS));

    LogDebug(("TC%d: Message transmission and reception between the CR core on X5H "
              "and the CR core on AIACC0 across UCIE.\n", TC_UCIE));
    /* Links up and waits for the EP; times out 10 s after link-up: retry. */
    static rcar_rpmsg_channel_t ch;
    rcar_rpmsg_open_cfg_t cfg = {
        .driver_id = ECHO_DRIVER_ID,
        .side      = RCAR_RPMSG_SIDE_FE,
    };
    e_rcar_status_t rc;
    while ((rc = rcar_rpmsg_open(&ch, cfg)) != RCAR_OK) {
        LogDebug(("[RC] rcar_rpmsg_open failed (%d), retry\n", (int)rc));
        vTaskDelay(pdMS_TO_TICKS(OPEN_RETRY_MS));
    }
    LogDebug(("[RC] endpoint ready\n"));

    uint32_t ok = 0U;
    for (uint32_t i = 0U; i < ECHO_COUNT; ) {
        char msg[RCAR_RPMSG_MAX_PAYLOAD];
        int len = snprintf(msg, sizeof(msg), "hello ucie %lu", (unsigned long)i);

        rc = rcar_rpmsg_send(&ch, msg, (uint16_t)len);
        if (rc == RCAR_ERR_NOT_CONNECTED) {
            LogDebug(("[RC] EP not bound yet, retry\n"));
            vTaskDelay(pdMS_TO_TICKS(SEND_PERIOD_MS));
            continue;
        }
        if (rc != RCAR_OK) {
            LogDebug(("[RC] send FAILED (%d)\n", (int)rc));
            vTaskDelay(pdMS_TO_TICKS(SEND_PERIOD_MS));
            continue;
        }
        LogDebug(("[RC] sent: %s\n", msg));

        char reply[RCAR_RPMSG_MAX_PAYLOAD + 1];
        uint16_t n = 0U;
        rc = rcar_rpmsg_receive(&ch, reply, RCAR_RPMSG_MAX_PAYLOAD, &n, RECEIVE_TIMEOUT_MS);
        if (rc == RCAR_OK) {
            reply[n] = '\0';
            bool match = (n == (uint16_t)len) && (memcmp(reply, msg, n) == 0);
            ok += match ? 1U : 0U;
            LogDebug(("[RC] received: %s (%s)\n", reply, match ? "OK" : "MISMATCH"));
        } else if (rc == RCAR_ERR_TIMEOUT) {
            LogDebug(("[RC] no reply within %ums\n", RECEIVE_TIMEOUT_MS));
        } else {
            LogDebug(("[RC] receive FAILED (%d)\n", (int)rc));
        }

        i++;
        vTaskDelay(pdMS_TO_TICKS(SEND_PERIOD_MS));
    }

    LogDebug(("=== RC RESULT: %lu/%u echoed ===\n", (unsigned long)ok, ECHO_COUNT));
    print_result(TC_UCIE, ok == ECHO_COUNT);
    vTaskDelete(NULL);
}

#else /* !UCIE_SIDE_FE */

static void echo_be_task(void *pvParameters)
{
    (void)pvParameters;

    /* Blocks until the X5H (FE) is up. */
    static rcar_rpmsg_channel_t ch;
    rcar_rpmsg_open_cfg_t cfg = {
        .driver_id = ECHO_DRIVER_ID,
        .side      = RCAR_RPMSG_SIDE_BE,
    };
    e_rcar_status_t rc = rcar_rpmsg_open(&ch, cfg);
    if (rc != RCAR_OK) {
        LogDebug(("[EP] rcar_rpmsg_open FAILED (%d)\n", (int)rc));
        print_result(TC_UCIE, false);
        vTaskDelete(NULL);
        return;
    }
    LogDebug(("[EP] endpoint ready\n"));

    uint32_t count = 0U;
    uint32_t errors = 0U;
    for (;;) {
        char buf[RCAR_RPMSG_MAX_PAYLOAD + 1];
        uint16_t n = 0U;
        rc = rcar_rpmsg_receive(&ch, buf, RCAR_RPMSG_MAX_PAYLOAD, &n, RCAR_RPMSG_WAIT_FOREVER);
        if (rc == RCAR_ERR_PEER_DISCONNECTED) {
            LogDebug(("[EP] RC closed the channel\n"));
            if (count < ECHO_COUNT) {
                print_result(TC_UCIE, false);
            }
            break;
        }
        if (rc != RCAR_OK) {
            LogDebug(("[EP] receive FAILED (%d)\n", (int)rc));
            errors++;
            continue;
        }
        buf[n] = '\0';
        LogDebug(("[EP] request: %s\n", buf));

        while ((rc = rcar_rpmsg_send(&ch, buf, n)) == RCAR_ERR_BUFFER_FULL) {
            vTaskDelay(pdMS_TO_TICKS(SEND_RETRY_MS));
        }
        if (rc != RCAR_OK) {
            LogDebug(("[EP] echo FAILED (%d)\n", (int)rc));
            errors++;
        }
        count++;
        if ((count == ECHO_COUNT) && (errors != 0U)) {
            print_result(TC_UCIE, false);
        }
    }

    vTaskDelete(NULL);
}

#endif /* UCIE_SIDE_FE */

static void print_result(int tc, bool passed)
{
    LogAlways(("TC%d Result: %s\n", tc, passed ? "Passed" : "Failed"));
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
