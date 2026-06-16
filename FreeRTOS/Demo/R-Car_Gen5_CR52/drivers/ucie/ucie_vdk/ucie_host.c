/*
* Copyright (c) 2025 Renesas Electronics Corporation
*
* SPDX-License-Identifier: MIT
*/

#include "pcie/r_pcie_host.h"
#include "ucie.h"

#include "stdio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "interrupts.h"
/* Implement functions for Host */

void rcar_ucie_setup_rc(uint16_t channel)
{
    uint32_t val;

    rcar_ucie_dbi_ro_wr_en(channel, true);

    rcar_ucie_setup(channel);

    /* Setup interrupt pins */
    val = R_UCIE_RegRead32(channel, UCIE_INTERRUPT_LINE);
    val &= 0xffff00ffu;
    val |= 0x00000100;
    R_UCIE_RegWrite32(channel, UCIE_INTERRUPT_LINE, val);

    /* Setup bus numbers */
    val = R_UCIE_RegRead32(channel, UCIE_PRIMARY_BUS);
    val &= 0xff000000u;
    val |= 0x00ff0100;
    R_UCIE_RegWrite32(channel, UCIE_PRIMARY_BUS, val);

    /* Setup command register */
    val = R_UCIE_RegRead32(channel, UCIE_COMMAND);
    val &= 0xffff0000u;
    val |= UCIE_COMMAND_IO | UCIE_COMMAND_MEMORY |
	   UCIE_COMMAND_MASTER | UCIE_COMMAND_SERR;
    R_UCIE_RegWrite32(channel, UCIE_COMMAND, val);

    /* Program correct class for RC */
    R_UCIE_RegWrite16(channel, UCIE_CLASS_DEVICE, UCIE_CLASS_BRIDGE_PCI);

    rcar_ucie_dbi_ro_wr_en(channel, false);
}

void rcar_ucie_hw_enable(uint16_t channel)
{
    /* Configure as Root Port */
    rcar_ucie_reg_write32(channel, false, true, IMP_CORECONFIG_CONFIG0, UCIECTL_DEF_RP_EN);
    rcar_ucie_controller_enable(channel);
}

int ucie_link_up(uint16_t channel)
{
    uint32_t val;

    val = R_UCIE_RegRead32(channel, UCIE_PORT_DEBUG1);
    return ((val & UCIE_PORT_DEBUG1_LINK_UP) &&
              (!(val & UCIE_PORT_DEBUG1_LINK_TRAINING)));
}

void R_PCIE_Host_Outbound_ATU(uint16_t channel)
{
    /* set max payload to 1024byte */
    R_UCIE_RegWrite32(channel, UCIE_EXCAP2, 0x102970);

    /* PORT_LOGIC TRGT_MAP_CTRL_OFF */
    R_UCIE_RegWrite32(channel, UCIE_PRTLGC24, 0x40);

    /* TYPE1_HDR  SEC_STAT_IO_LIMIT_IO_BASE_REG */
    R_UCIE_RegWrite32(channel, UCIE_PCICONF7, 0x101);

    /* TYPE1_HDR  SEC_STAT_IO_LIMIT_IO_BASE_REG */
    R_UCIE_RegWrite32(channel, UCIE_PCICONF12, 0);
    R_UCIE_RegWrite32(channel, UCIE_PCICONF7, 0xf00);

    /* TYPE1_HDR  MEM_LIMIT_MEM_BASE_REG */
    R_UCIE_RegWrite32(channel, UCIE_PCICONF8, 0x223f2220);
    R_UCIE_RegWrite32(channel, UCIE_PCICONF9, 0xffff0001u);
    R_UCIE_RegWrite32(channel, UCIE_PCICONF10, 0x612d3f00);
    R_UCIE_RegWrite32(channel, UCIE_PCICONF11, 0x612d3f0f);

    /* Outbound ATU configuration */
    R_PCIE_Outbound_ATU(channel, 0x62040000, (((uint64_t)UCIE_D2D_CH0_UPPER << 32) | UCIE_D2D_CH0_LOWER));

    /* bus master enable , memory space enable , IO space enable */
    R_UCIE_RegWrite32(channel, UCIE_PCICONF1, 0x100007);
}

void R_PCIE_InitHost(struct st_pcie_host *host, uint16_t channel)
{
    uint32_t ret;

    /* Init a controller in host mode */
    /* FIXME: Confirm the used of these registers */
/*  R_UCIE_RegWrite32(channel, 0xF01000, 0x08010000); //MmInitCtrl
    R_UCIE_RegWrite32(channel, 0xF0211C, 0x0003BF85); //AcsmLtmlndex0Var1
    R_UCIE_RegWrite32(channel, 0xF0212C, 0x00043FED); //AcsmLtmlndex0Var5
    R_UCIE_RegWrite32(channel, 0xF0214C, 0x000CF816); //AcsmLtmlndex0Var13

    writel(0, 0xDCE005E8);
    writel(0, 0xDDE005E8);

    writel(0x00000004, 0xDCE22000);
    writel(0x00000004, 0xDDE22000);

    R_UCIE_RegWrite32(channel, 0x0000A0, 0x00000003);
    R_UCIE_RegWrite32(channel, 0x000118, 0x0000E1E0);
    R_UCIE_RegWrite32(channel, 0, 0xABCD16C3);
    R_UCIE_RegWrite32(channel, 0x1C0, 0x00000200);
    R_UCIE_RegWrite32(channel, 0xC48, 0x00800000);
    R_UCIE_RegWrite32(channel, 0x70, 0x8042B010);
    R_UCIE_RegWrite32(channel, 0x8BC, 0x040BFF48);
*/

    rcar_ucie_hw_enable(channel);
    rcar_ucie_setup_rc(channel);

    ret = ucie_link_up(channel);
    if (ret == 1) {
        printf_delay("UCIe Link up\n");
    } else {
        printf_delay("UCIe Link down\n");
    }
}
