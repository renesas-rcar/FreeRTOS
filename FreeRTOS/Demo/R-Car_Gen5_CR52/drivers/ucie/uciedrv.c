/*
 *
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "ucie/r_ucie.h"
#include "ucie_private.h"
#include "ucie_common.h"
#include "r_ucie_conf_private.h"

#include "state-manager/r_clock_domain_id.h"
#include "state-manager/r_power_domain_id.h"
#include "state-manager/r_reset_domain_id.h"
#include "state-manager/r_state_manager.h"

#define NOT_SCP_SUPPORT 1 // SCP firmware not support.
#if (NOT_SCP_SUPPORT == 1)
#include "clock_controller/clock_controller.h"
#endif

static bool ucie_is_setup[] = {
    [UCIE_CH0] = false,
    [UCIE_CH1] = false
};

static void wait_time(uint32_t count)
{
    for (uint32_t i = 0; i < count; i++)
    {
        __asm__ volatile("nop");
    }
}

static void mem_write32(uintptr_t addr, uint32_t data)
{
    *(volatile uint32_t *)addr = data;
}

static uint32_t mem_read32(uintptr_t addr)
{
    return *(volatile uint32_t *)addr;
}

static void set_pll9_0(uint32_t f_Speed);
static void set_pll9_1(uint32_t f_Speed);

uint32_t R_UCIE_HDMA_SetConfig(st_ucie_hdma_cfg_t *cfg)
{
    uintptr_t base;
    e_ucie_ch_t ucieCh = cfg->ucie_ch;
    e_ucie_hdma_ch_t dmaCh = cfg->hdma_ch;
    uint64_t srcAddr = cfg->mSrcAddr;
    uint64_t dstAddr = cfg->mDestAddr;
    uint32_t size = cfg->size;
    e_ucie_hdma_mode_t rw = cfg->rw;
    
    if (ucieCh != UCIE_CH0 && ucieCh != UCIE_CH1) {
        printf("ERROR: Invalid UCIe channel\n");
        return 1;
    }

    if (dmaCh < HDMA_CH0 || dmaCh > HDMA_CH31) {
        printf("ERROR: Invalid HDMA channel\n");
        return 1;
    }

    if (rw != HDMA_WRITE && rw != HDMA_READ) {
        printf("ERROR: Invalid HDMA transfer mode\n");
        return 1;
    }

    base = UCIE_AXI_BASE(ucieCh) + PF0_HDMA_CAP_BASE_ADD + 
                            (dmaCh * HDMA_CH_BLOCK_SIZE) + (rw * HDMA_RW_BLOCK_SIZE);

    if (mem_read32(base + HDMA_STATUS_OFF) == 0x1) {
        printf("ERROR: Channel is running\n");
        return 1;
    }

    mem_write32(base + HDMA_EN_OFF, 0x00000001);

    mem_write32(base + HDMA_XFERSIZE_OFF, size);
    mem_write32(base + HDMA_SAR_LOW_OFF, (uint32_t)(srcAddr & 0xFFFFFFFF));
    mem_write32(base + HDMA_SAR_HIGH_OFF, (uint32_t)(srcAddr >> 32));
    mem_write32(base + HDMA_DAR_LOW_OFF, (uint32_t)(dstAddr & 0xFFFFFFFF));
    mem_write32(base + HDMA_DAR_HIGH_OFF, (uint32_t)(dstAddr >> 32));

    if (rw) {
        mem_write32(base + HDMA_CONTROL1_OFF, 0x00070010);
    } else {
        mem_write32(base + HDMA_CONTROL1_OFF, 0x00040010);
    }

    mem_write32(base + HDMA_FUNC_NUM_OFF, 0x00000000);
    mem_write32(base + HDMA_QOS_OFF, 0x00000000);

    mem_write32(base + HDMA_WATERMARK_EN_OFF, 0x00000000);
    mem_write32(base + HDMA_INT_SETUP_OFF, 0x00000078);	

    mem_write32(base + HDMA_MSI_STOP_LOW_OFF, MSI_STOP_BASE);
    mem_write32(base + HDMA_MSI_STOP_HIGH_OFF, 0x00000000);

    mem_write32(base + HDMA_MSI_WATERMARK_LOW_OFF, MSI_WATERMARK_BASE);
    mem_write32(base + HDMA_MSI_WATERMARK_HIGH_OFF, 0x00000000);

    mem_write32(base + HDMA_MSI_ABORT_LOW_OFF, MSI_ABORT_BASE);
    mem_write32(base + HDMA_MSI_ABORT_HIGH_OFF, 0x00000000);

    mem_write32(base + HDMA_MSI_MSGD_OFF, MSI_DATA);

    mem_write32(base + HDMA_ELEM_PF_OFF, 0x00000000);
    mem_write32(base + HDMA_LLP_LOW_OFF, 0x00000000);
    mem_write32(base + HDMA_LLP_HIGH_OFF, 0x00000000);
    mem_write32(base + HDMA_CYCLE_OFF, 0x00000000);

    return 0;
}

uint32_t R_UCIE_HDMA_Start(st_ucie_hdma_cfg_t *cfg)
{
    uintptr_t base;
    e_ucie_ch_t ucieCh = cfg->ucie_ch;
    e_ucie_hdma_ch_t dmaCh = cfg->hdma_ch;
    e_ucie_hdma_mode_t rw = cfg->rw;
    
    if (ucieCh != UCIE_CH0 && ucieCh != UCIE_CH1) {
        printf("ERROR: Invalid UCIe channel\n");
        return 1;
    }

    if (dmaCh < HDMA_CH0 || dmaCh > HDMA_CH31) {
        printf("ERROR: Invalid HDMA channel\n");
        return 1;
    }

    if (rw != HDMA_WRITE && rw != HDMA_READ) {
        printf("ERROR: Invalid HDMA transfer mode\n");
        return 1;
    }

    base = UCIE_AXI_BASE(ucieCh) + PF0_HDMA_CAP_BASE_ADD + 
                            (dmaCh * HDMA_CH_BLOCK_SIZE) + (rw * HDMA_RW_BLOCK_SIZE);

    if (mem_read32(base + HDMA_STATUS_OFF) == 0x1) {
        printf("ERROR: Channel is running\n");
        return 1;
    }

    mem_write32(base + HDMA_DOORBELL_OFF, 0x00000001);

    return 0;
}

uint32_t R_UCIE_HDMA_WaitStop(st_ucie_hdma_cfg_t *cfg)
{
    uintptr_t base;
    e_ucie_ch_t ucieCh = cfg->ucie_ch;
    e_ucie_hdma_ch_t dmaCh = cfg->hdma_ch;
    e_ucie_hdma_mode_t rw = cfg->rw;

    uint32_t ret;
    uint32_t val;
    uint32_t timeout;

    if (ucieCh != UCIE_CH0 && ucieCh != UCIE_CH1) {
        printf("ERROR: Invalid UCIe channel\n");
        return 1;
    }

    if (dmaCh < HDMA_CH0 || dmaCh > HDMA_CH31) {
        printf("ERROR: Invalid HDMA channel\n");
        return 1;
    }

    if (rw != HDMA_WRITE && rw != HDMA_READ) {
        printf("ERROR: Invalid HDMA transfer mode\n");
        return 1;
    }

    ret = 0;
    base = UCIE_AXI_BASE(ucieCh) + PF0_HDMA_CAP_BASE_ADD +
                            (dmaCh * HDMA_CH_BLOCK_SIZE) + (rw * HDMA_RW_BLOCK_SIZE);

    timeout = 4500000; // ~3sec
    while (mem_read32(base + HDMA_STATUS_OFF) != 0x3) {
        timeout--;
        if (timeout == 0) {
            ret = 1;
            break;
        }
    }

    val = mem_read32(base + HDMA_INT_STATUS_OFF);
    mem_write32(base + HDMA_INT_CLEAR_OFF, (val & 0x7));

    return ret;
}

uint32_t R_UCIE_HDMA_Stop(st_ucie_hdma_cfg_t *cfg)
{
    uintptr_t base;
    e_ucie_ch_t ucieCh = cfg->ucie_ch;
    e_ucie_hdma_ch_t dmaCh = cfg->hdma_ch;
    e_ucie_hdma_mode_t rw = cfg->rw;

    if (ucieCh != UCIE_CH0 && ucieCh != UCIE_CH1) {
        printf("ERROR: Invalid UCIe channel\n");
        return 1;
    }

    if (dmaCh < HDMA_CH0 || dmaCh > HDMA_CH31) {
        printf("ERROR: Invalid HDMA channel\n");
        return 1;
    }

    if (rw != HDMA_WRITE && rw != HDMA_READ) {
        printf("ERROR: Invalid HDMA transfer mode\n");
        return 1;
    }

    base = UCIE_AXI_BASE(ucieCh) + PF0_HDMA_CAP_BASE_ADD +
                            (dmaCh * HDMA_CH_BLOCK_SIZE) + (rw * HDMA_RW_BLOCK_SIZE);

    mem_write32(base + HDMA_DOORBELL_OFF, 0x00000002);
    mem_write32(base + HDMA_DOORBELL_OFF, 0x00000000);
    mem_write32(base + HDMA_EN_OFF, 0x00000000);

    return 0;
}

void Ucie_Setup_Pre(e_ucie_ch_t ch, e_ucie_mode_t mode)
{
    uintptr_t ucie_axi_base;
    uintptr_t ucie_apb_base;

    ucie_axi_base = UCIE_AXI_BASE(ch);
    ucie_apb_base = UCIE_APB_BASE(ch);

    //; axi adr = 41
    mem_write32(ucie_apb_base + 0xE005E8, 0x00000000);

    mem_write32(ucie_apb_base + 0xE005E8, 0x00004141);

    //; axi adr = 0
    mem_write32(ucie_apb_base + 0xE005E8, 0x00000000);

    //; UCIe0/1 apb setting
    if (mode == UCIE_MODE_RC)
    {
        //; UCIEFMIS
        mem_write32(ucie_apb_base + 0xE00308, 0x019A0000); // UCIEFMIS

        mem_write32(ucie_apb_base + 0xE00000, 0x00000010); // UCIE0 -> RC

        //; UCIEPCR00
        mem_write32(ucie_apb_base + 0xE21000, 0x00000001); // set UCIe device Endpoint enable

        //; UCIECSR00
        mem_write32(ucie_apb_base + 0xE20000, 0x00000001); // select CXL mode
    }
    else
    {
        //; UCIEFMIS
        mem_write32(ucie_apb_base + 0xE00308, 0x015C0000); // UCIEFMIS

        mem_write32(ucie_apb_base + 0xE00000, 0x00000000);

        //; UCIEPCR00
        mem_write32(ucie_apb_base + 0xE21000, 0x00000002); // set UCIe device Endpoint enable

        //; UCIECSR00
        mem_write32(ucie_apb_base + 0xE20000, 0x00000001); // select CXL mode
    }
    //; UCIEICR27
    mem_write32(ucie_apb_base + 0xE1007C, 0x00000002); // interrupt output enable freq_change_req

    //; UCIe0/1 axi setting 00
    if (mode == UCIE_MODE_RC)
    {
        mem_write32( ucie_axi_base + PF0_PCIE_CAP_LINK_CONTROL2_LINK_STATUS2_REG_ADD, 0x00000004 );
        // NOTE: EVB code was originally writing 0x0000_0003, which corresponds to 8 GT/s in the context of PCIe
    }
    mem_write32( ucie_axi_base + RCAR_UCIE_BASE_ADD(0x38C), 0x00000000 );
    mem_write32( ucie_axi_base + RCAR_UCIE_BASE_ADD(0x38C), 0x00000000 );
    // NOTE: above two lines do not exist on the EVB code

    mem_write32(ucie_axi_base + PF0_PL32G_CAP_PL32G_CONTROL_REG_ADD, 0x00000200); // PL32G_CONTROL_REG. Modified TS Usage Mode Selected.Alternate Protocol Negtiation.
    mem_write32(ucie_axi_base + PF0_PORT_LOGIC_CXL_VLSM_CSR_REG_OFF_ADD, 0x00800000); // CXL VLSM CSR REGISTER

    //; RASDP_ERROR_TRACER_CAPABILITY_OFF
    // VTB_adr : 0x00000420, C code adr : 0xd8b003bc
    mem_write32( ucie_axi_base + RCAR_UCIE_BASE_ADD(0x420), 0x00000027 );
    // NOTE: above two lines do not exist on the EVB code

    if(mode != UCIE_MODE_RC){
        mem_write32( ucie_axi_base + TMP_CXL_RCIEP_FLEXBUS_CNTRL_STATUS_OFF, 0x00000007 );	//CXL_RCIEP_FLEXBUS_CNTRL_STATUS_OFF
        mem_write32( ucie_axi_base + PF0_PCIE_CAP_PCIE_CAP_ID_PCIE_NEXT_CAP_PTR_PCIE_CAP_REG_ADD, 0x8002D010U ); // PCIE_CAP_ID_PCIE_NEXT_CAP_PTR_PCIE_CAP_REG
        mem_write32( ucie_axi_base + PF0_PORT_LOGIC_MISC_CONTROL_1_OFF_ADD, 0x080BFF48 ); // MISC_CONTROL_1_OFF
    }else{
        mem_write32( ucie_axi_base + PF0_PCIE_CAP_PCIE_CAP_ID_PCIE_NEXT_CAP_PTR_PCIE_CAP_REG_ADD, 0x8042D010U ); // PCIE_CAP_ID_PCIE_NEXT_CAP_PTR_PCIE_CAP_REG
        mem_write32( ucie_axi_base + PF0_PORT_LOGIC_MISC_CONTROL_1_OFF_ADD, 0x040BFF48 ); // MISC_CONTROL_1_OFF
    }
    
    mem_write32( ucie_axi_base + RCAR_UCIE_BASE_ADD(0x38c), 0x00000000 );
    
    //; ULT_PLLCTRL4 -> PF0_MEMBAR0_RAS_CAP_MEMBAR0_RAS_UNCOR_ERROR_MASK_REG_OFF_ADD
    mem_write32(ucie_axi_base + PF0_MEMBAR0_RAS_CAP_MEMBAR0_RAS_UNCOR_ERROR_MASK_REG_OFF_ADD, 0x00000000); // elbi
    mem_write32(ucie_axi_base + PF0_MEMBAR0_RAS_CAP_MEMBAR0_RAS_CORR_ERROR_MASK_REG_OFF_ADD, 0x00000000); // elbi

    //; UCIe0 axi setting 01
    //;  axi0 addres ON
    mem_write32(ucie_apb_base + 0xE005E8, 0x00004141);

    mem_write32(ucie_axi_base + IMP_SPECIFIC_SB_UNIT_IMP_SB_CONFIG3_ADD, 0x00018001);

    mem_write32(ucie_axi_base + IMP_SPECIFIC_SB_UNIT_IMP_SB_CONFIG0_ADD, 0x000a0190);

    mem_write32(ucie_axi_base + IMP_SPECIFIC_SB_UNIT_IMP_SB_CONFIG2_ADD, 0x000a0190);

    if(mode == UCIE_MODE_RC){
        mem_write32( ucie_axi_base + IMP_SPECIFIC_SB_UNIT_IMP_SB_CONFIG4_ADD, 0x002000AE ); //DSP
    } else {
        mem_write32( ucie_axi_base + IMP_SPECIFIC_SB_UNIT_IMP_SB_CONFIG4_ADD, 0x004000AE ); //USP
    }
    mem_write32( ucie_axi_base + IMP_SPECIFIC_SB_UNIT_IMP_SB_CONFIG5_ADD, 0x00000000 );

    mem_write32(ucie_axi_base + IMP_SPECIFIC_MB_UNIT_IMP_MB_CONFIG4_ADD, 0x00023500);

    //;  axi0 addres OFF
    mem_write32(ucie_apb_base + 0xE005E8, 0x00000000);

    mem_write32(ucie_axi_base + DVSEC_UNIT_DSP_DVSEC_UCIE_LINK_CONTROL_ADD, 0x00004000);

#ifdef RCAR_UCIE_V100
    mem_write32( ucie_axi_base + CXL_DVSEC_UNIT_DSP_CXL_DVSEC_FLEX_CTL_STATUS_ADD, 0x00000027 );
#endif
#if defined(RCAR_UCIE_V101) || defined(RCAR_UCIE_V102)
    // VTB_adr : 0x0000042c, C code adr : 0xd8000420
    mem_write32( ucie_axi_base + RCAR_UCIE_BASE_ADD(0x42C), 0x00000027 ); // from UT result
#endif
    //; UCIe0 axi setting 02
    //;  axi addres ON
    mem_write32(ucie_apb_base + 0xE005E8, 0x00004141);

#if defined(RCAR_UCIE_V101) || defined(RCAR_UCIE_V102)
    mem_write32( ucie_axi_base + IMP_SPECIFIC_MB_UNIT_IMP_MB_CONFIG11_ADD, 0x00103001 );

    mem_write32( ucie_axi_base + IMP_SPECIFIC_MB_UNIT_IMP_MB_CONFIG11_ADD, 0x00103001 );
#endif
#ifdef RCAR_UCIE_V100
    mem_write32( ucie_axi_base + IMP_SPECIFIC_MB_UNIT_IMP_MB_CONFIG11_ADD, 0x00103000 );
    mem_write32( ucie_axi_base + IMP_SPECIFIC_MB_UNIT_IMP_MB_CONFIG11_ADD, 0x00103000 ); // X5H: bit[0] = 0 
#endif

#ifdef RCAR_UCIE_V100
    mem_write32(ucie_axi_base + MMPL_MMTRKCTRL_ADD, 0x00000002);

    mem_write32(ucie_axi_base + MMPL_MODULEDEGRADESTATUS_ADD, 0x0000FFFC);

    mem_write32( ucie_axi_base + MMPL_ZCALCTRL1_ADD, 0x0C839CC0 );

    mem_write32( ucie_axi_base + MMPL_ZCALCTRL4_ADD, 0x00008483 );

    mem_write32(ucie_axi_base + MMPL_PLLCTRL0_ADD, 0x287F0D08); // 7'b111_1111<<24, 7'b010_1000<<24

    mem_write32( ucie_axi_base + MMPL_PLLCTRL1_ADD, 0x00002296 );

    mem_write32(ucie_axi_base + MMPL_PLLCTRL3_ADD, 0x50601009);

    mem_write32(ucie_axi_base + MMPL_PLLCTRL4_ADD, 0x041B6001);

    mem_write32(ucie_axi_base + MMPL_MMMODECTRL_ADD, 0x00000012);
#endif

    mem_write32(ucie_axi_base + DWORD_0_DWMODECTRL0_ADD, 0x0600000C);

    mem_write32(ucie_axi_base + DWORD_0_DWMODULEDEGRADESTATUS_ADD, 0x0000FFFC);

    mem_write32(ucie_axi_base + DWORD_0_DWVREFVAR_ADD, 0x007F0001);

    mem_write32(ucie_axi_base + DWORD_0_DWTXZCALSB_ADD, 0x00001B1D);

    mem_write32(ucie_axi_base + DWORD_0_DWPERBITCTRL0_ADD, 0x00440000);
    mem_write32(ucie_axi_base + DWORD_0_DWPERBITCTRL1_ADD, 0x00440000);
    mem_write32(ucie_axi_base + DWORD_0_DWPERBITCTRL2_ADD, 0x00440000);
    mem_write32(ucie_axi_base + DWORD_0_DWPERBITCTRL3_ADD, 0x00440000);
    mem_write32(ucie_axi_base + DWORD_0_DWPERBITCTRL4_ADD, 0x00440000);
    mem_write32(ucie_axi_base + DWORD_0_DWPERBITCTRL5_ADD, 0x00440000);
    mem_write32(ucie_axi_base + DWORD_0_DWPERBITCTRL6_ADD, 0x00440000);
    mem_write32(ucie_axi_base + DWORD_0_DWPERBITCTRL7_ADD, 0x00440000);
    mem_write32(ucie_axi_base + DWORD_0_DWPERBITCTRL8_ADD, 0x00440000);
    mem_write32(ucie_axi_base + DWORD_0_DWPERBITCTRL9_ADD, 0x00440000);
    mem_write32(ucie_axi_base + DWORD_0_DWPERBITCTRL10_ADD, 0x00440000);
    mem_write32(ucie_axi_base + DWORD_0_DWPERBITCTRL11_ADD, 0x00440000);
    mem_write32(ucie_axi_base + DWORD_0_DWPERBITCTRL12_ADD, 0x00440000);
    mem_write32(ucie_axi_base + DWORD_0_DWPERBITCTRL13_ADD, 0x00440000);
    mem_write32(ucie_axi_base + DWORD_0_DWPERBITCTRL14_ADD, 0x00440000);
    mem_write32(ucie_axi_base + DWORD_0_DWPERBITCTRL15_ADD, 0x00440000);
    mem_write32(ucie_axi_base + DWORD_0_DWPERBITCTRL16_ADD, 0x00440000);
    mem_write32(ucie_axi_base + DWORD_0_DWPERBITCTRL17_ADD, 0x00440000);
    mem_write32(ucie_axi_base + DWORD_0_DWPERBITCTRL18_ADD, 0x00440000);
    mem_write32(ucie_axi_base + DWORD_0_DWPERBITCTRL19_ADD, 0x00440000);

    mem_write32(ucie_axi_base + DWORD_0_DWMISCCTRL0_ADD, 0x3D00A001);

    mem_write32(ucie_axi_base + DWORD_0_DWMISCCTRL1_ADD, 0x01240800);

    mem_write32( ucie_axi_base + DWORD_1_DWMODECTRL0_ADD, 0x0600000C );
    
    mem_write32( ucie_axi_base + DWORD_1_DWMODULEDEGRADESTATUS_ADD, 0x0000FFFC );
    
    mem_write32( ucie_axi_base + DWORD_1_DWVREFVAR_ADD, 0x007F0001 );

    mem_write32(ucie_axi_base + DWORD_1_DWTXZCALSB_ADD, 0x00001B1D);

    mem_write32(ucie_axi_base + DWORD_1_DWPERBITCTRL0_ADD, 0x00440000);
    mem_write32(ucie_axi_base + DWORD_1_DWPERBITCTRL1_ADD, 0x00440000);
    mem_write32(ucie_axi_base + DWORD_1_DWPERBITCTRL2_ADD, 0x00440000);
    mem_write32(ucie_axi_base + DWORD_1_DWPERBITCTRL3_ADD, 0x00440000);
    mem_write32(ucie_axi_base + DWORD_1_DWPERBITCTRL4_ADD, 0x00440000);
    mem_write32(ucie_axi_base + DWORD_1_DWPERBITCTRL5_ADD, 0x00440000);
    mem_write32(ucie_axi_base + DWORD_1_DWPERBITCTRL6_ADD, 0x00440000);
    mem_write32(ucie_axi_base + DWORD_1_DWPERBITCTRL7_ADD, 0x00440000);
    mem_write32(ucie_axi_base + DWORD_1_DWPERBITCTRL8_ADD, 0x00440000);
    mem_write32(ucie_axi_base + DWORD_1_DWPERBITCTRL9_ADD, 0x00440000);
    mem_write32(ucie_axi_base + DWORD_1_DWPERBITCTRL10_ADD, 0x00440000);
    mem_write32(ucie_axi_base + DWORD_1_DWPERBITCTRL11_ADD, 0x00440000);
    mem_write32(ucie_axi_base + DWORD_1_DWPERBITCTRL12_ADD, 0x00440000);
    mem_write32(ucie_axi_base + DWORD_1_DWPERBITCTRL13_ADD, 0x00440000);
    mem_write32(ucie_axi_base + DWORD_1_DWPERBITCTRL14_ADD, 0x00440000);
    mem_write32(ucie_axi_base + DWORD_1_DWPERBITCTRL15_ADD, 0x00440000);
    mem_write32(ucie_axi_base + DWORD_1_DWPERBITCTRL16_ADD, 0x00440000);
    mem_write32(ucie_axi_base + DWORD_1_DWPERBITCTRL17_ADD, 0x00440000);
    mem_write32(ucie_axi_base + DWORD_1_DWPERBITCTRL18_ADD, 0x00440000);
    mem_write32(ucie_axi_base + DWORD_1_DWPERBITCTRL19_ADD, 0x00440000);
    
    mem_write32( ucie_axi_base + DWORD_1_DWMISCCTRL0_ADD, 0x3D00A001 );

    mem_write32(ucie_axi_base + DWORD_1_DWMISCCTRL1_ADD, 0x01240800);
   
#if (defined(RCAR_UCIE_V102) && defined( CONW_RCAR_UCIE_V102))
    if (UCIE_CH1 == ch)
    {
        mem_write32( ucie_axi_base + DWORD_0_DWMISCCTRL0_ADD, 0x3D002001U );
        mem_write32( ucie_axi_base + DWORD_1_DWMISCCTRL0_ADD, 0x3D002001U );
    }
    else
    {
        mem_write32( ucie_axi_base + DWORD_0_DWMISCCTRL0_ADD, 0x3D00A001U );
        mem_write32( ucie_axi_base + DWORD_1_DWMISCCTRL0_ADD, 0x3D00A001U );
    }
#else
	mem_write32( ucie_axi_base + DWORD_0_DWMISCCTRL0_ADD, 0x3D00A001U );
    mem_write32( ucie_axi_base + DWORD_1_DWMISCCTRL0_ADD, 0x3D00A001U );
#endif
    
    uint32_t reg_val;
#if (defined(RCAR_UCIE_V102) && defined(CONW_RCAR_UCIE_V100))
    // Setting CSR for reversalMB
    mem_write32( ucie_axi_base + ACSMIM_ACSMINSTRREG25_ADD, 0x83000061U);
    mem_write32( ucie_axi_base + ACSM_ACSMLTSMMSK0VAR4_ADD, 0x48E23803);
    mem_write32( ucie_axi_base + ACSM_ACSMLTSMMSK0VAR5_ADD, 0x48E23803);
    mem_write32( ucie_axi_base + ACSM_ACSMLTSMMSK0VAR6_ADD, 0x48E39805);
    mem_write32( ucie_axi_base + ACSM_ACSMLTSMMSK0VAR7_ADD, 0x4BDFD805);
    
    reg_val = mem_read32(ucie_axi_base + MMPL_MMMISCCTRL_ADD);
    reg_val = reg_val & MMPL_MMMISCCTRL_MASK;
    reg_val = reg_val &      ~( MMPL_MMMISCCTRL_MMPHYCOMPLIANCE_MASK  << MMPL_MMMISCCTRL_MMPHYCOMPLIANCE_SHIFT);
    reg_val = reg_val | ((0x1U & MMPL_MMMISCCTRL_MMPHYCOMPLIANCE_MASK) << MMPL_MMMISCCTRL_MMPHYCOMPLIANCE_SHIFT);
    mem_write32( ucie_axi_base + MMPL_MMMISCCTRL_ADD, reg_val );
    
    reg_val = mem_read32(ucie_axi_base + DWORD_0_DWMISCCTRL0_ADD);
    reg_val = reg_val & DWORD_0_DWMISCCTRL0_MASK;
    reg_val = reg_val &      ~( DWORD_0_DWMISCCTRL0_DWPHYCOMPLIANCE_MASK  << DWORD_0_DWMISCCTRL0_DWPHYCOMPLIANCE_SHIFT);
    reg_val = reg_val | ((0x1U & DWORD_0_DWMISCCTRL0_DWPHYCOMPLIANCE_MASK) << DWORD_0_DWMISCCTRL0_DWPHYCOMPLIANCE_SHIFT);
    mem_write32( ucie_axi_base + DWORD_0_DWMISCCTRL0_ADD, reg_val );
    
    reg_val = mem_read32(ucie_axi_base + DWORD_1_DWMISCCTRL0_ADD);
    reg_val = reg_val & DWORD_1_DWMISCCTRL0_MASK;
    reg_val = reg_val &      ~( DWORD_1_DWMISCCTRL0_DWPHYCOMPLIANCE_MASK  << DWORD_1_DWMISCCTRL0_DWPHYCOMPLIANCE_SHIFT);
    reg_val = reg_val | ((0x1U & DWORD_1_DWMISCCTRL0_DWPHYCOMPLIANCE_MASK) << DWORD_1_DWMISCCTRL0_DWPHYCOMPLIANCE_SHIFT);
    mem_write32( ucie_axi_base + DWORD_1_DWMISCCTRL0_ADD, reg_val );
    
    reg_val = mem_read32(ucie_axi_base + ACSM_ACSMTRAINVAR0I0_ADD);
    reg_val = reg_val & ACSM_ACSMTRAINVAR0I0_MASK;
    reg_val = reg_val | ((uint32_t)0x1 << 16U);
    mem_write32(ucie_axi_base + ACSM_ACSMTRAINVAR0I0_ADD, reg_val);

    reg_val = mem_read32(ucie_axi_base + ACSM_ACSMTRAINVAR0I1_ADD);
    reg_val = reg_val & ACSM_ACSMTRAINVAR0I1_MASK;
    reg_val = reg_val | ((uint32_t)0x1 << 16U);
    mem_write32(ucie_axi_base + ACSM_ACSMTRAINVAR0I1_ADD, reg_val);

    reg_val = mem_read32(ucie_axi_base + ACSM_ACSMTRAINVAR0I2_ADD);
    reg_val = reg_val & ACSM_ACSMTRAINVAR0I2_MASK;
    reg_val = reg_val | ((uint32_t)0x1 << 16U);
    mem_write32(ucie_axi_base + ACSM_ACSMTRAINVAR0I2_ADD, reg_val);

    reg_val = mem_read32(ucie_axi_base + ACSM_ACSMTRAINVAR1I1_ADD);
    reg_val = reg_val & ACSM_ACSMTRAINVAR1I1_MASK;
    reg_val = reg_val & 0xFFFFFEFF;
    mem_write32(ucie_axi_base + ACSM_ACSMTRAINVAR1I1_ADD, reg_val);

    // Setting PLL clock for X5H connection
    mem_write32(ucie_axi_base + MMPL_PLLCTRL0_P0_ADD, 0x3F3F100C);
    mem_write32(ucie_axi_base + MMPL_PLLCTRL0_P1_ADD, 0x3F3F100C);
    mem_write32(ucie_axi_base + MMPL_PLLCTRL0_P2_ADD, 0x303F0808);
    mem_write32(ucie_axi_base + MMPL_PLLCTRL0_P3_ADD, 0x3F3F1004);
    mem_write32(ucie_axi_base + MMPL_PLLCTRL1_P0_ADD, 0x14830C55);
    mem_write32(ucie_axi_base + MMPL_PLLCTRL1_P1_ADD, 0x14870C04);
    mem_write32(ucie_axi_base + MMPL_PLLCTRL1_P2_ADD, 0x03870C04);
    mem_write32(ucie_axi_base + MMPL_PLLCTRL1_P3_ADD, 0x06C70C04);
    mem_write32(ucie_axi_base + MMPL_PLLCTRL3_ADD, 0x04236B49);
    mem_write32(ucie_axi_base + MMPL_PLLCTRL4_ADD, 0x50601009);
    mem_write32(ucie_axi_base + ACSM_ACSMWAITDLY0_ADD, 0x000007D0);
    mem_write32(ucie_axi_base + ACSM_ACSMWAITDLY1_ADD, 0x00004A38);
#endif

#ifdef RCAR_UCIE_V100
    // old PHY X5H
    ////nat+:AIACC:20250328:start: Setting CSR for reversalMB
    mem_write32( ucie_axi_base + ACSMIM_ACSMINSTRREG95_ADD      , 0x000FFF53); 
    mem_write32( ucie_axi_base + ACSMIM_ACSMINSTRREG96_ADD      , 0x000FFFB3); 
    mem_write32( ucie_axi_base + ACSMIM_ACSMINSTRREG97_ADD      , 0x00080023); 
    mem_write32( ucie_axi_base + ACSMIM_ACSMINSTRREG98_ADD      , 0x0000B00B); 
    mem_write32( ucie_axi_base + ACSMIM_ACSMINSTRREG99_ADD      , 0x03970B6A); 
    // mem_write32( ACSM_ACSMLTSMINDEX0VAR14_ADD , 32'h0010_0000, 32'h0010_0000); //AcsmLtsmIndex0Var14[20] == 1  (1DVrefSel)
    reg_val = mem_read32(ucie_axi_base + ACSM_ACSMLTSMINDEX0VAR14_ADD);
    reg_val = reg_val & ACSM_ACSMLTSMINDEX0VAR14_MASK;
    reg_val = reg_val | ((uint32_t)0x1 << 20U);
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMINDEX0VAR14_ADD, reg_val);

    // mem_write32( ACSM_ACSMLTSMINDEX0VAR16_ADD , 32'h0010_0000, 32'h0010_0000); //AcsmLtsmIndex0Var16[20] == 1  (1DVrefSel)
    reg_val = mem_read32(ucie_axi_base + ACSM_ACSMLTSMINDEX0VAR16_ADD);
    reg_val = reg_val & ACSM_ACSMLTSMINDEX0VAR16_MASK;
    reg_val = reg_val | ((uint32_t)0x1 << 20U);
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMINDEX0VAR16_ADD, reg_val);

    // mem_write32( ucie_axi_base + ACSM_ACSMTRAINVAR0I0_ADD     , 32'h0001_0000, 32'h0001_0000); //AcsmTrainVar0I0[16] == 1
    reg_val = mem_read32(ucie_axi_base + ACSM_ACSMTRAINVAR0I0_ADD);
    reg_val = reg_val & ACSM_ACSMTRAINVAR0I0_MASK;
    reg_val = reg_val | ((uint32_t)0x1 << 16U);
    mem_write32(ucie_axi_base + ACSM_ACSMTRAINVAR0I0_ADD, reg_val);

    // mem_write32( ucie_axi_base + ACSM_ACSMTRAINVAR0I1_ADD     , 32'h0001_0000, 32'h0001_0000); //AcsmTrainVar0I1[16] == 1
    reg_val = mem_read32(ucie_axi_base + ACSM_ACSMTRAINVAR0I1_ADD);
    reg_val = reg_val & ACSM_ACSMTRAINVAR0I1_MASK;
    reg_val = reg_val | ((uint32_t)0x1 << 16U);
    mem_write32(ucie_axi_base + ACSM_ACSMTRAINVAR0I1_ADD, reg_val);

    // mem_write32( ucie_axi_base + ACSM_ACSMTRAINVAR0I2_ADD     , 32'h0001_0000, 32'h0001_0000); //AcsmTrainVar0I2[16] == 1
    reg_val = mem_read32(ucie_axi_base + ACSM_ACSMTRAINVAR0I2_ADD);
    reg_val = reg_val & ACSM_ACSMTRAINVAR0I2_MASK;
    reg_val = reg_val | ((uint32_t)0x1 << 16U);
    mem_write32(ucie_axi_base + ACSM_ACSMTRAINVAR0I2_ADD, reg_val);

    // mem_write32( ucie_axi_base + ACSM_ACSMTRAINVAR1I1_ADD     , 32'h0000_0100, 32'h0000_0000); //AcsmTrainVar1I1[8] = 0
    reg_val = mem_read32(ucie_axi_base + ACSM_ACSMTRAINVAR1I1_ADD);
    reg_val = reg_val & ACSM_ACSMTRAINVAR1I1_MASK;
    reg_val = reg_val & 0xFFFFFEFF;
    mem_write32(ucie_axi_base + ACSM_ACSMTRAINVAR1I1_ADD, reg_val);

    // setup_highspeed_link_initialization_and_training_steps_x5h(); //tmk+:AIACC:20250407
    // task setup_highspeed_link_initialization_and_training_steps_x5h();
    //// 2. Setup high-speed link initialization and training steps.
    //// a. Write MmTrkCtrl.MmTrkEn = 1'b0 to disable periodic track run time calibration.
    mem_write32( ucie_axi_base + RCAR_UCIE_BASE_ADD(0xF01128), 0x00000000 );
    
    //// b. Write DwRxLatCtrl.DwRxVldMargin = 1 to setup VLD lane training variables.
    reg_val = mem_read32(ucie_axi_base + DWORD_0_DWRXLATCTRL_ADD);
    reg_val = reg_val & DWORD_0_DWRXLATCTRL_MASK;
    reg_val = reg_val &      ~( DWORD_0_DWRXLATCTRL_DWRXVLDMARGIN_MASK  << DWORD_0_DWRXLATCTRL_DWRXVLDMARGIN_SHIFT);
    reg_val = reg_val | ((0x1U & DWORD_0_DWRXLATCTRL_DWRXVLDMARGIN_MASK) << DWORD_0_DWRXLATCTRL_DWRXVLDMARGIN_SHIFT);
    mem_write32( ucie_axi_base + DWORD_0_DWRXLATCTRL_ADD, reg_val );
    
    reg_val = mem_read32(ucie_axi_base + DWORD_1_DWRXLATCTRL_ADD);
    reg_val = reg_val & DWORD_1_DWRXLATCTRL_MASK;
    reg_val = reg_val &      ~( DWORD_1_DWRXLATCTRL_DWRXVLDMARGIN_MASK  << DWORD_1_DWRXLATCTRL_DWRXVLDMARGIN_SHIFT);
    reg_val = reg_val | ((0x1U & DWORD_1_DWRXLATCTRL_DWRXVLDMARGIN_MASK) << DWORD_1_DWRXLATCTRL_DWRXVLDMARGIN_SHIFT);
    mem_write32( ucie_axi_base + DWORD_1_DWRXLATCTRL_ADD, reg_val );
    
    //// d. Write 00800000 to UcieTrainingSetup1 to program PPGC pattern.
    mem_write32( ucie_apb_base+0xE005E8, 0x00004040 );
    
    // adr 0x4070_1*** is none in .h file
    mem_write32( ucie_axi_base + RCAR_UCIE_BASE_ADD(0x40701000) + 0x010, 0x00800000 );
    mem_write32( ucie_axi_base + RCAR_UCIE_BASE_ADD(0x40701000) + 0x014, 0x00800000 );
    
    //// e. Write 00010000 to UcieTrainingSetup2 to program PPGC pattern.
    // adr 0x4070_1*** is none in .h file
    mem_write32( ucie_axi_base + RCAR_UCIE_BASE_ADD(0x40701000) + 0x020, 0x00010000 );
    mem_write32( ucie_axi_base + RCAR_UCIE_BASE_ADD(0x40701000) + 0x024, 0x00010000 );

    //// f. Write DwMiscCtrl0.DwTxCkParkLevel=1'b1, clock is parked at its inactive level.
    mem_write32( ucie_apb_base+0xE005E8, 0x00004141 );
    
    mem_write32( ucie_axi_base + DWORD_0_DWMISCCTRL0_ADD, 0x3D00A001 );
    
    reg_val = mem_read32(ucie_axi_base + DWORD_1_DWMISCCTRL0_ADD);
    reg_val = reg_val & DWORD_1_DWMISCCTRL0_MASK;
    reg_val = reg_val &      ~( DWORD_1_DWMISCCTRL0_DWTXCKPARKLEVEL_MASK  << DWORD_1_DWMISCCTRL0_DWTXCKPARKLEVEL_SHIFT);
    reg_val = reg_val | (((uint32_t)0x1 & DWORD_1_DWMISCCTRL0_DWTXCKPARKLEVEL_MASK) << DWORD_1_DWMISCCTRL0_DWTXCKPARKLEVEL_SHIFT);
    mem_write32( ucie_axi_base + DWORD_1_DWMISCCTRL0_ADD, reg_val );
    
    //// g. Program DwModeCtrl0 with the following values.
    //// i. Write DwModeCtrl0.DwRxCtlClkSel=0x1 to select the receive clock.
    //// ii. Write DwModeCtrl0.DwRxLatAlign=0x1 to select the read latency alignment type.
    reg_val = mem_read32(ucie_axi_base + DWORD_0_DWMODECTRL0_ADD);
    reg_val = reg_val & DWORD_0_DWMODECTRL0_MASK;
    reg_val = reg_val &      ~( DWORD_0_DWMODECTRL0_DWRXCTLCLKSEL_MASK  << DWORD_0_DWMODECTRL0_DWRXCTLCLKSEL_SHIFT);
    reg_val = reg_val | (((uint32_t)0x1 & DWORD_0_DWMODECTRL0_DWRXCTLCLKSEL_MASK) << DWORD_0_DWMODECTRL0_DWRXCTLCLKSEL_SHIFT);
    reg_val = reg_val &      ~( DWORD_0_DWMODECTRL0_DWRXLATALIGN_MASK  << DWORD_0_DWMODECTRL0_DWRXLATALIGN_SHIFT);
    reg_val = reg_val | ((0x1U & DWORD_0_DWMODECTRL0_DWRXLATALIGN_MASK) << DWORD_0_DWMODECTRL0_DWRXLATALIGN_SHIFT);
    mem_write32( ucie_axi_base + DWORD_0_DWMODECTRL0_ADD, reg_val );
    
    reg_val = mem_read32(ucie_axi_base + DWORD_1_DWMODECTRL0_ADD);
    reg_val = reg_val & DWORD_1_DWMODECTRL0_MASK;
    reg_val = reg_val &      ~( DWORD_1_DWMODECTRL0_DWRXCTLCLKSEL_MASK  << DWORD_1_DWMODECTRL0_DWRXCTLCLKSEL_SHIFT);
    reg_val = reg_val | (((uint32_t)0x1 & DWORD_1_DWMODECTRL0_DWRXCTLCLKSEL_MASK) << DWORD_1_DWMODECTRL0_DWRXCTLCLKSEL_SHIFT);
    reg_val = reg_val &      ~( DWORD_1_DWMODECTRL0_DWRXLATALIGN_MASK  << DWORD_1_DWMODECTRL0_DWRXLATALIGN_SHIFT);
    reg_val = reg_val | ((0x1U & DWORD_1_DWMODECTRL0_DWRXLATALIGN_MASK) << DWORD_1_DWMODECTRL0_DWRXLATALIGN_SHIFT);
    mem_write32( ucie_axi_base + DWORD_1_DWMODECTRL0_ADD, reg_val );
    
    //// h. Write AcsmLtsmIndex0Var14=0x14efef to enable 1D Vref training.
    reg_val = mem_read32(ucie_axi_base + ACSM_ACSMLTSMINDEX0VAR14_ADD);
    reg_val = reg_val & ACSM_ACSMLTSMINDEX0VAR14_MASK;
    reg_val = reg_val &           ~( ACSM_ACSMLTSMINDEX0VAR14_ACSMLTSMINDEX0VAR14_MASK  << ACSM_ACSMLTSMINDEX0VAR14_ACSMLTSMINDEX0VAR14_SHIFT);
    reg_val = reg_val | ((0x14EFEFU & ACSM_ACSMLTSMINDEX0VAR14_ACSMLTSMINDEX0VAR14_MASK) << ACSM_ACSMLTSMINDEX0VAR14_ACSMLTSMINDEX0VAR14_SHIFT);
    mem_write32( ucie_axi_base + ACSM_ACSMLTSMINDEX0VAR14_ADD, reg_val );
    
    // adr symbol is none
    reg_val = mem_read32(ucie_axi_base + (RCAR_UCIE_BASE_ADD(0x41F12000)+0x150) );
    reg_val = reg_val & ACSM_ACSMLTSMINDEX0VAR14_MASK;
    reg_val = reg_val &           ~( ACSM_ACSMLTSMINDEX0VAR14_ACSMLTSMINDEX0VAR14_MASK  << ACSM_ACSMLTSMINDEX0VAR14_ACSMLTSMINDEX0VAR14_SHIFT);
    reg_val = reg_val | ((0x14EFEFU & ACSM_ACSMLTSMINDEX0VAR14_ACSMLTSMINDEX0VAR14_MASK) << ACSM_ACSMLTSMINDEX0VAR14_ACSMLTSMINDEX0VAR14_SHIFT);
    mem_write32( ucie_axi_base + (RCAR_UCIE_BASE_ADD(0x41F12000)+0x150), reg_val );
    
    //// i. Write AcsmLtsmIndex0Var16=0x10f7f1 to enable 1D Vref training.
    reg_val = mem_read32(ucie_axi_base + ACSM_ACSMLTSMINDEX0VAR16_ADD );
    reg_val = reg_val & ACSM_ACSMLTSMINDEX0VAR16_MASK;
    reg_val = reg_val &           ~( ACSM_ACSMLTSMINDEX0VAR16_ACSMLTSMINDEX0VAR16_MASK  << ACSM_ACSMLTSMINDEX0VAR16_ACSMLTSMINDEX0VAR16_SHIFT);
    reg_val = reg_val | ((0x10F7F1U & ACSM_ACSMLTSMINDEX0VAR16_ACSMLTSMINDEX0VAR16_MASK) << ACSM_ACSMLTSMINDEX0VAR16_ACSMLTSMINDEX0VAR16_SHIFT);
    mem_write32( ucie_axi_base + ACSM_ACSMLTSMINDEX0VAR16_ADD, reg_val );
    
    // adr symbol is none
    reg_val = mem_read32(ucie_axi_base + (RCAR_UCIE_BASE_ADD(0x41F12000)+0x158) );
    reg_val = reg_val & ACSM_ACSMLTSMINDEX0VAR16_MASK;
    reg_val = reg_val &           ~( ACSM_ACSMLTSMINDEX0VAR16_ACSMLTSMINDEX0VAR16_MASK  << ACSM_ACSMLTSMINDEX0VAR16_ACSMLTSMINDEX0VAR16_SHIFT);
    reg_val = reg_val | ((0x10F7F1U & ACSM_ACSMLTSMINDEX0VAR16_ACSMLTSMINDEX0VAR16_MASK) << ACSM_ACSMLTSMINDEX0VAR16_ACSMLTSMINDEX0VAR16_SHIFT);
    mem_write32( ucie_axi_base + (RCAR_UCIE_BASE_ADD(0x41F12000)+0x158), reg_val );
    
    //// j. Write 00001600 to AcsmLoopVar1 so that LTSM will go from RESET to LINKINIT
    mem_write32( ucie_axi_base + ACSM_ACSMLOOPVAR1_ADD, 0x0001600 );
    
    mem_write32( ucie_axi_base + ACSM_ACSMTIMEOUTCTRL1_ADD, 0x00000000 );
    // NOTE: Adopted from EVB code
    
    //update_instr_code_x5h();
    ////Instruction registers
    mem_write32(ucie_axi_base + ACSMIM_ACSMINSTRREG5_ADD, 0x03910133); // write_reg_field('h00000000_41F03014,32'hFFFF_FFFF, 32'h03910133  );//   AcsmInstrReg5
    mem_write32(ucie_axi_base + ACSMIM_ACSMINSTRREG6_ADD, 0x022403A3); // write_reg_field('h00000000_41F03018,32'hFFFF_FFFF, 32'h022403A3  );//   AcsmInstrReg6
    mem_write32(ucie_axi_base + ACSMIM_ACSMINSTRREG7_ADD, 0x03910E33); // write_reg_field('h00000000_41F0301C,32'hFFFF_FFFF, 32'h03910E33  );//   AcsmInstrReg7
    mem_write32(ucie_axi_base + ACSMIM_ACSMINSTRREG8_ADD, 0x022404A3); // write_reg_field('h00000000_41F03020,32'hFFFF_FFFF, 32'h022404A3  );//   AcsmInstrReg8
    mem_write32(ucie_axi_base + ACSMIM_ACSMINSTRREG9_ADD, 0x02210143); // write_reg_field('h00000000_41F03024,32'hFFFF_FFFF, 32'h02210143  );//   AcsmInstrReg9

    mem_write32(ucie_axi_base + ACSMIM_ACSMINSTRREG10_ADD, 0x02200000); // write_reg_field('h00000000_41F03028,32'hFFFF_FFFF, 32'h02200000  );//   AcsmInstrReg10
    mem_write32(ucie_axi_base + ACSMIM_ACSMINSTRREG11_ADD, 0x83100011U); // write_reg_field('h00000000_41F0302C,32'hFFFF_FFFF, 32'h83100011  );//   AcsmInstrReg11
    mem_write32(ucie_axi_base + ACSMIM_ACSMINSTRREG12_ADD, 0x0110C007); // write_reg_field('h00000000_41F03030,32'hFFFF_FFFF, 32'h0110C007  );//   AcsmInstrReg12
    mem_write32(ucie_axi_base + ACSMIM_ACSMINSTRREG13_ADD, 0x02711003); // write_reg_field('h00000000_41F03034,32'hFFFF_FFFF, 32'h02711003  );//   AcsmInstrReg13
    mem_write32(ucie_axi_base + ACSMIM_ACSMINSTRREG14_ADD, 0xA3100801U); // write_reg_field('h00000000_41F03038,32'hFFFF_FFFF, 32'hA3100801  );//   AcsmInstrReg14
    mem_write32(ucie_axi_base + ACSMIM_ACSMINSTRREG15_ADD, 0xA3100011U); // write_reg_field('h00000000_41F0303C,32'hFFFF_FFFF, 32'hA3100011  );//   AcsmInstrReg15
    mem_write32(ucie_axi_base + ACSMIM_ACSMINSTRREG16_ADD, 0xA3100021U); // write_reg_field('h00000000_41F03040,32'hFFFF_FFFF, 32'hA3100021  );//   AcsmInstrReg16
    mem_write32(ucie_axi_base + ACSMIM_ACSMINSTRREG17_ADD, 0x63970181); // write_reg_field('h00000000_41F03044,32'hFFFF_FFFF, 32'h63970181  );//   AcsmInstrReg17
    mem_write32(ucie_axi_base + ACSMIM_ACSMINSTRREG18_ADD, 0xA3100031U); // write_reg_field('h00000000_41F03048,32'hFFFF_FFFF, 32'hA3100031  );//   AcsmInstrReg18
    mem_write32(ucie_axi_base + ACSMIM_ACSMINSTRREG19_ADD, 0xA3100041U); // write_reg_field('h00000000_41F0304C,32'hFFFF_FFFF, 32'hA3100041  );//   AcsmInstrReg19
    mem_write32(ucie_axi_base + ACSMIM_ACSMINSTRREG20_ADD, 0xA3100851U); // write_reg_field('h00000000_41F03050,32'hFFFF_FFFF, 32'hA3100851  );//   AcsmInstrReg20
    mem_write32(ucie_axi_base + ACSMIM_ACSMINSTRREG21_ADD, 0x22712003); // write_reg_field('h00000000_41F03054,32'hFFFF_FFFF, 32'h22712003  );//   AcsmInstrReg21
    mem_write32(ucie_axi_base + ACSMIM_ACSMINSTRREG22_ADD, 0x000122C9); // write_reg_field('h00000000_41F03058,32'hFFFF_FFFF, 32'h000122C9  );//   AcsmInstrReg22
    mem_write32(ucie_axi_base + ACSMIM_ACSMINSTRREG23_ADD, 0x000C4125); // write_reg_field('h00000000_41F0305C,32'hFFFF_FFFF, 32'h000C4125  );//   AcsmInstrReg23
    mem_write32(ucie_axi_base + ACSMIM_ACSMINSTRREG24_ADD, 0x83045062U); // write_reg_field('h00000000_41F03060,32'hFFFF_FFFF, 32'h83045062  );//   AcsmInstrReg24
    mem_write32(ucie_axi_base + ACSMIM_ACSMINSTRREG25_ADD, 0x000100C6); // write_reg_field('h00000000_41F03064,32'hFFFF_FFFF, 32'h000100C6  );//   AcsmInstrReg25
    mem_write32(ucie_axi_base + ACSMIM_ACSMINSTRREG26_ADD, 0x83200091U); // write_reg_field('h00000000_41F03068,32'hFFFF_FFFF, 32'h83200091  );//   AcsmInstrReg26
    mem_write32(ucie_axi_base + ACSMIM_ACSMINSTRREG27_ADD, 0x02500000); // write_reg_field('h00000000_41F0306C,32'hFFFF_FFFF, 32'h02500000  );//   AcsmInstrReg27
    mem_write32(ucie_axi_base + ACSMIM_ACSMINSTRREG28_ADD, 0x83100041U); // write_reg_field('h00000000_41F03070,32'hFFFF_FFFF, 32'h83100041  );//   AcsmInstrReg28
    mem_write32(ucie_axi_base + ACSMIM_ACSMINSTRREG29_ADD, 0x02714003); // write_reg_field('h00000000_41F03074,32'hFFFF_FFFF, 32'h02714003  );//   AcsmInstrReg29
    mem_write32(ucie_axi_base + ACSMIM_ACSMINSTRREG30_ADD, 0x02210243); // write_reg_field('h00000000_41F03078,32'hFFFF_FFFF, 32'h02210243  );//   AcsmInstrReg30
    mem_write32(ucie_axi_base + ACSMIM_ACSMINSTRREG31_ADD, 0x00042EC9); // write_reg_field('h00000000_41F0307C,32'hFFFF_FFFF, 32'h00042EC9  );//   AcsmInstrReg31
    mem_write32(ucie_axi_base + ACSMIM_ACSMINSTRREG32_ADD, 0x000300A6); // write_reg_field('h00000000_41F03080,32'hFFFF_FFFF, 32'h000300A6  );//   AcsmInstrReg32
    mem_write32(ucie_axi_base + ACSMIM_ACSMINSTRREG33_ADD, 0x02210443); // write_reg_field('h00000000_41F03084,32'hFFFF_FFFF, 32'h02210443  );//   AcsmInstrReg33
    mem_write32(ucie_axi_base + ACSMIM_ACSMINSTRREG34_ADD, 0x8300D0C1U); // write_reg_field('h00000000_41F03088,32'hFFFF_FFFF, 32'h8300D0C1  );//   AcsmInstrReg34
    mem_write32(ucie_axi_base + ACSMIM_ACSMINSTRREG35_ADD, 0x832000D1U); // write_reg_field('h00000000_41F0308C,32'hFFFF_FFFF, 32'h832000D1  );//   AcsmInstrReg35
    mem_write32(ucie_axi_base + ACSMIM_ACSMINSTRREG36_ADD, 0x830001D1U); // write_reg_field('h00000000_41F03090,32'hFFFF_FFFF, 32'h830001D1  );//   AcsmInstrReg36
    mem_write32(ucie_axi_base + ACSMIM_ACSMINSTRREG37_ADD, 0x83102071U); // write_reg_field('h00000000_41F03094,32'hFFFF_FFFF, 32'h83102071  );//   AcsmInstrReg37
    mem_write32(ucie_axi_base + ACSMIM_ACSMINSTRREG38_ADD, 0x00034125); // write_reg_field('h00000000_41F03098,32'hFFFF_FFFF, 32'h00034125  );//   AcsmInstrReg38
    mem_write32(ucie_axi_base + ACSMIM_ACSMINSTRREG39_ADD, 0x000F7155); // write_reg_field('h00000000_41F0309C,32'hFFFF_FFFF, 32'h000F7155  );//   AcsmInstrReg39
    mem_write32(ucie_axi_base + ACSMIM_ACSMINSTRREG40_ADD, 0x00021006); // write_reg_field('h00000000_41F030A0,32'hFFFF_FFFF, 32'h00021006  );//   AcsmInstrReg40
    mem_write32(ucie_axi_base + ACSMIM_ACSMINSTRREG68_ADD, 0x00080423); // write_reg_field('h00000000_41F03110,32'hFFFF_FFFF, 32'h00080423  );//   AcsmInstrReg68
    mem_write32(ucie_axi_base + ACSMIM_ACSMINSTRREG69_ADD, 0x0004F853); // write_reg_field('h00000000_41F03114,32'hFFFF_FFFF, 32'h0004FE53  );//   AcsmInstrReg69
    mem_write32(ucie_axi_base + ACSMIM_ACSMINSTRREG70_ADD, 0x00010FB3); // write_reg_field('h00000000_41F03118,32'hFFFF_FFFF, 32'h0001CFB3  );//   AcsmInstrReg70
    mem_write32(ucie_axi_base + ACSMIM_ACSMINSTRREG71_ADD, 0x03973C6A); // write_reg_field('h00000000_41F0311C,32'hFFFF_FFFF, 32'h03973C6A  );//   AcsmInstrReg71
    mem_write32(ucie_axi_base + ACSMIM_ACSMINSTRREG72_ADD, 0x834090F1U); // write_reg_field('h00000000_41F03120,32'hFFFF_FFFF, 32'h834090F1  );//   AcsmInstrReg72
    mem_write32(ucie_axi_base + ACSMIM_ACSMINSTRREG73_ADD, 0x80006081U); // write_reg_field('h00000000_41F03124,32'hFFFF_FFFF, 32'h80006081  );//   AcsmInstrReg73
    mem_write32(ucie_axi_base + ACSMIM_ACSMINSTRREG74_ADD, 0x80000281U); // write_reg_field('h00000000_41F03128,32'hFFFF_FFFF, 32'h80000281  );//   AcsmInstrReg74
    mem_write32(ucie_axi_base + ACSMIM_ACSMINSTRREG75_ADD, 0x83209081U); // write_reg_field('h00000000_41F0312C,32'hFFFF_FFFF, 32'h83209081  );//   AcsmInstrReg75
    mem_write32(ucie_axi_base + ACSMIM_ACSMINSTRREG76_ADD, 0x83000281U); // write_reg_field('h00000000_41F03130,32'hFFFF_FFFF, 32'h83000281  );//   AcsmInstrReg76
    mem_write32(ucie_axi_base + ACSMIM_ACSMINSTRREG77_ADD, 0x80004081U); // write_reg_field('h00000000_41F03134,32'hFFFF_FFFF, 32'h80004081  );//   AcsmInstrReg77
    mem_write32(ucie_axi_base + ACSMIM_ACSMINSTRREG78_ADD, 0x80006481U); // write_reg_field('h00000000_41F03138,32'hFFFF_FFFF, 32'h80006481  );//   AcsmInstrReg78
    mem_write32(ucie_axi_base + ACSMIM_ACSMINSTRREG79_ADD, 0x8000A181U); // write_reg_field('h00000000_41F0313C,32'hFFFF_FFFF, 32'h8000A181  );//   AcsmInstrReg79
    mem_write32(ucie_axi_base + ACSMIM_ACSMINSTRREG80_ADD, 0x83000001U); // write_reg_field('h00000000_41F03140,32'hFFFF_FFFF, 32'h83000001  );//   AcsmInstrReg80
    mem_write32(ucie_axi_base + ACSMIM_ACSMINSTRREG81_ADD, 0x0397606A); // write_reg_field('h00000000_41F03144,32'hFFFF_FFFF, 32'h0397606A  );//   AcsmInstrReg81
    mem_write32(ucie_axi_base + ACSMIM_ACSMINSTRREG82_ADD, 0x83200001U); // write_reg_field('h00000000_41F03148,32'hFFFF_FFFF, 32'h83200001  );//   AcsmInstrReg82
    mem_write32(ucie_axi_base + ACSMIM_ACSMINSTRREG83_ADD, 0x83000101U); // write_reg_field('h00000000_41F0314C,32'hFFFF_FFFF, 32'h83000101  );//   AcsmInstrReg83
    mem_write32(ucie_axi_base + ACSMIM_ACSMINSTRREG84_ADD, 0x83005181U); // write_reg_field('h00000000_41F03150,32'hFFFF_FFFF, 32'h83005181  );//   AcsmInstrReg84
    mem_write32(ucie_axi_base + ACSMIM_ACSMINSTRREG85_ADD, 0x80004381U); // write_reg_field('h00000000_41F03154,32'hFFFF_FFFF, 32'h80004381  );//   AcsmInstrReg85
    mem_write32(ucie_axi_base + ACSMIM_ACSMINSTRREG86_ADD, 0x000F5135); // write_reg_field('h00000000_41F03158,32'hFFFF_FFFF, 32'h000F5135  );//   AcsmInstrReg86
    mem_write32(ucie_axi_base + ACSMIM_ACSMINSTRREG87_ADD, 0x00016145); // write_reg_field('h00000000_41F0315C,32'hFFFF_FFFF, 32'h00016145  );//   AcsmInstrReg87
    mem_write32(ucie_axi_base + ACSMIM_ACSMINSTRREG88_ADD, 0x03979B6A); // write_reg_field('h00000000_41F03160,32'hFFFF_FFFF, 32'h03979B6A  );//   AcsmInstrReg88
    mem_write32(ucie_axi_base + ACSMIM_ACSMINSTRREG89_ADD, 0x03978D6A); // write_reg_field('h00000000_41F03164,32'hFFFF_FFFF, 32'h03978D6A  );//   AcsmInstrReg89
    mem_write32(ucie_axi_base + ACSMIM_ACSMINSTRREG91_ADD, 0x0397406A); // write_reg_field('h00000000_41F0316C,32'hFFFF_FFFF, 32'h0397406A  );//   AcsmInstrReg91
    mem_write32(ucie_axi_base + ACSMIM_ACSMINSTRREG92_ADD, 0x01180873); // write_reg_field('h00000000_41F03170,32'hFFFF_FFFF, 32'h01180873  );//   AcsmInstrReg92
    mem_write32(ucie_axi_base + ACSMIM_ACSMINSTRREG93_ADD, 0x02780863); // write_reg_field('h00000000_41F03174,32'hFFFF_FFFF, 32'h02780863  );//   AcsmInstrReg93
    mem_write32(ucie_axi_base + ACSMIM_ACSMINSTRREG94_ADD, 0x01180073); // write_reg_field('h00000000_41F03178,32'hFFFF_FFFF, 32'h01180073  );//   AcsmInstrReg94
    mem_write32(ucie_axi_base + ACSMIM_ACSMINSTRREG95_ADD, 0x000FFF53); // write_reg_field('h00000000_41F0317C,32'hFFFF_FFFF, 32'h000FFF53  );//   AcsmInstrReg95
    mem_write32(ucie_axi_base + ACSMIM_ACSMINSTRREG96_ADD, 0x000FFFB3); // write_reg_field('h00000000_41F03180,32'hFFFF_FFFF, 32'h000FFFB3  );//   AcsmInstrReg96
    mem_write32(ucie_axi_base + ACSMIM_ACSMINSTRREG97_ADD, 0x00080023); // write_reg_field('h00000000_41F03184,32'hFFFF_FFFF, 32'h00080023  );//   AcsmInstrReg97
    mem_write32(ucie_axi_base + ACSMIM_ACSMINSTRREG98_ADD, 0x0000B00B); // write_reg_field('h00000000_41F03188,32'hFFFF_FFFF, 32'h0000B00B  );//   AcsmInstrReg98
    mem_write32(ucie_axi_base + ACSMIM_ACSMINSTRREG99_ADD, 0x03970B6A); // write_reg_field('h00000000_41F0318C,32'hFFFF_FFFF, 32'h03970B6A  );//   AcsmInstrReg99

    mem_write32(ucie_axi_base + ACSMIM_ACSMINSTRREG100_ADD, 0x00020173); // write_reg_field('h00000000_41F03190,32'hFFFF_FFFF, 32'h00020173  );//   AcsmInstrReg100

    ////Mask registers
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK0VAR0_ADD, 0x1f9);	   // write_reg_field('h00000000_41F02218,32'hFFFF_FFFF, 32'h1f9       );//   AcsmLtsmMsk0Var0
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK0VAR1_ADD, 0x2623001);  // write_reg_field('h00000000_41F0221c,32'hFFFF_FFFF, 32'h2623001   );//   AcsmLtsmMsk0Var1
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK0VAR2_ADD, 0x5);		   // write_reg_field('h00000000_41F02220,32'hFFFF_FFFF, 32'h5         );//   AcsmLtsmMsk0Var2
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK0VAR3_ADD, 0x1e1);	   // write_reg_field('h00000000_41F02224,32'hFFFF_FFFF, 32'h1e1       );//   AcsmLtsmMsk0Var3
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK0VAR4_ADD, 0x3727003);  // write_reg_field('h00000000_41F02228,32'hFFFF_FFFF, 32'h3727003   );//   AcsmLtsmMsk0Var4
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK0VAR5_ADD, 0x3727003);  // write_reg_field('h00000000_41F0222c,32'hFFFF_FFFF, 32'h3727003   );//   AcsmLtsmMsk0Var5
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK0VAR6_ADD, 0x2733005);  // write_reg_field('h00000000_41F02230,32'hFFFF_FFFF, 32'h2733005   );//   AcsmLtsmMsk0Var6
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK0VAR7_ADD, 0x36fb005);  // write_reg_field('h00000000_41F02234,32'hFFFF_FFFF, 32'h36fb005   );//   AcsmLtsmMsk0Var7
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK0VAR8_ADD, 0xde673e05u); // write_reg_field('h00000000_41F02238,32'hFFFF_FFFF, 32'hde673e05  );//   AcsmLtsmMsk0Var8
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK0VAR9_ADD, 0xde673e05u); // write_reg_field('h00000000_41F0223C,32'hFFFF_FFFF, 32'hde673e05  );//   AcsmLtsmMsk0Var9

    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK0VAR10_ADD, 0xb);		// write_reg_field('h00000000_41F02240,32'hFFFF_FFFF, 32'hb         );//   AcsmLtsmMsk0Var10
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK0VAR11_ADD, 0x1e3);		// write_reg_field('h00000000_41F02244,32'hFFFF_FFFF, 32'h1e3       );//   AcsmLtsmMsk0Var11
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK0VAR12_ADD, 0xc0000607u); // write_reg_field('h00000000_41F02248,32'hFFFF_FFFF, 32'hc0000607  );//   AcsmLtsmMsk0Var12
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK0VAR13_ADD, 0x32673805); // write_reg_field('h00000000_41F0224C,32'hFFFF_FFFF, 32'h32673805  );//   AcsmLtsmMsk0Var13
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK0VAR14_ADD, 0xde673e05u); // write_reg_field('h00000000_41F02250,32'hFFFF_FFFF, 32'hde673e05  );//   AcsmLtsmMsk0Var14
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK0VAR15_ADD, 0x32673805); // write_reg_field('h00000000_41F02254,32'hFFFF_FFFF, 32'h32673805  );//   AcsmLtsmMsk0Var15
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK0VAR16_ADD, 0xde673e05u); // write_reg_field('h00000000_41F02258,32'hFFFF_FFFF, 32'hde673e05  );//   AcsmLtsmMsk0Var16
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK0VAR17_ADD, 0x5);		// write_reg_field('h00000000_41F0225C,32'hFFFF_FFFF, 32'h5         );//   AcsmLtsmMsk0Var17
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK0VAR18_ADD, 0x32673805); // write_reg_field('h00000000_41F02260,32'hFFFF_FFFF, 32'h32673805  );//   AcsmLtsmMsk0Var18
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK0VAR19_ADD, 0x26fb007);	// write_reg_field('h00000000_41F02264,32'hFFFF_FFFF, 32'h26fb007   );//   AcsmLtsmMsk0Var19
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK0VAR20_ADD, 0x1603005);	// write_reg_field('h00000000_41F02268,32'hFFFF_FFFF, 32'h1603005   );//   AcsmLtsmMsk0Var20
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK0VAR21_ADD, 0xcec203);	// write_reg_field('h00000000_41F0226C,32'hFFFF_FFFF, 32'hcec203    );//   AcsmLtsmMsk0Var21
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK0VAR22_ADD, 0xc208985);	// write_reg_field('h00000000_41F02270,32'hFFFF_FFFF, 32'hc208985   );//   AcsmLtsmMsk0Var22
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK0VAR23_ADD, 0x1020605);	// write_reg_field('h00000000_41F02274,32'hFFFF_FFFF, 32'h1020605   );//   AcsmLtsmMsk0Var23
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK0VAR24_ADD, 0x4030005);	// write_reg_field('h00000000_41F02278,32'hFFFF_FFFF, 32'h4030005   );//   AcsmLtsmMsk0Var24
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK0VAR25_ADD, 0xf8100673u); // write_reg_field('h00000000_41F0227C,32'hFFFF_FFFF, 32'hf8100673  );//   AcsmLtsmMsk0Var25
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK0VAR26_ADD, 0xf8100673u); // write_reg_field('h00000000_41F02280,32'hFFFF_FFFF, 32'hf8100673  );//   AcsmLtsmMsk0Var26
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK0VAR27_ADD, 0x8800c277u); // write_reg_field('h00000000_41F02284,32'hFFFF_FFFF, 32'h8800c277  );//   AcsmLtsmMsk0Var27
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK0VAR28_ADD, 0x8800c275u); // write_reg_field('h00000000_41F02288,32'hFFFF_FFFF, 32'h8800c275  );//   AcsmLtsmMsk0Var28
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK0VAR29_ADD, 0x8800c277u); // write_reg_field('h00000000_41F0228C,32'hFFFF_FFFF, 32'h8800c277  );//   AcsmLtsmMsk0Var29
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK0VAR30_ADD, 0x7300000d); // write_reg_field('h00000000_41F02290,32'hFFFF_FFFF, 32'h7300000d  );//   AcsmLtsmMsk0Var30

    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK1VAR0_ADD, 0x140); // write_reg_field('h00000000_41F02298,32'hFFFF_FFFF, 32'h140       );//   AcsmLtsmMsk1Var0
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK1VAR1_ADD, 0x120); // write_reg_field('h00000000_41F0229C,32'hFFFF_FFFF, 32'h120       );//   AcsmLtsmMsk1Var1
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK1VAR2_ADD, 0x100); // write_reg_field('h00000000_41F022A0,32'hFFFF_FFFF, 32'h100       );//   AcsmLtsmMsk1Var2
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK1VAR3_ADD, 0x120); // write_reg_field('h00000000_41F022A4,32'hFFFF_FFFF, 32'h120       );//   AcsmLtsmMsk1Var3
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK1VAR4_ADD, 0x120); // write_reg_field('h00000000_41F022A8,32'hFFFF_FFFF, 32'h120       );//   AcsmLtsmMsk1Var4
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK1VAR5_ADD, 0x120); // write_reg_field('h00000000_41F022AC,32'hFFFF_FFFF, 32'h120       );//   AcsmLtsmMsk1Var5
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK1VAR6_ADD, 0x120); // write_reg_field('h00000000_41F022B0,32'hFFFF_FFFF, 32'h120       );//   AcsmLtsmMsk1Var6
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK1VAR7_ADD, 0x120); // write_reg_field('h00000000_41F022B4,32'hFFFF_FFFF, 32'h120       );//   AcsmLtsmMsk1Var7
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK1VAR8_ADD, 0x123); // write_reg_field('h00000000_41F022B8,32'hFFFF_FFFF, 32'h123       );//   AcsmLtsmMsk1Var8
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK1VAR9_ADD, 0x123); // write_reg_field('h00000000_41F022BC,32'hFFFF_FFFF, 32'h123       );//   AcsmLtsmMsk1Var9

    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK1VAR10_ADD, 0x120); // write_reg_field('h00000000_41F022C0,32'hFFFF_FFFF, 32'h120       );//   AcsmLtsmMsk1Var10
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK1VAR11_ADD, 0x120); // write_reg_field('h00000000_41F022C4,32'hFFFF_FFFF, 32'h120       );//   AcsmLtsmMsk1Var11
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK1VAR12_ADD, 0x123); // write_reg_field('h00000000_41F022C8,32'hFFFF_FFFF, 32'h123       );//   AcsmLtsmMsk1Var12
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK1VAR13_ADD, 0x120); // write_reg_field('h00000000_41F022CC,32'hFFFF_FFFF, 32'h120       );//   AcsmLtsmMsk1Var13
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK1VAR14_ADD, 0x123); // write_reg_field('h00000000_41F022D0,32'hFFFF_FFFF, 32'h123       );//   AcsmLtsmMsk1Var14
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK1VAR15_ADD, 0x120); // write_reg_field('h00000000_41F022D4,32'hFFFF_FFFF, 32'h120       );//   AcsmLtsmMsk1Var15
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK1VAR16_ADD, 0x123); // write_reg_field('h00000000_41F022D8,32'hFFFF_FFFF, 32'h123       );//   AcsmLtsmMsk1Var16
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK1VAR17_ADD, 0x120); // write_reg_field('h00000000_41F022DC,32'hFFFF_FFFF, 32'h120       );//   AcsmLtsmMsk1Var17
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK1VAR18_ADD, 0x120); // write_reg_field('h00000000_41F022E0,32'hFFFF_FFFF, 32'h120       );//   AcsmLtsmMsk1Var18
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK1VAR19_ADD, 0x1dc); // write_reg_field('h00000000_41F022E4,32'hFFFF_FFFF, 32'h1dc       );//   AcsmLtsmMsk1Var19
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK1VAR20_ADD, 0x120); // write_reg_field('h00000000_41F022E8,32'hFFFF_FFFF, 32'h120       );//   AcsmLtsmMsk1Var20
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK1VAR21_ADD, 0x4);   // write_reg_field('h00000000_41F022EC,32'hFFFF_FFFF, 32'h4         );//   AcsmLtsmMsk1Var21
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK1VAR22_ADD, 0x18);  // write_reg_field('h00000000_41F022F0,32'hFFFF_FFFF, 32'h18        );//   AcsmLtsmMsk1Var22
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK1VAR23_ADD, 0x4);   // write_reg_field('h00000000_41F022F4,32'hFFFF_FFFF, 32'h4         );//   AcsmLtsmMsk1Var23
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK1VAR24_ADD, 0x8);   // write_reg_field('h00000000_41F022F8,32'hFFFF_FFFF, 32'h8         );//   AcsmLtsmMsk1Var24
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK1VAR25_ADD, 0x3);   // write_reg_field('h00000000_41F022FC,32'hFFFF_FFFF, 32'h3         );//   AcsmLtsmMsk1Var25
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK1VAR26_ADD, 0x3);   // write_reg_field('h00000000_41F02300,32'hFFFF_FFFF, 32'h3         );//   AcsmLtsmMsk1Var26
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK1VAR27_ADD, 0xb);   // write_reg_field('h00000000_41F02304,32'hFFFF_FFFF, 32'hb         );//   AcsmLtsmMsk1Var27
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK1VAR28_ADD, 0xb);   // write_reg_field('h00000000_41F02308,32'hFFFF_FFFF, 32'hb         );//   AcsmLtsmMsk1Var28
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK1VAR29_ADD, 0xb);   // write_reg_field('h00000000_41F0230C,32'hFFFF_FFFF, 32'hb         );//   AcsmLtsmMsk1Var29
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK1VAR30_ADD, 0x4);   // write_reg_field('h00000000_41F02310,32'hFFFF_FFFF, 32'h4         );//   AcsmLtsmMsk1Var30

    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK0VAR21ALT_ADD, 0xcc800b);   // write_reg_field('h00000000_41F02318,32'hFFFF_FFFF, 32'hcc800b    );//   AcsmLtsmMsk0Var21Alt
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK0VAR22ALT_ADD, 0xc208985);  // write_reg_field('h00000000_41F0231C,32'hFFFF_FFFF, 32'hc208985   );//   AcsmLtsmMsk0Var22Alt
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK0VAR23ALT_ADD, 0x100210d);  // write_reg_field('h00000000_41F02320,32'hFFFF_FFFF, 32'h100210d   );//   AcsmLtsmMsk0Var23Alt
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK0VAR24ALT_ADD, 0x4020005);  // write_reg_field('h00000000_41F02324,32'hFFFF_FFFF, 32'h4020005   );//   AcsmLtsmMsk0Var24Alt
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK0VAR25ALT_ADD, 0xf820a173u); // write_reg_field('h00000000_41F02328,32'hFFFF_FFFF, 32'hf820a173  );//   AcsmLtsmMsk0Var25Alt
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK0VAR26ALT_ADD, 0xf820a173u); // write_reg_field('h00000000_41F0232C,32'hFFFF_FFFF, 32'hf820a173  );//   AcsmLtsmMsk0Var26Alt
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK0VAR27ALT_ADD, 0x88008077u); // write_reg_field('h00000000_41F02330,32'hFFFF_FFFF, 32'h88008077  );//   AcsmLtsmMsk0Var27Alt
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK0VAR28ALT_ADD, 0x88008075u); // write_reg_field('h00000000_41F02334,32'hFFFF_FFFF, 32'h88008075  );//   AcsmLtsmMsk0Var28Alt
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK0VAR29ALT_ADD, 0x88008077u); // write_reg_field('h00000000_41F02338,32'hFFFF_FFFF, 32'h88008077  );//   AcsmLtsmMsk0Var29Alt
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK0VAR30ALT_ADD, 0x7300000d); // write_reg_field('h00000000_41F0233C,32'hFFFF_FFFF, 32'h7300000d  );//   AcsmLtsmMsk0Var30Alt

    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK1VAR21ALT_ADD, 0x4);	 // write_reg_field('h00000000_41F02344,32'hFFFF_FFFF, 32'h4         );//   AcsmLtsmMsk1Var21Alt
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK1VAR22ALT_ADD, 0x18); // write_reg_field('h00000000_41F02348,32'hFFFF_FFFF, 32'h18        );//   AcsmLtsmMsk1Var22Alt
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK1VAR23ALT_ADD, 0x4);	 // write_reg_field('h00000000_41F0234C,32'hFFFF_FFFF, 32'h4         );//   AcsmLtsmMsk1Var23Alt
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK1VAR24ALT_ADD, 0x8);	 // write_reg_field('h00000000_41F02350,32'hFFFF_FFFF, 32'h8         );//   AcsmLtsmMsk1Var24Alt
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK1VAR25ALT_ADD, 0x3);	 // write_reg_field('h00000000_41F02354,32'hFFFF_FFFF, 32'h3         );//   AcsmLtsmMsk1Var25Alt
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK1VAR26ALT_ADD, 0x3);	 // write_reg_field('h00000000_41F02358,32'hFFFF_FFFF, 32'h3         );//   AcsmLtsmMsk1Var26Alt
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK1VAR27ALT_ADD, 0xb);	 // write_reg_field('h00000000_41F0235C,32'hFFFF_FFFF, 32'hb         );//   AcsmLtsmMsk1Var27Alt
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK1VAR28ALT_ADD, 0xb);	 // write_reg_field('h00000000_41F02360,32'hFFFF_FFFF, 32'hb         );//   AcsmLtsmMsk1Var28Alt
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK1VAR29ALT_ADD, 0xb);	 // write_reg_field('h00000000_41F02364,32'hFFFF_FFFF, 32'hb         );//   AcsmLtsmMsk1Var29Alt
    mem_write32(ucie_axi_base + ACSM_ACSMLTSMMSK1VAR30ALT_ADD, 0x4);	 // write_reg_field('h00000000_41F02368,32'hFFFF_FFFF, 32'h4         );//   AcsmLtsmMsk1Var30Alt

    ////Address registers
    mem_write32(ucie_axi_base + MMPL_CSRADDR2_ADD, 0x304004);  // write_reg_field('h00000000_41F01078,32'hFFFF_FFFF, 32'h304004    );//   CsrAddr2
    mem_write32(ucie_axi_base + MMPL_CSRADDR5_ADD, 0x304018);  // write_reg_field('h00000000_41F01084,32'hFFFF_FFFF, 32'h304018    );//   CsrAddr5
    mem_write32(ucie_axi_base + MMPL_CSRADDR10_ADD, 0x3042BC); // write_reg_field('h00000000_41F01098,32'hFFFF_FFFF, 32'h3042BC    );//   CsrAddr10
    mem_write32(ucie_axi_base + MMPL_CSRADDR11_ADD, 0x304020); // write_reg_field('h00000000_41F0109C,32'hFFFF_FFFF, 32'h304020    );//   CsrAddr11

    // write_reg_field('h00000000_41F02004,32'h0000_7F00, 32'h002800    );//   AcsmSeq0Ctrl.AcsmSeq0StopAddr
    reg_val = mem_read32(ucie_axi_base + ACSM_ACSMSEQ0CTRL_ADD);
    reg_val = reg_val & ACSM_ACSMSEQ0CTRL_MASK;
    reg_val = reg_val &       ~( ACSM_ACSMSEQ0CTRL_ACSMSEQ0STOPADDR_MASK  << ACSM_ACSMSEQ0CTRL_ACSMSEQ0STOPADDR_SHIFT);
    reg_val = reg_val | (((uint32_t)0x28 & ACSM_ACSMSEQ0CTRL_ACSMSEQ0STOPADDR_MASK) << ACSM_ACSMSEQ0CTRL_ACSMSEQ0STOPADDR_SHIFT);
    mem_write32( ucie_axi_base + ACSM_ACSMSEQ0CTRL_ADD, reg_val );
    
    reg_val = mem_read32(ucie_axi_base + ACSM_ACSMSEQ1CTRL_ADD);
    reg_val = reg_val & ACSM_ACSMSEQ1CTRL_MASK;
    reg_val = reg_val &       ~( ACSM_ACSMSEQ1CTRL_ACSMSEQ1STOPADDR_MASK  << ACSM_ACSMSEQ1CTRL_ACSMSEQ1STOPADDR_SHIFT);
    reg_val = reg_val | ((0x64 & ACSM_ACSMSEQ1CTRL_ACSMSEQ1STOPADDR_MASK) << ACSM_ACSMSEQ1CTRL_ACSMSEQ1STOPADDR_SHIFT);
    mem_write32( ucie_axi_base + ACSM_ACSMSEQ1CTRL_ADD, reg_val );
    // NOTE: assuming AcsmSeq1StartAddr is 0x40 by default.
    
    reg_val = mem_read32(ucie_axi_base + ACSM_ACSMCTRL_ADD);
    reg_val = reg_val & ACSM_ACSMCTRL_MASK;
    reg_val = reg_val &       ~( ACSM_ACSMCTRL_ACSMSTOPADDR_MASK  << ACSM_ACSMCTRL_ACSMSTOPADDR_SHIFT);
    reg_val = reg_val | (((uint32_t)0x28 & ACSM_ACSMCTRL_ACSMSTOPADDR_MASK) << ACSM_ACSMCTRL_ACSMSTOPADDR_SHIFT);
    mem_write32( ucie_axi_base + ACSM_ACSMCTRL_ADD, reg_val );
#endif
}

void Ucie_Start_Linkup(e_ucie_ch_t ch, e_ucie_mode_t mode, e_ucie_linkspeed_t speed)
{
    uint32_t ucie_axi_base;
    uint32_t ucie_apb_base;

    ucie_axi_base = UCIE_AXI_BASE(ch);
    ucie_apb_base = UCIE_APB_BASE(ch);

    //;  axi0 addres OFF
    mem_write32( ucie_apb_base + 0xE005E8, 0x00000000 );

    uint32_t dvsecLinkControl = ((speed & LINKSPEED_MASK) << LINKSPEED_OFFSET) | 0x00004000;
    mem_write32( ucie_axi_base + DVSEC_UNIT_DSP_DVSEC_UCIE_LINK_CONTROL_ADD, dvsecLinkControl);

    if(mode == UCIE_MODE_RC){
        mem_write32( ucie_axi_base + DVSEC_UNIT_DSP_DVSEC_UCIE_LINK_CONTROL_ADD, dvsecLinkControl | 0x00000400 );
   }
}

uint32_t Ucie_Wait_FreqChange_Req(e_ucie_ch_t ch)
{
    uint32_t ret = 0;
    uint32_t mask, expect;

    uint32_t ucie_apb_base;

    ucie_apb_base = UCIE_APB_BASE(ch);

    mask	= 0x000002;
    expect	= 0x000002;

    if ((mem_read32(ucie_apb_base + 0xE1003C) & mask) != expect) {
            ret = 1;
    }

    return ret;
}

uint32_t Ucie_Ack_FreqChange(e_ucie_ch_t ch, e_ucie_linkspeed_t speed)
{
    uint32_t ret = 0;
    uint32_t ucie_apb_base;
    uint32_t ucie_axi_base;
    uint32_t clock_div;

    PLLParam pllprm[] = {// there are some parameters that have the same value.
                        {0x296, 0b000, 0b001, 0b0001101, 0b0001000, 0b0101000, 0b1111111, 0x041B600150601009},
                        {0x29A, 0b100, 0b000, 0b0001011, 0b0000011, 0b0101000, 0b1111111, 0x041B600150601009},
                        {0x3CE, 0b100, 0b001, 0b0001101, 0b0000101, 0b0101000, 0b1111111, 0x041B600150601009},
                        {0x3CE, 0b100, 0b000, 0b0001011, 0b0000011, 0b0101000, 0b1111111, 0x041B600150601009}};

    FreqDepPrm freqdepprm={
        500,
        4750,
        500,
        125,
        100,
        50,
        50,
        25,
        25,
        50,
        50,
        2,
        4,
        0,
        0,
        0
    };

    ucie_apb_base = UCIE_APB_BASE(ch);
    ucie_axi_base = UCIE_AXI_BASE(ch);

    mem_write32( ucie_apb_base + 0xE005E8, 0x00004141 );

    mem_write32(ucie_axi_base + ACSM_ACSMWAITDLY0_ADD, freqdepprm.AcsmWaitDly0 * (speed + 1));
    mem_write32(ucie_axi_base + ACSM_ACSMWAITDLY1_ADD, freqdepprm.AcsmWaitDly1 * (speed + 1));
    mem_write32(ucie_axi_base + MMPL_ZCALCTRL0_ADD, (freqdepprm.zcalcompstartuptime * (speed + 1)) | (freqdepprm.zcalsampletime * (speed + 1) << 12) | (freqdepprm.zcaloffsetsampletime * (speed + 1) << 22));
    mem_write32(ucie_axi_base + DWORD_0_DWDCCCTRL1_ADD, freqdepprm.dwdcdsettletime * (speed + 1));
    mem_write32(ucie_axi_base + DWORD_1_DWDCCCTRL1_ADD, freqdepprm.dwdcdsettletimeDW1 * (speed + 1));
    *(volatile uint32_t*)(ucie_axi_base + DWORD_0_DWDCCCTRL1_ADD) |= freqdepprm.dwdcasettletime * (speed + 1) << 8;
    *(volatile uint32_t*)(ucie_axi_base + DWORD_1_DWDCCCTRL1_ADD) |= freqdepprm.dwdcasettletimeDW1 * (speed + 1) << 8;
    *(volatile uint32_t*)(ucie_axi_base + DWORD_0_DWDCCCTRL1_ADD) |= freqdepprm.dwdcdsampletime * (speed + 1) << 16;
    *(volatile uint32_t*)(ucie_axi_base + DWORD_1_DWDCCCTRL1_ADD) |= freqdepprm.dwdcdsampletimeDW1 * (speed + 1) << 16;
    mem_write32(ucie_axi_base + ACSM_ACSMTIMEOUTCTRL0_ADD, freqdepprm.acsmpmaborttimeout * (speed + 1) | (freqdepprm.acsmpmentrytimeout * (speed + 1) << 8));
    mem_write32(ucie_axi_base + ACSM_ACSMTIMEOUTCTRL1_ADD, freqdepprm.acsmltsmstatetimeout * (speed + 1) | (freqdepprm.acsmltsmmsgtimeout * (speed + 1) << 9) | (freqdepprm.acsmlinkerrtimeout * (speed + 1) << 18));

#ifdef RCAR_UCIE_V100
    mem_write32(ucie_axi_base + MMPL_PLLCTRL1_ADD, pllprm[speed].div_sel | (pllprm[speed].v2i_mode << 10) | (pllprm[speed].vco_low_freq << 13));
    mem_write32(ucie_axi_base + MMPL_PLLCTRL0_ADD, (pllprm[speed].cp_prop_cntrl << 8) | (pllprm[speed].cp_int_cntrl) | (pllprm[speed].cp_prop_gs_cntrl << 24) | (pllprm[speed].cp_int_gs_cntrl << 16));
#endif
    mem_write32(ucie_axi_base + MMPL_PLLCTRL3_ADD, pllprm[speed].upll_prog);
    mem_write32(ucie_axi_base + MMPL_PLLCTRL4_ADD, pllprm[speed].upll_prog >> 32);

    mem_write32( ucie_apb_base + 0xE21004, 0x00000001 ); // ack=1 -> req will negate after 1clk cycle

    return ret;
}

uint32_t Ucie_Wait_Linkup(e_ucie_ch_t ch)
{
    e_ucie_linkup_status_t ret = LINKUP_SUCCESS;
    uint32_t mask, expect;
    uint32_t ucie_axi_base;
    uint32_t ucie_apb_base;

    ucie_axi_base = UCIE_AXI_BASE(ch);
    ucie_apb_base = UCIE_APB_BASE(ch);
    
    //;  axi0 addres ON
    mem_write32(ucie_apb_base + 0xE005E8, 0x00004141);

    mask = 0x00001F;
    expect = 0x000016;

    if ((mem_read32(ucie_axi_base + ACSM_ACSMLTSMSTATUS_ADD) & mask) != expect)
    {   
            return LINKUP_TIMEOUT;
    }

    return ret;
}

void Ucie_Setup_PCIE_Pre(e_ucie_ch_t ch, e_ucie_mode_t mode)
{
    uint32_t ucie_axi_base;
    uint32_t ucie_apb_base;

    ucie_axi_base = UCIE_AXI_BASE(ch);
    ucie_apb_base = UCIE_APB_BASE(ch);

    //; After linkup
    //;  axi0 addres ON
    mem_write32(ucie_apb_base + 0xE005E8, 0x00004141);

    mem_write32(ucie_axi_base + DWORD_0_DWPUBMODECTRL_ADD, 0x00000000);

    mem_write32(ucie_axi_base + DWORD_1_DWPUBMODECTRL_ADD, 0x00000000);

    //; UCIEDBIADR
    mem_write32(ucie_apb_base + 0xE005E8, 0x00000000);

    if (mode == UCIE_MODE_RC)
    {
        mem_write32(ucie_axi_base + PF0_SPCIE_CAP_SPCIE_CAP_OFF_0CH_REG_ADD, 0x37300127);
        mem_write32(ucie_axi_base + PF0_SPCIE_CAP_SPCIE_CAP_OFF_10H_REG_ADD, 0x69144272);
        mem_write32(ucie_axi_base + PF0_SPCIE_CAP_SPCIE_CAP_OFF_14H_REG_ADD, 0x12474905);
        mem_write32(ucie_axi_base + PF0_SPCIE_CAP_SPCIE_CAP_OFF_18H_REG_ADD, 0x5A055370);
    }
    else
    {
        mem_write32(ucie_axi_base + PF0_SPCIE_CAP_SPCIE_CAP_OFF_0CH_REG_ADD, 0x2228174A);
        mem_write32(ucie_axi_base + PF0_SPCIE_CAP_SPCIE_CAP_OFF_10H_REG_ADD, 0x78540719);
        mem_write32(ucie_axi_base + PF0_SPCIE_CAP_SPCIE_CAP_OFF_14H_REG_ADD, 0x03630375);
        mem_write32(ucie_axi_base + PF0_SPCIE_CAP_SPCIE_CAP_OFF_18H_REG_ADD, 0x31522410);
    }

    if (mode == UCIE_MODE_RC)
    {
        mem_write32(ucie_axi_base + PF0_PL16G_CAP_PL16G_CAP_OFF_20H_REG_ADD, 0x8584443AU);
        mem_write32(ucie_axi_base + PF0_PL16G_CAP_PL16G_CAP_OFF_24H_REG_ADD, 0x6631482A);
    }
    else
    {
        mem_write32(ucie_axi_base + PF0_PL16G_CAP_PL16G_CAP_OFF_20H_REG_ADD, 0x390A0826);
        mem_write32(ucie_axi_base + PF0_PL16G_CAP_PL16G_CAP_OFF_24H_REG_ADD, 0x76145138);
    }

    if (mode == UCIE_MODE_RC)
    {
        mem_write32(ucie_axi_base + PF0_PL32G_CAP_PL32G_CAP_OFF_20H_REG_ADD, 0x915A4145U);
        mem_write32(ucie_axi_base + PF0_PL32G_CAP_PL32G_CAP_OFF_24H_REG_ADD, 0x81817220U);
    }
    else
    {
        mem_write32(ucie_axi_base + PF0_PL32G_CAP_PL32G_CAP_OFF_20H_REG_ADD, 0x00860582);
        mem_write32(ucie_axi_base + PF0_PL32G_CAP_PL32G_CAP_OFF_24H_REG_ADD, 0x96558A61U);
    }

    if (mode == UCIE_MODE_RC)
    {
        mem_write32(ucie_axi_base + PF0_PL64G_CAP_PL64G_LANE_EQ_10H_REG_ADD, 0xA3057527U);
        mem_write32(ucie_axi_base + PF0_PL64G_CAP_PL64G_LANE_EQ_14H_REG_ADD, 0x09548A39);
    }
    else
    {
        mem_write32(ucie_axi_base + PF0_PL64G_CAP_PL64G_LANE_EQ_10H_REG_ADD, 0x1A226372);
        mem_write32(ucie_axi_base + PF0_PL64G_CAP_PL64G_LANE_EQ_14H_REG_ADD, 0x01674946);
    }

}

void Ucie_Setup_PCIE_Start_LinkUp(e_ucie_ch_t ch, e_ucie_mode_t mode)
{
    uint32_t ucie_apb_base;
    
    ucie_apb_base = UCIE_APB_BASE(ch);

    //;UCIERSTCTRL1
    mem_write32(ucie_apb_base + 0xE00014, 0x00010001);

    if (mode == UCIE_MODE_RC)
    {
        //;UCIEPCR00
        mem_write32(ucie_apb_base + 0xE21000, 0x00000011);
    }
    else
    {
        //;UCIEPCR00
        mem_write32(ucie_apb_base + 0xE21000, 0x00000012);
    }
}

uint32_t Ucie_Setup_PCIE_Wait_LinkUp(e_ucie_ch_t ch)
{
    e_ucie_linkup_status_t ret = LINKUP_SUCCESS;
    uint32_t ucie_apb_base;
    uint32_t val;
    uint32_t mask, expect;

    if (ch != UCIE_CH0 && ch != UCIE_CH1) {
        printf("ERROR: Invalid UCIe channel\n");
        return 1;
    }

    ucie_apb_base = UCIE_APB_BASE(ch);

    /*
    PCIE link up:wait rdlh_link bit ([4])
    */
    mask = 0x000010;
    expect = 0x000010;

    //;UCIEICR00b
