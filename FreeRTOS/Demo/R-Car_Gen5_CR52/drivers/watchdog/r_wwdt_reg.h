/*
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef R_WWDT_REG_H
#define R_WWDT_REG_H

#include <stdint.h>
#include "watchdog/r_wwdt_api.h"
#include "r_wwdt_private.h"

#define WDTA0WDTE       0x0U
#define WDTA0RUN        0xACU
#define WDTA0MD		0xCU
#define WDTA0ERM	(1U << 2)
#define WDTA0WIE	(1U << 3)
#define WDTA0OVF(x) ((((uint32_t)(x)) & 0x7U) << 4)

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

#endif /* R_WWDT_REG_H */
