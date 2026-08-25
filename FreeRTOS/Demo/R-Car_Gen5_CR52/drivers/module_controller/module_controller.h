/*******************************************************************************
 * 
 * Copyright (c) 2026 Renesas Electronics Corporation
 * 
 * SPDX-License-Identifier: MIT
 * 
 *******************************************************************************/

/*******************************************************************************
 * DESCRIPTION   : Module Controller driver header
 ******************************************************************************/

#ifndef MODULE_CONTROLLER_H_
#define MODULE_CONTROLLER_H_

#include <stdint.h>
#include <module_controller_register.h>

/* CF-compliant (TS Rev.0.40) */

typedef struct
{
    const char *hier_name;      /* PD_hier name */
} MDLC_HIER_NAME_TABLE;

/* Module Power RUN transition execution information. */
typedef struct
{
    uint32_t hier_pdid_max;     /* Max value of PDID for each PD_hier. */
    uint32_t pd_init_stat;      /* Initial status of MPG at Full-RUN and Sentry startup. */
    uint32_t mpg_run_skip_flag; /* Flag to skip transition to Module Power RUN. */
} MDLC_MPG_EXEC_TABLE;


/* Module RUN transition execution information. */
typedef struct
{
    uint32_t mpg_reg_addr;      /* Module Power Domain Gating Register address */
    uint32_t mpg_stat_reg_addr; /* Module Power Domain Gating Status Register address */
    uint32_t pdr_assign;        /* MPG PDR assignment information */
    uint32_t set_val;           /* Register setting value */
} MDLC_MPG_CFG_TABLE;


/* Module RUN transition execution information. */
typedef struct
{
    uint32_t ms_reg_addr;       /* Module System Reset Register address */
    uint32_t ms_stat_reg_addr;  /* Module System Reset Status Register address */
    uint32_t bit_assign;        /* MS bit assignment information */
    uint32_t set_val;           /* Register setting value */
} MDLC_MS_CFG_TABLE;

/* MPG and MS result status */
#if 0
#define MDLC_RETRY_MAX          (32U)
#else
#define MDLC_RETRY_MAX          (128U)
#endif

/* Max number of registers. */
#define PD_HIER_MAX             (1U)

#ifdef BIT0
#undef BIT0
#endif

#define BIT0                    (0x00000001U)

/* Status of MS */
#define MS_BIT_MASK             (0x00000003U)   /* Module bit mask  */
#define MS_STANDBY              (0x0U)          /* Module Standby   */
#define MS_RESET                (0x1U)          /* Module Reset     */
#define MS_STOP                 (0x2U)          /* Module STOP      */
#define MS_RUN                  (0x3U)          /* Module RUN       */

/* PD_hier number */
#define PD_HIER_RT          (0U)

/* MPG and MS register setting value */
#define WRITE_KEY_CODE_DIS      (0xA5A5A500U)
#define WRITE_KEY_CODE_EN       (0xA5A5A501U)

/* NEW */

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
    uint32_t mpg_gating_reg_addr;   /* Module Power Domain Gating Register          */
    uint32_t mpg_stat_reg_addr;     /* Module Power Domain Gating Status Register   */
    uint32_t mpg_pdr_assign;        /* MPG PDR assign information                   */
} MDLC_MPG_REG_TABLE;

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

/* MDUC-Region base address of Hier Structure. */
#if 1
#define BASE_ADDR_HIER_RT   (0x19440000U)   /* Hier Structure: 14, RT */
#endif

#define MODULE_NUM_MAX              (1U)

#define MODULE_NUM_RT               (0U)

/* Classification Group Name                                                    */
#define CLASS_GROUP_NAME_RT         "Module Controller (RT)  "

/* MPG and MS execution flag for Target Classification Groups                   */
#define CLASS_GROUP_MPG_RESERVED    (0U)
#define CLASS_GROUP_MPG_SKIP        (CLASS_GROUP_MPG_RESERVED)
#define CLASS_GROUP_MPG_EXEC        (1U)
#define CLASS_GROUP_MS_RESERVED     (0U)
#define CLASS_GROUP_MS_SKIP         (CLASS_GROUP_MS_RESERVED)
#define CLASS_GROUP_MS_EXEC         (1U)

