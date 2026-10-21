/*
 *
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef R_STATE_MANAGER_PRIVATE_H
#define R_STATE_MANAGER_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "state-manager/r_power_domain_id.h"
#include "r_state_manager_common_private.h"

static domain_id_range_t power_domain_id_async_range = {
    .domain_id_low = X5H_POWER_DOMAIN_ID_P_RPU_CORE00,
    .domain_id_high = X5H_POWER_DOMAIN_ID_Q_APU_P07
};

#ifdef __cplusplus
}
#endif

#endif /* R_STATE_MANAGER_PRIVATE_H */

