/*
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 */

/* rcar_rpmsg over UCIe: RC window (SMMU + iATU), FE/BE start-up handshake. */

#include <errno.h>
#include <stdbool.h>

#include <metal/sleep.h>
#include <metal/time.h>

#include "board.h"
#include "cmsis_rcar_gen5.h"
#include "smmu/smmu.h"
#include "ucie/r_ucie.h"
#include "rsc_table_rpmsg.h"
#include "transport_ucie.h"

/* Logging Function include. */
#include "logging_stack.h"

/*
 * UCIe CH0: X5H = RC, maps the shared memory (AIACC DRAM). AIACC = EP, maps nothing.
 * Link-up is done elsewhere: X5H by the CA core or R_UCIE_Config() in main(),
 * AIACC by its system task. x5h_vdk: not supported.
 */
#if ((BOARD == MDP_X5H_HIL) || (BOARD == X5H_RFS2))
#define UCIE_SUPPORTED      1
#define UCIE_IS_RC          1
#elif ((BOARD == MDP_AIACC_HIL) || (BOARD == MDP_AIACC_RFS2))
#define UCIE_SUPPORTED      1
#define UCIE_IS_RC          0
#else
#define UCIE_SUPPORTED      0
#define UCIE_IS_RC          0
#endif

/* HW: CR52 VA -> SMMU_RT -> UCIe window -> iATU -> shm_base.
 * RFS2: no SMMU, iATU shm_base -> shm_base. */
#if ((BOARD == X5H_RFS2) || (BOARD == MDP_AIACC_RFS2))
#define UCIE_USE_SMMU       0
#else
#define UCIE_USE_SMMU       1
#endif

#define UCIE_CH             (UCIE_CH0)
#define UCIE_LINK_TIMEOUT_MS (10000U)  /* RC: wait for link-up */

/* Covers the shared memory (iATU and SMMU). */
#define UCIE_SHM_WINDOW     (0x200000UL)
#define UCIE_SHM_IATU_RGN   (IATU_RGN0)
#define UCIE_D2D_MEM(ch)    (0x20000000000ULL + ((uint64_t)(ch) * 0x4000000000ULL))

/* SMMU_RT of the X5H CR52. */
#define UCIE_RCTBUBYPSEN    (0x18B47800U)   /* Realtime Core TBU bypass enable */
#define UCIE_RCTBUBYPSEN_MSK (0x00000FFFU)

#define UCIE_POLL_US        (1000U)
#define UCIE_REWRITE_MS     (10U)      /* FE re-writes its nonce: BE clears it at start */
#define UCIE_FE_TIMEOUT_MS  (10000U)   /* handshake, after link-up */

/* Sync words: one writer each, on different 4 KB pages (RFS2 syncs whole
 * pages). FE: end of the rsc table page. BE: last page of the shm. */
#define UCIE_SYNC_FE_OFFSET (0xFC0UL)
#define UCIE_SYNC_BE_PAGE   (0x1000UL)       /* at shm_base + shm_size - 0x1000 */
#define UCIE_SYNC_MAGIC     (0x55434945UL)  /* "UCIE" */

typedef struct {
    volatile uint32_t fe_nonce;  /* FE (BE clears it at start) */
} ucie_sync_fe_t;

typedef struct {
    volatile uint32_t magic;     /* BE, for debug */
    volatile uint32_t be_nonce;  /* BE: = fe_nonce once the rsc table is written */
} ucie_sync_be_t;

_Static_assert(sizeof(struct remote_resource_table) <= UCIE_SYNC_FE_OFFSET,
               "rsc table overlaps the UCIe FE sync word");
_Static_assert((UCIE_SYNC_FE_OFFSET + sizeof(ucie_sync_fe_t)) <= 0x1000UL,
               "UCIe FE sync word outside RSC_TABLE_RESERVE");

#if UCIE_SUPPORTED

static int s_type = -1;   /* first prepared side; the other one is refused */

/* ms since start_us, wrap-safe. */
static uint32_t elapsed_ms(uint32_t start_us)
{
    return ((uint32_t)metal_get_timestamp() - start_us) / 1000U;
}

#if UCIE_IS_RC

static bool s_rc_ready;   /* link-up + window done */

/* Wait for link-up (done elsewhere). */
static int rc_linkup(void)
{
    uint32_t start_us = (uint32_t)metal_get_timestamp();

    LogDebug(("ucie: RC waiting for CH%u link-up\n", (unsigned int)UCIE_CH));
    while (R_UCIE_Get_Linkup_Status(UCIE_CH) != LINKUP_SUCCESS) {
        if (elapsed_ms(start_us) >= UCIE_LINK_TIMEOUT_MS) {
            LogDebug(("ucie: RC link-up timeout\n"));
            return -ETIMEDOUT;
        }
        metal_sleep_usec(10000U);
    }
    LogDebug(("ucie: RC CH%u linked up\n", (unsigned int)UCIE_CH));
    return 0;
}

