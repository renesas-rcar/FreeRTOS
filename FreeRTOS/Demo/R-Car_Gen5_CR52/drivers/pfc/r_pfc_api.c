/*
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "stdio.h"
#include "board.h"
#include "pfc/r_pfc_api.h"
#include "r_pfc_private.h"

#include "device_tree.h"

#define LIBRARY_LOG_LEVEL 0

/* Logging Function include. */
#include "logging_stack.h"

#define BIT(nr)             (1UL << (nr))

#define PFC_INVALID_ADDR           0x0

/* PFC register: offset address */
#define GP_PMMR             0x000
#define GP_PMMER            0x004
#define GP_PSER             0x008
#define GP_PSSR0            0x00C
#define GP_PSSR1            0x010
#define GP_DMPR0            0x020
#define GP_DMPR1            0x024
#define GP_DMPR2            0x028
#define GP_DMPR3            0x02C
#define GP_GPSR             0x040
#define GP_ALTSEL(id)       (0x060 + ((uint32_t)(id) * 0x004))
#define GP_DRVCTRL(id)      (0x080 + ((uint32_t)(id) * 0x004))
#define GP_TDSEL(id)        (0x094 + ((uint32_t)(id) * 0x004))
#define GP_PULLEN           0x0C0
#define GP_PUDSEL           0x0C4
#define GP_MODSEL           0x100

#define NUM_GP_ALTSEL       4
#define NUM_GP_DRVCTRL      3
#define NUM_GP_TDSEL        2

#define FIELD_MASK(bit_start, width)    \
(((1U << (width)) - 1) << (bit_start))

#define REG_TYPE_MASK       FIELD_MASK(REG_TYPE_POS, 4)
#define GROUP_MASK          FIELD_MASK(GROUP_START_POS, (REG_TYPE_POS - GROUP_START_POS))
#define PIN_MASK            FIELD_MASK(PIN_START_POS, (GROUP_START_POS - PIN_START_POS))
#define FUNC_MASK           FIELD_MASK(FUNC_START_POS, (PIN_START_POS - FUNC_START_POS))

/* Drive capability ids */
typedef enum e_drive_strength
{
    /* Sentinel */
    /* Do not add anything before */
    /* Drive capability:        GPn_DRVCTRL0[p]  | GPn_DRVCTRL1[p]   | GPn_DRVCTRL2[p] */
    INVALID_DRIVE_STRENGTH_18   = 0x0,

    DRIVE_STRENGTH_28           = 0x1,
    DRIVE_STRENGTH_38           = 0x2,
    DRIVE_STRENGTH_48           = 0x3,
    DRIVE_STRENGTH_58           = 0x4,
    DRIVE_STRENGTH_68           = 0x5,
    DRIVE_STRENGTH_78           = 0x6,
    DRIVE_STRENGTH_FULL         = 0x7,

    /* Sentinel */
    INVALID_DRIVE_STRENGTH_LAST,
    /* Do not add anything below */
} drive_strength_t;

/* Delay adjustment of the SDHI ids */
typedef enum e_tdsel_control
{
    /* Sentinel */
    /* Do not add anything before */
    /* Tartget delay:   GPn_TDSEL0[p]  | GPn_TDSEL1[p] */
    DELAY_10_PF         = 0x0,
    DELAY_20_PF         = 0x1,
    DELAY_30_PF         = 0x2,
    DELAY_40_PF         = 0x3,

    /* Sentinel */
    INVALID_DELAY,
    /* Do not add anything below */
} tdsel_control_t;

/******************* Define pin functions *******************/

/** Pin function mode */
enum e_rcar_pfc_mode {
    PFC_PERIPHERAL = 0,
    PFC_GPIO,
    PFC_ENABLE_PULL,
    PFC_DISABLE_PULL
};

static uint32_t getPfcRegister(rcar_pfc_group_t grp, uint32_t offset);

static void pfcWrite(rcar_pfc_group_t grp, uint32_t addr, uint32_t val);

static void pfcSetGPSR(uint8_t mode, rcar_pfc_group_t grp, rcar_pfc_pin_t pin);

static void writel(const uint32_t value, const uintptr_t address);

static uint32_t readl(const uintptr_t Address);

static void setbit_l(uint32_t addr, uint32_t pos);

static uint32_t getbit_l(uint32_t addr, uint32_t pos);

static void clearbit_l(uint32_t addr, uint32_t pos);

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
    writel(readl(addr) | BIT(pos), addr);
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

