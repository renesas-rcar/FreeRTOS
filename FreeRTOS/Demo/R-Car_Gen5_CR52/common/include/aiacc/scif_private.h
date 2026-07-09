/*
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

 /**
 * @defgroup SCIF_PRIVATE SCIF PRIVATE
 * @{
 * @brief SCIF of AIACC platform.
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

static uint32_t serial_channels_arr[] = {
    0x38000000, // SCIF0
    0x38004000, // SCIF1
    0x0,
    0x0,
    0x0,
    0x38010000, // HSCIF0
    0x38014000, // HSCIF1
    0x0,
    0x0
};

static const e_module_id_t dev_to_module_list[] = {
    MODULE_SCIF0,
    MODULE_SCIF1,
    MODULE_INVALID,
    MODULE_INVALID,
    MODULE_INVALID,
    MODULE_HSCIF0,
    MODULE_HSCIF1,
    MODULE_INVALID,
    MODULE_INVALID,
};

#ifdef __cplusplus
}
#endif

/** @} */ // end of SCIF_PRIVATE

#endif /* SCIF_PRIVATE_H */
 