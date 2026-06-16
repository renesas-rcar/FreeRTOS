/*
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "stdio.h"
#include "stdint.h"
#include "tcm.h"
#include "cmsis_rcar_gen5.h"

#define BIT(nr)                 (1UL << (nr))

#define TCM_ENABLEEL10          (BIT(0))                    // Bit field to enable TCM at EL1 and EL0.
#define TCM_ENABLEEL2           (BIT(1))                    // Bit field to enable TCM at EL2.

#define TCM_SIZE_8KB            (BIT(4))                    // Bit field to set TCM size 8KB.
#define TCM_SIZE_16KB           (BIT(4) | BIT(2))           // Bit field to set TCM size 16KB.
#define TCM_SIZE_32KB           (BIT(4) | BIT(3))           // Bit field to set TCM size 32KB.

#define IS_8KB_ALIGNED(addr)    (((addr) & 0x1FFFu) == 0)

static uint32_t ReadTCM(rcar_tcm_region_t region);

static void WriteTCM(rcar_tcm_region_t region, uint32_t reg_val);

static uint8_t Is_Size_Valid(uint32_t * p_size);

static uint32_t ReadTCM(rcar_tcm_region_t region)
{
    uint32_t reg_val = 0;

    switch (region) {
    case RCAR_TCM_A:
        __asm__ volatile ("mrc p15, 0, %0, c9, c1, 0" : "=r"(reg_val) : : "memory");
        break;
    case RCAR_TCM_B:
        __asm__ volatile ("mrc p15, 0, %0, c9, c1, 1" : "=r"(reg_val) : : "memory");
        break;
    case RCAR_TCM_C:
        __asm__ volatile ("mrc p15, 0, %0, c9, c1, 2" : "=r"(reg_val) : : "memory");
        break;
    default:
        reg_val = 0;
        break;
    }

    return reg_val;
}

void WriteTCM(rcar_tcm_region_t region, uint32_t reg_val)
{
    switch (region) {
    case RCAR_TCM_A:
        __asm__ volatile ("mcr p15, 0, %0, c9, c1, 0" : : "r"(reg_val) : "memory");
        break;
    case RCAR_TCM_B:
        __asm__ volatile ("mcr p15, 0, %0, c9, c1, 1" : : "r"(reg_val) : "memory");
        break;
    case RCAR_TCM_C:
        __asm__ volatile ("mcr p15, 0, %0, c9, c1, 2" : : "r"(reg_val) : "memory");
        break;
    default:
        break;
    }

    __DSB();
    __ISB();
}

static uint32_t Get_Bit_Field_TCM_Size(rcar_tcm_size_t * p_size)
{
    rcar_tcm_size_t size = *p_size;
    uint32_t bit_field = 0;

    if (size != RCAR_TCM_SIZE_8KB && size != RCAR_TCM_SIZE_16KB && size != RCAR_TCM_SIZE_32KB) {
        return 0;
    }

    switch (size) {
        case RCAR_TCM_SIZE_8KB:
            bit_field = TCM_SIZE_8KB;
            break;
        case RCAR_TCM_SIZE_16KB:
            bit_field = TCM_SIZE_16KB;
            break;
        case RCAR_TCM_SIZE_32KB:
            bit_field = TCM_SIZE_32KB;
            break;
        default:
            break;
    }

    return bit_field;
}

static uint32_t Get_Bit_Field_TCM_EL(rcar_tcm_el_t * p_el)
{
    rcar_tcm_el_t el= *p_el;
    uint32_t bit_field = 0;

    if (el != RCAR_TCM_EL0 && el != RCAR_TCM_EL1 && el != RCAR_TCM_EL2) {
        return 0;
    }

    switch (el) {
        case RCAR_TCM_EL0:
            bit_field = TCM_ENABLEEL10;
            break;
        case RCAR_TCM_EL1:
            bit_field = TCM_ENABLEEL10;
            break;
        case RCAR_TCM_EL2:
            bit_field = TCM_ENABLEEL2;
            break;
        default:
            break;
    }

    return bit_field;
}

void ConfigureTCM(rcar_tcm_region_t region, uint32_t base_addr, rcar_tcm_size_t size)
{
    uint32_t reg_val = 0, bit_field_size = 0;

    if (!IS_8KB_ALIGNED(base_addr)) {
        return;
    }

    bit_field_size = Get_Bit_Field_TCM_Size(&size);
    if (bit_field_size == 0) {
        return;
    }

    reg_val = base_addr | bit_field_size;
    WriteTCM(region, reg_val);
}

void ControlTCM(rcar_tcm_region_t region, rcar_tcm_el_t el, rcar_tcm_state_t state)
{
    uint32_t reg_val = 0, bit_field_el = 0;

    if (state != RCAR_TCM_ENABLE && state != RCAR_TCM_DISABLE) {
        return;
    }

    bit_field_el = Get_Bit_Field_TCM_EL(&el);
    if (bit_field_el == 0) {
        return;
    }

    reg_val = ReadTCM(region);

    reg_val = (state == RCAR_TCM_ENABLE)
            ? (reg_val | bit_field_el)
            : (reg_val & ~bit_field_el);

    WriteTCM(region, reg_val);
}
