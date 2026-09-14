/*
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

/**
 * @defgroup AUDIO_PRIVATE AUDIO PRIVATE
 * @{
 * @brief AUDIO PRIVATE of MDP X5H platform.
 *
 */

#ifndef R_AUDIO_PRIVATE_H
#define R_AUDIO_PRIVATE_H

#include "stdint.h"
#include "state-manager/r_clock_domain_id.h"

#ifdef __cplusplus
extern "C" {
#endif

static const uint32_t audio_clock_domain_id[] =
{
    X5H_CLOCK_ID_MDLC_ADG0,
    X5H_CLOCK_ID_MDLC_SSI0,
    X5H_CLOCK_ID_MDLC_SSI05
};

#define AUDIO_CLOCK_ID_COUNT \
    (sizeof(audio_clock_domain_id) / sizeof(audio_clock_domain_id[0]))

#ifdef __cplusplus
}
#endif

/** @} */ // end of AUDIO_PRIVATE

#endif /* R_AUDIO_PRIVATE_H */
