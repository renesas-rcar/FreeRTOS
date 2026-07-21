/*
 *
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 */

#include "smmu_private.h"
#include "translation_table.h"
#include "smmu/smmu.h"
#include "stdio.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <stdbool.h>
#include "cmsis_rcar_gen5.h"
#include "FreeRTOS.h"
#include "rcar_utils.h"

/**
 * @brief Array storing base addresses for each SMMU domain.
 */
static const uintptr_t smmu_base_addresses[] = {
    [SMMU_DSP]      = 0xFA000000,
    [SMMU_HCN]      = 0xF8800000,
    [SMMU_HCS0]     = 0xFCC00000,
    [SMMU_HCS1]     = 0xFCE00000,
    [SMMU_IMN]      = 0xF8000000,
    [SMMU_IMS]      = 0xFC000000,
    [SMMU_MP]       = 0xFE000000,
    [SMMU_NPU0]     = 0xF8400000,
    [SMMU_NPU1]     = 0xFC400000,
    [SMMU_PERE]     = 0xFD000000,
    [SMMU_PERW]     = 0xFD800000,
    [SMMU_PV]       = 0xF8600000,
    [SMMU_RT]       = 0xFB000000,
    [SMMU_SCP]      = 0xFA800000,
    [SMMU_VI0]      = 0xF9000000,
    [SMMU_VI1]      = 0xF9800000,
    [SMMU_VIPN]     = 0xF8200000,
    [SMMU_VIPS]     = 0xFC200000,
    [SMMU_DSP_FMU]  = 0xCBF60000,
    [SMMU_HCN_FMU]  = 0xC9CB0000,
    [SMMU_HCS0_FMU] = 0xDE860000,
    [SMMU_HCS1_FMU] = 0xDE870000,
    [SMMU_IMN_FMU]  = 0xC1980000,
    [SMMU_IMS_FMU]  = 0xC1D80000,
    [SMMU_MP_FMU]   = 0xED600000,
    [SMMU_NPU0_FMU] = 0xD2F60000,
    [SMMU_NPU1_FMU] = 0xD6F60000,
    [SMMU_PERE_FMU] = 0xC09E0000,
    [SMMU_PERW_FMU] = 0xC05B0000,
    [SMMU_PV_FMU]   = 0xCB500000,
    [SMMU_RT_FMU]   = 0x19450000,
    [SMMU_SCP_FMU]  = 0xC1180000,
    [SMMU_VI0_FMU]  = 0xC5840000,
    [SMMU_VI1_FMU]  = 0xC5850000,
    [SMMU_VIPN_FMU] = 0xC3130000,
    [SMMU_VIPS_FMU] = 0xC3530000,
};

/*Default configured values */
#define SMMU_QUEUE_SIZE     ((1U << SMMU_QUEUE_LOG2SIZE) * 16U)

#define GENMASK(h, l)  (((~0UL) - (1UL << (l)) + 1) & \
                       (~0UL >> (BITS_PER_LONG - 1 - (h))))

#define SMMU_CR0_OFFSET                 0x20
#define CR0_ATSCHK                      (1 << 4)
#define CR0_CMDQEN                      (1 << 3)
#define CR0_EVTQEN                      (1 << 2)
#define CR0_PRIQEN                      (1 << 1)
#define CR0_SMMUEN                      (1 << 0)

#define SMMU_CR0ACK_OFFSET              0x24

#define SMMU_STRTAB_OFFSET              0x80
#define STRTAB_BASE_RA                  (1UL << 62)
#define STRTAB_BASE_ADDR_MASK           GENMASK_ULL(51, 6)

#define SMMU_STRTAB_CFG_OFFSET          0x88

#define SMMU_CMDQ_BASE_OFFSET           0x90
#define SMMU_CMDQ_PROD_OFFSET           0x98
#define SMMU_CMDQ_CONS_OFFSET           0x9C

#define SMMU_EVTQ_BASE_OFFSET           0xA0
#define SMMU_EVTQ_PROD_OFFSET           0xA8
#define SMMU_EVTQ_CONS_OFFSET           0xAC

#define SMMU_SECURE_REGION_OFFSET       0x8000

#define STRTAB_LOG2SIZE         20
#define STRTAB_SPLIT            8
#define STRTAB_FMT_2LV          1
#define L1STE_MASK              0xFFF00
#define L2STE_MASK              0xFF

#define TIME_OUT    1000000

/**
 * @brief Initializes the SMMU.
 */

