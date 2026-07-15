/*
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

/**
 * @defgroup GPIO_PRIVATE GPIO PRIVATE
 * @{
 * @brief Base address of GPIO X5H platform.
 *
 */

#ifndef R_GPIO_PRIVATE_H
#define R_GPIO_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

/* GPIO base adrress */
#define GPIO_BASE_OFFSET    0x100U
#define GPIO_GR_NUM   11

static const uint32_t gpio_gr_base[GPIO_GR_NUM] =
{
    (0xC1080000U + GPIO_BASE_OFFSET),
    (0xC1080800U + GPIO_BASE_OFFSET),
    (0xC1081000U + GPIO_BASE_OFFSET),
    (0xC0800000U + GPIO_BASE_OFFSET),
    (0xC0800800U + GPIO_BASE_OFFSET),
    (0xC0400000U + GPIO_BASE_OFFSET),
    (0xC0400800U + GPIO_BASE_OFFSET),
    (0xC0401000U + GPIO_BASE_OFFSET),
    (0xC0401800U + GPIO_BASE_OFFSET),
    (0xC9B00000U + GPIO_BASE_OFFSET),
    (0xC9B00800U + GPIO_BASE_OFFSET)
};

#ifdef __cplusplus
}
#endif

#endif /* R_GPIO_PRIVATE_H */