/* Module Standby bit assign                                                    */
#define MS_REG_MAX_NUM              (22U)
#define MS_BIT_RESERVED             (0x00000000U)
#define MS_BIT_SKIP                 (MS_BIT_RESERVED)
#define MS_BIT_ASSIGNED             (0x00000003U)
#define MS_BIT_31_30                (MS_BIT_ASSIGNED << 30U)
#define MS_BIT_29_28                (MS_BIT_ASSIGNED << 28U)
#define MS_BIT_27_26                (MS_BIT_ASSIGNED << 26U)
#define MS_BIT_25_24                (MS_BIT_ASSIGNED << 24U)
#define MS_BIT_23_22                (MS_BIT_ASSIGNED << 22U)
#define MS_BIT_21_20                (MS_BIT_ASSIGNED << 20U)
#define MS_BIT_19_18                (MS_BIT_ASSIGNED << 18U)
#define MS_BIT_17_16                (MS_BIT_ASSIGNED << 16U)
#define MS_BIT_15_14                (MS_BIT_ASSIGNED << 14U)
#define MS_BIT_13_12                (MS_BIT_ASSIGNED << 12U)
#define MS_BIT_11_10                (MS_BIT_ASSIGNED << 10U)
#define MS_BIT_09_08                (MS_BIT_ASSIGNED << 8U)
#define MS_BIT_07_06                (MS_BIT_ASSIGNED << 6U)
#define MS_BIT_05_04                (MS_BIT_ASSIGNED << 4U)
#define MS_BIT_03_02                (MS_BIT_ASSIGNED << 2U)
#define MS_BIT_01_00                (MS_BIT_ASSIGNED << 0U)

static const uint32_t mdlcnpkcprot1_reg[PD_HIER_MAX] =
{
    MDLC14PKCPROT1   /* Hier Structure: 14, RT */
};

/* PD_hier neme */
#define PD_HIER_NAME_RT     "RT"

/* PD_hier name array. */
static const MDLC_HIER_NAME_TABLE hier_name_table[PD_HIER_MAX] =
{   /*                  *hier_name */
    [PD_HIER_RT]    =   {PD_HIER_NAME_RT},
};

static const MDLC_MODULE_NUM_EXEC_TABLE mpg_ms_exec_flag[MODULE_NUM_MAX] =
{
    /*                      mpg_exec_flag,              ms_exec_flag            */
    [MODULE_NUM_RT  ] =    {CLASS_GROUP_MPG_RESERVED,   CLASS_GROUP_MS_EXEC     },
};

static const MDLC_CLASS_GROUP_NAME_TABLE class_group_name_table[MODULE_NUM_MAX] =
{
    /*                      *class_group_name                                  */
    [MODULE_NUM_RT  ] =    {CLASS_GROUP_NAME_RT     },
};

