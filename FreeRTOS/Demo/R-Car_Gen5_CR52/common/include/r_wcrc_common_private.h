/*
 *
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef R_WCRC_COMMON_PRIVATE_H
#define R_WCRC_COMMON_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"

typedef struct {
    uint32_t wcrc_id;
    uint32_t crc_id;
    uint32_t kcrc_id;
} crc_domain_id_list;

#ifdef __cplusplus
}
#endif

#endif /* R_WCRC_COMMON_PRIVATE_H */