static smmu_l1ste_tbl_t* smmu_alloc_l1ste(e_smmu_domain_t smmu_domain, bool is_secure);
static st_smmu_cd_t* smmu_init_cd_table(st_smmu_streamid_instance_ctrl_t *p_ctrl);

static inline void write32(uintptr_t addr, uint32_t value);
static void smmu_init_cmdq(e_smmu_domain_t smmu_domain, bool is_secure);
static void smmu_init_evtq(e_smmu_domain_t smmu_domain, bool is_secure);
static int smmu_write_cmd(st_smmu_cmdq_t * cmdq, st_smmu_cmd_t *cmd);

static void cmdq_write(uint8_t * dest, st_smmu_cmd_t * cmd);
static bool cmdq_has_space(st_smmu_cmdq_t cmdq);
static bool cmdq_consumed(st_smmu_cmdq_t cmdq, uint32_t cur_rd, uint8_t cur_wrap);

static void smmu_disable(e_smmu_domain_t smmu_domain, bool is_secure);
static bool is_smmu_initialized(e_smmu_domain_t smmu_domain, bool is_secure);

int R_SMMU_Init(e_smmu_domain_t smmu_domain, bool is_secure) {

    if (is_smmu_initialized(smmu_domain, is_secure) == true)
    {
        return 0;
    }
    
    volatile st_smmu_cr0_t *reg_cr0;
    volatile st_smmu_cr0_t *reg_cr0ack;
    volatile st_smmu_strtab_cfg_t *strtab_cfg;
    void *l1ste_tbl;

    int res = -1;
    uint32_t count = 0;
    
    uint32_t base = smmu_base_addresses[smmu_domain];
    if (is_secure) {
        base += SMMU_SECURE_REGION_OFFSET;
    }

    reg_cr0 = (st_smmu_cr0_t *)(base + SMMU_CR0_OFFSET);
    reg_cr0ack = (st_smmu_cr0_t *)(base + SMMU_CR0ACK_OFFSET);
    strtab_cfg = (st_smmu_strtab_cfg_t *)(base + SMMU_STRTAB_CFG_OFFSET);

    strtab_cfg->FMT = STRTAB_FMT_2LV;
    strtab_cfg->SPLIT = STRTAB_SPLIT;
    strtab_cfg->LOG2SIZE = STRTAB_LOG2SIZE;

    l1ste_tbl = smmu_alloc_l1ste(smmu_domain, is_secure);
    if(l1ste_tbl == NULL){
        return res;
    }
    
    smmu_init_cmdq(smmu_domain, is_secure);
    reg_cr0->CMDQEN = ENABLE;
    
    while (reg_cr0ack->CMDQEN != ENABLE && count < TIME_OUT) {
        count++;
    }

    smmu_init_evtq(smmu_domain, is_secure);
    reg_cr0->EVENTQEN = ENABLE;
    
    count = 0;
    while (reg_cr0ack->EVENTQEN != ENABLE && count < TIME_OUT) {
        count++;
    }

    if ((reg_cr0ack->EVENTQEN != ENABLE) || (reg_cr0ack->CMDQEN != ENABLE)){
        return res;
    }

    return 0;
}

void R_SMMU_Deinit(e_smmu_domain_t smmu_domain, bool is_secure) {
    volatile st_smmu_cmdq_base_t *cmdq_base;
    volatile st_smmu_eventq_base_t *eventq_base;
    volatile st_smmu_strtab_t *strtab_base;
    volatile st_smmu_strtab_cfg_t *strtab_cfg;
    volatile st_smmu_cr0_t *reg_cr0;

    smmu_l1ste_tbl_t *l1ste_tbl;
    uint8_t l1ste_bits;
    uint8_t max_l1ste_bits;
    
    uint32_t base = smmu_base_addresses[smmu_domain];
    if (is_secure) {
        base += SMMU_SECURE_REGION_OFFSET;
    }

    cmdq_base = (st_smmu_cmdq_base_t *)(base + SMMU_CMDQ_BASE_OFFSET);
    eventq_base = (st_smmu_eventq_base_t *)(base + SMMU_EVTQ_BASE_OFFSET);
    strtab_base = (st_smmu_strtab_t *)(base + SMMU_STRTAB_OFFSET);
    strtab_cfg = (st_smmu_strtab_cfg_t *)(base + SMMU_STRTAB_CFG_OFFSET);
    reg_cr0 = (st_smmu_cr0_t *)(base + SMMU_CR0_OFFSET);

    l1ste_tbl = (smmu_l1ste_tbl_t *)(uintptr_t)(strtab_base->ADDR << 6); 
    l1ste_bits = strtab_cfg->LOG2SIZE - strtab_cfg->SPLIT;
    max_l1ste_bits = l1ste_bits < MAX_L1STE_BITS ? l1ste_bits : MAX_L1STE_BITS;

    for (uint16_t i = 0U; i < ((uint16_t)1 << max_l1ste_bits); i++) {
        if ((void*)(uintptr_t)((l1ste_tbl + i)->l2tbl_base << 6) != NULL) {
            aligned_free((void*)(uintptr_t)((l1ste_tbl + i)->l2tbl_base << 6));
        }
    }
    
    aligned_free(l1ste_tbl);
    
    aligned_free((void*)(uintptr_t)(cmdq_base->ADDR << 5));
    reg_cr0->CMDQEN = DISABLE;

    aligned_free((void*)(uintptr_t)(eventq_base->ADDR << 5));
    reg_cr0->EVENTQEN = DISABLE;

    smmu_disable(smmu_domain, is_secure);
}

