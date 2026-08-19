/*
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef R_SWDT_PRIVATE_H
#define R_SWDT_PRIVATE_H

#include "state-manager/r_clock_domain_id.h"
#include "state-manager/r_reset_domain_id.h"

#define SWDT_CLK_ID		AIACC_CLOCK_ID_WDT0
#define SWDT0_RST_ID	AIACC_RESET_DOMAIN_ID_SWDT0
#define SWDT1_RST_ID	AIACC_RESET_DOMAIN_ID_SWDT1

#endif /* R_SWDT_PRIVATE_H */
