/*
* Copyright (c) 2025 Renesas Electronics Corporation
*
* SPDX-License-Identifier: MIT
*/
#include <stdbool.h>
#include "pcie/r_pcie_ctrl.h"
#include "ucie.h"

/* Implement API functions here */

void writel(const uint32_t Value, const uintptr_t Address)
{
    *((volatile unsigned int*) Address)  = Value;
}

uint32_t readl(const uintptr_t Address)
{
    return *((volatile unsigned int*)Address);
}

void R_UCIE_RegWrite8(uint16_t channel, uint32_t Offset, uint8_t Value)
{
    uint32_t regAddr;

    regAddr = (channel == 0) ? (UCIE_CXL_CH0_BASE + Offset) : (UCIE_CXL_CH1_BASE + Offset);
    *(volatile uint8_t*)regAddr = Value;
}

uint8_t R_UCIE_RegRead8(uint16_t channel, uint32_t Offset)
{
    uint32_t regAddr;

    regAddr = (channel == 0) ? (UCIE_CXL_CH0_BASE + Offset) : (UCIE_CXL_CH1_BASE + Offset);
    return *(volatile uint8_t*)regAddr;
}

void R_UCIE_RegWrite16(uint16_t channel, uint32_t Offset, uint16_t Value)
{
    uint32_t regAddr;

    regAddr = (channel == 0) ? (UCIE_CXL_CH0_BASE + Offset) : (UCIE_CXL_CH1_BASE + Offset);
    *(volatile uint16_t*)regAddr = Value;
}

uint16_t R_UCIE_RegRead16(uint16_t channel, uint32_t Offset)
{
    uint32_t regAddr;

    regAddr = (channel == 0) ? (UCIE_CXL_CH0_BASE + Offset) : (UCIE_CXL_CH1_BASE + Offset);
    return *(volatile uint16_t*)regAddr;
}

void R_UCIE_RegWrite32(uint16_t channel, uint32_t Offset, uint32_t Value)
{
    uint32_t regAddr;

    regAddr = (channel == 0) ? (UCIE_CXL_CH0_BASE + Offset) : (UCIE_CXL_CH1_BASE + Offset);
    *(volatile uint32_t*)regAddr = Value;
}

uint32_t R_UCIE_RegRead32(uint16_t channel, uint32_t Offset)
{
    uint32_t regAddr;

    regAddr = (channel == 0) ? (UCIE_CXL_CH0_BASE + Offset) : (UCIE_CXL_CH1_BASE + Offset);
    return *(volatile uint32_t*)regAddr;
}

void rcar_ucie_dbi_ro_wr_en(uint16_t channel, bool enable)
{
    uint32_t val;

    if (enable) {
	val = R_UCIE_RegRead32(channel, UCIE_MISC_CONTROL_1_OFF);
	val |= UCIE_DBI_RO_WR_EN;
	R_UCIE_RegWrite32(channel, UCIE_MISC_CONTROL_1_OFF, val);
    } else {
	val = R_UCIE_RegRead32(channel, UCIE_MISC_CONTROL_1_OFF);
	val &= ~UCIE_DBI_RO_WR_EN;
	R_UCIE_RegWrite32(channel, UCIE_MISC_CONTROL_1_OFF, val);
    }
}

void rcar_ucie_setup(uint16_t channel)
{
    uint32_t val;

    val = R_UCIE_RegRead32(channel, UCIE_PORT_LINK_CONTROL);
    val &= ~PORT_LINK_FAST_LINK_MODE;
    val |= PORT_LINK_DLL_LINK_EN;
    //R_UCIE_RegWrite32(channel, UCIE_PORT_LINK_CONTROL, 0x4f0120);
    R_UCIE_RegWrite32(channel, UCIE_PORT_LINK_CONTROL, val);

    /* Set the number of lanes */
    val &= ~PORT_LINK_FAST_LINK_MODE;
    val &= ~PORT_LINK_MODE_MASK;
    val |= PORT_LINK_MODE_2_LANES;
    //R_UCIE_RegWrite32(channel, UCIE_PORT_LINK_CONTROL, 0x430120);
    R_UCIE_RegWrite32(channel, UCIE_PORT_LINK_CONTROL, val);

    /* Set link width speed control register */
    val = R_UCIE_RegRead32(channel, UCIE_LINK_WIDTH_SPEED_CONTROL);
    val &= ~PORT_LOGIC_LINK_WIDTH_MASK;
    val |= PORT_LOGIC_LINK_WIDTH_2_LANES;
    //R_UCIE_RegWrite32(channel, UCIE_LINK_WIDTH_SPEED_CONTROL, 0x102c8);
    R_UCIE_RegWrite32(channel, UCIE_LINK_WIDTH_SPEED_CONTROL, val);
}

