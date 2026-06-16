/*
 * FreeRTOS Kernel V11.1.0
 * Copyright (C) 2021 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 * Copyright (c) 2025 Renesas Electronics Corporation
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

/**
 * @file ucie_mdp_x5h_ca_aiaccl.c
 * @brief PCIe Endpoint Function Test - FreeRTOS CR52 port
 *
 * This application implements the pci-epf-test protocol on the Renesas R-Car
 * Gen5 CR52 (FreeRTOS) side.  It acts as the PCIe endpoint (EP) counterpart
 * to the Linux pci_endpoint_test host driver (drivers/misc/pci_endpoint_test.c)
 * when CONFIG_UCIE_DUMMY_RCAR is enabled.
 *
 * Memory layout
 * =========================================================================
 *  0x90000000 - 0x90000FFF   BAR0 test registers       (4 KB)
 *  0x90001000 - 0x90100FFF   Data buffer A              (1 MB)
 *                              - src for COMMAND_READ   (EP reads, verifies CRC)
 *                              - dst for COMMAND_WRITE  (EP writes random data)
 *  0x90101000 - 0x90200FFF   Data buffer B              (1 MB)
 *                              - dst for COMMAND_COPY   (EP copies A -> B)
 *
 * Both CR52 and the Linux RC access these addresses directly.  No dynamic
 * UCIe address-mapping changes are required during test execution; the single
 * shared-memory window is configured once by the Linux driver at probe time
 * (via the ucie_shared DTS node) and kept for the duration of operation.
 *
 * Protocol
 * ========
 * 1. EP sets BAR0+0x00 (MAGIC) = EPF_TEST_MAGIC_VAL after initialisation.
 * 2. EP polls BAR0+0x04 (COMMAND) for a non-zero value.
 * 3. EP clears COMMAND and STATUS, executes the command, then writes the
 *    appropriate STATUS bits plus STATUS_IRQ_RAISED.
 * 4. Linux polls STATUS_IRQ_RAISED, reads results, and clears the bit.
 *
 * Commands supported: RAISE_LEGACY_IRQ, RAISE_MSI_IRQ, RAISE_MSIX_IRQ,
 *                     READ, WRITE, COPY.
 */

/* Scheduler include files */
#include "FreeRTOS.h"
#include "task.h"

#include "interrupts.h"
#include "stdio.h"
#include "string.h"
#include "ucie/r_ucie.h"
#include "pfc/r_pfc_api.h"
#include "device_tree_mdp_aiacc.h"
#include "rcar_utils.h"

/*---------------------------------------------------------------------------
 * Task parameters
 *--------------------------------------------------------------------------*/
#define EPF_TEST_TASK_PRIORITY  (tskIDLE_PRIORITY + 1)
#define EPF_TEST_STACK_SIZE     (configMINIMAL_STACK_SIZE * 4)

/*---------------------------------------------------------------------------
 * BAR0 register offsets (must match pci_endpoint_test.c / pci-epf-test.c)
 *--------------------------------------------------------------------------*/
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

/* Magic value written to BAR0 to indicate EP readiness */
#define EPF_TEST_MAGIC_VAL      0xFACEFEEDU

/*---------------------------------------------------------------------------
 * Command bits (COMMAND register)
 *--------------------------------------------------------------------------*/
#define COMMAND_RAISE_LEGACY_IRQ    (1U << 0)
#define COMMAND_RAISE_MSI_IRQ       (1U << 1)
#define COMMAND_RAISE_MSIX_IRQ      (1U << 2)
#define COMMAND_READ                (1U << 3)
#define COMMAND_WRITE               (1U << 4)
#define COMMAND_COPY                (1U << 5)

/*---------------------------------------------------------------------------
 * Status bits (STATUS register)
 *--------------------------------------------------------------------------*/
