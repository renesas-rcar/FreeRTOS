/*
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef R_WWDT_PRIVATE_H
#define R_WWDT_PRIVATE_H

#include <stdint.h>
#include "watchdog/r_wwdt_api.h"

#define R_WWDT0_BASE	0x1C100000U
#define R_WWDT1_BASE	0x1C110000U
#define R_WWDT2_BASE	0xC1380000U

static const uintptr_t wwdt_base_tbl[] =
{
    R_WWDT0_BASE,
    R_WWDT1_BASE,
    R_WWDT2_BASE
};

#define WWDT_CH_NUM    ((uint32_t)(sizeof(wwdt_base_tbl) / sizeof(wwdt_base_tbl[0])))

#define WDTA0WDTE       0
#define WDTA0RUN        0xAC
#define WDTA0MD		0xC
#define WSIZE(x)	(x)
#define WDTA0ERM	(1U << 2)
#define WDTA0WIE	(1U << 3)
#define WDTA0OVF(x)	(((x) << 4) & 0x70)

#define RST_DM0_BASE	0xC1320000U
#define RST_KCPROT_DIS	0xA5A5A501U
#define RST_WDTRSTCR	0x0420U
#define WWDT_RSTMSK	(1U << 2)
#define RST_RESFC	0x0460U
#define RST_SRES1FC4    (1U << 7)
#define RST_RESKCPROT0	0x04F0U

uintptr_t R_WWDT_PRV_GetRegbase(wwdt_unit_t unit);
void r_wwdt_write(uintptr_t Addr, uint32_t val);
void r_wwdt_write8(uintptr_t Addr, uint8_t val);
uint32_t r_wwdt_read(uintptr_t Addr);
uint8_t r_wwdt_read8(uintptr_t Addr);

#endif /* R_WWDT_PRIVATE_H */
