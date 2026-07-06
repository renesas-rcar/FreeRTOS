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
#include "state-manager/r_clock_domain_id.h"
#include "state-manager/r_state_manager.h"
#include "r_wwdt_reg.h"
#include "board.h"

const uint16_t timeout_ch0_19[] = { 15, 31, 62, 124, 250, 500, 1000, 2000 };
const uint16_t timeout_ch20[] = { 2, 4, 8, 17, 34, 68, 136, 273 };
static uint32_t clk_rate;

#define DIV_ROUND_UP(a, b) (((a) + (b) - 1U) / (b))
#define TIMEOUT_TO_X(timeout_ms, channel)    \
    ({ \
        const uint16_t *timeout_array = ((channel) == 0xC1380000) ? timeout_ch20 : timeout_ch0_19; \
        uint8_t result = 7; \
        for (uint8_t i = 0; i < 8; ++i) { \
            if ((timeout_ms) <= timeout_array[i]) { \
                result = i; \
                break; \
            } \
        } \
        result; \
    })
#define CLK_LSIOSC	240000
#define RCLK		32800

static uint32_t r_rst_read(uintptr_t Addr)
{
	return *((volatile uint32_t *)Addr);
}

static void r_wdt_wait_cycles(uint8_t cycles)
{
	uint8_t delay;
	delay = DIV_ROUND_UP(cycles * 1000000U, clk_rate);

	vTaskDelay(delay);
}

