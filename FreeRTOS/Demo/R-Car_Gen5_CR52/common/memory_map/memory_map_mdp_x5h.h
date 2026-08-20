/*************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation
*
* SPDX-License-Identifier: MIT
 *************************************************************************************************************/

#ifndef MEMORY_MAP_MDP_X5H_H
#define MEMORY_MAP_MDP_X5H_H

#include <stdint.h>
#include "rcar_utils.h"

typedef enum e_mem_attr {
    DEVICE_ATTR,
    RAM_ATTR,
    RAM_NOCACHE_ATTR,
    RAM_TEXT_ATTR,
    RAM_RO_ATTR,
    SRAM_ATTR,
    FLASH_ATTR,
    TCM_ATTR
} e_mem_attr_t;

typedef struct st_memory_region {
    e_memory_type_t type;
    st_memory_t mem_addr;
    e_mem_attr_t attr;
} st_memory_region_t;

// Define all peripheral address regions
#define PERIPHERAL_START_0          0x18800000
#define PERIPHERAL_SIZE_0           0x1FA80000  // to 0x3828_0000

#define OSAL_MEMORY_ADDRESS         0x6B800000
#define OSAL_MEMORY_SIZE            0x1D600000  // to 0x88E0_0000

#define LDR_MEMORY_ADDRESS          0x8E200000 
#define LDR_MEMORY_SIZE             0x00100000  // to 0x8E30_0000

#define SHARE_CR_CA_ADDRESS         0x8E600000
#define SHARE_CR_CA_SIZE            0x10000000  // to 0x9E60_0000

#define UCIE_SHARE_ADDRESS          0x9E600000
#define UCIE_SHARE_SIZE             0x04000000  // to 0xA2600000

#define LINUX_CMA_ADDRESS_0         0xa2600000
#define LINUX_CMA_SIZE_0            0x1DA00000  // to 0xC000_0000

#define PERIPHERAL_START_1          0xC0000000
#define PERIPHERAL_SIZE_1           0x40000000  // to 0x1_0000_0000

#define MEM_REGION_NUM  7

static const st_memory_region_t RCAR_MEMMORY_ARR[MEM_REGION_NUM] = {
    {.type = OSAL,          .mem_addr = {.base_address = (uint32_t) OSAL_MEMORY_ADDRESS, .size = (uint32_t) OSAL_MEMORY_SIZE},    .attr = RAM_ATTR},
    {.type = SHARE_MEM,     .mem_addr = {.base_address = (uint32_t) LDR_MEMORY_ADDRESS,  .size = (uint32_t) LDR_MEMORY_SIZE},     .attr = RAM_NOCACHE_ATTR},
    {.type = SHARE_MEM,     .mem_addr = {.base_address = (uint32_t) LINUX_CMA_ADDRESS_0, .size = (uint32_t) LINUX_CMA_SIZE_0},    .attr = RAM_NOCACHE_ATTR},
    {.type = SHARE_MEM,     .mem_addr = {.base_address = (uint32_t) SHARE_CR_CA_ADDRESS, .size = (uint32_t) SHARE_CR_CA_SIZE},    .attr = RAM_NOCACHE_ATTR},
    {.type = PERIPHERAL,    .mem_addr = {.base_address = (uint32_t) PERIPHERAL_START_0,  .size = (uint32_t) PERIPHERAL_SIZE_0},   .attr = DEVICE_ATTR},
    {.type = PERIPHERAL,    .mem_addr = {.base_address = (uint32_t) PERIPHERAL_START_1,  .size = (uint32_t) PERIPHERAL_SIZE_1},   .attr = DEVICE_ATTR},
    {.type = SHARE_MEM,     .mem_addr = {.base_address = (uint32_t) UCIE_SHARE_ADDRESS,  .size = (uint32_t) UCIE_SHARE_SIZE},     .attr = DEVICE_ATTR}
};

#endif // MEMORY_MAP_MDP_X5H_H

