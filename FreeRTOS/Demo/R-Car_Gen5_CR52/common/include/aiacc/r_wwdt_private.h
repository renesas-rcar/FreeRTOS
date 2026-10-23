/*
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef R_WWDT_PRIVATE_H
#define R_WWDT_PRIVATE_H

#include <stdint.h>
#include "state-manager/r_clock_domain_id.h"
#include "state-manager/r_reset_domain_id.h"

#define R_WWDT0_BASE	0x1C100000U
#define R_WWDT1_BASE	0x1C110000U
#define R_WWDT2_BASE	0xC1380000U

static const uintptr_t wwdt_base_tbl[] =
{
    R_WWDT0_BASE,
    R_WWDT1_BASE,
    R_WWDT2_BASE
};

#define WWDT_CH_NUM    ((uint32_t)(sizeof(wwdt_base_tbl) / sizeof(wwdt_base_tbl[0])))

#define WWDT_ID_REQUIRED_NUM 2U

typedef struct
{
    int clock_id_0;
    int clock_id_1;
} wwdt_clock_t;

typedef struct
{
    int reset_id_0;
    int reset_id_1;
} wwdt_reset_t;

static const int wwdt_clock_required_tbl[WWDT_ID_REQUIRED_NUM] = {
    AIACC_CLOCK_ID_WDT0,
    AIACC_CLOCK_ID_WDT1
};

static const int wwdt_reset_required_tbl[WWDT_ID_REQUIRED_NUM] = {
    AIACC_RESET_DOMAIN_ID_WDT0,
    AIACC_RESET_DOMAIN_ID_WDT1
};

static const wwdt_clock_t wwdt_clock_tbl[] =
{
    { AIACC_CLOCK_ID_WWDT00,  AIACC_CLOCK_ID_WWDT01  },
    { AIACC_CLOCK_ID_WWDT10,  AIACC_CLOCK_ID_WWDT11  },
    { AIACC_CLOCK_ID_WWDT20,  AIACC_CLOCK_ID_WWDT21  },
};

static const wwdt_reset_t wwdt_reset_tbl[] =
{
    { AIACC_RESET_DOMAIN_ID_WWDT00,  AIACC_RESET_DOMAIN_ID_WWDT01  },
    { AIACC_RESET_DOMAIN_ID_WWDT10,  AIACC_RESET_DOMAIN_ID_WWDT11  },
    { AIACC_RESET_DOMAIN_ID_WWDT20,  AIACC_RESET_DOMAIN_ID_WWDT21  },
};

#endif /* R_WWDT_PRIVATE_H */
