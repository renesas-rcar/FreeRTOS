/*
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef TCM_H
#define TCM_H

/** RCAR TCM regions */
typedef enum e_rcar_tcm_region_t
{
    RCAR_TCM_A = 0,         ///< TCM region A
    RCAR_TCM_B,             ///< TCM region B
    RCAR_TCM_C              ///< TCM region C
} rcar_tcm_region_t;

/** RCAR TCM state */
typedef enum e_rcar_tcm_state_t
{
    RCAR_TCM_ENABLE = 0,    ///< Enable TCM
    RCAR_TCM_DISABLE,       ///< Disable TCM
} rcar_tcm_state_t;

/** RCAR TCM size:
 *  X5H CR52 only supports size 8KB, 16KB and 32KB.
 */
typedef enum e_rcar_tcm_size_t
{
    RCAR_TCM_SIZE_8KB = 0,  ///< TCM size 8KB
    RCAR_TCM_SIZE_16KB,     ///< TCM size 16KB
    RCAR_TCM_SIZE_32KB      ///< TCM size 32KB
} rcar_tcm_size_t;

/** RCAR TCM level of EL */
typedef enum e_rcar_tcm_el_t
{
    RCAR_TCM_EL0 = 0,       ///< EL0 can access TCM.
    RCAR_TCM_EL1,           ///< EL1 can access TCM.
    RCAR_TCM_EL2            ///< EL2 can access TCM.
} rcar_tcm_el_t;

/**
 * Configure TCM.
 *
 * @param[in] region            - TCM region, see @ref rcar_tcm_region_t.
 * @param[in] base_addr         - SW map address to access TCM.
 * @param[in] size              - TCM size, see @ref rcar_tcm_size_t.
 *
 */
void ConfigureTCM(rcar_tcm_region_t region, uint32_t base_addr, rcar_tcm_size_t size);

/**
 * Control TCM.
 *
 * @param[in] region            - TCM region, see @ref rcar_gpio_group_t.
 * @param[in] el_level          - Level of EL to access TCM, see @ref rcar_tcm_el_t.
 * @param[in] state             - Enable or disable, see @ref rcar_tcm_state_t.
 *
 */
void ControlTCM(rcar_tcm_region_t region, rcar_tcm_el_t el_level, rcar_tcm_state_t state);

#endif /* TCM_H*/
