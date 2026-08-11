/*
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include "watchdog/r_swdt_api.h"
#include "state-manager/r_state_manager.h"
#include "r_swdt_private.h"
#include "board.h"

#if (BOARD == MDP_AIACC_HIL)
#include "module_controller.h"
#endif

static const unsigned int clk_divs[] = { 1, 4, 16, 32, 64, 128, 1024, 4096 };

static uint8_t cks;
static uint8_t init_timeout;
static void r_swdt_write(uintptr_t Addr, uint32_t val)
{
	*((volatile uint32_t *)Addr) = val;
}

static uint32_t r_swdt_read(uintptr_t Addr)
{
	return (Addr == (uintptr_t)0x1C050000) ? *((volatile uint16_t *)Addr) : *((volatile uint8_t *)Addr);
}

static uint32_t r_rst_read(uintptr_t Addr)
{
	return *((volatile uint32_t *)Addr);
}

static void r_swdt_wait_cycles(uint8_t cycles) {
	uint8_t delay;
	delay = DIV_ROUND_UP(cycles * 10000000U, OSCCLK);

	vTaskDelay(delay);
}

#if (BOARD == MDP_AIACC_HIL)
static void SwdtRequestClockOn()
{
    uint32_t module_num = MODULE_NUM_RT;

    mdlc_ms_module_run_bit(module_num, 10, 2);
}
#endif

uint8_t R_SWDT_Init(uint8_t timeout_sec) {
	uint16_t clks_per_sec;
	uint8_t ret;
	int clock_id, reset_id_0, reset_id_1;

#if (BOARD == MDP_AIACC_HIL)
	SwdtRequestClockOn();
#endif

#if (BOARD == X5H_IRONHIDE) || (BOARD == MDP_X5H_HIL)
	clock_id = SWDT_CLK_ID;
	ret = R_StateManager_ClockOn(clock_id);
	if (ret != 0U) {
		(void)printf("Error: Failed to turn clock ID %d ON.\r\n", clock_id);
	}
#endif

	reset_id_0 = SWDT0_RST_ID;
	ret = R_StateManager_Reset(reset_id_0);
	if (ret != 0U) {
		(void)printf("Error: Failed to reset id %d ON.\r\n", reset_id_0);
	}

	reset_id_1 = SWDT1_RST_ID;
	ret = R_StateManager_Reset(reset_id_1);
	if (ret != 0U) {
		(void)printf("Error: Failed to reset id %d ON.\r\n", reset_id_1);
	}

	/* for SWDT */
	r_swdt_write(SWDT_BASE + SWTCSRA, (0xA5A5A5U << 8) | (r_swdt_read(SWDT_BASE + SWTCSRA) & ~SWTCSRA_TME));
	r_swdt_wait_cycles(2);

	r_swdt_write(SWDT_BASE + SWTCNT, 0x5A5A0000); //reset counter
	r_swdt_write(SWDT_BASE + SWTCSRA, (0xA5A5A5U << 8) | (r_swdt_read(SWDT_BASE + SWTCSRA) & ~SWTCSRA_WOVF));
	r_swdt_write(SWDT_BASE + SWTCSRB, (0xA5A5A5U << 8U) | 0U);

	for (uint8_t i = ARRAY_SIZE(clk_divs) - 1; i >= 0; i--) {
		clks_per_sec = OSCCLK / clk_divs[i];
			if (clks_per_sec && clks_per_sec < 65536) {
				cks = i;
			break;
		}
	}

	/* for RST_CTRL */
	r_swdt_write(RST_DM0_BASE + RST_RESKCPROT0, RST_KCPROT_DIS);

	init_timeout = timeout_sec;
	r_swdt_write(RST_DM0_BASE + RST_RESFC, r_rst_read(RST_DM0_BASE + RST_RESFC) & ~RST_SRES1FC5);

	/* Wait WRFLG becomes 0 */
	while ((r_swdt_read(SWDT_BASE + SWTCSRA) & SWTCSRA_WRFLG) != 0U) {}

	/* Enable Generating internal reset when SWDT overflow */
	r_swdt_write(RST_DM0_BASE + RST_WDTRSTCR, r_rst_read(RST_DM0_BASE + RST_WDTRSTCR) & ~SWDT_RSTMSK);
	r_swdt_write(RST_DM0_BASE + RST_RESKCPROT0, RST_KCPROT_EN);

	r_swdt_write(SWDT_BASE + SWTCNT, ((uint32_t)0x5A5A << 16U) | (65536U - MUL_BY_CLKS_PER_SEC(cks, timeout_sec)));

	return 0;
}

uint8_t R_SWDT_Ping(uint8_t ping_rate) {
	vTaskDelay(ping_rate*1000);
	r_swdt_write(SWDT_BASE + SWTCNT, ((uint32_t)0x5A5A << 16U) | (65536U - MUL_BY_CLKS_PER_SEC(cks, init_timeout)));

	return 0;
}

uint32_t R_SWDT_Start(void) {
	r_swdt_write(SWDT_BASE + SWTCSRA, (0xA5A5A5U << 8) | (r_swdt_read(SWDT_BASE + SWTCSRA) | SWTCSRA_TME));

	return 0;
}

uint32_t R_SWDT_Stop(void) {
	r_swdt_wait_cycles(3);
	r_swdt_write(SWDT_BASE + SWTCSRA, 0);

	return 0;
}
