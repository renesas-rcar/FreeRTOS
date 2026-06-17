/*
 *
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef R_UCIE_CONF_PRIVATE_H
#define R_UCIE_CONF_PRIVATE_H

#include "ucie/r_ucie.h"
#include "ucie_common.h"

static st_ucie_ctrl_t ucie_ctrl_arr[] = {
    [UCIE_CH0] = {UCIE_MODE_EP, LINKSPEED_4GTPS, true},
    [UCIE_CH1] = {UCIE_MODE_EP, LINKSPEED_4GTPS, false}
};

#endif
