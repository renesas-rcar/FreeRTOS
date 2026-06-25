/*
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "r_i2c_regs.h"
//#include "r_print_api.h"
#include "stdio.h"

void R_I2C_PRV_RegWrite32(uintptr_t Addr, uint32_t Data)
{
    *((volatile uint32_t *)Addr) = Data;
    return;
}

uint32_t R_I2C_PRV_RegRead32(uintptr_t Addr)
{
    return *((volatile uint32_t *)Addr);
}

extern uintptr_t R_I2C_PRV_GetRegbase(r_i2c_Unit_t I2cUnit)
{
    uintptr_t ret = 0;

    switch (I2cUnit) {
    case R_I2C_IF0:
        ret = R_I2C_IF0_BASE;
        break;
    case R_I2C_IF1:
        ret = R_I2C_IF1_BASE;
        break;
    case R_I2C_IF2:
        ret = R_I2C_IF2_BASE;
        break;
    case R_I2C_IF3:
        ret = R_I2C_IF3_BASE;
        break;
#if (BOARD == X5H_IRONHIDE || BOARD == X5H_RFS2 || BOARD == X5H_VDK || BOARD == MDP_X5H_HIL)
    case R_I2C_IF4:
        ret = R_I2C_IF4_BASE;
        break;
    case R_I2C_IF5:
        ret = R_I2C_IF5_BASE;
        break;
    case R_I2C_IF6:
        ret = R_I2C_IF6_BASE;
        break;
    case R_I2C_IF7:
        ret = R_I2C_IF7_BASE;
        break;
    case R_I2C_IF8:
        ret = R_I2C_IF8_BASE;
        break;
#endif
    default:
        printf("[R_I2C_PRV_GetRegbase] : Wrong I2C Unit %d\r\n", I2cUnit);
        break;
    }

    return ret;
}
