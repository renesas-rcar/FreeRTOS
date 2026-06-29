/*************************************************************************************************************
* Copyright (c) 2025 Renesas Electronics Corporation
*
* SPDX-License-Identifier: MIT
*************************************************************************************************************/

#ifndef R_GPIO_API_H
#define R_GPIO_API_H

#include "interrupts.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

/** GPIO group */
typedef enum e_rcar_gpio_group
{
    RCAR_GPIO_GROUP_00 = 0x00,           ///< GPIO Group 00
    RCAR_GPIO_GROUP_01 = 0x01,           ///< GPIO Group 01
    RCAR_GPIO_GROUP_02 = 0x02,           ///< GPIO Group 02
    RCAR_GPIO_GROUP_03 = 0x03,           ///< GPIO Group 03
    RCAR_GPIO_GROUP_04 = 0x04,           ///< GPIO Group 04
    RCAR_GPIO_GROUP_05 = 0x05,           ///< GPIO Group 05
    RCAR_GPIO_GROUP_06 = 0x06,           ///< GPIO Group 06
    RCAR_GPIO_GROUP_07 = 0x07,           ///< GPIO Group 07
    RCAR_GPIO_GROUP_08 = 0x08,           ///< GPIO Group 08
    RCAR_GPIO_GROUP_09 = 0x09,           ///< GPIO Group 09
    RCAR_GPIO_GROUP_10 = 0x0A,           ///< GPIO Group 10
} rcar_gpio_group_t;

/** Pin each group */
typedef enum e_rcar_pin
{
    RCAR_PIN_00 = 0x00,           ///< GPIO Pin 00
    RCAR_PIN_01 = 0x01,           ///< GPIO Pin 01
    RCAR_PIN_02 = 0x02,           ///< GPIO Pin 02
    RCAR_PIN_03 = 0x03,           ///< GPIO Pin 03
    RCAR_PIN_04 = 0x04,           ///< GPIO Pin 04
    RCAR_PIN_05 = 0x05,           ///< GPIO Pin 05
    RCAR_PIN_06 = 0x06,           ///< GPIO Pin 06
    RCAR_PIN_07 = 0x07,           ///< GPIO Pin 07
    RCAR_PIN_08 = 0x08,           ///< GPIO Pin 08
    RCAR_PIN_09 = 0x09,           ///< GPIO Pin 09
    RCAR_PIN_10 = 0x0A,           ///< GPIO Pin 10
    RCAR_PIN_11 = 0x0B,           ///< GPIO Pin 11
    RCAR_PIN_12 = 0x0C,           ///< GPIO Pin 12
    RCAR_PIN_13 = 0x0D,           ///< GPIO Pin 13
    RCAR_PIN_14 = 0x0E,           ///< GPIO Pin 14
    RCAR_PIN_15 = 0x0F,           ///< GPIO Pin 15
    RCAR_PIN_16 = 0x10,           ///< GPIO Pin 16
    RCAR_PIN_17 = 0x11,           ///< GPIO Pin 17
    RCAR_PIN_18 = 0x12,           ///< GPIO Pin 18
    RCAR_PIN_19 = 0x13,           ///< GPIO Pin 19
    RCAR_PIN_20 = 0x14,           ///< GPIO Pin 20
    RCAR_PIN_21 = 0x15,           ///< GPIO Pin 21
    RCAR_PIN_22 = 0x16,           ///< GPIO Pin 22
    RCAR_PIN_23 = 0x17,           ///< GPIO Pin 23
    RCAR_PIN_24 = 0x18,           ///< GPIO Pin 24
    RCAR_PIN_25 = 0x19,           ///< GPIO Pin 25
    RCAR_PIN_26 = 0x1A,           ///< GPIO Pin 26
    RCAR_PIN_27 = 0x1B,           ///< GPIO Pin 27
    RCAR_PIN_28 = 0x1C,           ///< GPIO Pin 28
    RCAR_PIN_29 = 0x1D,           ///< GPIO Pin 29
    RCAR_PIN_30 = 0x1E,           ///< GPIO Pin 30
    RCAR_PIN_31 = 0x1F,           ///< GPIO Pin 31
} rcar_pin_t;

/** Interrupt Input Mode*/
typedef enum e_rcar_interrupt_input
{
    RCAR_INTERRUPT_INPUT_RISING_EDGE = 10,  ///< Rising Edge
    RCAR_INTERRUPT_INPUT_FALLING_EDGE,      ///< Falling Edge
    RCAR_INTERRUPT_INPUT_BOTH_EDGE          ///< Both Edge
} rcar_interrupt_input_t;

/** Direction of individual pins */
typedef enum e_rcar_io_dir
{
    RCAR_IO_DIRECTION_INPUT = 0,        ///< Input
    RCAR_IO_DIRECTION_OUTPUT            ///< Output
} rcar_io_direction_t;

/** Data for GPIO handler: group and pin */
typedef struct st_rcar_gpio_data
{
    rcar_gpio_group_t group;        ///< GPIO group 
    rcar_pin_t          pin;        ///< Pin identifier
} rcar_gpio_data_t;