static uint32_t getPfcRegister(rcar_pfc_group_t grp, uint32_t offset)
{

    if ((uint32_t)grp >= (sizeof(pfc_gr_base) / sizeof(pfc_gr_base[0])))
    {
        printf("PFC group %d not exist!\n", grp);
        return PFC_INVALID_ADDR;
    }

    return pfc_gr_base[grp] + offset;
}

static void pfcWrite(rcar_pfc_group_t grp, uint32_t addr, uint32_t val)
{
    writel(~val, getPfcRegister(grp, GP_PMMR));
    writel(val, addr);
}

static uint32_t bitfield_extract(uint32_t value, uint8_t offset, uint8_t width)
{
    if (width == 0 || width > 32)
    {
        printf("%s: Invalid width %d\n", __func__, width);
        return 0;
    }

    if ((offset + width) > 32)
    {
        printf("%s: Invalid offset + width %d\n", __func__, (offset + width));
        return 0;
    }

    return (value >> offset) & ((1U << width) - 1);
}

static void pfcSetGPSR(uint8_t gpio, rcar_pfc_group_t grp, rcar_pfc_pin_t pin)
{
    uint32_t val, reg_addr;

    reg_addr = getPfcRegister(grp, GP_GPSR);
    val = readl(reg_addr);
    val = gpio ? val & ~BIT(pin) : val | BIT(pin);
    pfcWrite(grp, reg_addr, val);
}

int pfcSetGPIO(rcar_pfc_group_t grp, rcar_pfc_pin_t pin)
{
    pfcSetGPSR(PFC_GPIO, grp, pin);
    return 0;
}

static int pfcSetPeripheral(rcar_pfc_group_t grp, rcar_pfc_pin_t pin)
{
    pfcSetGPSR(PFC_PERIPHERAL, grp, pin);
    return 0;
}

static int pfcSetFunction(rcar_pfc_group_t grp, rcar_pfc_pin_t pin,
                         rcar_pfc_func_id_t f_id)
{
    uint32_t reg_addr, reg_val;
    uint32_t bit_val, bit_pos = pin;
    uint8_t i;

    if (grp < RCAR_PFC_GROUP_00 ||
        grp > (RCAR_PFC_GROUP_MAX - 1))
    {
        printf("%s: Invalid group %d\n", __func__, pin);
        return -1;
    }

    if (pin < RCAR_PFC_PIN_00 ||
        pin > RCAR_PFC_PIN_31)
    {
        printf("%s: Invalid pin %d\n", __func__, pin);
        return -1;
    }

    if (f_id >= INVALID_RCAR_PFC_FUNC)
    {
        printf("%s: Invalid function id %d\n", __func__, f_id);
        return -1;
    }

    for (i = 0; i < NUM_GP_ALTSEL; ++i)
    {
        reg_addr   = getPfcRegister(grp, GP_ALTSEL(i));
        reg_val    = readl(reg_addr);
        bit_val    = bitfield_extract(f_id, i, 1);

        reg_val &= ~(1U << bit_pos);
        reg_val |= (bit_val << bit_pos);
        LogDebug(("BF: reg <0x%x>: 0x%x", reg_addr, readl(reg_addr)));
        pfcWrite(grp, reg_addr, reg_val);
        LogDebug(("bit_pos: %d, bit_val: %d", bit_pos, bit_val));
        LogDebug(("AF: reg <0x%x>: 0x%x\n", reg_addr, readl(reg_addr)));
    }

    return 0;
}

static int pfcInitPeripheralFunction(rcar_pfc_group_t grp, rcar_pfc_pin_t pin,
                                    rcar_pfc_func_id_t f_id)
{
    int ret = 0;

    ret  = pfcSetPeripheral(grp, pin);
    ret |= pfcSetFunction(grp, pin, f_id);
    return ret;
}

static int pfcSetModeSel(rcar_pfc_group_t grp, rcar_pfc_pin_t pin,
                        modsel_func_t modsel_cfg)
{
    uint32_t reg_addr, reg_val;
    uint32_t bit_val, bit_pos;
    int ret = 0;

    if (grp < RCAR_PFC_GROUP_00 ||
        grp > (RCAR_PFC_GROUP_MAX - 1))
    {
        printf("%s: Invalid group %d\n", __func__, pin);
        return -1;
    }

    if (pin < RCAR_PFC_PIN_00 ||
        pin > RCAR_PFC_PIN_31)
    {
        printf("%s: Invalid pin %d\n", __func__, pin);
        return -1;
    }

    if (modsel_cfg >= INVALID_MODSEL)
    {
        printf("%s: Invalid modsel %d\n", __func__, modsel_cfg);
        return -1;
    }

    reg_addr = getPfcRegister(grp, GP_MODSEL);
    reg_val  = readl(reg_addr);
    bit_val  = modsel_cfg;
    bit_pos  = pin;

    reg_val &= ~(1U << bit_pos);
    reg_val |= (bit_val << bit_pos);
    LogDebug(("BF: reg <0x%x>: 0x%x", reg_addr, readl(reg_addr)));
    pfcWrite(grp, reg_addr, reg_val);
    LogDebug(("bit_pos: %d, bit_val: %d", bit_pos, bit_val));
    LogDebug(("AF: reg <0x%x>: 0x%x\n", reg_addr, readl(reg_addr)));

    return ret;
}

