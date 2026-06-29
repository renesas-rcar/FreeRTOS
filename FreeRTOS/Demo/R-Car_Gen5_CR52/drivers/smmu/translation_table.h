/*
 *
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef TRANSLATION_TABLE_H
#define TRANSLATION_TABLE_H

#include "smmu/smmu.h"

/*
 * Entry table definitions.
 *
 */
#define ENTRY_TYPE_MASK     (3 << 0)
#define ENTRY_TYPE_FAULT    (0 << 0)
#define ENTRY_TYPE_TABLE    (3 << 0)
#define ENTRY_TYPE_PAGE     (3 << 0)
#define ENTRY_TYPE_BLOCK    (1 << 0)

#define MAIR0_ATTR          (0xC080400)

typedef struct st_mm_region
{
    uint64_t virt_addr;
    uint64_t phys_addr;
    uint64_t mem_size;
    uint64_t mem_attrs;
} st_mm_region_t;

e_smmu_map_fault_code_t CreateTranslationTable(uint64_t **ttb, st_mm_region_t region_mem);
void freeMemoryRegion(uint64_t *ttb, uint64_t va, uint64_t pa, uint64_t size);

#endif /* TRANSLATION_TABLE_H */