int R_SMMU_Attach(st_smmu_streamid_instance_ctrl_t *p_ctrl) {
    void *res;
    if (!p_ctrl)
    {
        return -1; // Invalid input, return immediately
    }

    res = smmu_init_cd_table(p_ctrl);
    if (res == NULL) {
        return -1;
    }
    return 0;
}

void R_SMMU_Detach(st_smmu_streamid_instance_ctrl_t *p_ctrl) {
    volatile st_smmu_strtab_t *smmu_strtab ;
    volatile st_smmu_strtab_cfg_t *smmu_strtab_cfg ;
    
    uint32_t base;
    uint32_t l1ste_idx;
    uint16_t l2ste_idx;
    
    smmu_l1ste_tbl_t *l1ste_tbl;
    st_smmu_ste_t *l2ste_tbl;
    st_smmu_cd_t *cd_tbl;
    uint8_t split;
    uint8_t log2size;
    uint32_t stream_id;

    if (!p_ctrl)
    {
        return; // Invalid input, return immediately
    }
    
    base = smmu_base_addresses[p_ctrl->smmu_domain];
    if (p_ctrl->is_secure) {
        base += SMMU_SECURE_REGION_OFFSET;
    }

    smmu_strtab = (st_smmu_strtab_t *)(base + SMMU_STRTAB_OFFSET);
    smmu_strtab_cfg = (st_smmu_strtab_cfg_t *)(base + SMMU_STRTAB_CFG_OFFSET);
    
    stream_id = p_ctrl->stream_id;
    split = smmu_strtab_cfg->SPLIT;
    log2size = smmu_strtab_cfg->LOG2SIZE;
    l1ste_idx = (stream_id & L1STE_MASK) >> STRTAB_SPLIT;
    l2ste_idx = stream_id & L2STE_MASK;

    l1ste_tbl = (smmu_l1ste_tbl_t*)(uintptr_t)(smmu_strtab->ADDR << 6);
    l2ste_tbl = (st_smmu_ste_t*)(uintptr_t)((l1ste_tbl + l1ste_idx)->l2tbl_base << 6);
    cd_tbl = (st_smmu_cd_t*)(uintptr_t)((l2ste_tbl + l2ste_idx)->s1cdptr << 6);

    cd_tbl->valid = 0;
    aligned_free(cd_tbl);

    (l2ste_tbl + l2ste_idx)->valid = 0;
    __DSB();
}