#ifdef RCAR_UCIE_V100
    val = mem_read32(ucie_apb_base + 0xE10004);
#else
    val = mem_read32(ucie_apb_base + 0xE10010);
#endif

    if ((val & mask) != expect) {
        ret = LINKUP_TIMEOUT;
    }

    return ret;
}


void Ucie_Setup_PCIE_Post(e_ucie_ch_t ch, e_ucie_mode_t mode)
{
    uint32_t ucie_axi_base;
    uint32_t ucie_apb_base;

    ucie_axi_base = UCIE_AXI_BASE(ch);
    ucie_apb_base = UCIE_APB_BASE(ch);

    //;  axi0 addres ON
    mem_write32(ucie_apb_base + 0xE005E8, 0x00000000);

    mem_write32(ucie_axi_base + PF0_PCIE_CAP_DEVICE_CONTROL_DEVICE_STATUS_ADD, 0x00102150);

    if (mode != UCIE_MODE_RC)
    {
        mem_write32(ucie_axi_base + PF0_PCIE_CAP_DEVICE_CONTROL_DEVICE_STATUS_ADD + RCAR_UCIE_FN_OFS(1U), 0x00102150);
    }

    if (mode == UCIE_MODE_RC)
    {

        //;BAR0_MASK_REG
        mem_write32(ucie_axi_base + PF0_TYPE1_HDR_BAR0_REG_ADD + RCAR_UCIE_FN_OFS(1U), 0x00000000U);
        //;BAR1_MASK_REG
        mem_write32(ucie_axi_base + PF0_TYPE1_HDR_BAR1_REG_ADD + RCAR_UCIE_FN_OFS(1U), 0x00000000U);

        mem_write32(ucie_axi_base + PF0_TYPE1_HDR_BAR0_REG_ADD, 0x09100004);
        mem_write32(ucie_axi_base + PF0_TYPE1_HDR_BAR1_REG_ADD, 0x00000000);
        mem_write32(ucie_axi_base + PF0_PORT_LOGIC_TRGT_MAP_CTRL_OFF_ADD, 0x00000040);
        mem_write32(ucie_axi_base + PF0_TYPE1_HDR_SEC_STAT_IO_LIMIT_IO_BASE_REG_ADD, 0x00000101);
        mem_write32(ucie_axi_base + PF0_TYPE1_HDR_IO_LIMIT_UPPER_IO_BASE_UPPER_REG_ADD, 0x00000000);
        mem_write32(ucie_axi_base + PF0_TYPE1_HDR_SEC_STAT_IO_LIMIT_IO_BASE_REG_ADD, 0x00004F40);
        mem_write32(ucie_axi_base + PF0_TYPE1_HDR_MEM_LIMIT_MEM_BASE_REG_ADD, 0x08FF08C0);
        mem_write32(ucie_axi_base + PF0_TYPE1_HDR_PREF_MEM_LIMIT_PREF_MEM_BASE_REG_ADD, 0x00010001);
        mem_write32(ucie_axi_base + PF0_TYPE1_HDR_PREF_BASE_UPPER_REG_ADD, 0x00000000);
        mem_write32(ucie_axi_base + PF0_TYPE1_HDR_PREF_LIMIT_UPPER_REG_ADD, 0x00000000);
        mem_write32(ucie_axi_base + PF0_TYPE1_HDR_PREF_MEM_LIMIT_PREF_MEM_BASE_REG_ADD, 0xB00FB000U);
        mem_write32(ucie_axi_base + PF0_CXL_2_0_EXT_CAP_CXL_2_0_ALT_MEM_BASE_LIMIT_OFF_ADD, 0xAFEF0930U);
        mem_write32(ucie_axi_base + PF0_CXL_2_0_EXT_CAP_CXL_2_0_ALT_PREFETCH_MEM_BASE_LIMIT_OFF_ADD, 0x00000000);

        mem_write32(ucie_axi_base + PF0_CXL_2_0_EXT_CAP_CXL_2_0_CTRL_ALT_BUS_BASE_LIMIT_OFF_ADD, 0x00000000);
        mem_write32(ucie_axi_base + PF0_CXL_2_0_EXT_CAP_CXL_2_0_CTRL_ALT_BUS_BASE_LIMIT_OFF_ADD, 0x00000000);
        mem_write32(ucie_axi_base + PF0_CXL_2_0_EXT_CAP_CXL_2_0_CTRL_ALT_BUS_BASE_LIMIT_OFF_ADD, 0x00050000);
        mem_write32(ucie_axi_base + PF0_CXL_2_0_EXT_CAP_CXL_2_0_CTRL_ALT_BUS_BASE_LIMIT_OFF_ADD, 0x6D050000);
    }
    else
    {

        mem_write32(ucie_axi_base + PF0_TYPE1_HDR_BAR0_REG_ADD, 0x08C00004);
        mem_write32(ucie_axi_base + PF0_TYPE1_HDR_BAR1_REG_ADD, 0x00000000);
        //;BAR0_MASK_REG
        mem_write32(ucie_axi_base + PF0_TYPE1_HDR_BAR0_REG_ADD + RCAR_UCIE_FN_OFS(1U), 0x08E00004);
        //;BAR1_MASK_REG
        mem_write32(ucie_axi_base + PF0_TYPE1_HDR_BAR1_REG_ADD + RCAR_UCIE_FN_OFS(1U), 0x00000000);
        mem_write32(ucie_axi_base + PF0_TYPE1_HDR_SEC_LAT_TIMER_SUB_BUS_SEC_BUS_PRI_BUS_REG_ADD, 0x08D00000);
        mem_write32(ucie_axi_base + PF0_TYPE1_HDR_SEC_LAT_TIMER_SUB_BUS_SEC_BUS_PRI_BUS_REG_ADD + RCAR_UCIE_FN_OFS(1U), 0x08F00000);
        mem_write32(ucie_axi_base + PF0_TYPE1_HDR_MEM_LIMIT_MEM_BASE_REG_ADD, 0x00004001);
        mem_write32(ucie_axi_base + PF0_TYPE1_HDR_MEM_LIMIT_MEM_BASE_REG_ADD + RCAR_UCIE_FN_OFS(1U), 0x00004101);
        mem_write32(ucie_axi_base + PF0_TYPE1_HDR_IO_LIMIT_UPPER_IO_BASE_UPPER_REG_ADD, 0xB0000001U);
        mem_write32(ucie_axi_base + PF0_TYPE1_HDR_IO_LIMIT_UPPER_IO_BASE_UPPER_REG_ADD + RCAR_UCIE_FN_OFS(1U), 0xB0010001U);
        mem_write32(ucie_axi_base + PF0_PORT_LOGIC_TRGT_MAP_CTRL_OFF_ADD, 0x00000054);
        mem_write32(ucie_axi_base + PF0_PORT_LOGIC_TRGT_MAP_CTRL_OFF_ADD + RCAR_UCIE_FN_OFS(1U), 0x00010054);
    }

    mem_write32(ucie_axi_base + 0x4, 0x00110007);
    mem_write32(ucie_axi_base + 0x80, 0x00000000);
    mem_write32(ucie_axi_base + 0x8BC, 0x040BFF4A);
    mem_write32(ucie_axi_base + 0x8BC, 0x040BFF4E);
}

