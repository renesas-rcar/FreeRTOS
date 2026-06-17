/*
 *
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

/*******************************************************************************
 * DESCRIPTION   : Module Controller driver header
 ******************************************************************************/

#ifndef MODULE_CONTROLLER_H_
#define MODULE_CONTROLLER_H_

#include <stdint.h>
#include <module_controller/module_controller_register.h>

/* CF-compliant (TS Rev.0.40) */

typedef struct
{
    const char *hier_name;      /* PD_hier name */
} MDLC_HIER_NAME_TABLE;

typedef struct
{
    const char *name;               /* Classification Group name                    */
} MDLC_CLASS_GROUP_NAME_TABLE;

typedef struct
{
    uint32_t mpg_exec_flag;         /* MPG  execution flag                          */
    uint32_t ms_exec_flag;          /* MS execution flag                            */
} MDLC_MODULE_NUM_EXEC_TABLE;

typedef struct
{
    uint32_t bit_assign_info;   /* MS bit assign information                        */
} MDLC_MS_BIT_ASSIGN_TABLE;

typedef struct
{
    uint32_t ms_reset_reg_addr; /* Module System Reset Register                     */
    uint32_t ms_stat_reg_addr;  /* Module System Reset Status Register address      */
    uint32_t ms_reg_bit_assign; /* MS bit assign information                        */
} MDLC_MS_REG_TABLE;

#define MDLC_RETRY_MAX          (128U)

/* Max number of registers. */
#define PD_HIER_MAX             (PD_HIER_MAX_NON_SCP + PD_HIER_MAX_SCP)
#define PD_HIER_MAX_NON_SCP     (26U)
#define PD_HIER_MAX_SCP         (2U)

#define BIT0                    (0x00000001U)

#define PD_HIER_NOT_IMPL        (0x0U)

/* Status of MS */
#define MS_BIT_MASK             (0x00000003U)   /* Module bit mask  */
#define MS_STANDBY              (0x0U)          /* Module Standby   */
#define MS_STOP                 (0x2U)          /* Module STOP      */
#define MS_RESET                (0x1U)          /* Module Reset     */
#define MS_RUN                  (0x3U)          /* Module RUN       */

/* PD_hier number */
#define PD_HIER_HSCS        (16U)

/* MPG and MS register setting value */
#define WRITE_KEY_CODE_DIS      (0xA5A5A500U)
#define WRITE_KEY_CODE_EN       (0xA5A5A501U)

/* PD Hier virtual pin address */
#define PD_HIER_VR_PIN_ADDR (0x34004020U)

/* MDUC-Region base address of Hier Structure. */
#define BASE_ADDR_HIER_HSCS (0xDE200000U)   /* Hier Structure: 16, HSCS */

#define MODULE_NUM_MAX              (30U)

#define MODULE_NUM_HSCS             (16U)

/* Classification Group Name                                                    */
#define CLASS_GROUP_NAME_HSCS       "Module Controller (HSCS)"

/* MPG and MS execution flag for Target Classification Groups                   */
#define CLASS_GROUP_MPG_RESERVED    (0U)
#define CLASS_GROUP_MPG_SKIP        (CLASS_GROUP_MPG_RESERVED)
#define CLASS_GROUP_MS_RESERVED     (0U)
#define CLASS_GROUP_MS_SKIP         (CLASS_GROUP_MS_RESERVED)
#define CLASS_GROUP_MS_EXEC         (1U)

/* Module Standby bit assign                                                    */
#define MS_REG_MAX_NUM              (22U)
#define MS_BIT_RESERVED             (0x00000000U)
#define MS_BIT_ASSIGNED             (0x00000003U)
#define MS_BIT_07_06                (MS_BIT_ASSIGNED << 6U)
#define MS_BIT_05_04                (MS_BIT_ASSIGNED << 4U)
#define MS_BIT_03_02                (MS_BIT_ASSIGNED << 2U)
#define MS_BIT_01_00                (MS_BIT_ASSIGNED << 0U)

static const uint32_t mdlcnpkcprot1_reg[PD_HIER_MAX] =
{
    [MODULE_NUM_HSCS] = MDLC16PKCPROT1,   /* Hier Structure: 16, HSCS */
};

/* PD_hier neme */
#define PD_HIER_NAME_HSCS   "HSCS"

