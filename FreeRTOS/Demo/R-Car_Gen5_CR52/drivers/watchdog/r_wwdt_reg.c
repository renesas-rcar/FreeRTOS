/*
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include <stdint.h>
#include <stdio.h>
#include "r_wwdt_private.h"

void r_wwdt_write(uintptr_t Addr, uint32_t val)
{
	*((volatile uint32_t *)Addr) = val;

	return;
}

void r_wwdt_write8(uintptr_t Addr, uint8_t val)
{
    *(volatile uint8_t *)(Addr) = val;
}

uint32_t r_wwdt_read(uintptr_t Addr)
{
    return *((volatile uint32_t *)Addr);
}

uint8_t r_wwdt_read8(uintptr_t Addr)
{
    return *(volatile const uint8_t *)(Addr);
}

extern uintptr_t R_WWDT_PRV_GetRegbase(wwdt_unit_t unit)
{
    uintptr_t ret = 0;

    if ((uint32_t)unit < WWDT_CH_NUM) {
    	ret = wwdt_base_tbl[(uint32_t)unit];
    } else {
        printf("[R_WWDT_PRV_GetRegbase] : Wrong WWDT Unit %d\r\n", unit);
    }

    return ret;
}
