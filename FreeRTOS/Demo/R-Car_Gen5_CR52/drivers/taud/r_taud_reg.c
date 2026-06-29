/*
* Copyright (c) 2025 Renesas Electronics Corporation
*
* SPDX-License-Identifier: MIT
*
*/


/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "r_taud_reg.h"

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/


/***********************************************************************************************************************
 * Private function prototypes
 **********************************************************************************************************************/


/***********************************************************************************************************************
 * Functions
 **********************************************************************************************************************/
uint16_t R_TAUD_RegWrite16(uintptr_t Addr, uint16_t Data)
{
    volatile uint32_t *p_reg = (volatile uint32_t *)Addr;
    uint32_t reg = *p_reg;

    reg = (reg & 0xFFFF0000U) | (uint32_t)Data;   
    *p_reg = reg;
    return 0;
}

uint16_t  R_TAUD_RegRead16(uintptr_t Addr)
{
    return *((volatile uint16_t *)Addr);
}

uint8_t  R_TAUD_RegWrite8(uintptr_t Addr, uint8_t Data)
{
    volatile uint32_t *p_reg = (volatile uint32_t *)Addr;
    uint32_t reg = *p_reg;

    reg = (reg & 0xFFFFFF00U) | (uint32_t)Data;   
    *p_reg = reg;
    return 0;
}

uint8_t  R_TAUD_RegRead8(uintptr_t Addr)
{
    return *((volatile uint8_t *)Addr);
}

uint16_t R_TAUD_CH_Set(uintptr_t reg_addr, uint8_t ch)
{
    uint16_t data = R_TAUD_RegRead16(reg_addr);
    data |= (uint16_t)(1U << ch);
    R_TAUD_RegWrite16(reg_addr, data);
    return 0;
}

uint16_t R_TAUD_CH_Clear(uintptr_t reg_addr, uint8_t ch)
{
    uint16_t data = R_TAUD_RegRead16(reg_addr);
    data &= ~(1U << ch);
    R_TAUD_RegWrite16(reg_addr, data);
    return 0;
}

/***********************************************************************************************************************
 * Private Functions
 **********************************************************************************************************************/