/* Outbound: window (HW) or CR52 address (RFS2) -> shm_base. */
static int rc_map_iatu(uintptr_t shm_base)
{
    st_ucie_iatu_cfg_t cfg = {
        .ucie_ch   = UCIE_CH,
        .rgn       = UCIE_SHM_IATU_RGN,
        .type      = IATU_OUTBOUND,
#if UCIE_USE_SMMU
        .mSrcAddr  = UCIE_D2D_MEM(UCIE_CH),
#else
        .mSrcAddr  = (uint64_t)shm_base,
#endif
        .mDestAddr = (uint64_t)shm_base,
        .size      = UCIE_SHM_WINDOW,
    };
    return (R_UCIE_IATU_SetRegion(&cfg) == 0U) ? 0 : -EIO;
}

#if UCIE_USE_SMMU
/* Identity-mapped DRAM around the window (local shm included). */
#define UCIE_SMMU_DRAM_START    (0x60000000ULL)
#define UCIE_SMMU_DRAM_END      (0xC0000000ULL)

/* VA shm_base -> UCIe window, rest identity: SMMU_RT blocks unmapped accesses. */
static int rc_map_smmu(uintptr_t shm_base)
{
    static const uint32_t stream_id[] = { 0x000U, 0xC00U };
    static st_smmu_streamid_instance_ctrl_t ctrl[2] = {
        { .smmu_domain = SMMU_RT, .is_secure = true },
        { .smmu_domain = SMMU_RT, .is_secure = true },
    };
    _Static_assert((sizeof(stream_id) / sizeof(stream_id[0])) == (sizeof(ctrl) / sizeof(ctrl[0])),
                   "one SMMU ctrl per stream id");

    uint64_t win_start = (uint64_t)shm_base;
    uint64_t win_end   = win_start + UCIE_SHM_WINDOW;
    if ((win_start < UCIE_SMMU_DRAM_START) || (win_end > UCIE_SMMU_DRAM_END)) {
        return -EINVAL;
    }

    if (R_SMMU_Init(SMMU_RT, true) != 0) {
        return -EIO;
    }
    for (unsigned int i = 0; i < (sizeof(stream_id) / sizeof(stream_id[0])); i++) {
        ctrl[i].stream_id = stream_id[i];
        if ((R_SMMU_Attach(&ctrl[i]) != 0) ||
            (R_SMMU_Map(&ctrl[i], 0x00000000ULL, 0x00000000ULL, UCIE_SMMU_DRAM_START,
                        ATTR_DEVICE_NGNRNE_EL1_RW_EL0_RW) != MAP_SUCCESS) ||
            ((win_start > UCIE_SMMU_DRAM_START) &&
             (R_SMMU_Map(&ctrl[i], UCIE_SMMU_DRAM_START, UCIE_SMMU_DRAM_START,
                         win_start - UCIE_SMMU_DRAM_START,
                         ATTR_DEVICE_NGNRNE_EL1_RW_EL0_RW) != MAP_SUCCESS)) ||
            (R_SMMU_Map(&ctrl[i], win_start, UCIE_D2D_MEM(UCIE_CH), UCIE_SHM_WINDOW,
                        ATTR_DEVICE_NGNRNE_EL1_RW_EL0_RW) != MAP_SUCCESS) ||
            ((win_end < UCIE_SMMU_DRAM_END) &&
             (R_SMMU_Map(&ctrl[i], win_end, win_end, UCIE_SMMU_DRAM_END - win_end,
                         ATTR_DEVICE_NGNRNE_EL1_RW_EL0_RW) != MAP_SUCCESS)) ||
            (R_SMMU_Map(&ctrl[i], UCIE_SMMU_DRAM_END, UCIE_SMMU_DRAM_END, 0x40000000ULL,
                        ATTR_DEVICE_NGNRNE_EL1_RW_EL0_RW) != MAP_SUCCESS)) {
            return -EIO;
        }
    }

    /* Translate TBU0 only. */
    volatile uint32_t *rctbubypsen = (volatile uint32_t *)UCIE_RCTBUBYPSEN;
    uint32_t bypass = ~(1U << 0U) & UCIE_RCTBUBYPSEN_MSK;
    *rctbubypsen = (*rctbubypsen & ~UCIE_RCTBUBYPSEN_MSK) | bypass;

    return (R_SMMU_Enable(SMMU_RT, true) == 0) ? 0 : -EIO;
}
#endif /* UCIE_USE_SMMU */

