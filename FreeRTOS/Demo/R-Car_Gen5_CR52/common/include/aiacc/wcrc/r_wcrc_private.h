/*
 *
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef R_WCRC_PRIVATE_H
#define R_WCRC_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "wcrc/r_wcrc_common.h"
#include "r_wcrc_common_private.h"
#include "state-manager/r_clock_domain_id.h"

#define UNIT_MAX WCRC_02

static const crc_domain_id_list domain_id_lists[UNIT_MAX + 1] = {
    {AIACC_CLOCK_ID_WCRC0, AIACC_CLOCK_ID_CRC0, AIACC_CLOCK_ID_KCRC0},
    {AIACC_CLOCK_ID_WCRC1, AIACC_CLOCK_ID_CRC1, AIACC_CLOCK_ID_KCRC1},
    {AIACC_CLOCK_ID_WCRC2, AIACC_CLOCK_ID_CRC2, AIACC_CLOCK_ID_KCRC2},
};

#ifdef __cplusplus
}
#endif

#endif /* R_WCRC_PRIVATE_H */

