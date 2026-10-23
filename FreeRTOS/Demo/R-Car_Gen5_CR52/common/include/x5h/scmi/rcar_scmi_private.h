/*
 *
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef RCAR_SCMI_PRIVATE_H
#define RCAR_SCMI_PRIVATE_H

#include "cmsis_rcar_gen5.h"

/* Shared memory address */
#define SCMI_SHMEM_BASE_ADDR       (0xC1000000U)
#define SCMI_SHMEM_PLATFORM_MAIN   (SCMI_SHMEM_BASE_ADDR + (0x60200U))
#define SCMI_SHMEM_AGENT_MAIN      (SCMI_SHMEM_BASE_ADDR + (0x60300U))
#define SCMI_SHMEM_PLATFORM_2ND    (SCMI_SHMEM_BASE_ADDR + (0x60400U))
#define SCMI_SHMEM_AGENT_2ND       (SCMI_SHMEM_BASE_ADDR + (0x60500U))

/* Shared memory size */
#define SCMI_SHMEM_SIZE         (256U)

#endif /* RCAR_SCMI_PRIVATE_H */

