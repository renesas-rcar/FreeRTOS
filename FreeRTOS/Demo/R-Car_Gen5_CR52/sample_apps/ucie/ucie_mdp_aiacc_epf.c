/*
 * FreeRTOS Kernel V11.1.0
 * Copyright (C) 2021 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://github.com/FreeRTOS
 */

#include "FreeRTOS.h"
#include "task.h"

#include "interrupts.h"
#include "stdio.h"
#include "string.h"
#include "ucie/r_ucie.h"
#include "ucie_common.h"
#include "smmu/smmu.h"
#include "pfc/r_pfc_api.h"
#include "device_tree.h"
#include "rcar_utils.h"

#define EPF_TEST_TASK_PRIORITY  (tskIDLE_PRIORITY + 1)
#define EPF_TEST_STACK_SIZE     (configMINIMAL_STACK_SIZE * 4)

#define EPF_REG_MAGIC           0x00U
#define EPF_REG_COMMAND         0x04U
#define EPF_REG_STATUS          0x08U
#define EPF_REG_LOWER_SRC_ADDR  0x0CU
#define EPF_REG_UPPER_SRC_ADDR  0x10U
#define EPF_REG_LOWER_DST_ADDR  0x14U
#define EPF_REG_UPPER_DST_ADDR  0x18U
#define EPF_REG_SIZE            0x1CU
#define EPF_REG_CHECKSUM        0x20U
#define EPF_REG_IRQ_TYPE        0x24U
#define EPF_REG_IRQ_NUMBER      0x28U
#define EPF_REG_FLAGS           0x2CU

#define EPF_TEST_MAGIC_VAL      0xFACEFEEDU

#define COMMAND_RAISE_LEGACY_IRQ    (1U << 0)
#define COMMAND_RAISE_MSI_IRQ       (1U << 1)
#define COMMAND_RAISE_MSIX_IRQ      (1U << 2)
#define COMMAND_READ                (1U << 3)
#define COMMAND_WRITE               (1U << 4)
#define COMMAND_COPY                (1U << 5)

#define FLAG_USE_DMA                (1U << 0)

#define STATUS_READ_SUCCESS     (1U << 0)
#define STATUS_READ_FAIL        (1U << 1)
#define STATUS_WRITE_SUCCESS    (1U << 2)
#define STATUS_WRITE_FAIL       (1U << 3)
#define STATUS_COPY_SUCCESS     (1U << 4)
#define STATUS_COPY_FAIL        (1U << 5)
#define STATUS_IRQ_RAISED       (1U << 6)
#define STATUS_SRC_ADDR_INVALID (1U << 7)
#define STATUS_DST_ADDR_INVALID (1U << 8)

#define EPF_TEST_REG_BASE       0x9A000000UL
#define EPF_TEST_REG_BAR        (4)
#define EPF_TEST_REG_BAR_RGN    (0)

#define EPF_RC_DATA_WIN_PA      (0x20000000000ULL + (uint64_t)EPF_UCIE_CHANNEL * 0x4000000000ULL)
#define EPF_RC_DATA_WIN_SIZE    (0x200000U)

#define EPF_HDMA_CHANNEL        HDMA_CH0

#define EPF_LOCAL_DATA_BASE     0xB0000000UL
#define EPF_MAX_XFER_SIZE       (1U * 1024U * 1024U)   /* 1 MB */

#define EPF_UCIE_CHANNEL        UCIE_CH0
#define EPF_UCIE_LINKSPEED      LINKSPEED_4GTPS

#define EPF_IATU_RGN_OUTBOUND   IATU_RGN0

#define EPF_IATU_ALIGN          (0xFFFULL)

#define EPF_PIO_WIN_VA          0x70000000UL
#define EPF_SMMU_SECURE         true
#define EPF_SMMU_NUM_SID        2
#define EPF_RCTBUBYPSEN_ADDR    0x18B41010U
#define EPF_RCTBUBYPSEN_MASK    0x00000003U

#define UCIE_EP_AXI_BASE        0xD8000000U
#define UCIE_ATU_INBOUND_RGN(n) (UCIE_EP_AXI_BASE + 0x300100U + (n) * 0x200U)

#define UCIE_ATU_EN             0x80000000U
#define UCIE_ATU_BAR_MODE       0x40000000U
#define UCIE_ATU_FUNC_NUM_MATCH 0x00080000U
#define UCIE_ATU_BAR_NUM(b)     (((b) & 0x7) << 8)


