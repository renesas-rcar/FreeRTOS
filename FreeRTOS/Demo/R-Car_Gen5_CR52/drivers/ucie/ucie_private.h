/*
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef R_UCIE_PRIVATE_H
#define R_UCIE_PRIVATE_H

#define RCAR_UCIE_BASE_ADD(a) ((a) & 0x00ffffffU)
#define RCAR_UCIE_FN_OFS(f) (((uint32_t)(f) & 0x7U) << 16)

/*
 * The "rcar_ucie_v100" folder is header code for X5H
 * The "rcar_ucie_v102" folder is header code for AIACC SIM3.8.0.
 */
#ifdef RCAR_UCIE_V100
#include "rcar_ucie_v100/rcar_ucie_PCIe_Controller.h"
#include "rcar_ucie_v100/rcar_ucie_UCIe_Controller.h"
#include "rcar_ucie_v100/rcar_ucie_UCIe_PHY.h"
#include "rcar_ucie_v100/rcar_ucie_misc.h"
#endif  /* X5H UCIe */

#ifdef RCAR_UCIE_V102
#include "rcar_ucie_v102/rcar_ucie_PCIe_Controller.h"
#include "rcar_ucie_v102/rcar_ucie_UCIe_Controller.h"
#include "rcar_ucie_v102/rcar_ucie_UCIe_PHY.h"
#include "rcar_ucie_v102/rcar_ucie_misc.h"
#endif /* AI_ACC UCIe */

#endif // R_UCIE_PRIVATE_H
