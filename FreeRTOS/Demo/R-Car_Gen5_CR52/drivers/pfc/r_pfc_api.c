/*
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "stdio.h"
#include "pfc/r_pfc_api.h"
#include "device_tree_x5h.h"
#include "board.h"

#define LIBRARY_LOG_LEVEL 0

/* Logging Function include. */
#include "logging_stack.h"

#define BIT(nr)             (1UL << (nr))

#define PFC_INVALID_ADDR           0x0

/* PFC base adrress */
#define PFC_BASE_OFFSET    0x000
#define PFC_GR_0           (0xC1080000U + PFC_BASE_OFFSET)
#define PFC_GR_1           (0xC1080800U + PFC_BASE_OFFSET)
#define PFC_GR_2           (0xC1081000U + PFC_BASE_OFFSET)
#define PFC_GR_3           (0xC0800000U + PFC_BASE_OFFSET)
#define PFC_GR_4           (0xC0800800U + PFC_BASE_OFFSET)
#define PFC_GR_5           (0xC0400000U + PFC_BASE_OFFSET)
#define PFC_GR_6           (0xC0400800U + PFC_BASE_OFFSET)
#define PFC_GR_7           (0xC0401000U + PFC_BASE_OFFSET)
#define PFC_GR_8           (0xC0401800U + PFC_BASE_OFFSET)
#define PFC_GR_9           (0xC9B00000U + PFC_BASE_OFFSET)
#define PFC_GR_10          (0xC9B00800U + PFC_BASE_OFFSET)

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
#define GP_ALTSEL(id)       (0x060 + (id*0x004))
#define GP_DRVCTRL(id)      (0x080 + (id*0x004))
#define GP_TDSEL(id)        (0x094 + (id*0x004))
#define GP_PULLEN           0x0C0
#define GP_PUDSEL           0x0C4
#define GP_MODSEL           0x100

#define NUM_GP_ALTSEL       4
#define NUM_GP_DRVCTRL      3
#define NUM_GP_TDSEL        2

/* Mask field for enum id: to store PFC information */
#define REG_TYPE_POS        20
#define GROUP_START_POS     12
#define PIN_START_POS       4
#define FUNC_START_POS      0

#define FIELD_MASK(bit_start, width)    \
(((1U << (width)) - 1) << (bit_start))

#define REG_TYPE_MASK       FIELD_MASK(REG_TYPE_POS, 4)
#define GROUP_MASK          FIELD_MASK(GROUP_START_POS, (REG_TYPE_POS - GROUP_START_POS))
#define PIN_MASK            FIELD_MASK(PIN_START_POS, (GROUP_START_POS - PIN_START_POS))
#define FUNC_MASK           FIELD_MASK(FUNC_START_POS, (PIN_START_POS - FUNC_START_POS))

/** Pin function ids */
typedef enum e_rcar_pfc_func_id
{
    RCAR_PFC_FUNC_0 = 0,            ///< Function 0
    RCAR_PFC_FUNC_1,                ///< Function 1
    RCAR_PFC_FUNC_2,                ///< Function 2
    RCAR_PFC_FUNC_3,                ///< Function 3
    RCAR_PFC_FUNC_4,                ///< Function 4
    RCAR_PFC_FUNC_5,                ///< Function 5
    RCAR_PFC_FUNC_6,                ///< Function 6
    RCAR_PFC_FUNC_7,                ///< Function 7
    RCAR_PFC_FUNC_8,                ///< Function 8
    RCAR_PFC_FUNC_9,                ///< Function 9
    RCAR_PFC_FUNC_10,               ///< Function 10
    RCAR_PFC_FUNC_11,               ///< Function 11
    RCAR_PFC_FUNC_12,               ///< Function 12
    RCAR_PFC_FUNC_13,               ///< Function 13
    RCAR_PFC_FUNC_14,               ///< Function 14
    RCAR_PFC_FUNC_15,               ///< Function 15

    /* Sentinel */
    INVALID_RCAR_PFC_FUNC,
    /* Do not add anything below */
} rcar_pfc_func_id_t;

/* Register PFC ids */
typedef enum e_reg_pfc
{
    REG_ALTSEL = 0,
    REG_DRVCTRL,
    REG_TDSEL,
    REG_MODSEL,

    /* Sentinel */
    INVALID_REG,
    /* Do not add anything below */
} reg_pfc_t;

