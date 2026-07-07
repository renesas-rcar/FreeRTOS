/*
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "stdio.h"
#include "pfc/r_pfc_api.h"
#include "r_gpio_private.h"
#include "r_gpio_api.h"

#define PINS_EACH_GROUP 32

#define GPIO_BASE_ADDR_ERR           0xABCD

#define BIT(nr)             (1UL << (nr))

/* GPIO Bus Domain:
 * By default: use Bus Domain 0.
 */
#define BUS_DOMAIN_0            0x0
#define BUS_DOMAIN_1            0x2000
#define BUS_DOMAIN_2            0x4000
#define BUS_DOMAIN_3            0x6000

/* GPIO Registers Area:
 * By default: use R/W area.
 */
#define RW_AREA             0x0
#define SET_AREA            0x200
#define CLR_AREA            0x400

/* GPIO register: offset address */
#define GP_IOINTSEL         0x010
#define GP_INOUTSEL         0x014
#define GP_OUTDT            0x018
#define GP_OUTDTSEL         0x01C
#define GP_OUTDTH           0x020
#define GP_OUTDTL           0x024
#define GP_INEN             0x028
#define GP_INDT             0x02C
#define GP_INTDT            0x090
#define GP_INTCLR           0x094
#define GP_INTMSK           0x098
#define GP_MSKCLR           0x09C
#define GP_POSNEG           0x0A0
#define GP_EDGLEVEL         0x0A4
#define GP_FILONOFF         0x0A8
#define GP_FILCLKSEL        0x0AC
#define GP_BOTHEDGE         0x0CC

static void writel(const uint32_t value, const uintptr_t address);

static uint32_t readl(const uintptr_t Address);

static void setbit_l(uint32_t addr, uint32_t pos);

static uint32_t getbit_l(uint32_t addr, uint32_t pos);

static void clearbit_l(uint32_t addr, uint32_t pos);

static uint32_t getGpioRegister(rcar_gpio_group_t grp, uint32_t offset);

static void gpioSetGeneralOutputMode(rcar_gpio_group_t grp, rcar_pin_t pin);

static void gpioSetGeneralInputMode(rcar_gpio_group_t grp, rcar_pin_t pin);

int R_GPIO_PinWriteOutput(rcar_gpio_group_t grp, rcar_pin_t pin, bool lvl)
{
    uint32_t reg_addr;

    reg_addr = getGpioRegister(grp, GP_OUTDT);
    if (lvl) {
        setbit_l(reg_addr, pin);
    } else {
        clearbit_l(reg_addr, pin);
    }

    return 0;
}

int R_GPIO_GroupWriteOutput(rcar_gpio_group_t grp, uint32_t group_level,
               uint32_t mask_pins)
{
    uint32_t pin_num, mask_pos;
    bool pin_level;

    for (pin_num = 0U; pin_num < PINS_EACH_GROUP; pin_num++) {
        mask_pos = 1U << pin_num;
        if (mask_pins & mask_pos) {
            pin_level = (group_level & mask_pos) >> pin_num;
            R_GPIO_PinWriteOutput(grp, pin_num, pin_level);
        }
    }

    return 0;
}

bool R_GPIO_PinReadInput(rcar_gpio_group_t grp, rcar_pin_t pin)
{
    uint32_t bit = BIT(pin);
    bool pin_val;

    if (readl(getGpioRegister(grp, GP_INOUTSEL)) & bit) {
            pin_val = !!(readl(getGpioRegister(grp, GP_OUTDT)) & bit);
    } else {
            pin_val = !!(readl(getGpioRegister(grp, GP_INDT)) & bit);
    }

    return pin_val;
}

uint32_t R_GPIO_GroupRead(rcar_gpio_group_t grp)
{
    uint32_t pin_val;

    pin_val = readl(getGpioRegister(grp, GP_OUTDT));
    pin_val |= readl(getGpioRegister(grp, GP_INDT));

    return pin_val;
}

