/*
 *
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef R_SMMU_PRIVATE_H_
#define R_SMMU_PRIVATE_H_

#include <stdint.h>
#include <string.h>
#include <stdbool.h>

// SMMU_CR0
typedef struct st_smmu_cr0
{
    uint32_t SMMUEN : 1;     /**< [0] Non-secure SMMU enable (0 = Bypass, 1 = Enable translation) */
    uint32_t PRIQEN : 1;     /**< [1] Enable PRI queue writes (0 = Disabled, 1 = Enabled) */
    uint32_t EVENTQEN : 1;   /**< [2] Enable Event queue writes (0 = Disabled, 1 = Enabled) */
    uint32_t CMDQEN : 1;     /**< [3] Enable Command queue processing (0 = Disabled, 1 = Enabled) */
    uint32_t ATSCHK : 1;     /**< [4] ATS behavior (0 = Fast mode, 1 = Safe mode) */
    uint32_t Reserved1 : 1;  /**< [5] Reserved, must be 0 */
    uint32_t VMW : 3;        /**< [8:6] VMID Wildcard Matching */
    uint32_t Reserved2 : 23; /**< [31:9] Reserved, must be 0 */
} st_smmu_cr0_t;

// SMMU_STRTAB_BASE
typedef struct st_smmu_strtab_base
{
    volatile uint64_t Reserved1 : 6;  /**< [5:0] Reserved, must be 0 */
    volatile uint64_t ADDR : 46;      /**< [51:6] Physical address of Stream Table base (aligned to 64 bytes) */
    volatile uint64_t Reserved2 : 10; /**< [61:52] Reserved, must be 0 */
    volatile uint64_t RA : 1;         /**< [62] Read Allocate hint (0 = No Read-Allocate, 1 = Read-Allocate) */
    volatile uint64_t Reserved3 : 1;  /**< [63] Reserved, must be 0 */
} st_smmu_strtab_t;

// SMMU_STRTAB_BASE_CFG
typedef struct st_smmu_strtab_base_cfg
{
    uint32_t LOG2SIZE : 6;   /**< [5:0] Log2 of Stream Table size (entries) */
    uint32_t SPLIT : 5;      /**< [10:6] StreamID split point for multi-level table */
    uint32_t Reserved1 : 5;  /**< [15:11] Reserved, must be 0 */
    uint32_t FMT : 2;        /**< [17:16] Format of Stream Table (00 = Linear, 01 = 2-Level) */
    uint32_t Reserved2 : 14; /**< [31:18] Reserved, must be 0 */
} st_smmu_strtab_cfg_t;

/**
 * @brief Level 1 Stream Table Descriptor structure in 2-level Stream table
 */
typedef struct st_smmu_l1ste_tbl
{
    uint64_t span : 5;        /**< [4:0] total STEs in Level 2 table */
    uint64_t res1 : 1;        /**< [5] Reserved */
    uint64_t l2tbl_base : 46; /**< [51:6] base address of level 2 table */
    uint64_t res2 : 12;       /**< [63:52] Reserved */
} smmu_l1ste_tbl_t;

/**
 * @brief Stream Table Entry Structure
 */