static int pfcSetDrvControlReg(rcar_pfc_group_t grp, rcar_pfc_pin_t pin,
                              drive_strength_t  drv_str_cfg)
{
    uint32_t reg_addr, reg_val;
    uint32_t bit_val, bit_pos = pin;
    uint8_t i;

    if (grp < RCAR_PFC_GROUP_00 ||
        grp > (RCAR_PFC_GROUP_MAX - 1))
    {
        printf("%s: Invalid group %d\n", __func__, pin);
        return -1;
    }

    if (pin < RCAR_PFC_PIN_00 ||
        pin > RCAR_PFC_PIN_31)
    {
        printf("%s: Invalid pin %d\n", __func__, pin);
        return -1;
    }

    if (drv_str_cfg <= INVALID_DRIVE_STRENGTH_18 ||
        drv_str_cfg >= INVALID_DRIVE_STRENGTH_LAST)
    {
        printf("%s: Invalid drive stregth!\n", __func__);
        return -1;
    }

    for (i = 0; i < NUM_GP_DRVCTRL; ++i)
    {
        reg_addr   = getPfcRegister(grp, GP_DRVCTRL(i));
        reg_val    = readl(reg_addr);
        bit_val    = bitfield_extract(drv_str_cfg, i, 1);

        reg_val &= ~(1U << bit_pos);
        reg_val |= (bit_val << bit_pos);
        LogDebug(("BF: reg <0x%x>: 0x%x", reg_addr, readl(reg_addr)));
        pfcWrite(grp, reg_addr, reg_val);
        LogDebug(("bit_pos: %d, bit_val: %d", bit_pos, bit_val));
        LogDebug(("AF: reg <0x%x>: 0x%x\n", reg_addr, readl(reg_addr)));

    }

    return 0;
}

static int pfcSetTdselControlReg(rcar_pfc_group_t grp, rcar_pfc_pin_t pin,
                                tdsel_control_t tdsel_cfg)
{
    uint32_t reg_addr, reg_val;
    uint32_t bit_val, bit_pos = pin;
    uint8_t i;

    if (grp < RCAR_PFC_GROUP_00 ||
        grp > (RCAR_PFC_GROUP_MAX - 1))
    {
        printf("%s: Invalid group %d\n", __func__, pin);
        return -1;
    }

    if (pin < RCAR_PFC_PIN_00 ||
        pin > RCAR_PFC_PIN_31)
    {
        printf("%s: Invalid pin %d\n", __func__, pin);
        return -1;
    }

    if (tdsel_cfg >= INVALID_DELAY) 
    {
        printf("%s: Invalid TDSEL config!\n", __func__);
        return -1;
    }

    for (i = 0; i < NUM_GP_TDSEL; ++i)
    {
        reg_addr   = getPfcRegister(grp, GP_TDSEL(i));
        reg_val    = readl(reg_addr);
        bit_val    = bitfield_extract(tdsel_cfg, i, 1);

        reg_val &= ~(1U << bit_pos);
        reg_val |= (bit_val << bit_pos);
        LogDebug(("BF: reg <0x%x>: 0x%x", reg_addr, readl(reg_addr)));
        pfcWrite(grp, reg_addr, reg_val);
        LogDebug(("bit_pos: %d, bit_val: %d", bit_pos, bit_val));
        LogDebug(("AF: reg <0x%x>: 0x%x\n", reg_addr, readl(reg_addr)));

    }

    return 0;
}