/* Register PFC ids */
typedef enum e_modsel_func
{
    MODSEL_TAUJ_OUTPUT  = 0,
    MODSEL_TAUJ_INPUT   = 1,
    MODSEL_GPIO_MODE    = 0,
    MODSEL_I2C_MODE     = 1,

    /* Sentinel */
    INVALID_MODSEL,
    /* Do not add anything below */
} modsel_func_t;

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

#define GEN_ID(reg_pfc_t, grp, pin, fid)   \
((reg_pfc_t<<REG_TYPE_POS) + (grp<<GROUP_START_POS) + (pin<<PIN_START_POS) + (fid<<FUNC_START_POS))

/******************* Define pin functions *******************/
#define MAX_ITEM_IN_GROUP   16
#define CREATE_GROUP(name, ...) \
static int name[MAX_ITEM_IN_GROUP] = {__VA_ARGS__, -1};

#define ADD_GROUP(name)     name

#define HTX0                GEN_ID(REG_ALTSEL, RCAR_PFC_GROUP_05, RCAR_PFC_PIN_00, RCAR_PFC_FUNC_0)
#define HRX0                GEN_ID(REG_ALTSEL, RCAR_PFC_GROUP_05, RCAR_PFC_PIN_01, RCAR_PFC_FUNC_0)
#define HRTS0_N             GEN_ID(REG_ALTSEL, RCAR_PFC_GROUP_05, RCAR_PFC_PIN_02, RCAR_PFC_FUNC_0)
#define HCTS0_N             GEN_ID(REG_ALTSEL, RCAR_PFC_GROUP_05, RCAR_PFC_PIN_03, RCAR_PFC_FUNC_0)
#define HSCK0               GEN_ID(REG_ALTSEL, RCAR_PFC_GROUP_05, RCAR_PFC_PIN_04, RCAR_PFC_FUNC_0)
CREATE_GROUP(hscif0_grp, HTX0, HRX0, HRTS0_N, HCTS0_N, HSCK0)

#define TX0                 GEN_ID(REG_ALTSEL, RCAR_PFC_GROUP_05, RCAR_PFC_PIN_00, RCAR_PFC_FUNC_1)
#define RX0                 GEN_ID(REG_ALTSEL, RCAR_PFC_GROUP_05, RCAR_PFC_PIN_01, RCAR_PFC_FUNC_1)
#define RTS0_N              GEN_ID(REG_ALTSEL, RCAR_PFC_GROUP_05, RCAR_PFC_PIN_02, RCAR_PFC_FUNC_1)
#define CTS0_N              GEN_ID(REG_ALTSEL, RCAR_PFC_GROUP_05, RCAR_PFC_PIN_03, RCAR_PFC_FUNC_1)
#define SCK0                GEN_ID(REG_ALTSEL, RCAR_PFC_GROUP_05, RCAR_PFC_PIN_04, RCAR_PFC_FUNC_1)
#define SCIF_CLK            GEN_ID(REG_ALTSEL, RCAR_PFC_GROUP_05, RCAR_PFC_PIN_05, RCAR_PFC_FUNC_0)
CREATE_GROUP(scif0_grp, TX0, RX0, RTS0_N, CTS0_N, SCK0, SCIF_CLK)

#define HTX1                GEN_ID(REG_ALTSEL, RCAR_PFC_GROUP_05, RCAR_PFC_PIN_06, RCAR_PFC_FUNC_0)
#define HRX1                GEN_ID(REG_ALTSEL, RCAR_PFC_GROUP_05, RCAR_PFC_PIN_07, RCAR_PFC_FUNC_0)
#define HRTS1_N             GEN_ID(REG_ALTSEL, RCAR_PFC_GROUP_05, RCAR_PFC_PIN_08, RCAR_PFC_FUNC_0)
#define HCTS1_N             GEN_ID(REG_ALTSEL, RCAR_PFC_GROUP_05, RCAR_PFC_PIN_09, RCAR_PFC_FUNC_0)
#define HSCK1               GEN_ID(REG_ALTSEL, RCAR_PFC_GROUP_05, RCAR_PFC_PIN_10, RCAR_PFC_FUNC_0)
CREATE_GROUP(hscif1_grp, HTX1, HRX1, HRTS1_N, HCTS1_N, HSCK1)