e_smmu_map_fault_code_t R_SMMU_Map(st_smmu_streamid_instance_ctrl_t *p_ctrl,
                        uint64_t va, uint64_t pa, uint64_t size, uint64_t attr) {
    e_smmu_map_fault_code_t ret;
    volatile st_smmu_strtab_t *smmu_strtab ;
    volatile st_smmu_strtab_cfg_t *smmu_strtab_cfg ;
    uint32_t base;
    uint32_t l1ste_idx;
    uint16_t l2ste_idx;
    smmu_l1ste_tbl_t *l1ste_tbl;
    st_smmu_ste_t *l2ste_tbl;
    st_smmu_cd_t *cd_tbl;
    uint64_t *ttb0;
    uint8_t split;
    uint8_t log2size;
    uint32_t stream_id;
    st_mm_region_t region_mem;

    if (!p_ctrl)
    {
        ret = MAP_ERR_NULL;
        return ret; // Invalid input, return immediately
    }

    base = smmu_base_addresses[p_ctrl->smmu_domain];
    if(p_ctrl->is_secure) {
        base += SMMU_SECURE_REGION_OFFSET;
    }
    smmu_strtab = (st_smmu_strtab_t *)(base + SMMU_STRTAB_OFFSET);
    smmu_strtab_cfg = (st_smmu_strtab_cfg_t *)(base + SMMU_STRTAB_CFG_OFFSET);
    
    stream_id = p_ctrl->stream_id;
    split = smmu_strtab_cfg->SPLIT;
    log2size = smmu_strtab_cfg->LOG2SIZE;
    l1ste_idx = (stream_id & L1STE_MASK) >> STRTAB_SPLIT;
    l2ste_idx = stream_id & L2STE_MASK;

    l1ste_tbl = (smmu_l1ste_tbl_t*)(uintptr_t)(smmu_strtab->ADDR << 6);
    l2ste_tbl = (st_smmu_ste_t*)(uintptr_t)((l1ste_tbl + l1ste_idx)->l2tbl_base << 6);
    cd_tbl = (st_smmu_cd_t*)(uintptr_t)((l2ste_tbl + l2ste_idx)->s1cdptr << 6);

    region_mem = (st_mm_region_t){va, pa, size, attr};

    ttb0 = (uint64_t*)(uintptr_t)(cd_tbl->ttb0_base << 4);
    ret = CreateTranslationTable(&ttb0, region_mem);
    cd_tbl->ttb0_base = ((uint64_t)(uintptr_t)ttb0) >> 4;
    
    __DSB();

    R_SMMU_InvalidateTLB(p_ctrl->smmu_domain, p_ctrl->is_secure);

    return ret;
}

void R_SMMU_Unmap(st_smmu_streamid_instance_ctrl_t *p_ctrl, uint64_t va, uint64_t pa, uint64_t size) {
    volatile st_smmu_strtab_t *smmu_strtab ;
    volatile st_smmu_strtab_cfg_t *smmu_strtab_cfg ;
    uint32_t base;
    uint32_t l1ste_idx;
    uint16_t l2ste_idx;
    smmu_l1ste_tbl_t *l1ste_tbl;
    st_smmu_ste_t *l2ste_tbl;
    st_smmu_cd_t *cd_tbl;
    uint8_t split;
    uint8_t log2size;
    uint32_t stream_id;

    if (!p_ctrl)
    {
        return; // Invalid input, return immediately
    }
    
    base = smmu_base_addresses[p_ctrl->smmu_domain];
    if (p_ctrl->is_secure) {
        base += SMMU_SECURE_REGION_OFFSET;
    }

    smmu_strtab = (st_smmu_strtab_t *)(base + SMMU_STRTAB_OFFSET);
    smmu_strtab_cfg = (st_smmu_strtab_cfg_t *)(base + SMMU_STRTAB_CFG_OFFSET);
    
    stream_id = p_ctrl->stream_id;
    split = smmu_strtab_cfg->SPLIT;
    log2size = smmu_strtab_cfg->LOG2SIZE;
    l1ste_idx = (stream_id & L1STE_MASK) >> STRTAB_SPLIT;
    l2ste_idx = stream_id & L2STE_MASK;

    l1ste_tbl = (smmu_l1ste_tbl_t*)(uintptr_t)(smmu_strtab->ADDR << 6);
    l2ste_tbl = (st_smmu_ste_t*)(uintptr_t)((l1ste_tbl + l1ste_idx)->l2tbl_base << 6);
    cd_tbl = (st_smmu_cd_t*)(uintptr_t)((l2ste_tbl + l2ste_idx)->s1cdptr << 6);

    freeMemoryRegion((uint64_t*)(uintptr_t)(cd_tbl->ttb0_base << 4), va, pa, size);
    __DSB();
    R_SMMU_InvalidateTLB(p_ctrl->smmu_domain, p_ctrl->is_secure);
}

/**
 * @brief Reads and processes events from the Event Queue (EVTQ).
 */
void R_SMMU_ProcessEventQueue(void) {
    // TODO: Implement EVTQ processing
}

/**
 * @brief Issues a TLB invalidation command.
 */
int R_SMMU_InvalidateTLB(e_smmu_domain_t smmu_domain, bool is_secure) {
    int ret = 0;
    st_smmu_cmd_t cmd_inv_tlb = {0};

    uint32_t cmd_inv_cfg[4] = {0x4, 0, 31, 0};
    if (is_secure) {
        cmd_inv_cfg[0] = 0x404;
    }

    R_SMMU_IssueCommand(smmu_domain, is_secure, (st_smmu_cmd_t*)cmd_inv_cfg, 0);
    if (is_secure == true)
    {
        cmd_inv_tlb.opcode = CMD_TLBI_NH_ALL;
    }
    else
    {
        cmd_inv_tlb.opcode = CMDQ_OP_TLBI_NSNH_ALL;
    }
    ret = R_SMMU_IssueCommand(smmu_domain, is_secure, &cmd_inv_tlb, 1);

    return ret;
}

