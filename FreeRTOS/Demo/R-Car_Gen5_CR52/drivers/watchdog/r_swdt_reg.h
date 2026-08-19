/*
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef R_SWDT_REG_H
#define R_SWDT_REG_H

#include "r_swdt_private.h"

#define SWDT_BASE	0x1C050000U
#define SWTCNT		0x0U

#define SWTCSRA		0x04U
#define SWTCSRA_WOVF	(1U << 4)
#define SWTCSRA_WRFLG	(1U << 5)
#define SWTCSRA_TME	(1U << 7)

#define SWTCSRB		0x08U
#define OSCCLK		131570U

#define RST_DM0_BASE	0xC1320000U

#define RST_KCPROT_DIS	0xA5A5A501U
#define RST_KCPROT_EN	0xA5A5A500U
#define RST_WDTRSTCR	0x0420U
#define RST_RESKCPROT0	0x04F0U
#define SWDT_RSTMSK	(1U << 1)
#define RST_RESFC	0x0460U
#define RST_SRES1FC5	(1U << 25)

#define DIV_ROUND_UP(a, b) (((a) + (b) - 1U) / (b))
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#define MUL_BY_CLKS_PER_SEC(cks, d) \
            DIV_ROUND_UP((d) * OSCCLK, clk_divs[(cks)])

uint8_t R_SWDT_Init(uint8_t timeout_sec);
uint8_t R_SWDT_Ping(uint8_t timeout_new_sec);
uint32_t R_SWDT_Start(void);
uint32_t R_SWDT_Stop(void);

#endif /* R_SWDT_REG_H */
