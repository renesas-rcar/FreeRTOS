/*
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include <stdbool.h>
#include "watchdog/r_wwdt_api.h"
#include "state-manager/r_state_manager.h"
#include "r_wwdt_reg.h"
#include "board.h"

#if (BOARD == MDP_AIACC_HIL)
#include "module_controller.h"
#endif

static uint32_t clk_rate;

#define DIV_ROUND_UP(a, b) (((a) + (b) - 1U) / (b))
#define CLK_LSIOSC	240000
#define RCLK		32800

static const uint16_t timeout_ch0_19[] = { 15, 31, 62, 124, 250, 500, 1000, 2000 };
static const uint16_t timeout_ch20[] = { 2, 4, 8, 17, 34, 68, 136, 273 };

static inline uint8_t timeout_to_x(uint16_t timeout_ms, uint32_t channel)
{
    const uint16_t *timeout_array = (channel == 0xC1380000U) ? timeout_ch20 : timeout_ch0_19;

    uint8_t result = 7U;

    for (uint8_t i = 0U; i < 8U; ++i) {
        if (timeout_ms <= timeout_array[i]) {
            result = i;
            break;
        }
    }

    return result;
}

static uint32_t r_rst_read(uintptr_t Addr)
{
    return *((volatile uint32_t *)Addr);
}

static void r_wdt_wait_cycles(uint8_t cycles)
{
    uint8_t delay;
    delay = (uint8_t)DIV_ROUND_UP((uint32_t)cycles * 1000000U, clk_rate);

    vTaskDelay(delay);
}

#if (BOARD == MDP_AIACC_HIL)
static void WwdtRequestClockOn()
{
    uint32_t module_num = MODULE_NUM_RT;

    mdlc_ms_module_run_bit(module_num, 10, 2);
}
#endif

void R_WWDT_Init(wwdt_unit_t unit, wwdt_wsize_t wsize, uint32_t timeout_msec, bool irq_75p, wwdt_erm_t err_mode)
{
    uint8_t val;
    uintptr_t wwdt_base_addr = R_WWDT_PRV_GetRegbase(unit);
    int clock_id, clock_id_0, clock_id_1, ret;

    if ((uint32_t)unit >= WWDT_CH_NUM) {
        (void)printf("Wrong Unit for Clock ID\n");
        return;
    }

#if (BOARD == MDP_AIACC_HIL)
    WwdtRequestClockOn();
#endif

#if (BOARD == X5H_IRONHIDE) || (BOARD == MDP_X5H_HIL)
    clock_id = WWDT_CLK_ID;
    ret = R_StateManager_ClockOn(clock_id);
    if (ret != 0) {
        (void)printf("Error: Failed to set clock id %d ON.\r\n", clock_id);
    }

    clock_id_0 = wwdt_clock_tbl[unit].clock_id_0;
    clock_id_1 = wwdt_clock_tbl[unit].clock_id_1;

    ret = R_StateManager_ResetAssert(clock_id_0);
    if (ret != 0) {
        (void)printf("Error: Failed to reset clock id %d.\r\n", clock_id_0);
    }

    ret = R_StateManager_ResetAssert(clock_id_1);
    if (ret != 0) {
        (void)printf("Error: Failed to reset clock id %d.\r\n", clock_id_1);
    }

    ret = R_StateManager_ResetDeassert(clock_id_0);
    if (ret != 0) {
        (void)printf("Error: Failed to DeassertReset clock id %d.\r\n", clock_id_0);
    }

    ret = R_StateManager_ResetDeassert(clock_id_1);
    if (ret != 0) {
        (void)printf("Error: Failed to DeassertReset clock id %d.\r\n", clock_id_1);
    }
#endif

    clk_rate = (wwdt_base_addr == 0xC1380000) ? CLK_LSIOSC : RCLK;
    val = r_wwdt_read8(wwdt_base_addr + WDTA0MD);
    if (!err_mode) {
        val &= ~WDTA0ERM;
    }
    val |= WDTA0OVF(timeout_to_x(timeout_msec, wwdt_base_addr)) | wsize;
    if (irq_75p) {
        val |= WDTA0WIE;
    }

    r_wwdt_write8(wwdt_base_addr + WDTA0MD, val);

    /* Enable Generating internal reset when WWDT overflow */
    r_wwdt_write(RST_DM0_BASE + RST_RESKCPROT0, RST_KCPROT_DIS);
    r_wwdt_write(RST_DM0_BASE + RST_RESFC, r_rst_read(RST_DM0_BASE + RST_RESFC) & ~RST_SRES1FC4);
    r_wwdt_write(RST_DM0_BASE + RST_WDTRSTCR, r_rst_read(RST_DM0_BASE + RST_WDTRSTCR) & ~WWDT_RSTMSK);
}

uint32_t R_WWDT_Refresh(wwdt_unit_t unit)
{
    uintptr_t wwdt_base_addr = R_WWDT_PRV_GetRegbase(unit);

    r_wdt_wait_cycles(3);
    r_wwdt_write8(wwdt_base_addr + WDTA0WDTE, WDTA0RUN);

    return 0;
}