static void set_pll9_0(uint32_t f_Speed){

    // Parameter table for pll9 setting
    PLL9Param pll9prm0[]={
        {0x07700000,0x041895f9,0x00000018},// 4GT/s
        {0x07700000,0x041895f9,0x00000010},// 8GT/s
        {0x05900000,0x0012707a,0x00000000},// 12GT/s
        {0x07700000,0x041895f9,0x00000000} // 16GT/s
    };

    *(volatile uint32_t*)0xDE201370 = 0xA5A5A501U; // CLKHSCSD1WCR0
    *(volatile uint32_t*)0xDE201380 = 0xFFFFFFFFU; // CLKHSCSPKCPROT0

    // 2-1
    *(volatile uint32_t *)HSCS_APB_PLL9_0_CR0 = pll9prm0[f_Speed].PLL9_CR0;
    *(volatile uint32_t *)HSCS_APB_PLL9_0_CR1 = pll9prm0[f_Speed].PLL9_CR1;
    *(volatile uint32_t *)HSCS_APB_PLL9_0DCR  = pll9prm0[f_Speed].PLL9_DCR;
    *(volatile uint32_t *)HSCS_APB_PLL9_0_CR2 = 0x10000000; // PLL9_0_CR2

    // 2-2
    while(1){
        if((*(volatile uint32_t*)HSCS_APB_PLL9_0_CR2) & 0x80000000){
            break;
        }
    }
    *(volatile uint32_t *)HSCS_APB_PLL9_0SCR = 0x00000001;

    while(1){
        if((*(volatile uint32_t*)HSCS_APB_PLL9_0SCR) & 0x00010000){
            break;
        }
    }
    *(volatile uint32_t *)HSCS_APB_PLL9_0_CR2 = 0x20000000;

    while(1){
        if(!((*(volatile uint32_t*)HSCS_APB_PLL9_0_CR2) & 0x80000000)){
            break;
        }
    }

    // 2-3
    while(1){
        if(!((*(volatile uint32_t*)HSCS_APB_PLL9_0_CR2) &  0x80000000)){
            break;
        }
    }
    *(volatile uint32_t *)HSCS_APB_PLL9_0SCR = 0x00000000;

    while(1){
        if(!((*(volatile uint32_t*)HSCS_APB_PLL9_0SCR) & 0x00010000)){
            break;
        }
    }

    *(volatile uint32_t *)HSCS_APB_PLL9_0_CR2 = 0x10000000;

    while(1){
        if((*(volatile uint32_t*)HSCS_APB_PLL9_0_CR2) & 0x80000000){
            break;
        }
    }

    // Freq. = 1/(1+0) = 1/1 = PHY_CLK 2000MHz, CLOCK = OFF
    *(volatile uint32_t *)HSCS_APB_UCI0CORECKCR = 0x00000100;
    // Freq. = 1/(1+3) = 1/4 = PHY_CLK 500MHz, CLOCK = ON
    *(volatile uint32_t *)HSCS_APB_UCI0CORECKCR = 0x00000000;
}