static const int* findGroupByModule(st_module_config_t module) {
    for (int indx = 0; indx < sizeof(all_drv_groups)/sizeof(all_drv_groups[0]);indx++) {
        if (all_drv_groups[indx].module_id == module.module_id) {
            return all_drv_groups[indx].group;
        }
    }
    return NULL;
}
int pfcInitModule(st_module_config_t module)
{
    int ret = 0;
    uint8_t indx, grp, pin, fid;
    reg_pfc_t reg;
    const int * p_drv_grp = findGroupByModule(module);

    if (p_drv_grp == NULL) {
        printf("%s: no group found for module_id=%d\n", __func__, module.module_id);
        return -1;
    }

    for (indx = 0;;indx++)
    {
        if (p_drv_grp[indx] == -1) {
            break;
        }

        reg = (p_drv_grp[indx] & REG_TYPE_MASK) >>  (REG_TYPE_POS);
        grp = (p_drv_grp[indx] & GROUP_MASK)    >>  (GROUP_START_POS);
        pin = (p_drv_grp[indx] & PIN_MASK)      >>  (PIN_START_POS);
        fid = (p_drv_grp[indx] & FUNC_MASK)     >>  (FUNC_START_POS);

        LogDebug(("=== INFO ==="));
        LogDebug(("reg: %d", reg));
        LogDebug(("grp: %d", grp));
        LogDebug(("pin: %d", pin));
        LogDebug(("fid: %d", fid));

        switch(reg) {
        case REG_ALTSEL:
            ret = pfcInitPeripheralFunction(grp, pin, (rcar_pfc_func_id_t)fid);
            break;
        case REG_DRVCTRL:
            ret = pfcSetDrvControlReg(grp, pin, (drive_strength_t)fid);
            break;
        case REG_TDSEL:
            ret = pfcSetTdselControlReg(grp, pin, (tdsel_control_t)fid);
            break;
        case REG_MODSEL:
            ret = pfcSetModeSel(grp, pin, (modsel_func_t)fid);
            break;
        default:
            printf("%s: Invalid register!");
            return -1;
        }

        if (ret != 0) {
            printf("%s: FAILED\n", __func__);
            return -1;
        }
    }

    return 0;
}

int pfcInitModules(st_module_config_t* module_list)
{
#if (BOARD == X5H_RFS2 || BOARD == MDP_AIACC_RFS2)
    ( void ) module_list;
#else	// (BOARD == X5H_VDK || BOARD == MDP_AIACC_HIL || BOARD == X5H_IRONHIDE || BOARD == MDP_X5H_HIL)
    int module_indx = 0, ret = 0;
    st_module_config_t module;

    for (module_indx = 0;; module_indx++)
    {
        module = module_list[module_indx];
        if (module.module_id == MODULE_INVALID) {
            break;
        }
        if (module.is_enabled == 0) {
            continue;
        }

        ret |= pfcInitModule(module);
        if (ret != 0)
        {
            printf("%s: module index %d FAILED\n", __func__, module_indx);
            return -1;
        }
    }
#endif

    return 0;
}

static void pfcPullMode(uint8_t enable, rcar_pfc_group_t grp, rcar_pfc_pin_t pin)
{
    uint32_t val, reg_addr;

    reg_addr = getPfcRegister(grp, GP_PULLEN);
    val = readl(reg_addr);
    if (enable == PFC_ENABLE_PULL) {
        val |= BIT(pin);
    }
    if (enable == PFC_DISABLE_PULL) {
        val &= ~BIT(pin);
    }
    pfcWrite(grp, reg_addr, val);
}

static void pfcSetPullType(uint8_t option, rcar_pfc_group_t grp, rcar_pfc_pin_t pin)
{
    uint32_t val, reg_addr;

    reg_addr = getPfcRegister(grp, GP_PUDSEL);
    val = readl(reg_addr);
    if (option == RCAR_PFC_PULL_UP) {
        val |= BIT(pin);
    }
    if (option == RCAR_PFC_PULL_DOWN) {
        val &= ~BIT(pin);
    }
    pfcWrite(grp, reg_addr, val);
}

int pfcSetPullDown(rcar_pfc_group_t grp, rcar_pfc_pin_t pin)
{
    pfcSetPullType(RCAR_PFC_PULL_DOWN, grp, pin);
    pfcPullMode(PFC_ENABLE_PULL, grp, pin);
    return 0;
}

int pfcSetPullUp(rcar_pfc_group_t grp, rcar_pfc_pin_t pin)
{
    pfcSetPullType(RCAR_PFC_PULL_UP, grp, pin);
    pfcPullMode(PFC_ENABLE_PULL, grp, pin);
    return 0;
}

int pfcSetNoPull(rcar_pfc_group_t grp, rcar_pfc_pin_t pin)
{
    pfcPullMode(PFC_DISABLE_PULL, grp, pin);
    return 0;
}