#define TX1                 GEN_ID(REG_ALTSEL, RCAR_PFC_GROUP_05, RCAR_PFC_PIN_06, RCAR_PFC_FUNC_1)
#define RX1                 GEN_ID(REG_ALTSEL, RCAR_PFC_GROUP_05, RCAR_PFC_PIN_07, RCAR_PFC_FUNC_1)
#define RTS1_N              GEN_ID(REG_ALTSEL, RCAR_PFC_GROUP_05, RCAR_PFC_PIN_08, RCAR_PFC_FUNC_1)
#define CTS1_N              GEN_ID(REG_ALTSEL, RCAR_PFC_GROUP_05, RCAR_PFC_PIN_09, RCAR_PFC_FUNC_1)
#define SCK1                GEN_ID(REG_ALTSEL, RCAR_PFC_GROUP_05, RCAR_PFC_PIN_10, RCAR_PFC_FUNC_1)
CREATE_GROUP(scif1_grp, TX1, RX1, RTS1_N, CTS1_N, SCK1)

#define SDA0                GEN_ID(REG_ALTSEL, RCAR_PFC_GROUP_02, RCAR_PFC_PIN_20, RCAR_PFC_FUNC_0)
#define SCL0                GEN_ID(REG_ALTSEL, RCAR_PFC_GROUP_02, RCAR_PFC_PIN_19, RCAR_PFC_FUNC_0)
#define MODSEL_SDA0         GEN_ID(REG_MODSEL, RCAR_PFC_GROUP_02, RCAR_PFC_PIN_20, MODSEL_I2C_MODE)
#define MODSEL_SCL0         GEN_ID(REG_MODSEL, RCAR_PFC_GROUP_02, RCAR_PFC_PIN_19, MODSEL_I2C_MODE)
CREATE_GROUP(i2c0_grp, SDA0, SCL0, MODSEL_SDA0, MODSEL_SCL0)
//#define DRV_SDA0            GEN_ID(REG_DRVCTRL, RCAR_PFC_GROUP_02, RCAR_PFC_PIN_20, DRIVE_STRENGTH_28)
//#define DRV_SCL0            GEN_ID(REG_DRVCTRL, RCAR_PFC_GROUP_02, RCAR_PFC_PIN_19, DRIVE_STRENGTH_58)
//CREATE_GROUP(i2c0_grp, SDA0, SCL0, MODSEL_SDA0, MODSEL_SCL0, DRV_SDA0, DRV_SCL0)
//#define TDSEL0_SDA0         GEN_ID(REG_TDSEL, RCAR_PFC_GROUP_02, RCAR_PFC_PIN_20, DELAY_20_PF)
//#define TDSEL0_SCL0         GEN_ID(REG_TDSEL, RCAR_PFC_GROUP_02, RCAR_PFC_PIN_19, DELAY_40_PF)
//CREATE_GROUP(i2c0_grp, SDA0, SCL0, MODSEL_SDA0, MODSEL_SCL0, DRV_SDA0, DRV_SCL0, TDSEL0_SDA0, TDSEL0_SCL0)

#define SDA1                GEN_ID(REG_ALTSEL, RCAR_PFC_GROUP_08, RCAR_PFC_PIN_01, RCAR_PFC_FUNC_0)
#define SCL1                GEN_ID(REG_ALTSEL, RCAR_PFC_GROUP_08, RCAR_PFC_PIN_00, RCAR_PFC_FUNC_0)
#define MODSEL_SDA1         GEN_ID(REG_MODSEL, RCAR_PFC_GROUP_08, RCAR_PFC_PIN_01, MODSEL_I2C_MODE)
#define MODSEL_SCL1         GEN_ID(REG_MODSEL, RCAR_PFC_GROUP_08, RCAR_PFC_PIN_00, MODSEL_I2C_MODE)
CREATE_GROUP(i2c1_grp, SDA1, SCL1, MODSEL_SDA1, MODSEL_SCL1)

#define SDA2                GEN_ID(REG_ALTSEL, RCAR_PFC_GROUP_08, RCAR_PFC_PIN_03, RCAR_PFC_FUNC_0)
#define SCL2                GEN_ID(REG_ALTSEL, RCAR_PFC_GROUP_08, RCAR_PFC_PIN_02, RCAR_PFC_FUNC_0)
#define MODSEL_SDA2         GEN_ID(REG_MODSEL, RCAR_PFC_GROUP_08, RCAR_PFC_PIN_03, MODSEL_I2C_MODE)
#define MODSEL_SCL2         GEN_ID(REG_MODSEL, RCAR_PFC_GROUP_08, RCAR_PFC_PIN_02, MODSEL_I2C_MODE)
CREATE_GROUP(i2c2_grp, SDA2, SCL2, MODSEL_SDA2, MODSEL_SCL2)