bool rcar_ucie_calc_even_parity(uint64_t data)
{
    uint8_t i;

    for (i = 32; i > 0; i /= 2) {
        data ^= data >> i;
    }

    return (data & 1);
}

void rcar_ucie_reg_write32(uint32_t channel, bool phy, bool mem, uint32_t reg, uint32_t data)
{
    uint32_t phase0, phase1;
    uint64_t val;

    phase0 = mem ? OPCODE_MEM_WRITE32 : OPCODE_CONF_WRITE32;
    phase0 |= BYTE_ENABLES_32 | SRCID_PROTO_STACK0_ACCESS;

    R_UCIE_RegWrite32(channel, APB_BRIDGE_CTL0, phase0);

    phase1 = phy ? DSTID_PHY_STACK_ACCESS : DSTID_PROTO_STACK_ACCESS;
    phase1 |= reg;

    val = ((uint64_t)phase1 << 32) | phase0;
    phase1 |= CONTROL_PARITY(rcar_ucie_calc_even_parity(val));
    phase1 |= DATA_PARITY(rcar_ucie_calc_even_parity(data));

    R_UCIE_RegWrite32(channel, APB_BRIDGE_CTL1, phase1);
    R_UCIE_RegWrite32(channel, APB_BRIDGE_CTL2, data);
    R_UCIE_RegWrite32(channel, APB_BRIDGE_CTL3, 0);
}

void rcar_ucie_controller_enable(uint32_t channel)
{
    /* Write to memory register */
    rcar_ucie_reg_write32(channel, false, true, IMP_SB_CONFIG0, 0xa0190);
    rcar_ucie_reg_write32(channel, false, true, IMP_SB_CONFIG2, 0xa0190);
    rcar_ucie_reg_write32(channel, false, true, IMP_SB_CONFIG4, 0x91);

    /* Write to configuration register */
    rcar_ucie_reg_write32(channel, false, false, DVSEC_UCIE_LINK_CONTROL, 0x01);
}

void R_PCIE_Outbound_ATU(uint16_t channel, uint64_t base_addr, uint64_t target_addr)
{
    uint32_t lower_base, upper_base;
    uint32_t lower_target, upper_target;

    lower_base = base_addr & 0xFFFFFFFF;
    upper_base = (base_addr >> 32) & 0xFFFFFFFF;

    lower_target = target_addr & 0xFFFFFFFF;
    upper_target = (target_addr >> 32) & 0xFFFFFFFF;

    /* Outbound ATU configuration */
    R_UCIE_RegWrite32(channel, UCIE_OB_ATU_LOWER_BASE, lower_base);
    R_UCIE_RegWrite32(channel, UCIE_OB_ATU_UPPER_BASE, upper_base);
    R_UCIE_RegWrite32(channel, UCIE_OB_ATU_LIMIT_BASE, lower_base + 0xffff);
    R_UCIE_RegWrite32(channel, UCIE_OB_ATU_LOWER_TARGET, lower_target);
    R_UCIE_RegWrite32(channel, UCIE_OB_ATU_UPPER_TARGET, upper_target);
    R_UCIE_RegWrite32(channel, UCIE_OB_REGION_CTL1, 0x00000000);
    R_UCIE_RegWrite32(channel, UCIE_OB_REGION_CTL2, 0x80000000U);

}

void R_UCIE_ControllerInit(uint32_t channel, struct st_pcie_ctrl *ctrl)
{
    /* Config UCIe base address, configuration space, BAR address and range,... */
}
