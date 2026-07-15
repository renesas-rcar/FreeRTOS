/*
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "r_i2c_private.h"
#include "r_i2c_regs.h"
#include "stdio.h"

#define I2C_INVALID_ADDR           0x0

void R_I2C_PRV_RegWrite32(uintptr_t Addr, uint32_t Data)
{
    *((volatile uint32_t *)Addr) = Data;
    return;
}

uint32_t R_I2C_PRV_RegRead32(uintptr_t Addr)
{
    return *((volatile uint32_t *)Addr);
}

uintptr_t R_I2C_PRV_GetRegbase(r_i2c_Unit_t I2cUnit)
{
    if ((uint32_t)I2cUnit >= (sizeof(i2c_base) / sizeof(i2c_base[0])))
    {
        printf("I2C channel %d not exist!\n", I2cUnit);
        return I2C_INVALID_ADDR;
    }

    return i2c_base[I2cUnit];
}