/* PD_hier name array. */
static const MDLC_HIER_NAME_TABLE hier_name_table[PD_HIER_MAX] =
{   /*                  *hier_name */
    [PD_HIER_HSCS]  =   {PD_HIER_NAME_HSCS},
};

static const MDLC_MODULE_NUM_EXEC_TABLE mpg_ms_exec_flag[MODULE_NUM_MAX] =
{
    /*                      mpg_exec_flag,              ms_exec_flag            */
    [MODULE_NUM_HSCS] =    {CLASS_GROUP_MPG_RESERVED,   CLASS_GROUP_MS_EXEC     },
};

static const MDLC_CLASS_GROUP_NAME_TABLE class_group_name_table[MODULE_NUM_MAX] =
{
    /*                      *class_group_name                                  */
    [MODULE_NUM_HSCS] =    {CLASS_GROUP_NAME_HSCS   },
};

static const MDLC_MS_BIT_ASSIGN_TABLE ms_reg_bit_assign[MODULE_NUM_MAX][MS_REG_MAX_NUM] =
{
    /* Module Number 16, Module Controller (HSCS)   */
    [MODULE_NUM_HSCS] =
    {   /*          bit_assign_info                                                                                                                                                     *
         *              [31:30]	        | [29:28]           | [27:26]           | [25:24]           | [23:22]           | [21:20]           | [19:18]           | [17:16]           \   *
         *          |   [15:14]         | [13:12]           | [11:10]           | [9:8]             | [7:6]             | [5:4]             | [3:2]             | [1:0]                 */
        [0]     = {     MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   \
                    |   MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_01_00      },

        [1]     = {     MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   \
                    |   MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   },

        [2]     = {     MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   \
                    |   MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_07_06      | MS_BIT_05_04      | MS_BIT_03_02      | MS_BIT_01_00      },

        [3]     = {     MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   \
                    |   MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   },

        [4]     = {     MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   \
                    |   MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   },

        [5]     = {     MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   \
                    |   MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   },

        [6]     = {     MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   \
                    |   MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   },

        [7]     = {     MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   \
                    |   MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   },

        [8]     = {     MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   \
                    |   MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   },

        [9]     = {     MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   \
                    |   MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   },

        [10]    = {     MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   \
                    |   MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   },

        [11]    = {     MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   \
                    |   MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   },

        [12]    = {     MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   \
                    |   MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   },

        [13]    = {     MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   \
                    |   MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   },

        [14]    = {     MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   \
                    |   MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   },

        [15]    = {     MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   \
                    |   MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   },

        [16]    = {     MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   \
                    |   MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   },

        [17]    = {     MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   \
                    |   MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   },

        [18]    = {     MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   \
                    |   MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   },

        [19]    = {     MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   \
                    |   MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   },

        [20]    = {     MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   \
                    |   MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   },

        [21]    = {     MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   \
                    |   MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   },
    },
};