/**
 * @brief Enables SMMU for translation.
 */
int R_SMMU_Enable(e_smmu_domain_t smmu_domain, bool is_secure)
{
    volatile st_smmu_cr0_t *smmu_cr0ack;
    volatile st_smmu_cr0_t *reg_cr0;
    uint32_t count = 0;
    
    uint32_t base = smmu_base_addresses[smmu_domain];
    if (is_secure) {
        base += SMMU_SECURE_REGION_OFFSET;
    }

    smmu_cr0ack = (st_smmu_cr0_t*)(base + SMMU_CR0ACK_OFFSET);
    reg_cr0 = (st_smmu_cr0_t *)(base + SMMU_CR0_OFFSET);

    reg_cr0->SMMUEN = ENABLE;

    while (smmu_cr0ack->SMMUEN != ENABLE && count < TIME_OUT) {
        count++;
    }
    if (smmu_cr0ack->SMMUEN == ENABLE) {
        return 0;
    } else {
        return -1;
    }
}

/**
 * @brief Disables SMMU.
 */
static void smmu_disable(e_smmu_domain_t smmu_domain, bool is_secure)
{
    volatile st_smmu_cr0_t *reg_cr0;
    uint32_t base = smmu_base_addresses[smmu_domain];
    if (is_secure) {
        base += SMMU_SECURE_REGION_OFFSET;
    }

    reg_cr0 = (st_smmu_cr0_t *)(base + SMMU_CR0_OFFSET);
    reg_cr0->SMMUEN = DISABLE;
}

static bool is_smmu_initialized(e_smmu_domain_t smmu_domain, bool is_secure)
{
    volatile st_smmu_strtab_t *smmu_strtab;
    bool ret = false;
    uint32_t base = smmu_base_addresses[smmu_domain];
    if (is_secure) {
        base += SMMU_SECURE_REGION_OFFSET;
    }
    
    smmu_strtab = (st_smmu_strtab_t *)(base + SMMU_STRTAB_OFFSET);

    if(smmu_strtab->ADDR != 0)
    {
        ret = true;
    }

    return ret;
}

static inline void write32(uintptr_t addr, uint32_t value) {
    *(volatile uint32_t *)addr = value;
}

static smmu_l1ste_tbl_t* smmu_alloc_l1ste(e_smmu_domain_t smmu_domain, bool is_secure) {
    volatile st_smmu_strtab_t *smmu_strtab;
    volatile st_smmu_strtab_cfg_t *smmu_strtab_cfg;
    uint32_t num_l1_entry;
    smmu_l1ste_tbl_t *l1ste_tbl;
    uint8_t log2size;
    uint8_t split;
    uint32_t l1ste_size;
    uint32_t base = smmu_base_addresses[smmu_domain];
    if (is_secure) {
        base += SMMU_SECURE_REGION_OFFSET;
    }
    
    smmu_strtab = (st_smmu_strtab_t *)(base + SMMU_STRTAB_OFFSET);
    smmu_strtab_cfg = (st_smmu_strtab_cfg_t *)(base + SMMU_STRTAB_CFG_OFFSET);
    
    log2size = smmu_strtab_cfg->LOG2SIZE;
    split = smmu_strtab_cfg->SPLIT;
    num_l1_entry = ((uint32_t)1 << ((uint32_t)log2size - (uint32_t)split));
    num_l1_entry = num_l1_entry <= MAX_L1STE_ENTRY ? num_l1_entry : MAX_L1STE_ENTRY;
    
    l1ste_size = num_l1_entry*sizeof(smmu_l1ste_tbl_t);
    l1ste_tbl = aligned_malloc(1U << (log2size - split + 3U), l1ste_size);
    if (l1ste_tbl == NULL)  {
        printf("Allocate l1ste fail\n");
        return NULL;
    }

    memset(l1ste_tbl, 0, l1ste_size);

    smmu_strtab->ADDR = (uintptr_t)l1ste_tbl >> 6;
    __DSB();
    return l1ste_tbl;
}

