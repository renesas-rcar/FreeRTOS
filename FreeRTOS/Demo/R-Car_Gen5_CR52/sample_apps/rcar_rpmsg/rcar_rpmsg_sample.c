/*
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 */

#include "FreeRTOS.h"
#include "task.h"
#include "interrupts.h"

#include <stdlib.h>
#include "stdio.h"
#include "string.h"

#include "pfc/r_pfc_api.h"
#include "rcar_utils.h"

#include "rcar-rpmsg/rcar_rpmsg.h"

#include "board.h"

/*
 * Same binary on both cores:
 *   core0: FE, sends "hello smmu N" and "hello camera N" (N = 0..19)
 *   core1: BE, prints them. smmu blocks in receive(), camera polls
 *          with NO_WAIT.
 *
 * Test cases (X5H: TC1/TC2, AIACC0: TC3/TC4), result printed by core1:
 *   TC_MFIS: smmu,   MFIS ch0 (blocking receive)
 *   TC_POLL: camera, no MFIS (NO_WAIT polling)
 * core0 prints "Failed" only if its open/send fails.
 */

/* Logging Function include. */
#include "logging_stack.h"

#define FE_STARTUP_DELAY_MS   3000U   /* after BE wrote the rsc table */
#define BE_STARTUP_DELAY_MS   1000U
#define FE_SEND_PERIOD_MS     500U
#define CAMERA_POLL_PERIOD_MS 100U    /* < FE_SEND_PERIOD_MS: no message lost */
#define MSG_COUNT             20

#if ((BOARD == MDP_AIACC_RFS2) || (BOARD == MDP_AIACC_HIL))
#define TC_CHIP               "AIACC0"
#define TC_MFIS               3
#define TC_POLL               4
#else
#define TC_CHIP               "X5H"
#define TC_MFIS               1
#define TC_POLL               2
#endif

#define main_ucie_TASK_PRIORITY        (tskIDLE_PRIORITY + 1)
#define UCIE_RC_SIZE (configMINIMAL_STACK_SIZE * 10)

/*-----------------------------------------------------------*/
static void prvSetupHardware( void );

static void task_fe_smmu( void *pvParameters );
static void task_fe_camera( void *pvParameters );

static void task_be_smmu( void *pvParameters );
static void task_be_camera( void *pvParameters );
static void be_on_unbind(rcar_rpmsg_channel_t *ch, void *ctx);

/* Print payload as text. */
static void print_payload(const char *tag, const uint8_t *buf, uint16_t n)
{
    char text[RCAR_RPMSG_MAX_PAYLOAD + 1];
    memcpy(text, buf, n);
    text[n] = '\0';
    LogDebug(("%s%s\r\n", tag, text));
}

/* True if @buf is "hello @tag @index", as sent by fe_send_loop(). */
static bool payload_matches(const char *tag, int index, const uint8_t *buf, uint16_t n)
{
    char expect[24];
    int len = snprintf(expect, sizeof(expect), "hello %s %d", tag, index);
    return (n == (uint16_t)len) && (memcmp(buf, expect, n) == 0);
}

static void print_result(int tc, bool passed)
{
    LogAlways(("TC%d Result: %s\n", tc, passed ? "Passed" : "Failed"));
}

/*-----------------------------------------------------------*/

int main( void )
{
    prvSetupHardware();

    uint32_t cpu_id = R_UTILS_GetCpuID();

    if (cpu_id == 0U)
    {
        LogDebug(("=== CORE0 : FRONTEND (smmu + camera) ===\n"));
        xTaskCreate(task_fe_smmu, "FE_SMMU", UCIE_RC_SIZE, NULL, main_ucie_TASK_PRIORITY, NULL);
        xTaskCreate(task_fe_camera, "FE_CAMERA", UCIE_RC_SIZE, NULL, main_ucie_TASK_PRIORITY, NULL);
    }
    else
    {
        LogDebug(("=== CORE1 : BACKEND (smmu + camera) ===\n"));
        xTaskCreate(task_be_smmu, "BE_SMMU", UCIE_RC_SIZE, NULL, main_ucie_TASK_PRIORITY, NULL);
        xTaskCreate(task_be_camera, "BE_CAMERA", UCIE_RC_SIZE, NULL, main_ucie_TASK_PRIORITY, NULL);
    }

    vTaskStartScheduler();
    for( ;; )
    {
    }
    return 0;
}

static void prvSetupHardware( void )
{
    portDISABLE_INTERRUPTS();
    Irq_Setup();
    (void)pfcInitModules(getModuleConfigs());
}

/* CORE0 - FE: send only. Reports only failures of test case @tc. */
static void fe_send_loop(e_rcar_driver_id_t driver_id, const char *tag, int tc)
{
    vTaskDelay(FE_STARTUP_DELAY_MS);

    static rcar_rpmsg_channel_t s_fe_ch[2];
    rcar_rpmsg_channel_t *ch = &s_fe_ch[(driver_id == RCAR_RPMSG_ID_0) ? 0 : 1];
    rcar_rpmsg_open_cfg_t cfg = {
        .driver_id = driver_id, .side = RCAR_RPMSG_SIDE_FE,
    };
    if (rcar_rpmsg_open(ch, cfg) != RCAR_OK)
    {
        LogDebug(("[FE %s] rcar_rpmsg_open FAILED\n", tag));
        print_result(tc, false);
        return;
    }
    LogDebug(("[FE %s] endpoint ready\n", tag));

    char msg[24];
    int i = 0;
    int errors = 0;
    while (i < MSG_COUNT)
    {
        snprintf(msg, sizeof(msg), "hello %s %d", tag, i);
        e_rcar_status_t rc = rcar_rpmsg_send(ch, msg, (uint16_t)strlen(msg));
        if (rc == RCAR_OK)
        {
            i++;
        }
        else if (rc != RCAR_ERR_NOT_CONNECTED)   /* not bound yet: retry */
        {
            LogDebug(("[FE %s] send FAILED (%d)\n", tag, (int)rc));
            errors++;
        }
        vTaskDelay(FE_SEND_PERIOD_MS);
    }
    if (errors != 0)
    {
        print_result(tc, false);
    }
}

