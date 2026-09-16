/*
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "r_i2c_regs.h"

void R_I2C_PRV_RegWrite32(uintptr_t Addr, uint32_t Data)
{
    *((volatile uint32_t *)Addr) = Data;
    return;
}

uint32_t R_I2C_PRV_RegRead32(uintptr_t Addr)
{
    return *((volatile uint32_t *)Addr);
}