static inline uint32_t reg_rd(uint32_t base, uint32_t off)
{
    return *((volatile uint32_t *)(uintptr_t)(base + off));
}

static inline void reg_wr(uint32_t base, uint32_t off, uint32_t val)
{
    *((volatile uint32_t *)(uintptr_t)(base + off)) = val;
}

static void epf_set_inbound_bar(uint32_t rgn, uint32_t bar, uint32_t dst)
{
    uintptr_t base = UCIE_ATU_INBOUND_RGN(rgn);

    *(volatile uint32_t *)(base + 0x14) = dst;
    *(volatile uint32_t *)(base + 0x18) = 0x00000000;
    *(volatile uint32_t *)(base + 0x00) = 0x00000000;
    *(volatile uint32_t *)(base + 0x04) = UCIE_ATU_EN | UCIE_ATU_FUNC_NUM_MATCH |
                                          UCIE_ATU_BAR_MODE | UCIE_ATU_BAR_NUM(bar);

    printf("[EPF] iATU IB%u -> BAR%u at 0x%08X, ctrl2 0x%08X\n", rgn, bar, dst,
           *(volatile uint32_t *)(base + 0x04));
}

static int epf_setup_outbound(uint64_t rc_addr, uint32_t size, uint64_t *win)
{
    uint64_t page = rc_addr & ~EPF_IATU_ALIGN;
    uint32_t offset = (uint32_t)(rc_addr - page);
    uint32_t map_size = (offset + size + (uint32_t)EPF_IATU_ALIGN) &
                        ~(uint32_t)EPF_IATU_ALIGN;
    st_ucie_iatu_cfg_t cfg = {
        .ucie_ch   = EPF_UCIE_CHANNEL,
        .rgn       = EPF_IATU_RGN_OUTBOUND,
        .type      = IATU_OUTBOUND,
        .mSrcAddr  = EPF_RC_DATA_WIN_PA,
        .mDestAddr = page,
        .size      = map_size,
    };

    if (map_size > EPF_RC_DATA_WIN_SIZE) {
        printf("[EPF] RC buffer 0x%08X+%u exceeds the outbound window\n",
               (uint32_t)rc_addr, size);
        return -1;
    }

    if (R_UCIE_IATU_SetRegion(&cfg)) {
        printf("[EPF] failed to map RC 0x%08X\n", (uint32_t)rc_addr);
        return -1;
    }

    *win = EPF_RC_DATA_WIN_PA + offset;
    return 0;
}

static void epf_unset_outbound(void)
{
    st_ucie_iatu_cfg_t cfg = {
        .ucie_ch   = EPF_UCIE_CHANNEL,
        .rgn       = EPF_IATU_RGN_OUTBOUND,
        .type      = IATU_OUTBOUND,
        .mSrcAddr  = EPF_RC_DATA_WIN_PA,
        .mDestAddr = 0,
        .size      = 0,
    };

    R_UCIE_IATU_UnsetRegion(&cfg);
}

static bool g_pio_via_smmu = false;