#define SDA3                GEN_ID(REG_ALTSEL, RCAR_PFC_GROUP_08, RCAR_PFC_PIN_05, RCAR_PFC_FUNC_0)
#define SCL3                GEN_ID(REG_ALTSEL, RCAR_PFC_GROUP_08, RCAR_PFC_PIN_04, RCAR_PFC_FUNC_0)
#define MODSEL_SDA3         GEN_ID(REG_MODSEL, RCAR_PFC_GROUP_08, RCAR_PFC_PIN_05, MODSEL_I2C_MODE)
#define MODSEL_SCL3         GEN_ID(REG_MODSEL, RCAR_PFC_GROUP_08, RCAR_PFC_PIN_04, MODSEL_I2C_MODE)
CREATE_GROUP(i2c3_grp, SDA3, SCL3, MODSEL_SDA3, MODSEL_SCL3)

#define SDA4                GEN_ID(REG_ALTSEL, RCAR_PFC_GROUP_08, RCAR_PFC_PIN_07, RCAR_PFC_FUNC_0)
#define SCL4                GEN_ID(REG_ALTSEL, RCAR_PFC_GROUP_08, RCAR_PFC_PIN_06, RCAR_PFC_FUNC_0)
#define MODSEL_SDA4         GEN_ID(REG_MODSEL, RCAR_PFC_GROUP_08, RCAR_PFC_PIN_07, MODSEL_I2C_MODE)
#define MODSEL_SCL4         GEN_ID(REG_MODSEL, RCAR_PFC_GROUP_08, RCAR_PFC_PIN_06, MODSEL_I2C_MODE)
CREATE_GROUP(i2c4_grp, SDA4, SCL4, MODSEL_SDA4, MODSEL_SCL4)

#define SDA5                GEN_ID(REG_ALTSEL, RCAR_PFC_GROUP_08, RCAR_PFC_PIN_09, RCAR_PFC_FUNC_0)
#define SCL5                GEN_ID(REG_ALTSEL, RCAR_PFC_GROUP_08, RCAR_PFC_PIN_08, RCAR_PFC_FUNC_0)
#define MODSEL_SDA5         GEN_ID(REG_MODSEL, RCAR_PFC_GROUP_08, RCAR_PFC_PIN_09, MODSEL_I2C_MODE)
#define MODSEL_SCL5         GEN_ID(REG_MODSEL, RCAR_PFC_GROUP_08, RCAR_PFC_PIN_08, MODSEL_I2C_MODE)
CREATE_GROUP(i2c5_grp, SDA5, SCL5, MODSEL_SDA5, MODSEL_SCL5)

#define SDA6                GEN_ID(REG_ALTSEL, RCAR_PFC_GROUP_08, RCAR_PFC_PIN_11, RCAR_PFC_FUNC_0)
#define SCL6                GEN_ID(REG_ALTSEL, RCAR_PFC_GROUP_08, RCAR_PFC_PIN_10, RCAR_PFC_FUNC_0)
#define MODSEL_SDA6         GEN_ID(REG_MODSEL, RCAR_PFC_GROUP_08, RCAR_PFC_PIN_11, MODSEL_I2C_MODE)
#define MODSEL_SCL6         GEN_ID(REG_MODSEL, RCAR_PFC_GROUP_08, RCAR_PFC_PIN_10, MODSEL_I2C_MODE)
CREATE_GROUP(i2c6_grp, SDA6, SCL6, MODSEL_SDA6, MODSEL_SCL6)

#define SDA7                GEN_ID(REG_ALTSEL, RCAR_PFC_GROUP_08, RCAR_PFC_PIN_13, RCAR_PFC_FUNC_0)
#define SCL7                GEN_ID(REG_ALTSEL, RCAR_PFC_GROUP_08, RCAR_PFC_PIN_12, RCAR_PFC_FUNC_0)
#define MODSEL_SDA7         GEN_ID(REG_MODSEL, RCAR_PFC_GROUP_08, RCAR_PFC_PIN_13, MODSEL_I2C_MODE)
#define MODSEL_SCL7         GEN_ID(REG_MODSEL, RCAR_PFC_GROUP_08, RCAR_PFC_PIN_12, MODSEL_I2C_MODE)
CREATE_GROUP(i2c7_grp, SDA7, SCL7, MODSEL_SDA7, MODSEL_SCL7)