static const MDLC_MS_BIT_ASSIGN_TABLE ms_reg_bit_assign[MODULE_NUM_MAX][MS_REG_MAX_NUM] =
{
    /* Module Number 14, Module Controller (RT)     */
    [MODULE_NUM_RT] =
    {   /*          bit_assign_info                                                                                                                                                     *
         *              [31:30]	        | [29:28]           | [27:26]           | [25:24]           | [23:22]           | [21:20]           | [19:18]           | [17:16]           \   *
         *          |   [15:14]         | [13:12]           | [11:10]           | [9:8]             | [7:6]             | [5:4]             | [3:2]             | [1:0]                 */
        [0]     = {     MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_19_18      | MS_BIT_17_16      \
                    |   MS_BIT_15_14    | MS_BIT_13_12      | MS_BIT_11_10      | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_03_02      | MS_BIT_01_00      },

        [1]     = {     MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   \
                    |   MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   },

        [2]     = {     MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   \
                    |   MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   },

        [3]     = {     MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   \
                    |   MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   },

        [4]     = {     MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   \
                    |   MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   },

        [5]     = {     MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   \
                    |   MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_03_02      | MS_BIT_01_00      },

        [6]     = {     MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   \
                    |   MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   },

        [7]     = {     MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   \
                    |   MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   },

        [8]     = {     MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   \
                    |   MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   },

        [9]     = {     MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   \
                    |   MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   },

        [10]    = {     MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   \
                    |   MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_09_08      | MS_BIT_07_06      | MS_BIT_05_04      | MS_BIT_03_02      | MS_BIT_01_00      },

        [11]    = {     MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   \
                    |   MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_05_04      | MS_BIT_03_02      | MS_BIT_RESERVED   },

        [12]    = {     MS_BIT_31_30    | MS_BIT_29_28      | MS_BIT_27_26      | MS_BIT_25_24      | MS_BIT_23_22      | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   \
                    |   MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   },

        [13]    = {     MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_21_20      | MS_BIT_19_18      | MS_BIT_17_16      \
                    |   MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   },

        [14]    = {     MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   \
                    |   MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_11_10      | MS_BIT_09_08      | MS_BIT_07_06      | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   },

        [15]    = {     MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_17_16      \
                    |   MS_BIT_15_14    | MS_BIT_RESERVED   | MS_BIT_11_10      | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   },

        [16]    = {     MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   \
                    |   MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_11_10      | MS_BIT_09_08      | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   },

        [17]    = {     MS_BIT_31_30    | MS_BIT_29_28      | MS_BIT_27_26      | MS_BIT_25_24      | MS_BIT_23_22      | MS_BIT_21_20      | MS_BIT_19_18      | MS_BIT_17_16      \
                    |   MS_BIT_15_14    | MS_BIT_13_12      | MS_BIT_11_10      | MS_BIT_09_08      | MS_BIT_07_06      | MS_BIT_05_04      | MS_BIT_03_02      | MS_BIT_RESERVED   },

        [18]    = {     MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   \
                    |   MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_11_10      | MS_BIT_09_08      | MS_BIT_07_06      | MS_BIT_05_04      | MS_BIT_03_02      | MS_BIT_01_00      },

        [19]    = {     MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   \
                    |   MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_09_08      | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_01_00      },

        [20]    = {     MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_23_22      | MS_BIT_21_20      | MS_BIT_19_18      | MS_BIT_17_16      \
                    |   MS_BIT_RESERVED | MS_BIT_13_12      | MS_BIT_11_10      | MS_BIT_09_08      | MS_BIT_07_06      | MS_BIT_05_04      | MS_BIT_03_02      | MS_BIT_01_00      },

        [21]    = {     MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   \
                    |   MS_BIT_RESERVED | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   | MS_BIT_RESERVED   },
    }
};