typedef struct st_smmu_ste
{
    uint64_t valid : 1;    /**< [0] validity of ste */
    uint64_t config : 3;   /**< [3:1] Stream configuration */
    uint64_t s1fmt : 2;    /**< [5:4] type of CD table is pointed */
    uint64_t s1cdptr : 46; /**< [51:6] base address of CD table */
    uint64_t res1 : 7;     /**< [58:52] Unknown */
    uint64_t s1cdmax : 5;  /**< [63:59] Number of CDs pointed to by S1ContextPtr */
    uint64_t s1dss : 2;    /**< [65:64] Default Substream */
    uint64_t s1cir : 2;    /**< [67:66] S1ContextPtr memory Inner Region attribute */
    uint64_t s1cor : 2;    /**< [69:68] S1ContextPtr memory Outer Region attribute */
    uint64_t s1csh : 2;    /**< [71:70] S1ContextPtr memory Shareability attribute */
    uint64_t s2hwu59 : 1;
    uint64_t s2hwu60 : 1;
    uint64_t s2hwu61 : 1;
    uint64_t s2hwu62 : 1;
    uint64_t dre : 1;         /**< [76]  Destructive Read Enable */
    uint64_t cont : 4;        /**< [80:77]  Contiguous Hint */
    uint64_t dcp : 1;         /**< [81] Directed Cache Prefetch */
    uint64_t ppar : 1;        /**< [82] PRI Page request Auto Responses */
    uint64_t mev : 1;         /**< [83] Merge Events arising from terminated transactions from this stream */
    uint64_t res_sw : 4;      /**< [87:84] Reserved */
    uint64_t res2 : 3;        /**< [90:88] Unknown */
    uint64_t s1stalld : 1;    /**< [91] Stage 1 Stall Disable */
    uint64_t eats : 2;        /**< [93:92] Enable PCIe ATS translation and traffic */
    uint64_t strw : 2;        /**< [95:94] StreamWorld control */
    uint64_t MemAttr : 4;     /**< [99:96] Memory Attributes */
    uint64_t mtcfg : 1;       /**< [100] Memory Type configuration */
    uint64_t alloccfg : 4;    /**< [104:101] Allocation hints override */
    uint64_t res3 : 3;        /**< [107:105] Unknown */
    uint64_t shcfg : 2;       /**< [109:108] Shareability override */
    uint64_t nscfg : 2;       /**< [111:110] Bypass NS attribute configuration */
    uint64_t privcfg : 2;     /**< [113:112] User/privileged attribute configuration */
    uint64_t instcfg : 2;     /**< [115:114] Inst/Data attribute configuration */
    uint64_t impdef1 : 12;    /**< [127:116] IMPLEMENTATION DEFINED per-stream configuration */
    uint64_t s2vmid : 16;     /**< [143:128]  Virtual Machine Identifier */
    uint64_t impdef2 : 16;    /**< [159:144] IMPLEMENTATION DEFINED */
    uint64_t s2t0sz : 6;      /**< [165:160]  Size of IPA input region covered by stage 2 translation table */
    uint64_t s2sl0 : 2;       /**< [167:166] Starting level of stage 2 translation table walk*/
    uint64_t s2ir0 : 2;       /**< [169:168] Inner region Cacheability for stage 2 translation table access */
    uint64_t s2or0 : 2;       /**< [171:170] Outer region Cacheability for stage 2 translation table access */
    uint64_t s2sh0 : 2;       /**< [173:172] Shareability for stage 2 translation table access */
    uint64_t s2tg : 2;        /**< [175:174] Stage 2 Translation Granule size */
    uint64_t s2ps : 3;        /**< [178:176] Physical address Size */
    uint64_t s2aa64 : 1;      /**< [179] Stage 2 translation table type 32(LPAE) or 64 */
    uint64_t s2endi : 1;      /**< [180] Stage 2 translation table endianness */
    uint64_t s2affd : 1;      /**< [181] Stage 2 Access Flag Fault Disable */
    uint64_t s2ptw : 1;       /**< [182] Protected Table Walk */
    uint64_t s2ha_hd : 2;     /**< [184:183] Hardware Translation Table Update of stage 2 Access/Dirty flags */
    uint64_t s2r_s : 2;       /**< [186:185] Stage 2 fault behavior: Record and Stall */
    uint64_t s2ttb_base : 48; /**< [243:196] Address of Translation Table base */
    uint64_t res4 : 12;       /**< Unknown */
    uint64_t impdef3 : 16;    /**< IMPLEMENTATION DEFINED */
    uint64_t res5 : 16;       /**< Unknown */
    uint32_t res6[7];         /**< Unknown */
} st_smmu_ste_t;

/**
 * @brief CD table define
 */
#define CTXDESC_CD_IR_RAWAWB    1
#define CTXDESC_CD_OR_RAWAWB    1
#define CTXDESC_CD_SH_ISH       3

#define CTXDESC_CD_TCR_EPD1     1

#define CTXDESC_CD_V            1

#define CTXDESC_CD_TCR_TBI0     1

#define CTXDESC_CD_48BIT_IPA    5
#define CTXDESC_CD_AA64         1
#define CTXDESC_CD_S            1
#define CTXDESC_CD_R            1U
#define CTXDESC_CD_A            1U
#define CTXDESC_CD_ASET         1
#define CTXDESC_CD_HAD0_DIS     1

/**
 * @brief Level 1 Context Descriptor structure in two-level Context descriptor tables
 */
typedef struct st_smmu_l1cd_tbl
{
    uint64_t valid : 1;       /**< [0] validity of CD */
    uint64_t res1 : 11;       /**< [11:1] Unknown */
    uint64_t l2tbl_base : 40; /**< [51:12] Base address of next-level table */
    uint64_t res2 : 12;       /**< [63:52] Unknown */
} st_smmu_l1cd_tbl_t;

/**
 * @brief Context Descriptor Structure
 */