#define STATUS_READ_SUCCESS     (1U << 0)
#define STATUS_READ_FAIL        (1U << 1)
#define STATUS_WRITE_SUCCESS    (1U << 2)
#define STATUS_WRITE_FAIL       (1U << 3)
#define STATUS_COPY_SUCCESS     (1U << 4)
#define STATUS_COPY_FAIL        (1U << 5)
#define STATUS_IRQ_RAISED       (1U << 6)
#define STATUS_SRC_ADDR_INVALID (1U << 7)
#define STATUS_DST_ADDR_INVALID (1U << 8)

/*---------------------------------------------------------------------------
 * Shared memory addresses
 *--------------------------------------------------------------------------*/

/** BAR0 test registers: 4 KB at the start of the ucie_shared DTS region. */
#define EPF_TEST_REG_BASE       0x90000000UL

/** BAR0 region size */
#define EPF_BAR0_SIZE           0x1000U

/**
 * Data buffer A (1 MB): used as src for COMMAND_READ (Linux fills it,
 * EP reads from it) and as dst for COMMAND_WRITE (EP fills it, Linux reads).
 */
#define EPF_DATA_BUF_A_BASE     (EPF_TEST_REG_BASE + EPF_BAR0_SIZE)    /* 0x90001000 */

/**
 * Data buffer B (1 MB): used as dst for COMMAND_COPY (EP copies A -> B,
 * Linux reads B to verify).
 */
#define EPF_DATA_BUF_B_BASE     (EPF_DATA_BUF_A_BASE + 0x100000U)      /* 0x90101000 */

/** Maximum transfer size (must not exceed 1 MB per buffer) */
#define EPF_MAX_XFER_SIZE       (1U * 1024U * 1024U)

/*---------------------------------------------------------------------------
 * UCIe configuration
 *--------------------------------------------------------------------------*/
#define EPF_UCIE_CHANNEL        UCIE_CH0

/*---------------------------------------------------------------------------
 * Register accessors (volatile 32-bit MMIO)
 *--------------------------------------------------------------------------*/
static inline uint32_t reg_rd(uint32_t base, uint32_t off)
{
    return *((volatile uint32_t *)(uintptr_t)(base + off));
}

static inline void reg_wr(uint32_t base, uint32_t off, uint32_t val)
{
    *((volatile uint32_t *)(uintptr_t)(base + off)) = val;
}

/*---------------------------------------------------------------------------
 * Simple CRC32-LE (polynomial 0xEDB88320) - compatible with Linux crc32_le()
 *--------------------------------------------------------------------------*/
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

/*---------------------------------------------------------------------------
 * Simple xorshift32 PRNG for test data generation
 *--------------------------------------------------------------------------*/
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

/*---------------------------------------------------------------------------
 * Command handlers
 *--------------------------------------------------------------------------*/

/**
 * epf_test_read - handle COMMAND_READ.
 *
 * The Linux RC has written test data into data buffer A (EPF_DATA_BUF_A_BASE)
 * and stored the expected CRC32 in the CHECKSUM register.  The EP reads the
 * data directly from the shared buffer and verifies the checksum.
 *
 * Returns 0 on CRC match, non-zero on mismatch or invalid parameters.
 */
static int epf_test_read(void)
{
    uint32_t size     = reg_rd(EPF_TEST_REG_BASE, EPF_REG_SIZE);
    uint32_t checksum = reg_rd(EPF_TEST_REG_BASE, EPF_REG_CHECKSUM);
    uint32_t src_lo   = reg_rd(EPF_TEST_REG_BASE, EPF_REG_LOWER_SRC_ADDR);
    uint32_t src_hi   = reg_rd(EPF_TEST_REG_BASE, EPF_REG_UPPER_SRC_ADDR);
    uint64_t src_addr = ((uint64_t)src_hi << 32) | src_lo;
    uint32_t crc32;

    if (size == 0U || size > EPF_MAX_XFER_SIZE) {
        printf("[EPF] READ: invalid size %u\n", size);
        return -1;
    }

    /*
     * Linux sets src_addr = data_buf_phys = EPF_DATA_BUF_A_BASE.
     */
    crc32 = epf_crc32_le(~0U, (const void *)(uintptr_t)src_addr, size);
    if (crc32 != checksum) {
        printf("[EPF] READ: CRC mismatch (got 0x%08X, expected 0x%08X)\n",
               crc32, checksum);
        return -1;
    }

    printf("[EPF] READ: %u bytes OK (crc=0x%08X)\n", size, crc32);
    return 0;
}