static void set_pll9_1(uint32_t f_Speed)
{
    // Parameter table for pll9 setting
    PLL9Param pll9prm1[]={
        {0x07700000,0x041895f9,0x00000018},// 4GT/s
        {0x07700000,0x041895f9,0x00000010},// 8GT/s
        {0x05900000,0x0012707a,0x00000000},// 12GT/s
        {0x07700000,0x041895f9,0x00000000} // 16GT/s
    };

    *(volatile uint32_t*)0xDE201370 = 0xA5A5A501U; // CLKHSCSD1WCR0
    *(volatile uint32_t*)0xDE201380 = 0xFFFFFFFFU; // CLKHSCSPKCPROT0

    // 2-4  
    *(volatile uint32_t *)HSCS_APB_PLL9_1_CR0 = pll9prm1[f_Speed].PLL9_CR0;
    *(volatile uint32_t *)HSCS_APB_PLL9_1_CR1 = pll9prm1[f_Speed].PLL9_CR1;
    *(volatile uint32_t *)HSCS_APB_PLL9_1DCR  = pll9prm1[f_Speed].PLL9_DCR;
    *(volatile uint32_t*)0xDE201200 = 0x10000000; // PLL9_1_CR2

    // 2-5
    while(1){
        if((*(volatile uint32_t*)HSCS_APB_PLL9_1_CR2) & 0x80000000){
            break;
        }
    }
    *(volatile uint32_t *)HSCS_APB_PLL9_1SCR = 0x00000001;
    while(1){
        if((*(volatile uint32_t*)HSCS_APB_PLL9_1SCR) & 0x00010000){
            break;
        }
    }
    *(volatile uint32_t *)HSCS_APB_PLL9_1_CR2 = 0x20000000;

    while(1){
        if((*(volatile uint32_t*)HSCS_APB_PLL9_1_CR2) & 0x80000000){
            break;
        }
    }

    // 2-6
    while(1){
        if(!((*(volatile uint32_t*)HSCS_APB_PLL9_1_CR2) & 0x80000000)){
            break;
        }
    }
    *(volatile uint32_t *)HSCS_APB_PLL9_1SCR = 0x00000000;

    while(1){
        if(!((*(volatile uint32_t*)HSCS_APB_PLL9_1SCR) & 0x00010000)){
            break;
        }
    }
    *(volatile uint32_t *)HSCS_APB_PLL9_1_CR2 = 0x10000000;
    while(1){
        if((*(volatile uint32_t*)HSCS_APB_PLL9_1_CR2) & 0x80000000){
            break;
        }
    }

    // Freq. = 1/(1+0) = 1/1 = PHY_CLK 2000MHz, CLOCK = OFF
    *(volatile uint32_t *)HSCS_APB_UCI1CORECKCR = 0x00000100;

    // Freq. = 1/(1+3) = 1/4 = PHY_CLK 500MHz, CLOCK = ON
    *(volatile uint32_t *)HSCS_APB_UCI1CORECKCR = 0x00000000;
}