static bool epf_pio_smmu_setup(void)
{
    static const uint32_t stream_id[2][EPF_SMMU_NUM_SID] = {
        { 0x00800U, 0x00900U },   /* core0 */
        { 0x00B00U, 0x00A00U },   /* core1 */
    };
    st_smmu_streamid_instance_ctrl_t ctrl = {
        .smmu_domain = SMMU_RT,
        .is_secure   = EPF_SMMU_SECURE,
    };
    uint32_t coreid = R_UTILS_GetCpuID();
    uint32_t win_end = (uint32_t)EPF_PIO_WIN_VA + (uint32_t)EPF_RC_DATA_WIN_SIZE;
    uint32_t rb = 0;
    uint32_t i;

    if (coreid >= 2U) {
        printf("[EPF] PIO: unexpected core id %u, staying on HDMA\n", coreid);
        return false;
    }

    R_SMMU_Init(SMMU_RT, EPF_SMMU_SECURE);
    R_SMMU_InvalidateTLB(SMMU_RT, EPF_SMMU_SECURE);

    for (i = 0; i < EPF_SMMU_NUM_SID; i++) {
        ctrl.stream_id = stream_id[coreid][i];
        if (R_SMMU_Attach(&ctrl) != 0) {
            printf("[EPF] PIO: attach SID 0x%X failed\n", ctrl.stream_id);
            return false;
        }

        R_SMMU_Map(&ctrl, 0x00000000UL, 0x00000000UL, 0x40000000UL,
                   ATTR_DEVICE_NGNRNE_EL1_RW_EL0_RW);
        R_SMMU_Map(&ctrl, 0x40000000UL, 0x40000000UL,
                   EPF_PIO_WIN_VA - 0x40000000UL,
                   ATTR_DEVICE_NGNRNE_EL1_RW_EL0_RW);
        R_SMMU_Map(&ctrl, EPF_PIO_WIN_VA, EPF_RC_DATA_WIN_PA,
                   EPF_RC_DATA_WIN_SIZE, ATTR_DEVICE_NGNRNE_EL1_RW_EL0_RW);
        R_SMMU_Map(&ctrl, win_end, win_end, 0x80000000UL - win_end,
                   ATTR_DEVICE_NGNRNE_EL1_RW_EL0_RW);
        R_SMMU_Map(&ctrl, 0x80000000UL, 0x80000000UL, 0x40000000UL,
                   ATTR_DEVICE_NGNRNE_EL1_RW_EL0_RW);
        R_SMMU_Map(&ctrl, 0xC0000000UL, 0xC0000000UL, 0x40000000UL,
                   ATTR_DEVICE_NGNRNE_EL1_RW_EL0_RW);
    }

    {
        volatile uint32_t *bypass = (volatile uint32_t *)EPF_RCTBUBYPSEN_ADDR;
        uint32_t clr = ~(1U << coreid) & EPF_RCTBUBYPSEN_MASK;

        *bypass = (*bypass & ~EPF_RCTBUBYPSEN_MASK) | clr;
        for (i = 0; i < 10000U; i++) {
            __asm__ volatile("nop");
        }
        rb = *bypass;
    }

    if (R_SMMU_Enable(SMMU_RT, EPF_SMMU_SECURE) != 0) {
        printf("[EPF] PIO: SMMU enable failed, using HDMA for data\n");
        return false;
    }

    if ((rb & (1U << coreid)) != 0U) {
        printf("[EPF] PIO: TBU bypass stuck on (RCTBUBYPSEN=0x%08X), "
               "using HDMA for data\n", rb);
        return false;
    }

    printf("[EPF] PIO: SMMU-RT up, VA 0x%08lX -> PA 0x%llX\n",
           (unsigned long)EPF_PIO_WIN_VA,
           (unsigned long long)EPF_RC_DATA_WIN_PA);
    return true;
}

static void epf_pio_copy(volatile void *dst, const volatile void *src, uint32_t n)
{
    volatile uint32_t *d32 = (volatile uint32_t *)dst;
    const volatile uint32_t *s32 = (const volatile uint32_t *)src;
    volatile uint8_t *d8;
    const volatile uint8_t *s8;

    if ((((uintptr_t)dst | (uintptr_t)src) & 3U) == 0U) {
        while (n >= 4U) {
            *d32++ = *s32++;
            n -= 4U;
        }
    }

    d8 = (volatile uint8_t *)d32;
    s8 = (const volatile uint8_t *)s32;
    while (n-- > 0U) {
        *d8++ = *s8++;
    }
}

static int epf_hdma_xfer(uint64_t src, uint64_t dst, uint32_t size,
                         e_ucie_hdma_mode_t rw);

static int epf_move(uint64_t rc_addr, uint64_t local, uint32_t size,
                    e_ucie_hdma_mode_t rw, uint32_t *status, uint64_t *ticks)
{
    uint32_t fail = (rw == HDMA_READ) ? STATUS_SRC_ADDR_INVALID
                                      : STATUS_DST_ADDR_INVALID;
    bool pio = g_pio_via_smmu &&
               !(reg_rd(EPF_TEST_REG_BASE, EPF_REG_FLAGS) & FLAG_USE_DMA);
    uint32_t done = 0;

    *ticks = 0;

    while (done < size) {
        uint32_t chunk = size - done;
        uint64_t win;
        uint64_t mark;
        int ret;

        if (epf_setup_outbound(rc_addr + done, chunk, &win)) {
            *status |= fail;
            return -1;
        }

        mark = R_UTILS_GetTimerCounter();

        if (pio) {
            uint32_t win_va = EPF_PIO_WIN_VA +
                              (uint32_t)(win - EPF_RC_DATA_WIN_PA);
            void *wv = (void *)(uintptr_t)win_va;
            void *lv = (void *)(uintptr_t)(local + done);

            if (rw == HDMA_READ) {
                R_UTILS_InvalidateDCache(win_va, chunk);
                epf_pio_copy(lv, wv, chunk);
            } else {
                epf_pio_copy(wv, lv, chunk);
                R_UTILS_FlushDCache(win_va, chunk);
                __asm__ volatile("dsb sy" ::: "memory");
                if (chunk >= 4U) {
                    (void)*(volatile uint32_t *)wv;
                }
            }
            ret = 0;
        } else {
            ret = (rw == HDMA_READ)
                ? epf_hdma_xfer(win, local + done, chunk, rw)
                : epf_hdma_xfer(local + done, win, chunk, rw);
        }

        *ticks += R_UTILS_GetTimerCounter() - mark;

        epf_unset_outbound();

        if (ret) {
            return -1;
        }

        done += chunk;
    }

    return 0;
}