/**
 * epf_test_write - handle COMMAND_WRITE.
 *
 * The EP generates pseudo-random test data directly into data buffer A
 * (EPF_DATA_BUF_A_BASE), stores the CRC32 in the CHECKSUM register, and
 * returns.  Linux reads the data back from the shared buffer and verifies.
 *
 * Returns 0 on success.
 */
static int epf_test_write(void)
{
    uint32_t size   = reg_rd(EPF_TEST_REG_BASE, EPF_REG_SIZE);
    uint32_t dst_lo = reg_rd(EPF_TEST_REG_BASE, EPF_REG_LOWER_DST_ADDR);
    uint32_t dst_hi = reg_rd(EPF_TEST_REG_BASE, EPF_REG_UPPER_DST_ADDR);
    uint64_t dst_addr = ((uint64_t)dst_hi << 32) | dst_lo;
    uint32_t crc32;

    if (size == 0U || size > EPF_MAX_XFER_SIZE) {
        printf("[EPF] WRITE: invalid size %u\n", size);
        return -1;
    }

    /*
     * Linux sets dst_addr = data_buf_phys = EPF_DATA_BUF_A_BASE.
     * Fill the shared buffer directly and compute CRC for Linux to verify.
     */
    rng_fill((void *)(uintptr_t)dst_addr, size);
    crc32 = epf_crc32_le(~0U, (const void *)(uintptr_t)dst_addr, size);
    reg_wr(EPF_TEST_REG_BASE, EPF_REG_CHECKSUM, crc32);

    printf("[EPF] WRITE: %u bytes OK (crc=0x%08X)\n", size, crc32);
    return 0;
}

/**
 * epf_test_copy - handle COMMAND_COPY.
 *
 * The Linux RC has filled data buffer A (src_addr = EPF_DATA_BUF_A_BASE)
 * with random data.  The EP copies it to data buffer B
 * (dst_addr = EPF_DATA_BUF_A_BASE + SZ_1M).  Linux then reads B and
 * verifies the CRC matches A.
 *
 * Returns 0 on success, non-zero on invalid parameters.
 */
static int epf_test_copy(void)
{
    uint32_t size   = reg_rd(EPF_TEST_REG_BASE, EPF_REG_SIZE);
    uint32_t src_lo = reg_rd(EPF_TEST_REG_BASE, EPF_REG_LOWER_SRC_ADDR);
    uint32_t src_hi = reg_rd(EPF_TEST_REG_BASE, EPF_REG_UPPER_SRC_ADDR);
    uint32_t dst_lo = reg_rd(EPF_TEST_REG_BASE, EPF_REG_LOWER_DST_ADDR);
    uint32_t dst_hi = reg_rd(EPF_TEST_REG_BASE, EPF_REG_UPPER_DST_ADDR);
    uint64_t src_addr = ((uint64_t)src_hi << 32) | src_lo;
    uint64_t dst_addr = ((uint64_t)dst_hi << 32) | dst_lo;

    if (size == 0U || size > EPF_MAX_XFER_SIZE) {
        printf("[EPF] COPY: invalid size %u\n", size);
        return -1;
    }

    /*
     * src_addr = EPF_DATA_BUF_A_BASE, dst_addr = EPF_DATA_BUF_B_BASE.
     * Both are in the shared DRAM region — copy directly.
     */
    memcpy((void *)(uintptr_t)dst_addr,
           (const void *)(uintptr_t)src_addr,
           size);

    printf("[EPF] COPY: %u bytes OK (0x%08X -> 0x%08X)\n",
           size, (uint32_t)src_addr, (uint32_t)dst_addr);
    return 0;
}

/*---------------------------------------------------------------------------
 * Main EP test task
 *--------------------------------------------------------------------------*/
