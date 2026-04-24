/*
 * Copyright (c) 2025 Renesas Electronics Corporation
 * SPDX-License-Identifier: MIT
 */

/**
 * @file rcar_utils.h
 * @brief Common utility APIs for memory, timer, cache, and CPU information
 *
 * This header provides utility functions commonly used across
 * R-Car-based software components, including:
 * - Memory region queries
 * - Generic timer access
 * - Cache maintenance operations
 * - CPU identification and cycle counting
 */

#ifndef RCAR_UTILS_H
#define RCAR_UTILS_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                              Memory Utilities                              */
/* ========================================================================== */

/**
 * @defgroup UTILS_Mem Memory Utilities
 * @brief APIs for querying system memory layout and regions
 *
 * These APIs provide information about different memory regions available
 * in the system such as peripheral memory, CMA, OSAL, shared memory, and TCM.
 *
 * @{
 */

/**
 * @typedef e_memory_type
 * @brief Types of memory regions available in the system
 */
typedef enum e_memory_type {
    PERIPHERAL,   /**< Peripheral address space */
    CMA,          /**< Contiguous Memory Allocator regions */
    OSAL,         /**< OS Abstraction Layer managed memory */
    SHARE_MEM,    /**< Shared memory regions */
    TCM           /**< Tightly Coupled Memory */
} e_memory_type_t;

/**
 * @typedef st_memory
 * @brief Memory region descriptor
 *
 * Represents a single memory region with a base address and size.
 */
typedef struct st_memory {
    uint32_t base_address; /**< Base physical address of the region */
    uint32_t size;         /**< Size of the region in bytes */
} st_memory_t;

/**
 * @brief Get information about a specific memory region
 *
 * Retrieves the base address and size of a memory region of the given type.
 *
 * @param[in] type        Memory type (@ref e_memory_type_t)
 * @param[in] region_idx  Index of the region (0-based)
 *
 * @return st_memory_t    Memory region information.
 *                       If the index is invalid, returned values are undefined.
 */
st_memory_t R_UTILS_GetMemoryRegionInfo(
    e_memory_type_t type,
    uint8_t region_idx
);

/**
 * @brief Get total number of regions for a memory type
 *
 * @param[in] type        Memory type (@ref e_memory_type_t)
 *
 * @return uint8_t        Number of regions of the given memory type
 */
uint8_t R_UTILS_GetTotalRegionOfMemory(e_memory_type_t type);

/** @} */ /* end of UTILS_Mem */

/* ========================================================================== */
/*                              Timer Utilities                               */
/* ========================================================================== */

/**
 * @defgroup UTILS_Timer Generic Timer Utilities
 * @brief
 * Provides access to the ARM Generic Timer counter and frequency. In R-Car X5H,
 * there is one Generic Timer accessible by the Cortex R52 CPU. The timer
 * operates at a frequency of 66,666,667 Hz. Each counter increment increases by
 * 16, which means the counter frequency is equal to 1066,666,667 Hz.
 * @{
 */

/**
 * @brief Get the current timer counter value
 *
 * @return uint64_t Current value of the generic timer counter
 */
uint64_t R_UTILS_GetTimerCounter(void);

/**
 * @brief Get the generic timer frequency
 *
 * @return uint32_t Timer frequency in Hertz
 */
uint32_t R_UTILS_GetTimerFrequency(void);

/** @} */ /* end of UTILS_Timer */

/* ========================================================================== */
/*                         Aligned Memory Utilities                            */
/* ========================================================================== */

/**
 * @brief Allocate aligned memory
 *
 * Allocates a memory block of the given size with the specified alignment.
 *
 * @param[in] align Alignment in bytes (must be power of two)
 * @param[in] size  Size of the allocation in bytes
 *
 * @return Pointer to allocated memory, or NULL on failure
 */
void *aligned_malloc(size_t align, size_t size);

/**
 * @brief Free memory allocated by aligned_malloc
 *
 * @param[in] ptr Pointer returned by aligned_malloc
 */
void aligned_free(void *ptr);

/* ========================================================================== */
/*                              Cache Utilities                               */
/* ========================================================================== */

/**
 * @defgroup UTILS_Cache Cache Utilities
 * @brief Data cache maintenance APIs
 *
 * Provides APIs to clean and invalidate data cache regions.
 *
 * @{
 */

/**
 * @brief Clean (flush) data cache for a memory range
 *
 * Writes back dirty cache lines to memory so that RAM contents are up to date.
 *
 * @param[in] addr Physical start address
 * @param[in] size Size of memory range in bytes
 */
void R_UTILS_FlushDCache(uint32_t addr, uint32_t size);

/**
 * @brief Invalidate data cache for a memory range
 *
 * Ensures that subsequent CPU reads fetch fresh data from memory.
 *
 * @param[in] addr Physical start address
 * @param[in] size Size of memory range in bytes
 */
void R_UTILS_InvalidateDCache(uint32_t addr, uint32_t size);

/**
 * @brief Invalidate entire data cache
 *
 * Invalidates all data cache lines for the current CPU.
 */
void R_UTILS_InvalidateDCacheAll(void);

/**
 * @brief Read memory safely after a DMA transfer
 *
 * Invalidates data cache for the given address range and performs a read
 * to ensure fresh data is visible to the CPU.
 *
 * @param[in] addr Address to read from
 * @param[in] size Size of the memory region to invalidate
 *
 * @return uint32_t Value read from memory
 */
uint32_t R_UTILS_ReadMemForDMA(void *addr, uint32_t size);

/** @} */ /* end of UTILS_Cache */

/* ========================================================================== */
/*                                CPU Utilities                               */
/* ========================================================================== */

/**
 * @defgroup UTILS_CPU CPU Utilities
 * @brief CPU information and performance utilities
 *
 * @{
 */

/**
 * @brief Get CPU cycle counter
 *
 * Returns the number of CPU cycles elapsed since the performance monitor
 * unit (PMU) was enabled.
 *
 * @return uint64_t CPU cycle count
 */
uint64_t R_UTILS_GetCPUCycles(void);

/**
 * @brief Get unique CPU identifier
 *
 * CPU ID is derived from the cluster ID and core ID.
 *
 * @return uint32_t Unique CPU ID, equals to:
 * ClusterID * CoresPerCluster + CPUIDInCluster
 */
uint32_t R_UTILS_GetCpuID(void);

/** @} */ /* end of UTILS_CPU */

#ifdef __cplusplus
}
#endif

#endif /* RCAR_UTILS_H */
