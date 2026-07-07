/*
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

/**
 * @defgroup GPIO_PRIVATE GPIO PRIVATE
 * @{
 * @brief Base address of GPIO AIACC platform.
 *
 */

#ifndef R_GPIO_PRVIATE_H
#define R_GPIO_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

/* GPIO base adrress */
#define GPIO_BASE_OFFSET    0x100U
#define GPIO_GR_NUM   3

static const uint32_t gpio_gr_base[GPIO_GR_NUM] =
{
    (0x38080000U + GPIO_BASE_OFFSET),
    (0x38080800U + GPIO_BASE_OFFSET),
    (0x38081000U + GPIO_BASE_OFFSET)
};

#ifdef __cplusplus
}
#endif

#endif /* R_GPIO_PRIVATE_H */