static void task_fe_smmu(void *pvParameters)
{
    (void)pvParameters;
    fe_send_loop(RCAR_RPMSG_ID_0, "smmu", TC_MFIS);
    vTaskDelete(NULL);
}

static void task_fe_camera(void *pvParameters)
{
    (void)pvParameters;
    fe_send_loop(RCAR_RPMSG_ID_3, "camera", TC_POLL);
    vTaskDelete(NULL);
}

/* CORE1 - BE smmu: blocking receive. */
static void task_be_smmu(void *pvParameters)
{
    (void)pvParameters;
    vTaskDelay(BE_STARTUP_DELAY_MS);

    LogDebug(("TC%d: Message transmission and reception between Core 0 and Core 1 on "
              TC_CHIP " using MFIS.\n", TC_MFIS));
    static rcar_rpmsg_channel_t s_be_smmu;
    rcar_rpmsg_open_cfg_t cfg = {
        .driver_id = RCAR_RPMSG_ID_0, .side = RCAR_RPMSG_SIDE_BE,
        .on_unbind = be_on_unbind, .ctx = (void *)"smmu",
    };
    if (rcar_rpmsg_open(&s_be_smmu, cfg) != RCAR_OK)
    {
        LogDebug(("[BE smmu] rcar_rpmsg_open FAILED\n")); print_result(TC_MFIS, false); vTaskDelete(NULL); return;
    }
    LogDebug(("[BE smmu] endpoint ready\n"));

    int count = 0;
    int errors = 0;
    for (;;)
    {
        uint8_t buf[RCAR_RPMSG_MAX_PAYLOAD];
        uint16_t n = 0U;
        e_rcar_status_t rc = rcar_rpmsg_receive(&s_be_smmu, buf, sizeof(buf), &n,
                                                RCAR_RPMSG_WAIT_FOREVER);
        if (rc == RCAR_OK)
        {
            print_payload("[BE smmu] request: ", buf, n);
            errors += payload_matches("smmu", count, buf, n) ? 0 : 1;
            count++;
            if (count == MSG_COUNT)
            {
                print_result(TC_MFIS, errors == 0);
            }
        }
        else if (rc == RCAR_ERR_PEER_DISCONNECTED)
        {
            if (count < MSG_COUNT)
            {
                print_result(TC_MFIS, false);
            }
            break;
        }
    }
    vTaskDelete(NULL);
}

/* CORE1 - BE camera: periodic NO_WAIT receive. */
static void task_be_camera(void *pvParameters)
{
    (void)pvParameters;
    vTaskDelay(BE_STARTUP_DELAY_MS);

    LogDebug(("TC%d: Message transmission and reception between Core 0 and Core 1 on "
              TC_CHIP " using polling.\n", TC_POLL));
    static rcar_rpmsg_channel_t s_be_camera;
    rcar_rpmsg_open_cfg_t cfg = {
        .driver_id = RCAR_RPMSG_ID_3, .side = RCAR_RPMSG_SIDE_BE,
        .on_unbind = be_on_unbind, .ctx = (void *)"camera",
    };
    if (rcar_rpmsg_open(&s_be_camera, cfg) != RCAR_OK)
    {
        LogDebug(("[BE camera] rcar_rpmsg_open FAILED\n")); print_result(TC_POLL, false); vTaskDelete(NULL); return;
    }
    LogDebug(("[BE camera] endpoint ready\n"));

    int count = 0;
    int errors = 0;
    for (;;)
    {
        vTaskDelay(CAMERA_POLL_PERIOD_MS);

        uint8_t buf[RCAR_RPMSG_MAX_PAYLOAD];
        uint16_t n = 0U;
        /* TIMEOUT: nothing yet. */
        e_rcar_status_t rc = rcar_rpmsg_receive(&s_be_camera, buf, sizeof(buf), &n,
                                                RCAR_RPMSG_NO_WAIT);
        if (rc == RCAR_OK)
        {
            print_payload("[BE camera] request: ", buf, n);
            errors += payload_matches("camera", count, buf, n) ? 0 : 1;
            count++;
            if (count == MSG_COUNT)
            {
                print_result(TC_POLL, errors == 0);
            }
        }
        else if (rc != RCAR_ERR_TIMEOUT)
        {
            LogDebug(("[BE camera] receive FAILED (%d)\n", (int)rc));
            errors++;
        }
    }
}

/* May run in another task: keep short. */
static void be_on_unbind(rcar_rpmsg_channel_t *ch, void *ctx)
{
    (void)ch;
    LogDebug(("[BE %s] peer unbound\n", (const char *)ctx));
}

/*-----------------------------------------------------------*/

int printf_raw(const char *format, ...);

void vMainAssertCalled( const char *pcFileName, uint32_t ulLineNumber )
{
    printf_raw("ASSERT!  Line %d of file %s\n", ulLineNumber, pcFileName);
    taskENTER_CRITICAL();
    for( ;; );
}

void vDeleteCallingTask( void )
{
     vTaskDelete( NULL );
}