void R_WWDT_Init(wwdt_unit_t unit, wwdt_wsize_t wsize, uint32_t timeout_msec, bool irq_75p, wwdt_erm_t err_mode)
{
	uint8_t val;
	uintptr_t wwdt_base_addr = R_WWDT_PRV_GetRegbase(unit);
	int clock_id, clock_id_0, clock_id_1, ret;

#if (BOARD == X5H_IRONHIDE) || (BOARD == MDP_X5H_HIL)
	clock_id = X5H_CLOCK_ID_MDLC_WDT0;
	ret = R_StateManager_ClockOn(clock_id);
    if (ret != 0) {
		printf("Error: Failed to set clock id %d ON.\r\n", clock_id);
    }
#endif

	switch(unit) {
#if (BOARD == X5H_IRONHIDE) || (BOARD == MDP_X5H_HIL)
		case R_WWDT0:
			clock_id_0 = X5H_CLOCK_ID_MDLC_WWDT00;
			clock_id_1 = X5H_CLOCK_ID_MDLC_WWDT01;
			break;
		case R_WWDT1:
			clock_id_0 = X5H_CLOCK_ID_MDLC_WWDT10;
			clock_id_1 = X5H_CLOCK_ID_MDLC_WWDT11;
			break;
		case R_WWDT2:
			clock_id_0 = X5H_CLOCK_ID_MDLC_WWDT20;
			clock_id_1 = X5H_CLOCK_ID_MDLC_WWDT21;
			break;

		case R_WWDT3:
			clock_id_0 = X5H_CLOCK_ID_MDLC_WWDT30;
			clock_id_1 = X5H_CLOCK_ID_MDLC_WWDT31;
			break;

		case R_WWDT4:
			clock_id_0 = X5H_CLOCK_ID_MDLC_WWDT40;
			clock_id_1 = X5H_CLOCK_ID_MDLC_WWDT41;
			break;

		case R_WWDT5:
			clock_id_0 = X5H_CLOCK_ID_MDLC_WWDT50;
			clock_id_1 = X5H_CLOCK_ID_MDLC_WWDT51;
			break;

		case R_WWDT6:
			clock_id_0 = X5H_CLOCK_ID_MDLC_WWDT60;
			clock_id_1 = X5H_CLOCK_ID_MDLC_WWDT61;
			break;

		case R_WWDT7:
			clock_id_0 = X5H_CLOCK_ID_MDLC_WWDT70;
			clock_id_1 = X5H_CLOCK_ID_MDLC_WWDT71;
			break;

		case R_WWDT8:
			clock_id_0 = X5H_CLOCK_ID_MDLC_WWDT80;
			clock_id_1 = X5H_CLOCK_ID_MDLC_WWDT81;
			break;

		case R_WWDT9:
			clock_id_0 = X5H_CLOCK_ID_MDLC_WWDT90;
			clock_id_1 = X5H_CLOCK_ID_MDLC_WWDT91;
			break;

		case R_WWDT10:
			clock_id_0 = X5H_CLOCK_ID_MDLC_WWDT100;
			clock_id_1 = X5H_CLOCK_ID_MDLC_WWDT101;
			break;

		case R_WWDT11:
			clock_id_0 = X5H_CLOCK_ID_MDLC_WWDT110;
			clock_id_1 = X5H_CLOCK_ID_MDLC_WWDT111;
			break;

		case R_WWDT12:
			clock_id_0 = X5H_CLOCK_ID_MDLC_WWDT120;
			clock_id_1 = X5H_CLOCK_ID_MDLC_WWDT121;
			break;

		case R_WWDT13:
			clock_id_0 = X5H_CLOCK_ID_MDLC_WWDT130;
			clock_id_1 = X5H_CLOCK_ID_MDLC_WWDT131;
			break;

		case R_WWDT14:
			clock_id_0 = X5H_CLOCK_ID_MDLC_WWDT140;
			clock_id_1 = X5H_CLOCK_ID_MDLC_WWDT141;
			break;

		case R_WWDT15:
			clock_id_0 = X5H_CLOCK_ID_MDLC_WWDT150;
			clock_id_1 = X5H_CLOCK_ID_MDLC_WWDT151;
			break;

		case R_WWDT16:
			clock_id_0 = X5H_CLOCK_ID_MDLC_WWDT160;
			clock_id_1 = X5H_CLOCK_ID_MDLC_WWDT161;
			break;

		case R_WWDT17:
			clock_id_0 = X5H_CLOCK_ID_MDLC_WWDT170;
			clock_id_1 = X5H_CLOCK_ID_MDLC_WWDT171;
			break;

		case R_WWDT18:
			clock_id_0 = X5H_CLOCK_ID_MDLC_WWDT180;
			clock_id_1 = X5H_CLOCK_ID_MDLC_WWDT181;
			break;

		case R_WWDT19:
			clock_id_0 = X5H_CLOCK_ID_MDLC_WWDT190;
			clock_id_1 = X5H_CLOCK_ID_MDLC_WWDT191;
			break;

		case R_WWDT20:
			clock_id_0 = X5H_CLOCK_ID_MDLC_WWDT200;
			clock_id_1 = X5H_CLOCK_ID_MDLC_WWDT201;
			break;
#endif
		default:
			printf("Wrong Unit for Clock ID\n");
			break;
		}

       ret = R_StateManager_ResetAssert(clock_id_0);
        if (ret != 0) {
               printf("Error: Failed to reset clock id %d.\r\n", clock_id_0);
        }

       ret = R_StateManager_ResetAssert(clock_id_1);
        if (ret != 0) {
               printf("Error: Failed to reset clock id %d.\r\n", clock_id_1);
        }

       ret = R_StateManager_ResetDeassert(clock_id_0);
        if (ret != 0) {
               printf("Error: Failed to DeassertReset clock id %d.\r\n", clock_id_0);
        }

       ret = R_StateManager_ResetDeassert(clock_id_1);
        if (ret != 0) {
               printf("Error: Failed to DeassertReset clock id %d.\r\n", clock_id_1);
        }

	clk_rate = (wwdt_base_addr == 0xC1380000) ? CLK_LSIOSC : RCLK;
	val = r_wwdt_read8(wwdt_base_addr + WDTA0MD);
    if (!err_mode) {
		val &= ~WDTA0ERM;
    }
	val |= WDTA0OVF(TIMEOUT_TO_X(timeout_msec, wwdt_base_addr)) | WSIZE(wsize);
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
