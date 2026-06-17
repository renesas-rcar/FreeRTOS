/*
 *
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

/*******************************************************************************
 * DESCRIPTION   : Clock Controller register header
 ******************************************************************************/
#ifndef CLOCK_CONTROLLER_REGISTER_H_
#define CLOCK_CONTROLLER_REGISTER_H_
#include "module_controller/module_controller.h"

/* PLL9_0 Register, HSCS Hierarchy */
#define PLL9_0_CR0  (BASE_ADDR_HIER_HSCS + 0x000011ECU)
#define PLL9_0_CR1  (BASE_ADDR_HIER_HSCS + 0x000011F0U)
#define PLL9_0_CR2  (BASE_ADDR_HIER_HSCS + 0x000011F4U)
#define PLL9_0SCR   (BASE_ADDR_HIER_HSCS + 0x00001308U)
#define PLL9_0DCR   (BASE_ADDR_HIER_HSCS + 0x0000130CU)

/* PLL9_1 Register, HSCS Hierarchy */
#define PLL9_1_CR0  (BASE_ADDR_HIER_HSCS + 0x000011F8U)
#define PLL9_1_CR1  (BASE_ADDR_HIER_HSCS + 0x000011FCU)
#define PLL9_1_CR2  (BASE_ADDR_HIER_HSCS + 0x00001200U)
#define PLL9_1SCR   (BASE_ADDR_HIER_HSCS + 0x00001310U)
#define PLL9_1DCR   (BASE_ADDR_HIER_HSCS + 0x00001314U)

/* Clock Controller Register 0 Key Code Protection Register for Hierarchy */
#define OFFSET_CLK_HIER_PKCPROT0    (0x00001370U)
#define CLKHSCSPKCPROT0             (BASE_ADDR_HIER_HSCS + OFFSET_CLK_HIER_PKCPROT0)    /* HSCS Hierarchy */

#endif  /* CLOCK_CONTROLLER_REGISTER_H_ */