static const MDLC_MS_REG_TABLE ms_reg_table[MODULE_NUM_MAX][MS_REG_MAX_NUM] =
{
    /* Module Number 16, Module Controller (HSCS)   */
    [MODULE_NUM_HSCS] =
    {   /*          ms_reset_reg_addr,  ms_stat_reg_addr,   ms_reg_bit_assign                                          */
        [0]     = { MDLC16MSRES00,      MDLC16MSRESS00,     ms_reg_bit_assign[MODULE_NUM_HSCS][0U].bit_assign_info     },
        [1]     = { MDLC16MSRES01,      MDLC16MSRESS01,     ms_reg_bit_assign[MODULE_NUM_HSCS][1U].bit_assign_info     },
        [2]     = { MDLC16MSRES02,      MDLC16MSRESS02,     ms_reg_bit_assign[MODULE_NUM_HSCS][2U].bit_assign_info     },
        [3]     = { MDLC16MSRES03,      MDLC16MSRESS03,     ms_reg_bit_assign[MODULE_NUM_HSCS][3U].bit_assign_info     },
        [4]     = { MDLC16MSRES04,      MDLC16MSRESS04,     ms_reg_bit_assign[MODULE_NUM_HSCS][4U].bit_assign_info     },
        [5]     = { MDLC16MSRES05,      MDLC16MSRESS05,     ms_reg_bit_assign[MODULE_NUM_HSCS][5U].bit_assign_info     },
        [6]     = { MDLC16MSRES06,      MDLC16MSRESS06,     ms_reg_bit_assign[MODULE_NUM_HSCS][6U].bit_assign_info     },
        [7]     = { MDLC16MSRES07,      MDLC16MSRESS07,     ms_reg_bit_assign[MODULE_NUM_HSCS][7U].bit_assign_info     },
        [8]     = { MDLC16MSRES08,      MDLC16MSRESS08,     ms_reg_bit_assign[MODULE_NUM_HSCS][8U].bit_assign_info     },
        [9]     = { MDLC16MSRES09,      MDLC16MSRESS09,     ms_reg_bit_assign[MODULE_NUM_HSCS][9U].bit_assign_info     },
        [10]    = { MDLC16MSRES10,      MDLC16MSRESS10,     ms_reg_bit_assign[MODULE_NUM_HSCS][10U].bit_assign_info    },
        [11]    = { MDLC16MSRES11,      MDLC16MSRESS11,     ms_reg_bit_assign[MODULE_NUM_HSCS][11U].bit_assign_info    },
        [12]    = { MDLC16MSRES12,      MDLC16MSRESS12,     ms_reg_bit_assign[MODULE_NUM_HSCS][12U].bit_assign_info    },
        [13]    = { MDLC16MSRES13,      MDLC16MSRESS13,     ms_reg_bit_assign[MODULE_NUM_HSCS][13U].bit_assign_info    },
        [14]    = { MDLC16MSRES14,      MDLC16MSRESS14,     ms_reg_bit_assign[MODULE_NUM_HSCS][14U].bit_assign_info    },
        [15]    = { MDLC16MSRES15,      MDLC16MSRESS15,     ms_reg_bit_assign[MODULE_NUM_HSCS][15U].bit_assign_info    },
        [16]    = { MDLC16MSRES16,      MDLC16MSRESS16,     ms_reg_bit_assign[MODULE_NUM_HSCS][16U].bit_assign_info    },
        [17]    = { MDLC16MSRES17,      MDLC16MSRESS17,     ms_reg_bit_assign[MODULE_NUM_HSCS][17U].bit_assign_info    },
        [18]    = { MDLC16MSRES18,      MDLC16MSRESS18,     ms_reg_bit_assign[MODULE_NUM_HSCS][18U].bit_assign_info    },
        [19]    = { MDLC16MSRES19,      MDLC16MSRESS19,     ms_reg_bit_assign[MODULE_NUM_HSCS][19U].bit_assign_info    },
        [20]    = { MDLC16MSRES20,      MDLC16MSRESS20,     ms_reg_bit_assign[MODULE_NUM_HSCS][20U].bit_assign_info    },
        [21]    = { MDLC16MSRES21,      MDLC16MSRESS21,     ms_reg_bit_assign[MODULE_NUM_HSCS][21U].bit_assign_info    },
    },
};


/*****************************************************************************
 * Module Controller common
 *****************************************************************************/
const char* get_class_group_name(uint32_t module_num);


/*****************************************************************************
 * Module Controller common
 *****************************************************************************/
const char* get_class_group_name(uint32_t module_num);
uint32_t get_mx_setting(void);

/*****************************************************************************
 * Module Standby
 *****************************************************************************/
void mdlc_ms_init(uint32_t module_num, uint32_t reg_num, uint32_t *init_bit_array);
void mdlc_ms_init_class_group(uint32_t module_num, uint32_t reg_num, uint32_t *init_bit_array);
void mdlc_ms_module_run_class_group(uint32_t module_num, uint32_t reg_num, uint32_t *init_bit_array);
void mdlc_ms_init_reg(uint32_t module_num, uint32_t reg_num, uint32_t *init_bit_array);
void mdlc_ms_module_run_reg(uint32_t module_num, uint32_t reg_num, uint32_t *init_bit_array);
void mdlc_ms_module_run_bit(uint32_t module_num, uint32_t reg_num, uint32_t bit_num);

void mdlc_check_ms_status(uint32_t module_num, uint32_t reg_num, uint32_t bit_num);
void mdlc_transition_ms(uint32_t module_num, uint32_t reg_num, uint32_t bit_num, uint32_t ms_dest);
void mdlc_ms_reg_write(uint32_t module_num, uint32_t reg_addr, uint32_t reg_val);

#endif  /* MODULE_CONTROLLER_H_ */

