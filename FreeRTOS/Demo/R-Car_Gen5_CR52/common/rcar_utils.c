/*************************************************************************************************************
* Copyright (c) 2025 Renesas Electronics Corporation
*
* SPDX-License-Identifier: MIT
 *************************************************************************************************************/

#include "memory_map/memory_map.h"
#include "cmsis_rcar_gen5.h"
#include "drivers/timer/arm_generic_timer.h"
#include <stdlib.h>
#include <stddef.h>
#include "FreeRTOS.h"
#include "cmsis_cp15.h"

#define NUMBER_OF_CORES_IN_CLUSTER          4

st_memory_t R_UTILS_GetMemoryRegionInfo(e_memory_type_t type, uint8_t region_idx)
{   
    st_memory_t region = {.base_address = 0, .size = 0};

    uint8_t idx = 0;

    for (uint8_t i = 0; i < sizeof(RCAR_MEMMORY_ARR)/sizeof(st_memory_region_t); i++) {
        if (RCAR_MEMMORY_ARR[i].type == type) {
            if (idx == region_idx) {
                region.base_address = RCAR_MEMMORY_ARR[i].mem_addr.base_address;
                region.size = RCAR_MEMMORY_ARR[i].mem_addr.size;
                break;
            }
            idx ++;
        }
    }
   
    return region;
}

uint8_t R_UTILS_GetTotalRegionOfMemory(e_memory_type_t type)
{    
    uint8_t total_region = 0;
    
    for (uint8_t i = 0; i < sizeof(RCAR_MEMMORY_ARR)/sizeof(st_memory_region_t); i++) {
        if (RCAR_MEMMORY_ARR[i].type == type) {
            total_region++;
        }
    }

    return total_region;
}

uint64_t R_UTILS_GetTimerCounter(void)
{
    return CNTPCT_READ();
}

uint32_t R_UTILS_GetTimerFrequency(void)
{
    return CNTFRQ_READ();

}

uint32_t R_UTILS_GetCpuID(void)
{
    uint32_t cpuid = __get_MPIDR();
    cpuid = ((cpuid>>8) & 0xff) * NUMBER_OF_CORES_IN_CLUSTER + cpuid & 0xff;
    return cpuid;

}

#define offset_t uint32_t

#define PTR_OFFSET_SZ sizeof(uint32_t)

#define align_up(num, align)    (((num) + ((align) - 1)) & ~((align) - 1))

void * aligned_malloc(size_t align, size_t size)
{
    void *ptr = NULL;
    
    // configASSERT((align & (align - 1)) == 0);

    if(align && size)
    {
        uint32_t hdr_size = PTR_OFFSET_SZ + (align - 1);
        void *p = pvPortMalloc(size + hdr_size);

        if (p != NULL)
        {
            ptr = (void *) align_up(((uintptr_t)p + PTR_OFFSET_SZ), align);

            *((offset_t *)ptr - 1) = (offset_t)((uintptr_t)ptr - (uintptr_t)p);
        }
    }

    return ptr;
}

void aligned_free(void * ptr)
{
    if (ptr != NULL) {
        offset_t offset = *((offset_t *)ptr - 1);

        void * p = (void *)((uint8_t *)ptr - offset);
        vPortFree(p);
    }
}

void R_UTILS_FlushDCache(uint32_t addr, uint32_t size)
{
    L1C_CleanDCacheAddress(addr, size);
}

void R_UTILS_InvalidateDCache(uint32_t addr, uint32_t size)
{
   L1C_InvalidateDCacheAddress(addr, size);
}

void R_UTILS_InvalidateDCacheAll()
{
   L1C_InvalidateDCacheAll();
}

uint32_t R_UTILS_ReadMemForDMA(void *addr, uint32_t size)
{
    R_UTILS_InvalidateDCache((uint32_t)addr, size);
    return *(volatile uint32_t *)addr;
}

uint64_t R_UTILS_GetCPUCycles(void) {
    uint64_t value;
    __get_CP64(15, 0, value, 9);  // PMCCNTR
    return value;
}