static st_smmu_ste_t* smmu_alloc_l2ste(e_smmu_domain_t smmu_domain, bool is_secure, uint32_t l1ste_idx) {
    volatile st_smmu_strtab_t *smmu_strtab;
    volatile st_smmu_strtab_cfg_t *smmu_strtab_cfg;
    uintptr_t l1ste_base;
    smmu_l1ste_tbl_t *l1ste_tbl;
    uint32_t l2_tbl_size;
    st_smmu_ste_t *l2ste_tbl;
    uint8_t split; 
    
    uint32_t base = smmu_base_addresses[smmu_domain];
    if (is_secure) {
        base += SMMU_SECURE_REGION_OFFSET;
    }

    smmu_strtab = (st_smmu_strtab_t *)(base + SMMU_STRTAB_OFFSET);
    smmu_strtab_cfg = (st_smmu_strtab_cfg_t *)(base + SMMU_STRTAB_CFG_OFFSET);

    l1ste_base = (smmu_strtab->ADDR) << 6;
    l1ste_tbl = (smmu_l1ste_tbl_t*)l1ste_base;

    if(l1ste_tbl == NULL) {
        l1ste_tbl = smmu_alloc_l1ste(smmu_domain, is_secure);
        if(l1ste_tbl == NULL) {
            return NULL;
        }
    }
    
    split = smmu_strtab_cfg->SPLIT;
    l2_tbl_size = (1U << split)*sizeof(st_smmu_ste_t);
    l2ste_tbl = aligned_malloc(1U << (6U + split), l2_tbl_size);
    if (l2ste_tbl == NULL) {
        printf("Allocate l2ste fail\n");
        return NULL;
    }
    
    memset(l2ste_tbl, 0, l2_tbl_size);

    (l1ste_tbl + l1ste_idx)->l2tbl_base = (uintptr_t)l2ste_tbl >> 6;
    (l1ste_tbl + l1ste_idx)->span = split + 1;
    __DSB();
    return l2ste_tbl;
}

static st_smmu_ste_t* smmu_init_ste(st_smmu_streamid_instance_ctrl_t *p_ctrl){
    e_smmu_domain_t smmu_domain = p_ctrl->smmu_domain;
    bool is_secure = p_ctrl->is_secure;
    uint32_t stream_id = p_ctrl->stream_id;

    volatile st_smmu_strtab_t *smmu_strtab;
    volatile st_smmu_strtab_cfg_t *smmu_strtab_cfg;
    uint32_t stream_id_bits;
    uint32_t l1ste_idx;
    uint16_t l2ste_idx;
    uintptr_t l1ste_base;
    smmu_l1ste_tbl_t *l1ste_tbl;
    uintptr_t l2ste_base;
    st_smmu_ste_t *l2ste_tbl;
    uint8_t split;
    uint8_t log2size;

    uint32_t base = smmu_base_addresses[smmu_domain];
    if (is_secure) {
        base += SMMU_SECURE_REGION_OFFSET;
    }

    smmu_strtab = (st_smmu_strtab_t *)(base + SMMU_STRTAB_OFFSET);
    smmu_strtab_cfg = (st_smmu_strtab_cfg_t *)(base + SMMU_STRTAB_CFG_OFFSET);
    
    log2size = smmu_strtab_cfg->LOG2SIZE;
    split = smmu_strtab_cfg->SPLIT;

    stream_id_bits = MAX_L1STE_BITS + split;
    stream_id_bits = log2size < stream_id_bits ? log2size : stream_id_bits;
    if (stream_id > (((uint32_t)1 << stream_id_bits) - 1U)) {
        printf("Fail to init stream table entry. Stream id is too large\n");
        return NULL;
    }

    l1ste_idx = (stream_id & L1STE_MASK) >> STRTAB_SPLIT;
    l2ste_idx = stream_id & L2STE_MASK;

    l1ste_base = smmu_strtab->ADDR << 6;
    l1ste_tbl = (smmu_l1ste_tbl_t*)l1ste_base;
    if (l1ste_tbl == NULL) {
        l1ste_tbl = smmu_alloc_l1ste(smmu_domain, is_secure);
        if(l1ste_tbl == NULL){
            return NULL;
        }
    }

    l2ste_base = (l1ste_tbl + l1ste_idx)->l2tbl_base << 6;
    l2ste_tbl = (st_smmu_ste_t*)l2ste_base;
    if(l2ste_tbl == NULL) {
        l2ste_tbl = smmu_alloc_l2ste(smmu_domain, is_secure, l1ste_idx);
        if(l2ste_tbl == NULL) {
            return NULL;
        }
    }

    (l2ste_tbl + l2ste_idx)->valid = STRTAB_STE_V;
    (l2ste_tbl + l2ste_idx)->config = STRTAB_STE_CFG_S1_TRANS;
    (l2ste_tbl + l2ste_idx)->dre = STRTAB_STE_DRE_EN;

    __DSB();

    return l2ste_tbl + l2ste_idx;
}