#if (NOT_SCP_SUPPORT == 1)
void Ucie_PowerOn(e_ucie_ch_t ucie_ch)
{
    uint32_t ucixcoreclkcr;
    uint32_t pll_num;
    uint32_t ucie_ms_core_bit;
    uint32_t ucie_ms_peri_bit;
    volatile uintptr_t uciepwrmngctrl;
    uint32_t ucie_apb_base;
    uint32_t val;
    if (ucie_ch == UCIE_CH0){
        ucixcoreclkcr = 0xDE201080U;
        pll_num = 19;
        ucie_ms_peri_bit=2;
        ucie_ms_core_bit=0;
        ucie_apb_base=0xDC000000;
    } else {
        ucixcoreclkcr = 0xDE201084U;
        pll_num = 20;
        ucie_ms_peri_bit=6;
        ucie_ms_core_bit=4;
        ucie_apb_base=0xDD000000;
    }

    /* ms ucie core reset*/
    mdlc_transition_ms(16, 2, ucie_ms_core_bit, 0x1);
    mdlc_check_ms_status(16, 2, ucie_ms_core_bit);

    /* ms ucie peri reset */
    mdlc_transition_ms(16, 2, ucie_ms_peri_bit, 0x1);
    mdlc_check_ms_status(16, 2, ucie_ms_peri_bit);
    /* ms ucie peri run */
    mdlc_transition_ms(16, 2, ucie_ms_peri_bit, 0x3);
    mdlc_check_ms_status(16, 2, ucie_ms_peri_bit);

    uciepwrmngctrl=ucie_apb_base+0x00E00070U;
    val = mem_read32(uciepwrmngctrl);
    val &= ~(1U<<4);                 // Clear sys_aux_pwr_det bit (bit 4 )
    val |=  (1U<<6);                 // Set app_ready_entr_l23 bit (bit 6)
    mem_write32(uciepwrmngctrl, val);

    switch_clock_source_pll(pll_num);

    /* ms ucie core run */
    mdlc_transition_ms(16, 2, ucie_ms_core_bit, 0x3);
    mdlc_check_ms_status(16, 2, ucie_ms_core_bit);
}
#else
void Ucie_PowerOn(e_ucie_ch_t ch)
{
    e_x5h_clock_id_t ucie_peri_clk_id;
    e_x5h_clock_id_t ucie_core_clk_id;
    e_x5h_reset_domain_id_t ucie_peri_reset_id;
    e_x5h_reset_domain_id_t ucie_core_reset_id;

    if(ch == UCIE_CH0) {
        ucie_peri_clk_id = X5H_CLOCK_ID_MDLC_UCIE02;
        ucie_core_clk_id = X5H_CLOCK_ID_MDLC_UCIE01;
        ucie_peri_reset_id = X5H_RESET_DOMAIN_ID_UCIE02;
        ucie_core_reset_id = X5H_RESET_DOMAIN_ID_UCIE01;
    }
    else if (ch == UCIE_CH1) {
        ucie_peri_clk_id = X5H_CLOCK_ID_MDLC_UCIE12;
        ucie_core_clk_id = X5H_CLOCK_ID_MDLC_UCIE11;
        ucie_peri_reset_id = X5H_RESET_DOMAIN_ID_UCIE12;
        ucie_core_reset_id = X5H_RESET_DOMAIN_ID_UCIE11;
    }
    
    R_StateManager_PowerOn(X5H_POWER_DOMAIN_ID_UCI);
    
    R_StateManager_ClockOn(ucie_peri_clk_id);
    R_StateManager_ResetAssert(ucie_peri_reset_id);
    R_StateManager_ResetDeassert(ucie_peri_reset_id);
    
    if (ch == UCIE_CH0) {
        *(volatile uint32_t *)(UCIE_APB0_UCIEPWRMNGCTRL) = 0x00000040;
    }
    else if (ch == UCIE_CH1) {
        *(volatile uint32_t *)(UCIE_APB1_UCIEPWRMNGCTRL) = 0x00000040;
    }

    if (ch == UCIE_CH0) {
        set_pll9_0(LINKSPEED_4GTPS);
    }
    else if (ch == UCIE_CH1) {
        set_pll9_1(LINKSPEED_4GTPS);
    }

    R_StateManager_ClockOn(ucie_core_clk_id);
    R_StateManager_ResetAssert(ucie_core_reset_id);
    R_StateManager_ResetDeassert(ucie_core_reset_id);
}
#endif

