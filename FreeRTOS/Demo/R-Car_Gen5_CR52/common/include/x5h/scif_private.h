/*
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

/**
 * @defgroup SCIF_PRIVATE SCIF PRIVATE
 * @{
 * @brief SCIF of X5H platform.
 *
 */
#ifndef SCIF_PRIVATE_H
#define SCIF_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "device_tree.h"

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

static const uint32_t serial_channels_arr[] = {
    0xc0700000, // SCIF0
    0xc0704000, // SCIF1
    0x0,        // Unsupported
    0xc0708000, // SCIF3
    0xc070C000, // SCIF4
    0xc0710000, // HSCIF0
    0xc0714000, // HSCIF1
    0xc0718000, // HSCIF2
    0xc071C000  // HSCIF3
};

static const e_module_id_t dev_to_module_list[] = {
    MODULE_SCIF0,
    MODULE_SCIF1,
    MODULE_INVALID,
    MODULE_SCIF3,
    MODULE_SCIF4,
    MODULE_HSCIF0,
    MODULE_HSCIF1,
    MODULE_HSCIF2,
    MODULE_HSCIF3,
};

#ifdef __cplusplus
}
#endif

/** @} */ // end of SCIF_PRIVATE

#endif /* SCIF_PRIVATE_H */