static const MDLC_MS_REG_TABLE ms_reg_table[MODULE_NUM_MAX][MS_REG_MAX_NUM] =
{
    /* Module Number 14, Module Controller (RT)     */
    [MODULE_NUM_RT] =
    {   /*          ms_reset_reg_addr,  ms_stat_reg_addr,   ms_reg_bit_assign                                          */
        [0]     = { MDLC14MSRES00,      MDLC14MSRESS00,     ms_reg_bit_assign[MODULE_NUM_RT][0U].bit_assign_info       },
        [1]     = { MDLC14MSRES01,      MDLC14MSRESS01,     ms_reg_bit_assign[MODULE_NUM_RT][1U].bit_assign_info       },
        [2]     = { MDLC14MSRES02,      MDLC14MSRESS02,     ms_reg_bit_assign[MODULE_NUM_RT][2U].bit_assign_info       },
        [3]     = { MDLC14MSRES03,      MDLC14MSRESS03,     ms_reg_bit_assign[MODULE_NUM_RT][3U].bit_assign_info       },
        [4]     = { MDLC14MSRES04,      MDLC14MSRESS04,     ms_reg_bit_assign[MODULE_NUM_RT][4U].bit_assign_info       },
        [5]     = { MDLC14MSRES05,      MDLC14MSRESS05,     ms_reg_bit_assign[MODULE_NUM_RT][5U].bit_assign_info       },
        [6]     = { MDLC14MSRES06,      MDLC14MSRESS06,     ms_reg_bit_assign[MODULE_NUM_RT][6U].bit_assign_info       },
        [7]     = { MDLC14MSRES07,      MDLC14MSRESS07,     ms_reg_bit_assign[MODULE_NUM_RT][7U].bit_assign_info       },
        [8]     = { MDLC14MSRES08,      MDLC14MSRESS08,     ms_reg_bit_assign[MODULE_NUM_RT][8U].bit_assign_info       },
        [9]     = { MDLC14MSRES09,      MDLC14MSRESS09,     ms_reg_bit_assign[MODULE_NUM_RT][9U].bit_assign_info       },
        [10]    = { MDLC14MSRES10,      MDLC14MSRESS10,     ms_reg_bit_assign[MODULE_NUM_RT][10U].bit_assign_info      },
        [11]    = { MDLC14MSRES11,      MDLC14MSRESS11,     ms_reg_bit_assign[MODULE_NUM_RT][11U].bit_assign_info      },
        [12]    = { MDLC14MSRES12,      MDLC14MSRESS12,     ms_reg_bit_assign[MODULE_NUM_RT][12U].bit_assign_info      },
        [13]    = { MDLC14MSRES13,      MDLC14MSRESS13,     ms_reg_bit_assign[MODULE_NUM_RT][13U].bit_assign_info      },
        [14]    = { MDLC14MSRES14,      MDLC14MSRESS14,     ms_reg_bit_assign[MODULE_NUM_RT][14U].bit_assign_info      },
        [15]    = { MDLC14MSRES15,      MDLC14MSRESS15,     ms_reg_bit_assign[MODULE_NUM_RT][15U].bit_assign_info      },
        [16]    = { MDLC14MSRES16,      MDLC14MSRESS16,     ms_reg_bit_assign[MODULE_NUM_RT][16U].bit_assign_info      },
        [17]    = { MDLC14MSRES17,      MDLC14MSRESS17,     ms_reg_bit_assign[MODULE_NUM_RT][17U].bit_assign_info      },
        [18]    = { MDLC14MSRES18,      MDLC14MSRESS18,     ms_reg_bit_assign[MODULE_NUM_RT][18U].bit_assign_info      },
        [19]    = { MDLC14MSRES19,      MDLC14MSRESS19,     ms_reg_bit_assign[MODULE_NUM_RT][19U].bit_assign_info      },
        [20]    = { MDLC14MSRES20,      MDLC14MSRESS20,     ms_reg_bit_assign[MODULE_NUM_RT][20U].bit_assign_info      },
        [21]    = { MDLC14MSRES21,      MDLC14MSRESS21,     ms_reg_bit_assign[MODULE_NUM_RT][21U].bit_assign_info      },
    }
};

/*****************************************************************************
 * Module Controller common
 *****************************************************************************/
const char* get_class_group_name_f(uint32_t module_num);
const char *get_hier_name_f(uint32_t pd_hier);

/*****************************************************************************
 * Module Controller common
 *****************************************************************************/
const char* get_class_group_name_f(uint32_t module_num);

/*****************************************************************************
 * Module Standby
 *****************************************************************************/
void mdlc_ms_module_run_bit_f(uint32_t module_num, uint32_t reg_num, uint32_t bit_num);

void mdlc_check_ms_status_f(uint32_t module_num, uint32_t reg_num, uint32_t bit_num);
void mdlc_transition_ms_f(uint32_t module_num, uint32_t reg_num, uint32_t bit_num, uint32_t ms_dest);
void mdlc_ms_reg_write_f(uint32_t module_num, uint32_t reg_addr, uint32_t reg_val);

#endif  /* MODULE_CONTROLLER_H_ */