#if (NOT_SCP_SUPPORT == 1)
void Ucie_PowerOFF(e_ucie_ch_t ch)
{
    uint32_t ucixcoreclkcr;
    uint32_t pll_num;
    uint32_t ucie_ms_core_bit;
    uint32_t ucie_ms_peri_bit;
    volatile uintptr_t uciepwrmngctrl;
    uint32_t ucie_apb_base;
    uint32_t val;

    if (ch == UCIE_CH0){
        ucixcoreclkcr = 0xDE201080U;
        pll_num = 19;
        ucie_ms_peri_bit=2;
        ucie_ms_core_bit=0;
        ucie_apb_base=0xDC000000;
    } else {
        ucixcoreclkcr = 0xDE201084U;
        pll_num = 20;
        ucie_ms_peri_bit=6;
        ucie_ms_core_bit=4;
        ucie_apb_base=0xDD000000;
    }

    mdlc_transition_ms(16, 2, ucie_ms_core_bit, 0x1);
    mdlc_transition_ms(16, 2, ucie_ms_peri_bit, 0x1);
    mdlc_transition_ms(16, 2, ucie_ms_core_bit, 0x0);
    mdlc_transition_ms(16, 2, ucie_ms_peri_bit, 0x0);
}
#else
void Ucie_PowerOFF(e_ucie_ch_t ch)
{
    e_x5h_clock_id_t ucie_peri_clk_id;
    e_x5h_clock_id_t ucie_core_clk_id;
    e_x5h_reset_domain_id_t ucie_peri_reset_id;
    e_x5h_reset_domain_id_t ucie_core_reset_id;

    if(ch == UCIE_CH0) {
        ucie_peri_clk_id = X5H_CLOCK_ID_MDLC_UCIE02;
        ucie_core_clk_id = X5H_CLOCK_ID_MDLC_UCIE01;
        ucie_peri_reset_id = X5H_RESET_DOMAIN_ID_UCIE02;
        ucie_core_reset_id = X5H_RESET_DOMAIN_ID_UCIE01;
    }
    else if (ch == UCIE_CH1) {
        ucie_peri_clk_id = X5H_CLOCK_ID_MDLC_UCIE12;
        ucie_core_clk_id = X5H_CLOCK_ID_MDLC_UCIE11;
        ucie_peri_reset_id = X5H_RESET_DOMAIN_ID_UCIE12;
        ucie_core_reset_id = X5H_RESET_DOMAIN_ID_UCIE11;
    }
    
    R_StateManager_ResetAssert(ucie_core_reset_id);
    R_StateManager_ClockOff(ucie_core_clk_id);

    R_StateManager_ResetAssert(ucie_peri_reset_id);
    R_StateManager_ClockOff(ucie_peri_clk_id);
}
#endif

