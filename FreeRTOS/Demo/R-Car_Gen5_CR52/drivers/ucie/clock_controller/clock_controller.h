/*
 *
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

/*******************************************************************************
 * DESCRIPTION   : Clock Controller driver header
 ******************************************************************************/
#ifndef CLOCK_CONTROLLER_H_
#define CLOCK_CONTROLLER_H_

#include <stdint.h>
#include <clock_controller/clock_controller_register.h>

typedef struct
{
    const char *cpgm_name;   /* CPGM name */
} CPGM_NAME_TABLE;

#define PLL_SUCCESS     (0U)
#define PLL_RETRY_MAX   (1000U)

/* Maxium number of PLLs */
#define PLL_MAX     (30U)
#define REG_ADDR    (0U)
#define REG_VAL     (1U)

#define PLL9_0      (19U)   /* HSCS Hierarchy */
#define PLL9_1      (20U)   /* HSCS Hierarchy */

/* CPGM neme */
#define CPGM_NAME_PLL9_0    "CPGMUCI0 (PLL9_0)"
#define CPGM_NAME_PLL9_1    "CPGMUCI1 (PLL9_1)"

/* PLLn_CR status */
#define PLL_CR2_PLLENTRG_START      (0x10000000U)
#define PLL_CR2_PLLCLKSTAB_MASK     (0x80000000U)
#define PLL_CR2_PLLCLKSTAB_UNSTABLE (0x00000000U)
#define PLL_CR2_PLLCLKSTAB_STABLE   (0x80000000U)

/* PLLn_SCR status */
#define PLL_SCR_PLLSELID_CLK_PLL    (0x00000000U)
#define PLL_SCR_PLLSELACT_MASK      (0x00010000U)
#define PLL_SCR_PLLSELACT_CLK_PLL   (0x00000000U)
#define PLL_SCR_PLLSELACT_CLK_IOSC  (0x00010000U)

/* CPGM name array. */
static const CPGM_NAME_TABLE cpgm_name_table[PLL_MAX] =
{   /*              *cpgm_name */
    [PLL9_0]    =   {CPGM_NAME_PLL9_0},
    [PLL9_1]    =   {CPGM_NAME_PLL9_1},
};

/* PLLn Control Register 2 list */
static const uint32_t pll_ctrl_reg2[PLL_MAX] =
{   /* Register address */
    [PLL9_0]    =   PLL9_0_CR2,
    [PLL9_1]    =   PLL9_1_CR2,
};

/* PLLn Selector Control Register list */
static const uint32_t pll_sel_ctrl_reg[PLL_MAX][2U] =
{   /* Register address, Setting value */
    [PLL9_0]    =   {PLL9_0SCR,     PLL_SCR_PLLSELID_CLK_PLL},
    [PLL9_1]    =   {PLL9_1SCR,     PLL_SCR_PLLSELID_CLK_PLL},
};

uint32_t switch_clock_source_pll(uint32_t pll_num);
void clock_controller_reg_write(uint32_t pd_hier, uint32_t reg_addr, uint32_t reg_val);
void pll_reg_write(uint32_t pll_num);
uint32_t get_pd_hier_from_pll(uint32_t pll_num);
const char* get_cpgm_name(uint32_t pll_num);

#endif /* CLOCK_CONTROLLER_H_ */