static int epf_hdma_xfer(uint64_t src, uint64_t dst, uint32_t size,
                         e_ucie_hdma_mode_t rw)
{
    st_ucie_hdma_cfg_t cfg = {
        .ucie_ch   = EPF_UCIE_CHANNEL,
        .hdma_ch   = EPF_HDMA_CHANNEL,
        .mSrcAddr  = src,
        .mDestAddr = dst,
        .size      = size,
        .rw        = rw,
    };

    if (R_UCIE_HDMA_SetConfig(&cfg) || R_UCIE_HDMA_Start(&cfg)) {
        printf("[EPF] HDMA setup failed\n");
        return -1;
    }

    if (R_UCIE_HDMA_WaitStop(&cfg)) {
        printf("[EPF] HDMA transfer timeout\n");
        return -1;
    }

    return 0;
}

static uint32_t epf_crc32_le(uint32_t crc, const void *buf, uint32_t len)
{
    const uint8_t *p = (const uint8_t *)buf;

    while (len--) {
        uint32_t byte = *p++;
        int bit;

        for (bit = 0; bit < 8; bit++) {
            if ((crc ^ byte) & 1U)
                crc = (crc >> 1) ^ 0xEDB88320U;
            else
                crc >>= 1;
            byte >>= 1;
        }
    }
    return crc;
}

static uint32_t g_rng_state = 0xDEADBEEFU;

static uint32_t rng_u32(void)
{
    g_rng_state ^= g_rng_state << 13;
    g_rng_state ^= g_rng_state >> 17;
    g_rng_state ^= g_rng_state << 5;
    return g_rng_state;
}

static void rng_fill(void *buf, uint32_t len)
{
    uint32_t *p = (uint32_t *)buf;
    uint32_t words = len / 4U;
    uint32_t i;

    for (i = 0; i < words; i++)
        p[i] = rng_u32();

    /* handle any remaining bytes */
    if (len & 3U) {
        uint32_t tail = rng_u32();
        uint8_t *bp = (uint8_t *)(p + words);
        uint32_t rem = len & 3U;

        for (i = 0; i < rem; i++) {
            bp[i] = (uint8_t)(tail & 0xFFU);
            tail >>= 8;
        }
    }
}

static void epf_report(const char *op, uint32_t size, uint64_t ticks)
{
    uint32_t freq = R_UTILS_GetTimerFrequency();
    bool pio = g_pio_via_smmu &&
               !(reg_rd(EPF_TEST_REG_BASE, EPF_REG_FLAGS) & FLAG_USE_DMA);
    uint32_t sec;
    uint32_t nsec;
    uint32_t rate;

    if (ticks == 0U) {
        ticks = 1U;
    }

    sec  = (uint32_t)(ticks / freq);
    nsec = (uint32_t)(((ticks % freq) * 1000000000ULL) / freq);
    rate = (uint32_t)(((uint64_t)size * (uint64_t)freq) / (ticks * 1024ULL));

    printf("%s => Size: %u bytes\t1 times DMA: %s\tTime: %u.%09u seconds\tRate: %u KB/s\n",
           op, size, pio ? "NO" : "YES", sec, nsec, rate);
}

