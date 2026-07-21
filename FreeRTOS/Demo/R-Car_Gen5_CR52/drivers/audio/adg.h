/*
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef ADG_H
#define ADG_H

#include <audio/rcar_audio.h>
#include "r_audio_conf.h"
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>

#define ADG0_BASE			0xec530000u
#define BRRA0				(*(volatile uint32_t*)(ADG0_BASE))
#define BRRB0				(*(volatile uint32_t*)(ADG0_BASE+0x004u))
#define BRGCKR0				(*(volatile uint32_t*)(ADG0_BASE+0x008u))
#define AUDIO_CLK_SEL0_(channel)      (*(volatile uint32_t*)(0xec54a000u + (channel)*0x10000u))


//Value for BRGCKR
#define BRGCKR_BRGA_CLKA	(0x0U << 20)
#define BRGCKR_BRGA_CLKB	(0x1U << 20)
#define BRGCKR_BRGA_S0D4	(0x2U << 20)
#define BRGCKR_BRGA_CLKC	(0x4U << 20)

#define BRGCKR_BRGB_CLKA	(0x0U << 16)
#define BRGCKR_BRGB_CLKB	(0x1U << 16)
#define BRGCKR_BRGB_S0D4	(0x2U << 16)
#define BRGCKR_BRGB_CLKC	(0x4U << 16)
#define BRGCKR_OUT_BRGA		(0x0U << 31)
#define BRGCKR_OUT_BRGB		(0x1U << 31)

typedef struct {
	uint32_t freq;		
	uint32_t brgckr_brga;
	uint32_t brgckr_brgb;
} ClockInput;

// Clock_input
static const ClockInput clock_input[] = {
	{AUDIO_CLKA_FREQ, BRGCKR_BRGA_CLKA, BRGCKR_BRGB_CLKA},
	{AUDIO_CLKB_FREQ, BRGCKR_BRGA_CLKB, BRGCKR_BRGB_CLKB},
	{AUDIO_CLKC_FREQ, BRGCKR_BRGA_CLKC, BRGCKR_BRGB_CLKC},
	{S0D4PHI_FREQ, BRGCKR_BRGA_S0D4, BRGCKR_BRGB_S0D4},
};

typedef struct _adg_conf_t {
	uint32_t word_size;
	uint32_t channel;
	uint32_t sample_rate;
	uint32_t div_ssi;
}adg_conf_t;

int adg_init(adg_conf_t *p_conf);
void adg_reset_reg(adg_conf_t *p_conf);
#endif 
