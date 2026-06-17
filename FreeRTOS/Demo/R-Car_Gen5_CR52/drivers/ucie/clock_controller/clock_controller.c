/*
 *
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include <stdint.h>
#include <stdio.h>
#include "clock_controller/clock_controller.h"
#include "clock_controller/clock_controller_register.h"
#include "module_controller/module_controller.h"

#define NOTICE(...)     printf(__VA_ARGS__)

static uint32_t mem_read32(const volatile uintptr_t addr)
{
    return *((volatile uint32_t*)(addr));
}

static void mem_write32(volatile uintptr_t addr, uint32_t data)
{
    (void)mem_read32 (addr);
    *((volatile uint32_t*)(addr)) = data;
    (void)mem_read32 (addr);
}

static void panic(void)
{
	printf("PANIC\n");
	while(1);
}

uint32_t switch_clock_source_pll(uint32_t pll_num)
{
    uint32_t clk_stable_stat = PLL_CR2_PLLCLKSTAB_UNSTABLE; /* Initial value of stable status is set to unstable. */
    uint32_t clk_src_stat = PLL_SCR_PLLSELACT_CLK_IOSC;     /* Initial value of clock source status is set to IOSC. */
    uint32_t cnt;
    uint32_t pd_hier;
    uint32_t pd_hier_impl_bit;
    uint32_t mx_info;
    uint32_t impl_bit;

    mx_info = get_mx_setting();
    pd_hier = get_pd_hier_from_pll(pll_num);
    pd_hier_impl_bit = BIT0 << pd_hier;
    if((mx_info & pd_hier_impl_bit) == PD_HIER_NOT_IMPL)
    {
        return PLL_SUCCESS;
    }
    else
    {
        ;
    }

    /* 1. Turn on Power Domain (PD). This process is usually performed automatically by HW. */

    /* 1-1. Additional process for PLL9_0 and PLL9_1. */
        clock_controller_reg_write(pd_hier, pll_ctrl_reg2[pll_num], PLL_CR2_PLLENTRG_START);

    /* 2. Wait for PLL to become stable status. */
    cnt = 0U;
    do
    {
        clk_stable_stat = PLL_CR2_PLLCLKSTAB_MASK & mem_read32(pll_ctrl_reg2[pll_num]);
        cnt++;
        if(cnt > PLL_RETRY_MAX)
        {
            NOTICE("CPGM = %s:, Give up waiting for clock stable state (PLLn_CR2.PLLCLKSTAB). Retry count: %d, Func: %s\r\n", get_cpgm_name(pll_num), cnt, __func__);
            break;
        }
    } while (clk_stable_stat != PLL_CR2_PLLCLKSTAB_STABLE);

    /* 3. Change clock source to PLL. */
    pll_reg_write(pll_num);

    /* 4. Confirm that the clock source change is complete. */
    cnt = 0U;
    do
    {
        clk_src_stat = PLL_SCR_PLLSELACT_MASK & mem_read32(pll_sel_ctrl_reg[pll_num][REG_ADDR]);
        cnt++;
        if(cnt > PLL_RETRY_MAX)
        {
            NOTICE("CPGM = %s:, Give up waiting for switching to PLL. (PLLnSCRegister.PLLnSELACT). Retry count: %d, Func: %s, Line: %d\r\n", get_cpgm_name(pll_num), cnt, __func__);
            break;
        }
    } while (clk_src_stat != PLL_SCR_PLLSELACT_CLK_PLL);

    return PLL_SUCCESS;
}
/* End of function switch_clock_source_pll(void) */


void pll_reg_write(uint32_t pll_num)
{
    uint32_t pd_hier = 0xFFFFFFFFU;

    pd_hier = get_pd_hier_from_pll(pll_num);

    clock_controller_reg_write(pd_hier, pll_sel_ctrl_reg[pll_num][REG_ADDR], pll_sel_ctrl_reg[pll_num][REG_VAL]);

} /* End of function pll_reg_write(uint32_t pll_num) */


/* Write protect is released and write to Clock Controller register. */
void clock_controller_reg_write(uint32_t pd_hier, uint32_t reg_addr, uint32_t reg_val)
{

    /* Dummy read   */
    (void)mem_read32(reg_addr);

    switch(pd_hier)
    {
        case PD_HIER_HSCS:
            mem_write32(CLKHSCSPKCPROT0, WRITE_KEY_CODE_EN);
            mem_write32(reg_addr, reg_val);
            mem_write32(CLKHSCSPKCPROT0, WRITE_KEY_CODE_DIS);
            break;
        default:
            /* This PD Hier does not have Key Code Protection. */
            mem_write32(reg_addr, reg_val);
            break;
    }

} /* End of function clock_controller_reg_write(uint32_t pd_hier, uint32_t reg_addr, uint32_t reg_val) */


/* Get PD Hier number of PLLn. */
uint32_t get_pd_hier_from_pll(uint32_t pll_num)
{
    uint32_t pd_hier = 0xFFFFFFFFU;

    switch(pll_num)
    {
        case PLL9_0:
            pd_hier = PD_HIER_HSCS;
            break;
        case PLL9_1:
            pd_hier = PD_HIER_HSCS;
            break;
        default:
            //NOTICE("PD_hier = %s (%d): Unknown PD_Hier error. Func: %s\r\n", get_hier_name(pd_hier), pd_hier, __func__);
            panic;
    }

    return pd_hier;
}
/* End of function get_pd_hier_from_pll(uint32_t pll_num) */


const char* get_cpgm_name(uint32_t pll_num)
{
    const char* cpgm_name;

    if(pll_num < PLL_MAX)
    {
        cpgm_name = cpgm_name_table[pll_num].cpgm_name;
    }
    else
    {
        NOTICE("PLL number = %d: PLL number is out of range. (Unknown PLL error.) Func: %s\r\n", pll_num, __func__);
        panic;
    }

    return cpgm_name;
} /* End of function get_cpgm_name(uint32_t pd_hier) */