uint32_t R_UCIE_Config(e_ucie_ch_t ch, e_ucie_mode_t mode,
                       e_ucie_linkspeed_t speed, bool init_with_system)
{
    ucie_ctrl_arr[ch].mode = mode;
    ucie_ctrl_arr[ch].speed = speed;
    ucie_ctrl_arr[ch].init_with_system = init_with_system;
    return 0;
}

e_ucie_linkup_status_t R_UCIE_Setup(e_ucie_ch_t ch, e_ucie_mode_t mode, 
                                                    e_ucie_linkspeed_t speed)
{
    e_ucie_linkup_status_t ret;

    if (ch != UCIE_CH0 && ch != UCIE_CH1) {
        printf("ERROR: Invalid UCIe channel\n");
        return LINKUP_ERROR;
    }

    if (mode != UCIE_MODE_EP && mode != UCIE_MODE_RC) {
        printf("ERROR: Invalid UCIe mode\n");
        return LINKUP_ERROR;
    }

    if (speed < LINKSPEED_4GTPS || speed > LINKSPEED_16GTPS){
        printf("ERROR: Invalid UCIe link speed\n");
        return LINKUP_ERROR;
    }

    ret = Ucie_Setup_PCIE_Wait_LinkUp(ch);
    if (ret == LINKUP_SUCCESS) {
        if (ucie_is_setup[ch] == true)
        {
            return ret;
        }

        /* Power off UCIe linkup by IPL and relinkup */
        Ucie_PowerOFF(ch);
    }

    ucie_is_setup[ch] = true;

    /* [Step 0] UCIe Power ON */
    Ucie_PowerOn(ch);

    /* [Step 1] UCIe Setup Pre */
    Ucie_Setup_Pre(ch, mode);

    wait_time(0x8000);

    /* [Step 2] UCIe Start Linkup */
    Ucie_Start_Linkup(ch, mode, speed);

    /* [Step 3] UCIe Wait FreqChange Req */
    Ucie_Wait_FreqChange_Req(ch);

    /* [Step 4] UCIe Ack FreqChange */
    Ucie_Ack_FreqChange(ch, speed);

    wait_time(0x8000);
    /* [Step 5] UCIe Wait Linkup */
    Ucie_Wait_Linkup(ch);
    wait_time(0x8000);

    /*  PCIE linkup */
    /* [Step 6] UCIe Setup PCIe pre */
    Ucie_Setup_PCIE_Pre(ch, mode);
    wait_time(0x8000);

    /* [Step 7] UCIe PCIe Start LinkUp */
    Ucie_Setup_PCIE_Start_LinkUp(ch, mode);

    wait_time(0x8000);
    /* [Step 8] UCIe PCIe Wait LinkUp */
    ret = Ucie_Setup_PCIE_Wait_LinkUp(ch);
    wait_time(0x8000);

    /* [Step 9] UCIe PCIe Post */
    Ucie_Setup_PCIE_Post(ch, mode);
    wait_time(0x8000);

    return ret;
}