typedef struct st_smmu_cd
{
    uint64_t t0sz : 6;       /**< [5:0] VA region size covered by TT0*/
    uint64_t tg0 : 2;        /**< [7:6] TT0 Translation Granule size */
    uint64_t ir0 : 2;        /**< [9:8] Inner region Cacheability for TT0 access */
    uint64_t or0 : 2;        /**< [11:10] Outer region Cacheability for TT0 access */
    uint64_t sh0 : 2;        /**< [13:12] Shareability for TT0 access */
    uint64_t epd0 : 1;       /**< [14] TT0 translation table walk disable*/
    uint64_t endi : 1;       /**< [15] Translation table endianness */
    uint64_t t1sz : 6;       /**< [21:16] VA region size covered by TT1 */
    uint64_t tg1 : 2;        /**< [23:22] TT1 Translation Granule size */
    uint64_t ir1 : 2;        /**< [25:24] Inner region Cacheability for TT1 access */
    uint64_t or1 : 2;        /**< [27:26] Outer region Cacheability for TT1 access */
    uint64_t sh1 : 2;        /**< [29:28] Shareability for TT1 access */
    uint64_t epd1 : 1;       /**< [30] TT1 translation table walk disable */
    uint64_t valid : 1;      /**< [31] validity of CD */
    uint64_t ips : 3;        /**< [34:32] Intermediate Physical Size */
    uint64_t affd : 1;       /**< [35] Access Flag Fault Disable */
    uint64_t wxn : 1;        /**< [36] Write eXecute Never */
    uint64_t uwxn : 1;       /**< [37] Unprivileged Write eXecute Never */
    uint64_t tbi0 : 1;       /**< [38] Top Byte Ignore for TTB0 */
    uint64_t tbi1 : 1;       /**< [39] Top Byte Ignore for TTB1 */
    uint64_t pan : 1;        /**< [40] Privileged Access Never */
    uint64_t aa64 : 1;       /**< [41] Translation table format */
    uint64_t hahd : 2;       /**< [43:42] Hardware Translation Table Update of Access/Dirty flags for TT0 and TT1*/
    uint64_t ars : 3;        /**< [46:44] Stage 1 fault behavior */
    uint64_t aset : 1;       /**< [47] ASID Set */
    uint64_t asid : 16;      /**< [63:48] Address Space Identifier */
    uint64_t nscfg0 : 1;     /**< [64] Non-secure attribute for the memory associated with TTB0 */
    uint64_t had0 : 1;       /**< [65] Hierarchical Attribute Disable for the TTB0 region */
    uint64_t res1 : 2;       /**< [67:66] Unknown */
    uint64_t ttb0_base : 48; /**< [115:68] Address of TT0 base */
    uint64_t res2 : 8;       /**< [123:116] Unknown */
    uint64_t hwu059 : 1;
    uint64_t hwu060 : 1;
    uint64_t hwu061 : 1;
    uint64_t hwu062 : 1;
    uint64_t nscfg1 : 1;     /**< [128] Non-secure attribute for the memory associated with TTB1 */
    uint64_t had1 : 1;       /**< [129] Hierarchical Attribute Disable for the TTB1 region */
    uint64_t res3 : 2;       /**< [131:130] Unknown */
    uint64_t ttb1_base : 48; /**< [179:132] Address of TT1 base */
    uint64_t res4 : 8;       /**< [187:180] Unknown */
    uint64_t hwu159 : 1;
    uint64_t hwuint16_t0 : 1;
    uint64_t hwuint16_t1 : 1;
    uint64_t hwuint16_t2 : 1;
    uint64_t mair0 : 32;  /**< [223:192] Memory Attribute TT0 */
    uint64_t mair1 : 32;  /**< [255:224] Memory Attribute TT1 */
    uint64_t amair0 : 32; /**< [287:256] Equivalent to PE Auxiliary Memory Attribute Indirection Registers */
    uint64_t amair1 : 32; /**< [319:288] */
    uint64_t impdf : 32;  /**< [351:320] IMPLEMENTATION DEFINED */
    uint32_t res5[5];     /**< Unknown */
} st_smmu_cd_t;

/**
 * @brief Stream table define
 */

#define STRTAB_STE_V                1

#define STRTAB_STE_CFG_ABORT        0
#define STRTAB_STE_CFG_BYPASS       4
#define STRTAB_STE_CFG_S1_TRANS     5

#define STRTAB_STE_S1FMT_LINEAR     0

#define STRTAB_STE_1_S1DSS_SSID0    2

#define STRTAB_STE_1_S1C_CACHE_WBRA 1
#define STRTAB_STE_1_S1C_SH_ISH     3

#define STRTAB_STE_1_S1STALLD       1

#define STRTAB_STE_1_STRW_EL2       2

#define STRTAB_STE_DRE_EN           1

#define MAX_L1STE_BITS     12
#define MAX_L1STE_ENTRY  (1U << MAX_L1STE_BITS)

/**
 * @brief Defines the log2 size of the SMMU queue.
 */
