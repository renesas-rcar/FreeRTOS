/*
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef TRANSPORT_UCIE_H
#define TRANSPORT_UCIE_H

#include <stddef.h>
#include <stdint.h>
#include "platform_rcar_rpmsg.h"

/*
 * Prepare the PLATFORM_LINK_UCIE0 link for side @type, once per side.
 * BE: after the rsc table is written. FE: before it is read.
 * RC: links up, then maps [shm_base, shm_base + shm_size) through SMMU_RT
 *     (rest of DRAM identity mapped, so the local link keeps working).
 * EP: waits for its system task to link up, then maps the window with an
 *     inbound iATU.
 * Handshake: FE writes a nonce, BE echoes it once its rsc table is
 * written. BE waits forever, FE times out.
 * Returns 0, -EINVAL, -EIO (iATU / SMMU), -ETIMEDOUT or -ENODEV (no UCIe on
 * BOARD).
 */
int platform_ucie_prepare(virtio_type_t type, uintptr_t shm_base, size_t shm_size);

#endif /* TRANSPORT_UCIE_H */