static int epf_test_read(uint32_t *status)
{
    uint32_t size     = reg_rd(EPF_TEST_REG_BASE, EPF_REG_SIZE);
    uint32_t checksum = reg_rd(EPF_TEST_REG_BASE, EPF_REG_CHECKSUM);
    uint32_t src_lo   = reg_rd(EPF_TEST_REG_BASE, EPF_REG_LOWER_SRC_ADDR);
    uint32_t src_hi   = reg_rd(EPF_TEST_REG_BASE, EPF_REG_UPPER_SRC_ADDR);
    uint64_t src_addr = ((uint64_t)src_hi << 32) | src_lo;
    void *local_buf   = (void *)(uintptr_t)EPF_LOCAL_DATA_BASE;
    uint64_t ticks;
    uint32_t crc32;

    if (size == 0U || size > EPF_MAX_XFER_SIZE) {
        printf("[EPF] READ: invalid size %u\n", size);
        return -1;
    }

    if (epf_move(src_addr, EPF_LOCAL_DATA_BASE, size, HDMA_READ, status, &ticks)) {
        return -1;
    }

    crc32 = epf_crc32_le(~0U, local_buf, size);
    if (crc32 != checksum) {
        printf("[EPF] READ: CRC mismatch (got 0x%08X, expected 0x%08X)\n",
               crc32, checksum);
        return -1;
    }

    epf_report("READ", size, ticks);
    return 0;
}

static int epf_test_write(uint32_t *status)
{
    uint32_t size     = reg_rd(EPF_TEST_REG_BASE, EPF_REG_SIZE);
    uint32_t dst_lo   = reg_rd(EPF_TEST_REG_BASE, EPF_REG_LOWER_DST_ADDR);
    uint32_t dst_hi   = reg_rd(EPF_TEST_REG_BASE, EPF_REG_UPPER_DST_ADDR);
    uint64_t dst_addr = ((uint64_t)dst_hi << 32) | dst_lo;
    void *local_buf   = (void *)(uintptr_t)EPF_LOCAL_DATA_BASE;
    uint64_t ticks;
    uint32_t crc32;

    if (size == 0U || size > EPF_MAX_XFER_SIZE) {
        printf("[EPF] WRITE: invalid size %u\n", size);
        return -1;
    }

    rng_fill(local_buf, size);
    crc32 = epf_crc32_le(~0U, local_buf, size);
    reg_wr(EPF_TEST_REG_BASE, EPF_REG_CHECKSUM, crc32);

    if (epf_move(dst_addr, EPF_LOCAL_DATA_BASE, size, HDMA_WRITE, status, &ticks)) {
        return -1;
    }

    epf_report("WRITE", size, ticks);
    return 0;
}

static int epf_test_copy(uint32_t *status)
{
    uint32_t size     = reg_rd(EPF_TEST_REG_BASE, EPF_REG_SIZE);
    uint32_t src_lo   = reg_rd(EPF_TEST_REG_BASE, EPF_REG_LOWER_SRC_ADDR);
    uint32_t src_hi   = reg_rd(EPF_TEST_REG_BASE, EPF_REG_UPPER_SRC_ADDR);
    uint32_t dst_lo   = reg_rd(EPF_TEST_REG_BASE, EPF_REG_LOWER_DST_ADDR);
    uint32_t dst_hi   = reg_rd(EPF_TEST_REG_BASE, EPF_REG_UPPER_DST_ADDR);
    uint64_t src_addr = ((uint64_t)src_hi << 32) | src_lo;
    uint64_t dst_addr = ((uint64_t)dst_hi << 32) | dst_lo;
    uint64_t ticks;
    uint64_t mark;

    if (size == 0U || size > EPF_MAX_XFER_SIZE) {
        printf("[EPF] COPY: invalid size %u\n", size);
        return -1;
    }

    if (epf_move(src_addr, EPF_LOCAL_DATA_BASE, size, HDMA_READ, status, &ticks)) {
        return -1;
    }

    if (epf_move(dst_addr, EPF_LOCAL_DATA_BASE, size, HDMA_WRITE, status, &mark)) {
        return -1;
    }

    ticks += mark;

    epf_report("COPY", size, ticks);
    return 0;
}

static const char *epf_command_name(uint32_t command)
{
    if (command & COMMAND_READ) {
        return "READ";
    }
    if (command & COMMAND_WRITE) {
        return "WRITE";
    }
    if (command & COMMAND_COPY) {
        return "COPY";
    }
    if (command & COMMAND_RAISE_LEGACY_IRQ) {
        return "RAISE_LEGACY_IRQ";
    }
    if (command & COMMAND_RAISE_MSI_IRQ) {
        return "RAISE_MSI_IRQ";
    }
    if (command & COMMAND_RAISE_MSIX_IRQ) {
        return "RAISE_MSIX_IRQ";
    }
    return "UNKNOWN";
}