/** GPIO request pin function control */
typedef enum e_rcar_io_req_pfc_functions
{
    RCAR_IO_REQ_PFC_PULL_DOWN = 0,         ///< Pull down
    RCAR_IO_REQ_PFC_PULL_UP,               ///< Pull up
    RCAR_IO_REQ_PFC_NO_PULL                ///< No pull
} rcar_req_pfc_functions_t;

/**
 * Sets a pin's output either high or low.
 *
 * @param[in] grp               - GPIO group, see @ref rcar_gpio_group_t
 * @param[in] pin               - GPIO pin, see @ref rcar_pin_t
 * @param[in] lvl               - GPIO pin level (1 is high, 0 is low)
 *
 * @retval 0 if successful
 *
 */
int R_GPIO_PinWriteOutput(rcar_gpio_group_t grp, rcar_pin_t pin, bool lvl);

/**
 * Writes to multiple pins on a GPIO group.
 *
 * @param[in] grp               - GPIO group, see @ref rcar_gpio_group_t
 * @param[in] group_level       - GPIO group level. The output value will be written to the specified group.
                                  Each bit in the value parameter corresponds to a bit on the group.
 * @param[in] mask_pins         - Mask pins to be used in the group (1 is used, 0 is not used)
                                  Each bit in the value parameter corresponds to a pin on the group.
 *
 * @retval 0 if successful
 *
 */
int R_GPIO_GroupWriteOutput(rcar_gpio_group_t grp, uint32_t group_level,
                            uint32_t mask_pins);

/**
 * Reads the level on a pin.
 *
 * @param[in] grp               - GPIO group, see @ref rcar_gpio_group_t
 * @param[in] pin               - GPIO pin, see @ref rcar_pin_t
 *
 * @retval 0 if successful
 *
 */
bool R_GPIO_PinReadInput(rcar_gpio_group_t grp, rcar_pin_t pin);

/**
 * Reads the value on an GPIO group.
 *
 * @param[in] grp               - GPIO group, see @ref rcar_gpio_group_t
 *
 * @retval 0 if successful
 *
 */
uint32_t R_GPIO_GroupRead(rcar_gpio_group_t grp);

/**
 * Configures the settings of a pin.
 *
 * @param[in] grp               - GPIO group, see @ref rcar_gpio_group_t
 * @param[in] pin               - GPIO pin, see @ref rcar_pin_t
 * @param[in] option            - GPIO modes, see @ref rcar_io_direction_t 
 *
 * @retval 0 if successful
 *
 */
int R_GPIO_PinConfigMode(rcar_gpio_group_t grp, rcar_pin_t pin,
                         rcar_io_direction_t option);

/**
 * Configures the functions of multiple pins.
 *
 * @param[in] grp               - GPIO group, see @ref rcar_gpio_group_t
 * @param[in] mask_directions   - GPIO group directions. The value will be written to the specified group.
                                  Each bit in the value parameter corresponds to a bit on the group.
 * @param[in] mask_pins         - Mask pins to be used in the group (1 is used, 0 is not used)
                                  Each bit in the value parameter corresponds to a pin on the group.
 *
 * @retval 0 if successful
 *
 */
int R_GPIO_GroupConfigMode(rcar_gpio_group_t grp, uint32_t mask_directions,
                           uint32_t mask_pins);

/**
 * Sets a pin to Input Interrupt Mode.
 *
 * @param[in] grp               - GPIO group, see @ref rcar_gpio_group_t
 * @param[in] pin               - GPIO pin, see @ref rcar_pin_t
 * @param[in] option            - Input Interrupt modes, see @ref rcar_interrupt_input_t
 *
 * @retval 0 if successful
 *
 */
int R_GPIO_PinConfigInterruptMode(rcar_gpio_group_t grp, rcar_pin_t pin,
                                 rcar_interrupt_input_t option);

/**
 * GPIO pin requests pin funtion control.
 *
 * @param[in] grp               - GPIO group, see @ref rcar_gpio_group_t
 * @param[in] pin               - GPIO pin, see @ref rcar_pin_t
 * @param[in] option            - Pin function, see @ref rcar_pfc_functions_t
 *
 * @retval 0 if successful
 *
 */
int R_GPIO_PinRequestPinFunction(rcar_gpio_group_t grp, rcar_pin_t pin,
                                rcar_req_pfc_functions_t option);

/**
 * Clear pin's flag input interrupt 
 *
 * @param[in] grp               - GPIO group, see @ref rcar_gpio_group_t
 * @param[in] pin               - GPIO pin, see @ref rcar_pin_t
 *
 * @retval 0 if successful
 *
 */
void R_GPIO_ClearInterrupt(rcar_gpio_group_t grp, rcar_pin_t pin);

/**
 * Set GPIO callback.
 *
 * @param[in] grp               - GPIO group, see @ref rcar_gpio_group_t
 * @param[in] handler           - GPIO handle function
 * @param[in] ctx               - GPIO handle function's data
 *
 * @retval 0 if successful
 *
 */
int R_GPIO_SetInterruptCallback(rcar_gpio_group_t grp, IrqHandlerFn handler, void *ctx);

#ifdef __cplusplus
}
#endif

#endif /* R_GPIO_API_H */