static int rc_prepare(uintptr_t shm_base, size_t shm_size)
{
    if (shm_size > UCIE_SHM_WINDOW) {
        return -EINVAL;
    }
    if (s_rc_ready) {
        return 0;
    }

    /* iATU after link-up: power-on resets it. */
    int ret = rc_linkup();
#if UCIE_USE_SMMU
    if (ret == 0) {
        LogDebug(("ucie: SMMU_RT map start\n"));
        ret = rc_map_smmu(shm_base);
        LogDebug(("ucie: SMMU_RT map ret=%d\n", ret));
    }
#endif
    if (ret == 0) {
        LogDebug(("ucie: iATU outbound start\n"));
        ret = rc_map_iatu(shm_base);
        LogDebug(("ucie: iATU outbound ret=%d\n", ret));
    }
    if (ret == 0) {
        s_rc_ready = true;
    }
    return ret;
}

#endif /* UCIE_IS_RC */

/* Runs after rcar_rsc_table_init(). No timeout: BE blocks until FE is up. */
static void be_handshake(ucie_sync_fe_t *fe, ucie_sync_be_t *be)
{
    uint32_t n;

    /* Drop what an earlier boot left. */
    be->be_nonce = 0U;
    be->magic    = UCIE_SYNC_MAGIC;
    fe->fe_nonce = 0U;   /* the FE writes it again every UCIE_REWRITE_MS */
    __DSB();
    LogDebug(("ucie: BE waiting for FE nonce at %p\n", (void *)fe));

    while ((n = fe->fe_nonce) == 0U) {
        metal_sleep_usec(UCIE_POLL_US);
    }

    /* rsc table written before the answer. */
    __DSB();
    be->be_nonce = n;
    __DSB();
    LogDebug(("ucie: BE answered nonce 0x%08lx\n", (unsigned long)n));
}

/* Before the FE reads the rsc table. */
static int fe_handshake(ucie_sync_fe_t *fe, ucie_sync_be_t *be)
{
    uint32_t start_us = (uint32_t)metal_get_timestamp();

    /* Differs from any stale answer, so only a fresh BE can match it. */
    LogDebug(("ucie: FE first read at %p\n", (void *)be));
    uint32_t old = be->be_nonce;
    uint32_t n = start_us | 1U;
    if (n == old) {
        n += 2U;
    }

    LogDebug(("ucie: FE writing nonce 0x%08lx at %p (old be_nonce 0x%08lx)\n",
         (unsigned long)n, (void *)fe, (unsigned long)old));

    uint32_t last_write_us = 0U;
    bool written = false;
    for (;;) {
        uint32_t now_us = (uint32_t)metal_get_timestamp();
        if (!written || (((now_us - last_write_us) / 1000U) >= UCIE_REWRITE_MS)) {
            fe->fe_nonce = n;
            __DSB();
            (void)fe->fe_nonce;  /* read back: the posted write reached the EP */
            last_write_us = now_us;
            written = true;
        }
        if (be->be_nonce == n) {
            break;
        }
        if (elapsed_ms(start_us) >= UCIE_FE_TIMEOUT_MS) {
            LogDebug(("ucie: FE handshake timeout (be_nonce 0x%08lx)\n", (unsigned long)be->be_nonce));
            return -ETIMEDOUT;
        }
        metal_sleep_usec(UCIE_POLL_US);
    }
    __DSB();
    LogDebug(("ucie: FE handshake done, nonce 0x%08lx\n", (unsigned long)n));
    return 0;
}

/* Re-link not handled: if the RC reboots alone, the FE times out. */
int platform_ucie_prepare(virtio_type_t type, uintptr_t shm_base, size_t shm_size)
{
    if (shm_size < (2U * UCIE_SYNC_BE_PAGE)) {
        return -EINVAL;   /* rsc table page + BE sync page */
    }
    if ((s_type >= 0) && (s_type != (int)type)) {
        return -EINVAL;   /* one side per chip over UCIe */
    }
    s_type = (int)type;
    LogDebug(("ucie: prepare %s, %s, shm 0x%08lx\n", (type == VIRTIO_BACK_END) ? "BE" : "FE",
         UCIE_IS_RC ? "RC" : "EP", (unsigned long)shm_base));

#if UCIE_IS_RC
    int ret = rc_prepare(shm_base, shm_size);
    if (ret != 0) {
        return ret;
    }
#endif   /* EP: shm is local, nothing to map */

    ucie_sync_fe_t *fe = (ucie_sync_fe_t *)(shm_base + UCIE_SYNC_FE_OFFSET);
    ucie_sync_be_t *be = (ucie_sync_be_t *)(shm_base + shm_size - UCIE_SYNC_BE_PAGE);
    if (type == VIRTIO_BACK_END) {
        be_handshake(fe, be);
        return 0;
    }
    return fe_handshake(fe, be);
}

#else /* !UCIE_SUPPORTED */

int platform_ucie_prepare(virtio_type_t type, uintptr_t shm_base, size_t shm_size)
{
    (void)type;
    (void)shm_base;
    (void)shm_size;
    return -ENODEV;   /* no UCIe on this BOARD */
}

#endif /* UCIE_SUPPORTED */