int R_GPIO_PinConfigMode(rcar_gpio_group_t grp, rcar_pin_t pin,
              rcar_io_direction_t option)
{
    switch (option) {
    case RCAR_IO_DIRECTION_OUTPUT:
        gpioSetGeneralOutputMode(grp, pin);
        break;
    case RCAR_IO_DIRECTION_INPUT:
        gpioSetGeneralInputMode(grp, pin);
        break;
    default:
        return -1;
    }

    return 0;
}

int R_GPIO_PinRequestPinFunction(rcar_gpio_group_t grp, rcar_pin_t pin,
                                rcar_req_pfc_functions_t option)
{
    switch (option) {
    case RCAR_IO_REQ_PFC_PULL_DOWN:
        pfcSetPullDown(grp, pin);
        break;
    case RCAR_IO_REQ_PFC_PULL_UP:
        pfcSetPullUp(grp, pin);
        break;
    case RCAR_IO_REQ_PFC_NO_PULL:
        pfcSetNoPull(grp, pin);
        break;
    default:
        return -1;
    }

    return 0;
}

int R_GPIO_GroupConfigMode(rcar_gpio_group_t grp, uint32_t mask_directions,
               uint32_t mask_pins)
{
    uint32_t pin_num, mask_pos;
    rcar_io_direction_t pin_option;

    for (pin_num = 0U; pin_num < PINS_EACH_GROUP; pin_num++) {
        mask_pos = 1U << pin_num;
        if (mask_pins & mask_pos) {
            pin_option = (mask_directions & mask_pos) >> pin_num;
            (void) R_GPIO_PinConfigMode(grp, pin_num, pin_option);
        }
    }

    return 0;
}

int R_GPIO_PinConfigInterruptMode(rcar_gpio_group_t grp, rcar_pin_t pin,
                                 rcar_interrupt_input_t trigger_mode)
{
    /* Set Peripheral Function to GPIO */
    (void)pfcSetGPIO(grp, pin);

    /* (1) Set the positive or negative logic as the interrupt
     *     input condition in POSNEG.
     */
    if (trigger_mode == RCAR_INTERRUPT_INPUT_RISING_EDGE) {
        clearbit_l(getGpioRegister(grp, GP_POSNEG), pin);
    } else {
        setbit_l(getGpioRegister(grp, GP_POSNEG), pin);
    }
    
    /* (2) Set the edge (set to 1) as the interrupt input
     *     condition in EDGLEVEL.
     */
    setbit_l(getGpioRegister(grp, GP_EDGLEVEL), pin);

    /* (3) Set the one edge/both edge as the interrupt input
     *     condition in BOTHEDGE.
     */
    if (trigger_mode == RCAR_INTERRUPT_INPUT_BOTH_EDGE) {
        setbit_l(getGpioRegister(grp, GP_BOTHEDGE), pin);
    } else {
        clearbit_l(getGpioRegister(grp, GP_BOTHEDGE), pin);
    }

    /* Select "Input Enable" in INEN */
    setbit_l(getGpioRegister(grp, GP_INEN), pin);

    /* (4) Set interrupt input mode in IOINTSEL. */
    setbit_l(getGpioRegister(grp, GP_IOINTSEL), pin);

    /* (5) Write 1 to INTCLR to clear (initialize) the interrupt
     *     display bit in edge-sensitive interrupt mode.
     */
    setbit_l(getGpioRegister(grp, GP_INTCLR), pin);

    /* (6) Write 1 to MSKCLR (or MSKCLRS depending on
     *     interrupt or alternative interrupt is used) cancel the
     *     interrupt mask.
     */
    setbit_l(getGpioRegister(grp, GP_MSKCLR), pin);

    return 0;
}

void R_GPIO_ClearInterrupt(rcar_gpio_group_t grp, rcar_pin_t pin)
{
    /*  Clear interrupt status flag */
    setbit_l(getGpioRegister(grp, GP_INTCLR), pin);
}

