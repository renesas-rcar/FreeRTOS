/*
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "gpio/r_gpio.h"
#include "r_gpio_api.h"

#define GPIO_OPEN                (0x00000001U)
#define GPIO_CLOSED              (0x00000000U)

static void r_gpio_pins_config (const gpio_cfg_t * p_cfg);

static void r_gpio_pin_set(gpio_port_pin_t pin, uint32_t cfg);

static int r_gpio_isr_handler (void * const p_context);

int R_GPIO_Open(gpio_ctrl_t * const p_ctrl, const gpio_cfg_t * p_cfg) {
    gpio_instance_ctrl_t * p_instance_ctrl = (gpio_instance_ctrl_t *) p_ctrl;

    /* Set driver status to open */
    p_instance_ctrl->open = GPIO_OPEN;

    p_instance_ctrl->p_cfg = p_cfg;

    r_gpio_pins_config(p_cfg);

    return 0;
}

int R_GPIO_Close(gpio_ctrl_t * const p_ctrl) {
    gpio_instance_ctrl_t * p_instance_ctrl = (gpio_instance_ctrl_t *) p_ctrl;

    /* Set state to closed */
    p_instance_ctrl->open = GPIO_CLOSED;
    return 0;
}

int R_GPIO_PinsCfg(gpio_ctrl_t * const p_ctrl, const gpio_cfg_t * p_cfg) {
    gpio_instance_ctrl_t * p_instance_ctrl = (gpio_instance_ctrl_t *) p_ctrl;
    r_gpio_pins_config(p_cfg);

}

int R_GPIO_PinCfg(gpio_ctrl_t * const p_ctrl, gpio_port_pin_t pin, uint32_t cfg) {
    gpio_instance_ctrl_t * p_instance_ctrl = (gpio_instance_ctrl_t *) p_ctrl;

    r_gpio_pin_set(pin, cfg);

    return 0;
}

int R_GPIO_PinInterruptInput(gpio_ctrl_t * const p_ctrl, gpio_port_pin_t pin, gpio_interrupt_input_t option) {
    /* Get port and pin number */
    uint32_t port_num = (GPIO_PRV_PORT_BITS & (uint32_t)pin) >> GPIO_PRV_PORT_OFFSET;
    uint32_t pin_num  = (GPIO_PRV_PIN_BITS & (uint32_t)pin);
    R_GPIO_PinConfigInterruptMode(port_num, pin_num, option);
    return 0;
}

int R_GPIO_CallbackSet(gpio_ctrl_t * const p_ctrl, void ( *p_callback)(void *), void * const p_context) {
    gpio_instance_ctrl_t * p_instance_ctrl = (gpio_instance_ctrl_t *) p_ctrl;

    /* Store callback and context */
    p_instance_ctrl->p_callback        = p_callback;
    p_instance_ctrl->p_context         = p_context;

    /* Get port and pin number */
    uint32_t port_pin = p_instance_ctrl->p_cfg->p_pin_cfg_data->pin;
    uint32_t port_num = (GPIO_PRV_PORT_BITS & port_pin) >> GPIO_PRV_PORT_OFFSET;

    R_GPIO_SetInterruptCallback(port_num, (void *)r_gpio_isr_handler, p_context);
}

int R_GPIO_PinRead(gpio_ctrl_t * const p_ctrl, gpio_port_pin_t pin, gpio_level_t * p_pin_value) {

    uint32_t port_num = (GPIO_PRV_PORT_BITS & (uint32_t)pin) >> GPIO_PRV_PORT_OFFSET;
    uint32_t pin_num  = (GPIO_PRV_PIN_BITS & (uint32_t)pin);
    *p_pin_value = R_GPIO_PinReadInput(port_num, pin_num);

    return 0;
}

int R_GPIO_PinWrite(gpio_ctrl_t * const p_ctrl, gpio_port_pin_t pin, gpio_level_t level) {
    /* Get port and pin number */
    uint32_t port_num = (GPIO_PRV_PORT_BITS & (uint32_t)pin) >> GPIO_PRV_PORT_OFFSET;
    uint32_t pin_num  = (GPIO_PRV_PIN_BITS & (uint32_t)pin);
    R_GPIO_PinWriteOutput(port_num, pin_num, level);
    return 0;
}

int R_GPIO_PortDirectionSet(gpio_ctrl_t * const p_ctrl,
                                    gpio_port_t         port,
                                    uint32_t            direction_values,
                                    uint32_t            mask) {
    R_GPIO_GroupConfigMode(port, direction_values, mask);
    return 0;

}

int R_GPIO_PortRead(gpio_ctrl_t * const p_ctrl, gpio_port_t port, uint32_t * p_port_value) {
    *p_port_value = R_GPIO_GroupRead(port);
    return 0;
}

int R_GPIO_PortWrite(gpio_ctrl_t * const p_ctrl, gpio_port_t port, uint32_t value, uint32_t mask) {
    R_GPIO_GroupWriteOutput(port, value, mask);
    return 0;
}

int R_GPIO_PinSetPull(gpio_ctrl_t * const p_ctrl, gpio_port_pin_t pin, gpio_request_pull_t option) {

    uint32_t port_num = (GPIO_PRV_PORT_BITS & (uint32_t)pin) >> GPIO_PRV_PORT_OFFSET;
    uint32_t pin_num  = (GPIO_PRV_PIN_BITS & (uint32_t)pin);
    (void)R_GPIO_PinRequestPinFunction(port_num, pin_num, option);
    return 0;
}

static void r_gpio_pin_set(gpio_port_pin_t pin, uint32_t cfg) {

    uint32_t port_num = (GPIO_PRV_PORT_BITS & (uint32_t)pin) >> GPIO_PRV_PORT_OFFSET;
    uint32_t pin_num  = (GPIO_PRV_PIN_BITS & (uint32_t)pin);
    if (cfg == GPIO_DIRECTION_OUTPUT || cfg == GPIO_DIRECTION_INPUT) {
        (void) R_GPIO_PinConfigMode(port_num, pin_num, cfg);
    } else if (cfg == GPIO_INTERRUPT_INPUT_RISING_EDGE ||
            cfg == GPIO_INTERRUPT_INPUT_FALLING_EDGE ||
            cfg == GPIO_INTERRUPT_INPUT_BOTH_EDGE)
    {
        (void) R_GPIO_PinConfigInterruptMode(port_num, pin_num, cfg);
    }
}

static void r_gpio_pins_config (const gpio_cfg_t * p_cfg) {
    uint16_t       pin_count;
    gpio_cfg_t * p_pin_data;

    p_pin_data = (gpio_cfg_t *) p_cfg;

    for (pin_count = 0U; pin_count < p_pin_data->number_of_pins; pin_count++)
    {
        r_gpio_pin_set(p_pin_data->p_pin_cfg_data[pin_count].pin, p_pin_data->p_pin_cfg_data[pin_count].pin_cfg);
    }
}

static int r_gpio_isr_handler (void * const p_context) {
    gpio_instance_ctrl_t * p_instance_ctrl = (gpio_instance_ctrl_t *) p_context;

    uint32_t group_pin = p_instance_ctrl->p_cfg->p_pin_cfg_data->pin;
    uint32_t port_num = (GPIO_PRV_PORT_BITS & (uint32_t)group_pin) >> GPIO_PRV_PORT_OFFSET;
    uint32_t pin_num  = (GPIO_PRV_PIN_BITS & (uint32_t)group_pin);

    if (p_instance_ctrl->p_callback != NULL) {
        p_instance_ctrl->p_callback(p_context);
    }
    R_GPIO_ClearInterrupt(port_num, pin_num);
    return 0;
}