static st_smmu_cd_t* smmu_init_cd_table(st_smmu_streamid_instance_ctrl_t *p_ctrl) {
    e_smmu_domain_t smmu_domain = p_ctrl->smmu_domain;
    uint32_t stream_id = p_ctrl->stream_id;

    st_smmu_ste_t *l2ste_ptr;
    st_smmu_cd_t *cd_tbl;
    
    l2ste_ptr = smmu_init_ste(p_ctrl);
    if (l2ste_ptr == NULL) {
        return NULL;
    }

    cd_tbl = aligned_malloc(1U << 6, sizeof(st_smmu_cd_t));
    if (cd_tbl == NULL) {
        printf("Allocate cd table fail\n");
        return NULL;
    }
    
    memset(cd_tbl, 0, sizeof(st_smmu_cd_t));

    cd_tbl->t0sz = 16;
    cd_tbl->ir0 = CTXDESC_CD_IR_RAWAWB;
    cd_tbl->or0 = CTXDESC_CD_OR_RAWAWB;
    cd_tbl->epd1 = CTXDESC_CD_TCR_EPD1;
    cd_tbl->valid = CTXDESC_CD_TCR_EPD1;
    cd_tbl->ips = CTXDESC_CD_48BIT_IPA;
    cd_tbl->aa64 = CTXDESC_CD_AA64;
    cd_tbl->ars = (CTXDESC_CD_A << 2 | CTXDESC_CD_R << 1);
    cd_tbl->had0 = CTXDESC_CD_HAD0_DIS;
    cd_tbl->mair0 = MAIR0_ATTR;

    l2ste_ptr->s1cdptr = (uintptr_t)cd_tbl >> 6;

    __DSB();

    return cd_tbl;
}

int R_SMMU_IssueCommand(e_smmu_domain_t smmu_domain, bool is_secure, st_smmu_cmd_t *p_cmd, bool sync){
    st_smmu_cmdq_t cmdq;
    uint32_t base = smmu_base_addresses[smmu_domain];
    if (is_secure) {
        base += SMMU_SECURE_REGION_OFFSET;
    }

    cmdq.base_reg = (st_smmu_cmdq_base_t *)(base + SMMU_CMDQ_BASE_OFFSET);
    cmdq.cons_reg = (st_smmu_cmdq_cons_t *)(base + SMMU_CMDQ_CONS_OFFSET);
    cmdq.prod_reg = (st_smmu_cmdq_prod_t *)(base + SMMU_CMDQ_PROD_OFFSET);

    /* 1. Determine if there is space to insert commands */
    // Increase WR of CMDQ_PROD
    if ( !cmdq_has_space(cmdq)) {
        return - 1;
    }

    /*2. Write command into the queue */
    smmu_write_cmd(&cmdq, p_cmd);

    /* 5. If we are inserting a CMD_SYNC, we must wait for it to complete */
    if (sync) {
        // Get Read index before sending CMD_SYNC
        uint32_t cur_rd = cmdq.cons_reg->RD;
        uint8_t cur_wrap = cmdq.cons_reg->RD_WRAP;
        uint32_t count = 0;
        st_smmu_cmd_t cmd_sync = {0};
        cmd_sync.opcode = CMDQ_OP_CMD_SYNC;
        smmu_write_cmd(&(cmdq), &cmd_sync);
        
        // Pull until completion
        while (!cmdq_consumed(cmdq, cur_rd, cur_wrap)) {
            count++;
            if (count >= TIME_OUT) {
                return -1;
            }
        }
    }
    return 0; // Don't verify if command has completed
}

