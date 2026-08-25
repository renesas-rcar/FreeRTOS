/*******************************************************************************
 * 
 * Copyright (c) 2026 Renesas Electronics Corporation
 * 
 * SPDX-License-Identifier: MIT
 * 
 *******************************************************************************/

/*******************************************************************************
 * DESCRIPTION   : Module Controller initialize
 ******************************************************************************/
 /******************************************************************************
 * @file          module_controller.c
 * - Version      : 0.01
 * @brief         Initial setting process of Module Controller.
 * .
 *****************************************************************************/
/******************************************************************************
 * History : DD.MM.YYYY Version  Description
 *         : 20.07.2026 0.01     First Release
 *****************************************************************************/

#include <stdio.h>
#include <stdint.h>
// #include <mem_io.h>
#include "module_controller.h"

#define NOTICE(...)	printf(__VA_ARGS__)

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

/*****************************************************************************
 * Module Controller common
 *****************************************************************************/
const char* get_class_group_name_f(uint32_t module_num)
{
    const char* class_group_name;

    if(module_num < MODULE_NUM_MAX)
    {
        class_group_name = class_group_name_table[module_num].name;
    }
    else
    {
        NOTICE("Unknown Module Error! Module Number: %d, Func: %s\r\n", module_num, __func__);
        panic;
    }

    return class_group_name;
}

void mdlc_ms_module_run_bit_f(uint32_t module_num, uint32_t reg_num, uint32_t bit_num)
{
    uint32_t ms_stat_reg = ms_reg_table[module_num][reg_num].ms_stat_reg_addr;
    uint32_t bit_assign = ms_reg_table[module_num][reg_num].ms_reg_bit_assign;
    uint32_t ms_stat_val;
    uint32_t ms_mask;

    ms_mask = MS_BIT_MASK << bit_num;
    if((bit_assign & ms_mask) != MS_BIT_RESERVED)
    {
        /* Refer MPG flow to confirm corresponding Power Domain already ON. */

        /* Check Module Standby status                                      */
        ms_stat_val = mem_read32(ms_stat_reg);
        ms_stat_val = ms_stat_val >> bit_num;
        ms_stat_val &= MS_BIT_MASK;

        switch(ms_stat_val)
        {
            case MS_STANDBY:
                /* Module Standby   -> Module Reset     -> Module RUN       */

                /* Start of Module Reset.                                   */
                NOTICE("BIT: %d, STAT: 0x%x, Module Standby -> Module Reset -> Module RUN\r\n", bit_num, ms_stat_val);

                /* Check MDLC[n]MSRESS[l] = MDLC[n]MSRES[l]?                */
                mdlc_check_ms_status_f(module_num, reg_num, bit_num);

                /* Set MDLC[n]MSRES[l]                                      */
                mdlc_transition_ms_f(module_num, reg_num, bit_num, MS_RESET);

                /* Wait MDLC[n]MSRESS[l] = MDLC[n]MSRES[l]?                 */
                mdlc_check_ms_status_f(module_num, reg_num, bit_num);

                /* Completion of Destination State                          */
                /* Check Module Standby status                              */
                ms_stat_val = mem_read32(ms_stat_reg);
                ms_stat_val = ms_stat_val >> bit_num;
                ms_stat_val &= MS_BIT_MASK;
                NOTICE("BIT: %d, STAT: 0x%x, After MS.\r\n", bit_num, ms_stat_val);

                /* Start of Module RUN.                                     */

                /* Set MDLC[n]MSRESS[l]                                     */
                mdlc_transition_ms_f(module_num, reg_num, bit_num, MS_RUN);

                /* Wait MDLC[n]MSRESS[l] = MDLC[n]MSRES[l]?                 */
                mdlc_check_ms_status_f(module_num, reg_num, bit_num);

                /* Completion of Destination State                          */
                /* Check Module Standby status                              */
                ms_stat_val = mem_read32(ms_stat_reg);
                ms_stat_val = ms_stat_val >> bit_num;
                ms_stat_val &= MS_BIT_MASK;
                NOTICE("BIT: %d, STAT: 0x%x, After MS.\r\n", bit_num, ms_stat_val);

                break;

            case MS_RESET:
                /* Module Reset     -> Module RUN                           */

                /* Start of Module RUN.                                     */
                NOTICE("BIT: %d, STAT: 0x%x, Module Reset -> Module RUN\r\n", bit_num, ms_stat_val);

                /* Check MDLC[n]MSRESS[l] = MDLC[n]MSRES[l]?                */
                mdlc_check_ms_status_f(module_num, reg_num, bit_num);

                /* Set MDLC[n]MSRESS[l]                                     */
                mdlc_transition_ms_f(module_num, reg_num, bit_num, MS_RUN);

                /* Wait MDLC[n]MSRESS[l] = MDLC[n]MSRES[l]?                 */
                mdlc_check_ms_status_f(module_num, reg_num, bit_num);

                /* Completion of Destination State                          */
                /* Check Module Standby status                              */
                ms_stat_val = mem_read32(ms_stat_reg);
                ms_stat_val = ms_stat_val >> bit_num;
                ms_stat_val &= MS_BIT_MASK;
                NOTICE("BIT: %d, STAT: 0x%x, After MS.\r\n", bit_num, ms_stat_val);

                break;

            case MS_STOP:
                /* Module STOP      -> Module RUN                           */

                /* Start of Module RUN.                                     */
                NOTICE("BIT: %d, STAT: 0x%x, Module STOP -> Module RUN\r\n", bit_num, ms_stat_val);

                /* Check MDLC[n]MSRESS[l] = MDLC[n]MSRES[l]?                */
                mdlc_check_ms_status_f(module_num, reg_num, bit_num);

                /* Set MDLC[n]MSRESS[l]                                     */
                mdlc_transition_ms_f(module_num, reg_num, bit_num, MS_RUN);

                /* Wait MDLC[n]MSRESS[l] = MDLC[n]MSRES[l]?                 */
                mdlc_check_ms_status_f(module_num, reg_num, bit_num);

                /* Completion of Destination State                          */
                /* Check Module Standby status                              */
                ms_stat_val = mem_read32(ms_stat_reg);
                ms_stat_val = ms_stat_val >> bit_num;
                ms_stat_val &= MS_BIT_MASK;
                NOTICE("BIT: %d, STAT: 0x%x, After MS.\r\n", bit_num, ms_stat_val);

                break;

            case MS_RUN:
                NOTICE("BIT: %d, STAT: 0x%x, Already Module RUN\r\n", bit_num, ms_stat_val);
                break;
            default:
                NOTICE("Unknown MS Status Error! Module Number: %d, Class Group: %s, REG: %d, BIT: %d, STAT: 0x%x, Func: %s\r\n", module_num, get_class_group_name_f(module_num), reg_num, bit_num, ms_stat_val, __func__);
                panic;
                break;
        }
    }
    else
    {
        NOTICE("BIT: %d, Reserved or Skip\r\n", bit_num);
    }

    return;
}