e_ucie_linkup_status_t R_UCIE_Retry_Linkup(e_ucie_ch_t ch, e_ucie_mode_t mode, 
                                           e_ucie_linkspeed_t speed, uint16_t retry)
{
    e_ucie_linkup_status_t ret;

    while(retry--) {
        ret = R_UCIE_Setup(ch, mode, speed);
        if (ret != LINKUP_TIMEOUT) {
            return ret;
        }
    }

    return LINKUP_TIMEOUT;
}

e_ucie_linkup_status_t R_UCIE_Get_Linkup_Status(e_ucie_ch_t ch)
{
    if (ucie_is_setup[ch])
    {
        return Ucie_Setup_PCIE_Wait_LinkUp(ch);
    }
    return LINKUP_ERROR;
}

uint32_t R_UCIE_IATU_SetRegion(st_ucie_iatu_cfg_t *cfg)
{ 
    e_ucie_ch_t ucie_ch = cfg->ucie_ch;
    e_ucie_iatu_region_t rgn = cfg->rgn;
    e_ucie_iatu_type_t type = cfg->type;
    uint64_t mSrcAddr = cfg->mSrcAddr;
    uint64_t mDestAddr = cfg->mDestAddr;
    uint32_t size = cfg->size;
    
    uint32_t base;

    if (ucie_ch != UCIE_CH0 && ucie_ch != UCIE_CH1) {
        printf("ERROR: Invalid UCIe channel\n");
        return 1;
    }

    if (rgn < IATU_RGN0 || rgn > IATU_RGN31) {
        printf("ERROR: Invalid iATU region\n");
        return 1;
    }

    if (type != IATU_OUTBOUND && type != IATU_INBOUND) {
        printf("ERROR: Invalid iATU type\n");
        return 1;
    }

    if ((mSrcAddr & IATU_ADDR_MASK) || (mDestAddr & IATU_ADDR_MASK) || 
                                                    (size & IATU_ADDR_MASK)) {
        printf("ERROR: Src addr, dest addr and size must be aligned with 4KB\n");
        return 1;
    }
    
    base = UCIE_AXI_BASE(ucie_ch) + PF0_ATU_CAP_BASE_ADD + 
                                   (type * IATU_INBOUND_OFFSET) + (rgn * IATU_BLOCK_SIZE);

    mem_write32(base + IATU_LWR_BASE_ADDR_OFF, (mSrcAddr & 0xFFFFFFFF));
    mem_write32(base + IATU_UPPER_BASE_ADDR_OFF, (mSrcAddr >> 32));
    mem_write32(base + IATU_LIMIT_ADDR_OFF, (size - 0x10));
    mem_write32(base + IATU_LWR_TARGET_ADDR_OFF, (mDestAddr & 0xFFFFFFFF));
    mem_write32(base + IATU_UPPER_TARGET_ADDR_OFF, (mDestAddr >> 32));
    mem_write32(base + IATU_REGION_CTRL_1_OFF, 0x00000000);
    mem_write32(base + IATU_REGION_CTRL_2_OFF, 0x80000000U);

    return 0;
}

uint32_t R_UCIE_IATU_UnsetRegion(st_ucie_iatu_cfg_t *cfg)
{
    e_ucie_ch_t ucie_ch = cfg->ucie_ch;
    e_ucie_iatu_region_t rgn = cfg->rgn;
    e_ucie_iatu_type_t type = cfg->type;

    uint32_t base;

    if (ucie_ch != UCIE_CH0 && ucie_ch != UCIE_CH1) {
        printf("ERROR: Invalid UCIe channel\n");
        return 1;
    }

    if (rgn < IATU_RGN0 || rgn > IATU_RGN31) {
        printf("ERROR: Invalid iATU region\n");
        return 1;
    }

    if (type != IATU_OUTBOUND && type != IATU_INBOUND) {
        printf("ERROR: Invalid iATU type\n");
        return 1;
    }

    base = UCIE_AXI_BASE(ucie_ch) + PF0_ATU_CAP_BASE_ADD +
                                   (type * IATU_INBOUND_OFFSET) + (rgn * IATU_BLOCK_SIZE);

    mem_write32(base + IATU_REGION_CTRL_2_OFF, 0x00000000U);
}

st_ucie_ctrl_t ucie_get_config(e_ucie_ch_t ch)
{
    return ucie_ctrl_arr[ch];
}

void ucie_set_setup_flag(e_ucie_ch_t ch)
{
    ucie_is_setup[ch] = true;
}