int R_GPIO_SetInterruptCallback(rcar_gpio_group_t grp, IrqHandlerFn handler, void *ctx)
{
    uint32_t int_id;

    switch (grp) {
    case 0:
        int_id = INTID_GPIO_GRP0;
        break;
    case 1:
        int_id = INTID_GPIO_GRP1;
        break;
    case 2:
        int_id = INTID_GPIO_GRP2;
        break;
    case 3:
        int_id = INTID_GPIO_GRP3;
        break;
    case 4:
        int_id = INTID_GPIO_GRP4;
        break;
    case 5:
        int_id = INTID_GPIO_GRP5;
        break;
    case 6:
        int_id = INTID_GPIO_GRP6;
        break;
    case 7:
        int_id = INTID_GPIO_GRP7;
        break;
    case 8:
        int_id = INTID_GPIO_GRP8;
        break;
    case 9:
        int_id = INTID_GPIO_GRP9;
        break;
    case 10:
        int_id = INTID_GPIO_GRP10;
        break;
    default:
        int_id = INTID_GPIO_NO_EXIST;
        goto setup_irq_fail;
    }

    /* Set Handler for Irq */
    Irq_SetupEntry(int_id, handler, ctx);

    /* Set priority for Irq */
    Irq_SetPriority(int_id, IPRIORITY(3));
    
    /* Enable Irq */
    Irq_Enable(int_id);

    return 0;

setup_irq_fail:
    printf("IRQ FAILED: group no exist!\n");
    return -1;
}

static void writel(const uint32_t value, const uintptr_t address)
{
    *((volatile unsigned int*) address)  = value;
}

static uint32_t readl(const uintptr_t address)
{
    return *((volatile unsigned int*)address);
}

static void setbit_l(uint32_t addr, uint32_t pos)
{
    uint32_t val = readl(addr);

    writel(val |= BIT(pos), addr);
}

static uint32_t getbit_l(uint32_t addr, uint32_t pos)
{
    return !!(readl(addr) & BIT(pos));
}

static void clearbit_l(uint32_t addr, uint32_t pos)
{
    uint32_t val = readl(addr);

    writel(val &= ~BIT(pos), addr);
}


static uint32_t getGpioRegister(rcar_gpio_group_t grp, uint32_t offset)
{
    if ((uint32_t)grp >= (sizeof(gpio_gr_base) / sizeof(gpio_gr_base[0])))
    {
        (void)printf("GPIO group %d not exist!\n", grp);
        return GPIO_BASE_ADDR_ERR;
    }

    return gpio_gr_base[grp] + offset;
}


static void gpioSetGeneralOutputMode(rcar_gpio_group_t grp, rcar_pin_t pin)
{
    uint8_t lvl = 0;

    /* Set Peripheral Function to GPIO */
    (void)pfcSetGPIO(grp, pin);

    /* (1) Set the initial values of the output ports in OUTDT.  
     *     Set the positive or negative logic in POSNEG.
     */
    R_GPIO_PinWriteOutput(grp, pin, lvl);
    clearbit_l(getGpioRegister(grp, GP_POSNEG), pin);

    /* Select "Input Disable" in INEN */
    clearbit_l(getGpioRegister(grp, GP_INEN), pin);

    /* (2) Set general input/output mode in IOINTSEL. */
    clearbit_l(getGpioRegister(grp, GP_IOINTSEL), pin);

    /* (3) Set general output mode in INOUTSEL, which allows
     * the ports to output the signal of the specified level.
     */
    setbit_l(getGpioRegister(grp, GP_INOUTSEL), pin);

    /* Output mode by configuring OUTDTSEL register to change output by OUTDT */
    clearbit_l(getGpioRegister(grp, GP_OUTDTSEL), pin);
}

static void gpioSetGeneralInputMode(rcar_gpio_group_t grp, rcar_pin_t pin)
{
    /* Set Peripheral Function to GPIO */
    (void)pfcSetGPIO(grp, pin);

    /* (1) Set the positive (not inverted) or negative (inverted)
     *     logic for processing the input signals in POSNEG.
     */
    clearbit_l(getGpioRegister(grp, GP_POSNEG), pin);

    /* Select "Input Enable" in INEN */
    setbit_l(getGpioRegister(grp, GP_INEN), pin);

    /* (2) Set general input/output mode in IOINTSEL. */
    clearbit_l(getGpioRegister(grp, GP_IOINTSEL), pin);

    /* (3) Set general input mode in INOUTSEL. */
    clearbit_l(getGpioRegister(grp, GP_INOUTSEL), pin);
}