void mdlc_transition_ms_f(uint32_t module_num, uint32_t reg_num, uint32_t bit_num, uint32_t ms_dest)
{
    uint32_t ms_reset_reg = ms_reg_table[module_num][reg_num].ms_reset_reg_addr;
    uint32_t ms_stat_reg = ms_reg_table[module_num][reg_num].ms_stat_reg_addr;
    uint32_t ms_reset_val;
    uint32_t ms_stat_val;
    uint32_t ms_mask;

    ms_stat_val = mem_read32(ms_stat_reg);
    ms_stat_val = ms_stat_val >> bit_num;
    ms_stat_val &= MS_BIT_MASK;

    if(ms_stat_val != ms_dest)
    {
        ms_mask = ~(MS_BIT_MASK << bit_num);
        ms_reset_val = mem_read32(ms_reset_reg) & ms_mask;
        ms_reset_val |= (ms_dest << bit_num);
        mdlc_ms_reg_write_f(module_num, ms_reset_reg, ms_reset_val);
    }
    else
    {
        /* do nothing   */
    }

    return;
}

/* Write protect is released and write to MS register of MDLC. */
void mdlc_ms_reg_write_f(uint32_t pd_hier, uint32_t reg_addr, uint32_t reg_val)
{
    /* Enable write access of protected MS registers. */
    mem_write32(mdlcnpkcprot1_reg[pd_hier], WRITE_KEY_CODE_EN);

    /* Write MDLC register. */
    mem_write32(reg_addr, reg_val);

    /* Disable write access of protected MS registers. */
    mem_write32(mdlcnpkcprot1_reg[pd_hier], WRITE_KEY_CODE_DIS);
} /* End of function mdlc_ms_reg_write_f(uint32_t pd_hier, uint32_t reg_addr, uint32_t reg_val) */

void mdlc_check_ms_status_f(uint32_t module_num, uint32_t reg_num, uint32_t bit_num)
{
    uint32_t ms_reset_reg = ms_reg_table[module_num][reg_num].ms_reset_reg_addr;
    uint32_t ms_stat_reg = ms_reg_table[module_num][reg_num].ms_stat_reg_addr;
    uint32_t ms_reset_val;
    uint32_t ms_stat_val;
    uint32_t cnt = 0U;

    do
    {
        ms_reset_val = mem_read32(ms_reset_reg);
        ms_stat_val = mem_read32(ms_stat_reg);
        
        cnt++;
        if(cnt > MDLC_RETRY_MAX)
        {
            NOTICE("MS Give up Error! Module Number: %d, Class Group: %s, REG: %d, BIT: %d, STAT: 0x%x, Func: %s\r\n", module_num, get_class_group_name_f(module_num), reg_num, bit_num, ms_stat_val, __func__);
            break;
        }
    } while (ms_stat_val != ms_reset_val);

    return;
}

const char *get_hier_name_f(uint32_t pd_hier)
{
    const char *hier_name;

    if (pd_hier < PD_HIER_MAX)
    {
        hier_name = hier_name_table[pd_hier].hier_name;
    }
    else
    {
        NOTICE("PD_hier = %d: PD_hier number is out of range. (Unknown PD_Hier error.) Func: %s\r\n", pd_hier, __func__);
        panic;
    }

    return hier_name;
} /* End of function get_hier_name_f(uint32_t pd_hier) */