static void smmu_init_cmdq(e_smmu_domain_t smmu_domain, bool is_secure){
    volatile st_smmu_cmdq_t cmdq;
    uint64_t cmdq_ptr;
    uint32_t base = smmu_base_addresses[smmu_domain];
    if (is_secure) {
        base += SMMU_SECURE_REGION_OFFSET;
    }

    // Initialize CMDQ_BASE
    cmdq.base_reg = (st_smmu_cmdq_base_t *)(base + SMMU_CMDQ_BASE_OFFSET);
    cmdq_ptr = (uintptr_t)aligned_malloc(((uint32_t)1 << 12), SMMU_QUEUE_SIZE);
    cmdq.base_reg->ADDR = cmdq_ptr >> 5;
    cmdq.base_reg->LOG2SIZE = SMMU_QUEUE_LOG2SIZE;
    cmdq.base_reg->RA = 0;


    // Initialize CMDQ_PROD and CMDQ_CONS
    cmdq.prod_reg = (st_smmu_cmdq_prod_t *)(base + SMMU_CMDQ_PROD_OFFSET);
    cmdq.cons_reg = (st_smmu_cmdq_cons_t *)(base + SMMU_CMDQ_CONS_OFFSET);

    cmdq.prod_reg->WR = 0;
    cmdq.prod_reg->WR_WRAP = 0;

    cmdq.cons_reg->RD = 0;
    cmdq.cons_reg->RD_WRAP = 0;
}

static void smmu_init_evtq(e_smmu_domain_t smmu_domain, bool is_secure){
    volatile st_smmu_eventq_t evtq;
    uint64_t evtq_ptr;
    uint32_t base = smmu_base_addresses[smmu_domain];

    if (is_secure) {
        base += SMMU_SECURE_REGION_OFFSET;
    }
    // Initialize EVTQ_BASE
    evtq.base_reg = (st_smmu_eventq_base_t *)(base + SMMU_EVTQ_BASE_OFFSET);
    evtq_ptr = (uintptr_t)aligned_malloc(((uint32_t)1 << 12), SMMU_QUEUE_SIZE);
    evtq.base_reg->ADDR = evtq_ptr >> 5;
    evtq.base_reg->LOG2SIZE = SMMU_QUEUE_LOG2SIZE;
    evtq.base_reg->WA = 0;

    // Initialize EVTQ_PROD and EVTQ_CONS
    evtq.prod_reg = (st_smmu_eventq_prod_t *)(base + SMMU_EVTQ_PROD_OFFSET);
    evtq.cons_reg = (st_smmu_eventq_cons_t *)(base + SMMU_EVTQ_CONS_OFFSET);

    evtq.prod_reg->WR = 0;
    evtq.prod_reg->WR_WRAP = 0;

    evtq.cons_reg->RD = 0;
    evtq.cons_reg->RD_WRAP = 0;
}

static int smmu_write_cmd(st_smmu_cmdq_t * cmdq, st_smmu_cmd_t *cmd) {
    uint32_t q_index = cmdq->prod_reg->WR;       // Get current WR index
    uint8_t q_wrap = cmdq->prod_reg->WR_WRAP;
    uint16_t wr_mask = (uint16_t)(((uint32_t)1 << SMMU_QUEUE_LOG2SIZE) - 1U);

    // Get pointer to queue entry
    uint8_t *entry_addr = (uint8_t *)(uintptr_t)(cmdq->base_reg->ADDR << 5);
    entry_addr += 16 * q_index; // go to next entry

    // Write the command to queue
    cmdq_write(entry_addr, cmd);

    // Ensure memory ordering before updating producer register
    __DSB();

    // Increment producer index (WR)
    q_index++;
    if (q_index >= (1U << SMMU_QUEUE_LOG2SIZE)) {
        q_index = 0;
        q_wrap ^= 1; // Toggle wrap bit on wraparound
    }

    // Update WR and WR_WRAP at the same time
    *((volatile uint32_t *)cmdq->prod_reg) = (q_index & wr_mask) \
                                            | (q_wrap << SMMU_QUEUE_LOG2SIZE);
}

static void cmdq_write(uint8_t * dest, st_smmu_cmd_t * cmd)
{
    for (uint8_t i = 0; i < 16; i++) {
       dest[i] = *((uint8_t *)cmd + i);
    }
}

static bool cmdq_has_space(st_smmu_cmdq_t cmdq) {
    volatile st_smmu_cmdq_cons_t *cons = cmdq.cons_reg;
    volatile st_smmu_cmdq_prod_t *prod = cmdq.prod_reg;

    if ((cons->RD != prod->WR) || (prod->WR_WRAP == cons->RD_WRAP)) {
        return true;
    }
    return false;
}

static bool cmdq_consumed(st_smmu_cmdq_t cmdq, uint32_t cur_rd, uint8_t cur_wrap) {
     volatile st_smmu_cmdq_cons_t *cons = cmdq.cons_reg;

     if (cur_wrap == cons->RD_WRAP && cur_rd < cons->RD) {
         return true;
     }
     if (cur_wrap != cons->RD_WRAP && cur_rd > cons->RD) {
         return true;
    }
        return false;
}