static void pci_epf_test_task(void *pvParameters)
{
    uint32_t ret;
    uint32_t command;
    uint32_t status;

    (void)pvParameters;

    printf("\n--------- UCIe MDP ---------\n");
    printf("[EPF] Bringing up UCIe CH%d as Endpoint...\n", EPF_UCIE_CHANNEL);
    printf("[EPF] Waiting for UCIe link up...\n");

    while (R_UCIE_Get_Linkup_Status(EPF_UCIE_CHANNEL) != LINKUP_SUCCESS)
        __asm__ volatile("nop");

    printf("[EPF] UCIe link-up OK\n");

    Ucie_Setup_PCIE_Post(EPF_UCIE_CHANNEL, UCIE_MODE_EP);
    R_UCIE_Setup_EP_BAR(EPF_UCIE_CHANNEL);
    epf_set_inbound_bar(EPF_TEST_REG_BAR_RGN, EPF_TEST_REG_BAR,
                        (uint32_t)EPF_TEST_REG_BASE);

    g_pio_via_smmu = epf_pio_smmu_setup();

    reg_wr(EPF_TEST_REG_BASE, EPF_REG_COMMAND,  0U);
    reg_wr(EPF_TEST_REG_BASE, EPF_REG_STATUS,   0U);
    reg_wr(EPF_TEST_REG_BASE, EPF_REG_CHECKSUM, 0U);

    reg_wr(EPF_TEST_REG_BASE, EPF_REG_MAGIC, EPF_TEST_MAGIC_VAL);
    printf("[EPF] Ready (MAGIC=0x%08X at 0x%08X). Polling for commands...\n",
           EPF_TEST_MAGIC_VAL, (uint32_t)EPF_TEST_REG_BASE);

    for (;;) {
        command = reg_rd(EPF_TEST_REG_BASE, EPF_REG_COMMAND);
        if (command == 0U) {
            vTaskDelay(1);
            continue;
        }

        printf("[EPF] command %s (0x%08X)\n", epf_command_name(command),
               command);

        reg_wr(EPF_TEST_REG_BASE, EPF_REG_COMMAND, 0U);
        reg_wr(EPF_TEST_REG_BASE, EPF_REG_STATUS,  0U);

        if (command & (COMMAND_RAISE_LEGACY_IRQ |
                       COMMAND_RAISE_MSI_IRQ    |
                       COMMAND_RAISE_MSIX_IRQ)) {
            reg_wr(EPF_TEST_REG_BASE, EPF_REG_STATUS, STATUS_IRQ_RAISED);
            continue;
        }

        status = 0U;

        if (command & COMMAND_READ) {
            ret = (uint32_t)epf_test_read(&status);
            status |= (ret == 0U) ? STATUS_READ_SUCCESS : STATUS_READ_FAIL;
            status |= STATUS_IRQ_RAISED;
        } else if (command & COMMAND_WRITE) {
            ret = (uint32_t)epf_test_write(&status);
            status |= (ret == 0U) ? STATUS_WRITE_SUCCESS : STATUS_WRITE_FAIL;
            status |= STATUS_IRQ_RAISED;
        } else if (command & COMMAND_COPY) {
            ret = (uint32_t)epf_test_copy(&status);
            status |= (ret == 0U) ? STATUS_COPY_SUCCESS : STATUS_COPY_FAIL;
            status |= STATUS_IRQ_RAISED;
        } else {
            printf("[EPF] Unknown command 0x%08X\n", command);
            status = STATUS_IRQ_RAISED;
        }

        reg_wr(EPF_TEST_REG_BASE, EPF_REG_STATUS, status);
    }
}

static void prvSetupHardware(void)
{
    portDISABLE_INTERRUPTS();
    Irq_Setup();
    (void)pfcInitModules(getModuleConfigs());
}

int main(void)
{
    prvSetupHardware();

    xTaskCreate(pci_epf_test_task, "epf_test",
                EPF_TEST_STACK_SIZE, NULL,
                EPF_TEST_TASK_PRIORITY, NULL);

    vTaskStartScheduler();

    for (;;) {
    }

    return 0;
}

/*-----------------------------------------------------------*/

int printf_raw(const char *format, ...);

void vMainAssertCalled(const char *pcFileName, uint32_t ulLineNumber)
{
    printf_raw("ASSERT! Line %u of file %s\n", ulLineNumber, pcFileName);
    taskENTER_CRITICAL();
    for (;;) {
    }
}

void vDeleteCallingTask(void)
{
    vTaskDelete(NULL);
}