static void pci_epf_test_task(void *pvParameters)
{
    uint32_t ret;
    uint32_t command;
    uint32_t status;

    (void)pvParameters;

    /* Step 1: UCIe link up in endpoint mode */
    printf("[EPF] Bringing up UCIe CH%d as Endpoint...\n", EPF_UCIE_CHANNEL);

    ret = R_UCIE_Setup(EPF_UCIE_CHANNEL, UCIE_MODE_EP, LINKSPEED_16GTPS);
    if (ret == LINKUP_TIMEOUT) {
        printf("[EPF] Link-up timeout, retrying...\n");
        ret = R_UCIE_Retry_Linkup(EPF_UCIE_CHANNEL, UCIE_MODE_EP,
                                  LINKSPEED_16GTPS, 30);
    }
    if (ret != LINKUP_SUCCESS) {
        printf("[EPF] UCIe link-up FAILED (err=%u)\n", ret);
        vTaskDelete(NULL);
        return;
    }
    printf("[EPF] UCIe link-up OK\n");

    /* Step 2: Initialise test registers */
    reg_wr(EPF_TEST_REG_BASE, EPF_REG_COMMAND,  0U);
    reg_wr(EPF_TEST_REG_BASE, EPF_REG_STATUS,   0U);
    reg_wr(EPF_TEST_REG_BASE, EPF_REG_CHECKSUM, 0U);

    /* Write magic value so host can verify EP is ready */
    reg_wr(EPF_TEST_REG_BASE, EPF_REG_MAGIC, EPF_TEST_MAGIC_VAL);
    printf("[EPF] Ready (MAGIC=0x%08X written). Polling for commands...\n",
           EPF_TEST_MAGIC_VAL);

    /* Step 3: Command dispatch loop */
    for (;;) {
        command = reg_rd(EPF_TEST_REG_BASE, EPF_REG_COMMAND);
        if (command == 0U) {
            vTaskDelay(1);  /* yield for 1 tick between polls */
            continue;
        }

        /* Acknowledge: clear command and status before processing */
        reg_wr(EPF_TEST_REG_BASE, EPF_REG_COMMAND, 0U);
        reg_wr(EPF_TEST_REG_BASE, EPF_REG_STATUS,  0U);

        /*
         * IRQ raise commands: host uses these to test the notification path.
         * In polling mode the host detects STATUS_IRQ_RAISED by polling, so
         * no hardware IRQ needs to be raised — just set the status bit.
         */
        if (command & (COMMAND_RAISE_LEGACY_IRQ |
                       COMMAND_RAISE_MSI_IRQ    |
                       COMMAND_RAISE_MSIX_IRQ)) {
            reg_wr(EPF_TEST_REG_BASE, EPF_REG_STATUS, STATUS_IRQ_RAISED);
            continue;
        }

        status = 0U;

        if (command & COMMAND_READ) {
            ret = (uint32_t)epf_test_read();
            status = (ret == 0U) ? STATUS_READ_SUCCESS : STATUS_READ_FAIL;
            status |= STATUS_IRQ_RAISED;
        } else if (command & COMMAND_WRITE) {
            ret = (uint32_t)epf_test_write();
            status = (ret == 0U) ? STATUS_WRITE_SUCCESS : STATUS_WRITE_FAIL;
            status |= STATUS_IRQ_RAISED;
        } else if (command & COMMAND_COPY) {
            ret = (uint32_t)epf_test_copy();
            status = (ret == 0U) ? STATUS_COPY_SUCCESS : STATUS_COPY_FAIL;
            status |= STATUS_IRQ_RAISED;
        } else {
            printf("[EPF] Unknown command 0x%08X\n", command);
            status = STATUS_IRQ_RAISED;
        }

        reg_wr(EPF_TEST_REG_BASE, EPF_REG_STATUS, status);
    }
}

/*---------------------------------------------------------------------------
 * Hardware initialisation and entry point
 *--------------------------------------------------------------------------*/
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

/*---------------------------------------------------------------------------
 * FreeRTOS support callbacks
 *--------------------------------------------------------------------------*/
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