#define SMMU_QUEUE_LOG2SIZE 6

/**
 * @brief Structure for Command Queue base register.
 */
typedef struct st_smmu_cmdq_base
{
    uint64_t LOG2SIZE  : 5;   /**< [4:0] Log2 of queue size (entries) */
    uint64_t ADDR      : 47;      /**< [51:5] Physical address of Command Queue base (ignores [4:0]) */
    uint64_t Reserved1 : 10; /**< [61:52] Reserved */
    uint64_t RA        : 1;         /**< [62] Read Allocate hint */
    uint64_t Reserved2 : 1;  /**< [63] Reserved */
} st_smmu_cmdq_base_t;

/**
 * @brief Structure for Command Queue Consumer register.
 */
typedef struct st_smmu_cmdq_cons
{
    volatile uint32_t RD        : SMMU_QUEUE_LOG2SIZE;                       /**< [QS-1:0] Read index */
    volatile uint32_t RD_WRAP   : 1;                                    /**< [QS] Read index wrap flag */
    volatile uint32_t Reserved1 : (20 - SMMU_QUEUE_LOG2SIZE - 1 + 4); /**< [23:20] Reserved */
    volatile uint32_t ERR       : 7;                                        /**< [30:24] Error code */
    volatile uint32_t Reserved  : 1;                                   /**< [31] Reserved */
} st_smmu_cmdq_cons_t;

/**
 * @brief Structure for Command Queue Producer register.
 */
typedef struct smmu_cmdq_prod_st
{
    volatile uint32_t WR       : SMMU_QUEUE_LOG2SIZE;                  /**< [QS-1:0] Write index */
    volatile uint32_t WR_WRAP  : 1;                               /**< [QS] Write index wrap flag */
    volatile uint32_t Reserved : (32 - 1 - SMMU_QUEUE_LOG2SIZE); /**< [31:20] Reserved */
} st_smmu_cmdq_prod_t;

/**
 * @brief Structure for Event Queue base register.
 */
typedef struct st_smmu_eventq_base
{
    volatile uint64_t LOG2SIZE  : 5;   /**< [4:0] Log2 of queue size (entries) */
    volatile uint64_t ADDR      : 47;      /**< [51:5] Physical address of Event Queue base */
    volatile uint64_t Reserved1 : 10; /**< [61:52] Reserved */
    volatile uint64_t WA        : 1;         /**< [62] Write Allocate hint */
    volatile uint64_t Reserved2 : 1;  /**< [63] Reserved */
} st_smmu_eventq_base_t;

/**
 * @brief Structure for Event Queue Consumer register.
 */
typedef struct st_smmu_eventq_cons
{
    volatile uint32_t RD       : SMMU_QUEUE_LOG2SIZE;                       /**< [QS-1:0] Read index */
    volatile uint32_t RD_WRAP  : 1;                                    /**< [QS] Read index wrap flag */
    volatile uint32_t Reserved : (20 - SMMU_QUEUE_LOG2SIZE - 1 + 11); /**< [30:20] Reserved */
    volatile uint32_t OVACKFLG : 1;                                   /**< [31] Overflow acknowledge flag */
} st_smmu_eventq_cons_t;

/**
 * @brief Structure for Event Queue Producer register.
 */
typedef struct st_smmu_eventq_prod
{
    volatile uint32_t WR       : SMMU_QUEUE_LOG2SIZE;                       /**< [QS-1:0] Write index */
    volatile uint32_t WR_WRAP  : 1;                                    /**< [QS] Write index wrap flag */
    volatile uint32_t Reserved : (20 - SMMU_QUEUE_LOG2SIZE - 1 + 11); /**< [30:20] Reserved */
    volatile uint32_t OVFLG    : 1;                                      /**< [31] Overflow flag */
} st_smmu_eventq_prod_t;

/**
 * @brief Structure for SMMU Command Queue.
 */
typedef struct st_smmu_cmdq
{
    volatile st_smmu_cmdq_base_t *base_reg; /**< Base register */
    volatile st_smmu_cmdq_prod_t *prod_reg; /**< Producer register */
    volatile st_smmu_cmdq_cons_t *cons_reg; /**< Consumer register */
} st_smmu_cmdq_t;

/**
 * @brief Structure for SMMU Event Queue.
 */
typedef struct st_smmu_eventq
{
    volatile st_smmu_eventq_base_t *base_reg; /**< Base register */
    volatile st_smmu_eventq_prod_t *prod_reg; /**< Producer register */
    volatile st_smmu_eventq_cons_t *cons_reg; /**< Consumer register */
} st_smmu_eventq_t;

#endif // R_SMMU_PRIVATE_H