#define SDA8                GEN_ID(REG_ALTSEL, RCAR_PFC_GROUP_08, RCAR_PFC_PIN_15, RCAR_PFC_FUNC_0)
#define SCL8                GEN_ID(REG_ALTSEL, RCAR_PFC_GROUP_08, RCAR_PFC_PIN_14, RCAR_PFC_FUNC_0)
#define MODSEL_SDA8         GEN_ID(REG_MODSEL, RCAR_PFC_GROUP_08, RCAR_PFC_PIN_15, MODSEL_I2C_MODE)
#define MODSEL_SCL8         GEN_ID(REG_MODSEL, RCAR_PFC_GROUP_08, RCAR_PFC_PIN_14, MODSEL_I2C_MODE)
CREATE_GROUP(i2c8_grp, SDA8, SCL8, MODSEL_SDA8, MODSEL_SCL8)

#define AUDIO_CLKA          GEN_ID(REG_ALTSEL, RCAR_PFC_GROUP_07, RCAR_PFC_PIN_08, RCAR_PFC_FUNC_0)
#define AUDIO0_CLKOUT0      GEN_ID(REG_ALTSEL, RCAR_PFC_GROUP_06, RCAR_PFC_PIN_19, RCAR_PFC_FUNC_0)
#define SSI5_SCK            GEN_ID(REG_ALTSEL, RCAR_PFC_GROUP_07, RCAR_PFC_PIN_09, RCAR_PFC_FUNC_0)
#define SSI5_WS             GEN_ID(REG_ALTSEL, RCAR_PFC_GROUP_07, RCAR_PFC_PIN_10, RCAR_PFC_FUNC_0)
#define SSI5_SD             GEN_ID(REG_ALTSEL, RCAR_PFC_GROUP_07, RCAR_PFC_PIN_11, RCAR_PFC_FUNC_0)
CREATE_GROUP(audio0_grp, AUDIO_CLKA, AUDIO0_CLKOUT0, SSI5_SCK, SSI5_WS, SSI5_SD)

typedef struct {
    e_module_id_t module_id;
    int *group;
} st_driver_group_t;

static const st_driver_group_t all_drv_groups[] = {
    {.module_id = MODULE_HSCIF0, .group = ADD_GROUP(hscif0_grp)},
    {.module_id = MODULE_SCIF0, .group = ADD_GROUP(scif0_grp)},
    {.module_id = MODULE_SCIF1, .group = ADD_GROUP(scif1_grp)},
    {.module_id = MODULE_I2C0,  .group = ADD_GROUP(i2c0_grp)},
    {.module_id = MODULE_I2C1,  .group = ADD_GROUP(i2c1_grp)},
    {.module_id = MODULE_I2C2,  .group = ADD_GROUP(i2c2_grp)},
    {.module_id = MODULE_I2C3,  .group = ADD_GROUP(i2c3_grp)},
    {.module_id = MODULE_I2C4,  .group = ADD_GROUP(i2c4_grp)},
    {.module_id = MODULE_I2C5,  .group = ADD_GROUP(i2c5_grp)},
    {.module_id = MODULE_I2C6,  .group = ADD_GROUP(i2c6_grp)},
    {.module_id = MODULE_I2C7,  .group = ADD_GROUP(i2c7_grp)},
    {.module_id = MODULE_I2C8,  .group = ADD_GROUP(i2c8_grp)},
    {.module_id = MODULE_AUDIO_0, .group = ADD_GROUP(audio0_grp)},
    // Add driver groups.
    {.module_id = MODULE_INVALID, .group = 0},
};

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
    uint32_t base_addr;
    uint32_t reg_addr;

    switch (grp) {
    case 0:
        base_addr = PFC_GR_0;
        break;
    case 1:
        base_addr = PFC_GR_1;
        break;
    case 2:
        base_addr = PFC_GR_2;
        break;
    case 3:
        base_addr = PFC_GR_3;
        break;
    case 4:
        base_addr = PFC_GR_4;
        break;
    case 5:
        base_addr = PFC_GR_5;
        break;
    case 6:
        base_addr = PFC_GR_6;
        break;
    case 7:
        base_addr = PFC_GR_7;
        break;
    case 8:
        base_addr = PFC_GR_8;
        break;
    case 9:
        base_addr = PFC_GR_9;
        break;
    case 10:
        base_addr = PFC_GR_10;
        break;
    default:
        printf("PFC group %d not exist!\n", grp);
        return PFC_INVALID_ADDR;
    }

    reg_addr = base_addr + offset;

    return reg_addr;
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
        grp > RCAR_PFC_GROUP_10)
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
        grp > RCAR_PFC_GROUP_10)
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
        grp > RCAR_PFC_GROUP_10)
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
        grp > RCAR_PFC_GROUP_10)
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
#if (BOARD == X5H_RFS2 || BOARD == AI_ACC)
    ( void ) module_list;
#else
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
