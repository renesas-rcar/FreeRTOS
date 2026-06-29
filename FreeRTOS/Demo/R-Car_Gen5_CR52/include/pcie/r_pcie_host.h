/*
* Copyright (c) 2025 Renesas Electronics Corporation
*
* SPDX-License-Identifier: MIT
*/
#ifndef PCIE_HOST
#define PCIE_HOST

/**
 * @defgroup PCIe_Host_Module PCIe Host Module
 * @{
 * @brief This module provides functions to configure and control PCIe host communication.
 *
 * The PCIe host module allows for the configuration and control of PCIe communication between devices.
 */
#ifdef __cplusplus
extern "C" {
#endif

#include "pcie/r_pcie_ctrl.h"
#include <stdbool.h>

/*------------------------- PCIe common definition and structure ------*/
/**
 * @typedef pcie_id_t
 * @brief A unique PCI(e) identifier (vendor ID, device ID).
 *
 * The PCIE_CONF_ID register for each endpoint is a (vendor ID, device ID)
 * pair, which is meant to tell the system what the PCI(e) endpoint is. Again,
 * look to PCIE_ID_* macros in include/dt-bindings/pcie/pcie.h for more.
 */
typedef uint32_t pcie_id_t;

/*------------------------------- PUBLIC APIs -------------------------*/

/** 
 * @brief Callback type used for scanning for PCI endpoints
 *
 * @param[in] bdf      BDF value for a found endpoint.
 * @param[in] id       Vendor & Device ID for the found endpoint.
 * @param[inout] cb_data  Custom, use case specific data.
 *
 * @return true to continue scanning, false to stop scanning.
 */
typedef bool (*pcie_scan_cb_t)(pcie_bdf_t bdf, pcie_id_t id, void *cb_data);

/**
 * @brief enum
 */
enum {
    /**
     * @brief Scan all available PCI host controllers and sub-busses
     */
    PCIE_SCAN_RECURSIVE = 0x00000001,     ///< Bit 0
    /** 
     * @brief Do the callback for all endpoint types, including bridges
     */
    PCIE_SCAN_CB_ALL = 0x00000002,         ///< Bit 1
};

// /** API mapping functions for Host */
// struct st_pcie_host_api
// {
//     /* Functions for Host */
//     bool     (*region_allocate)(struct st_pcie_host *host, pcie_bdf_t bdf, bool mem, bool mem64, size_t bar_size, uint32_t *bar_bus_addr);
//     bool     (*region_get_allocate_base)(struct st_pcie_host *host, pcie_bdf_t bdf, bool mem, bool mem64, size_t align, uint32_t *bar_base_addr);
//     bool     (*region_translate)(struct st_pcie_host *host, pcie_bdf_t bdf, bool mem, bool mem64, size_t bar_size, uint32_t *bar_addr);
//     int32_t  (*scan)(struct st_pcie_host *host, const struct st_pcie_scan_opt *opt);
//     bool     (*get_mbar)(struct st_pcie_host *host, pcie_bdf_t bdf, unsigned int bar_index, struct pcie_bar *mbar);
//     bool     (*probe_mbar)(struct st_pcie_host *host, pcie_bdf_t bdf, unsigned int index, struct pcie_bar *mbar);
//     bool     (*get_iobar)(struct st_pcie_host *host, pcie_bdf_t bdf, unsigned int bar_index, struct pcie_bar *iobar);
//     bool     (*probe_iobar)(struct st_pcie_host *host, pcie_bdf_t bdf, unsigned int index, struct pcie_bar *iobar);
//     void     (*set_cmd)(struct st_pcie_host *host, pcie_bdf_t bdf, uint32_t bits, bool on);
//     uint32_t (*alloc_irq)(struct st_pcie_host *host, pcie_bdf_t bdf);
//     uint32_t (*get_irq)(struct st_pcie_host *host, pcie_bdf_t bdf);
//     void     (*enable_irq)(struct st_pcie_host *host, pcie_bdf_t bdf, unsigned int irq);
//     uint32_t (*get_cap)(struct st_pcie_host *host, pcie_bdf_t bdf, uint32_t cap_id);
//     uint32_t (*get_ext_cap)(struct st_pcie_host *host, pcie_bdf_t bdf, uint32_t cap_id);
//     bool     (*connect_dynamic_irq)(struct st_pcie_host *host, 
//                             pcie_bdf_t bdf,
//                             unsigned int irq,
//                             unsigned int priority,
//                             void (*routine)(const void *parameter),
//                             const void *parameter,
//                             uint32_t flags);
// };

/**
 * @brief Struct to describe the PCIe Host device.
 */
struct st_pcie_host
{
    struct st_pcie_ctrl ctrl; ///< The controller that control this host.
    pcie_irq_t irq[];         ///< List of interrupt source and its callback function.
};


/** 
 * @brief Options for performing a scan for PCI devices.
 */
struct st_pcie_scan_opt {
    uint8_t bus;        ///< Initial bus number to scan.
    pcie_scan_cb_t cb;  ///< Function to call for each found endpoint.
    void *cb_data;      ///< Custom data to pass to the scan callback.
    uint32_t flags;     ///< Scan flags.
};

/**
 * @brief Init a PCIe device channel to a Host Device.
 * 
 */
void R_PCIE_InitHost(struct st_pcie_host *host, uint16_t channel);

/** 
 * @brief Scan for PCIe devices.
 *
 * Scan the PCI bus (or buses) for available endpoints.
 *
 * @param[in] ctrl PCI Express Controller device pointer
 * @param opt Options determining how to perform the scan.
 *
 * @return 0 on success, negative POSIX error number on failure.
 */
int R_PCIE_Scan(struct st_pcie_host *host, const struct st_pcie_scan_opt *opt);

/**
 * @brief Get the MBAR at a specific BAR index.
 *
 * @param[in] ctrl PCI Express Controller device pointer.
 * @param bdf the PCI(e) endpoint.
 * @param bar_index 0-based BAR index.
 * @param mbar Pointer to struct pcie_bar.
 *
 * @return True if the mbar was found and is valid, false otherwise.
 */
bool R_PCIE_GetMBAR(struct st_pcie_host *host, pcie_bdf_t bdf, unsigned int bar_index, struct pcie_bar *mbar);

/**
 * @brief Probe the nth MMIO address assigned to an endpoint.
 *
 * A PCI(e) endpoint has 0 or more memory-mapped regions. This function
 * allows the caller to enumerate them by calling with index=0..n.
 * Value of n has to be below 6, as there is a maximum of 6 BARs. The indices
 * are order-preserving with respect to the endpoint BARs: e.g., index 0
 * will return the lowest-numbered memory BAR on the endpoint.
 *
 * @param[in] ctrl PCI Express Controller device pointer.
 * @param bdf the PCI(e) endpoint.
 * @param index (0-based) index.
 * @param mbar Pointer to struct pcie_bar.
 *
 * @return True if the mbar was found and is valid, false otherwise.
 */
bool R_PCIE_ProbeMBAR(struct st_pcie_host *host, pcie_bdf_t bdf, unsigned int index, struct pcie_bar *mbar);

/**
 * @brief Get the I/O BAR at a specific BAR index.
 *
 * @param[in] ctrl PCI Express Controller device pointer.
 * @param bdf the PCI(e) endpoint.
 * @param bar_index 0-based BAR index.
 * @param iobar Pointer to struct pcie_bar.
 *
 * @return True if the I/O BAR was found and is valid, false otherwise.
 */
bool R_PCIE_GetIOBAR(struct st_pcie_host *host, pcie_bdf_t bdf, unsigned int bar_index, struct pcie_bar *iobar);

/**
 * @brief Probe the nth I/O BAR address assigned to an endpoint.
 *
 * A PCI(e) endpoint has 0 or more I/O regions. This function
 * allows the caller to enumerate them by calling with index=0..n.
 * Value of n has to be below 6, as there is a maximum of 6 BARs. The indices
 * are order-preserving with respect to the endpoint BARs: e.g., index 0
 * will return the lowest-numbered I/O BAR on the endpoint.
 * 
 * @param[in] ctrl PCI Express Controller device pointer.
 * @param bdf The PCI(e) endpoint.
 * @param index (0-based) index.
 * @param iobar Pointer to struct pcie_bar.
 *
 * @return True if the I/O BAR was found and is valid, false otherwise.
 */
bool R_PCIE_ProbeIOBAR(struct st_pcie_host *host, pcie_bdf_t bdf, unsigned int index, struct pcie_bar *iobar);

/**
 * @brief Set or reset bits in the endpoint command/status register.
 *
 * @param[in] ctrl PCI Express Controller device pointer.
 * @param bdf The PCI(e) endpoint.
 * @param bits The powerset of bits of interest.
 * @param on Use true to set bits, false to reset them.
 */
void R_PCIE_SetCmd(struct st_pcie_host *host, pcie_bdf_t bdf, uint32_t bits, bool on);

/**
 * @brief Allocate an IRQ for an endpoint.
 *
 * This function first checks the IRQ register and if it contains a valid
 * value this is returned. If the register does not contain a valid value
 * allocation of a new one is attempted.
 * Such function is only exposed if CONFIG_PCIE_CONTROLLER is unset.
 * It is thus available where architecture tied dynamic IRQ allocation for
 * PCIe device makes sense.
 *
 * @param[in] ctrl PCI Express Controller device pointer.
 * @param bdf the PCI(e) endpoint.
 *
 * @return The IRQ number, or PCIE_CONF_INTR_IRQ_NONE if allocation failed.
 */
unsigned int R_PCIE_AllocIRQ(struct st_pcie_host *host, pcie_bdf_t bdf);

/**
 * @brief Return the IRQ assigned by the firmware/board to an endpoint.
 *
 * @param[in] ctrl PCI Express Controller device pointer.
 * @param bdf The PCI(e) endpoint.
 *
 * @return The IRQ number, or PCIE_CONF_INTR_IRQ_NONE if unknown.
 */
unsigned int R_PCIE_GetIRQ(struct st_pcie_host *host, pcie_bdf_t bdf);

/**
 * @brief Enable the PCI(e) endpoint to generate the specified IRQ.
 *
 * If MSI is enabled and the endpoint supports it, the endpoint will
 * be configured to generate the specified IRQ via MSI. Otherwise, it
 * is assumed that the IRQ has been routed by the boot firmware
 * to the specified IRQ, and the IRQ is enabled (at the I/O APIC, or
 * wherever appropriate).
 *
 * @param bdf The PCI(e) endpoint.
 * @param irq The IRQ to generate.
 *
 */
void R_PCIE_EnableIRQ(struct st_pcie_host *host, pcie_bdf_t bdf, unsigned int irq);

/**
 * @brief Find a PCI(e) capability in an endpoint's configuration space.
 *
 * @param bdf The PCI endpoint to examine.
 * @param cap_id The capability ID of interest.
 *
 * @return The index of the configuration word, or 0 if no capability.
 */
uint32_t R_PCIE_GetCap(struct st_pcie_host *host, pcie_bdf_t bdf, uint32_t cap_id);

/**
 * @brief Find an Extended PCI(e) capability in an endpoint's configuration space.
 *
 * @param bdf The PCI endpoint to examine.
 * @param cap_id The capability ID of interest.
 * 
 * @return The index of the configuration word, or 0 if no capability.
 */
uint32_t R_PCIE_GetExtCap(struct st_pcie_host *host, pcie_bdf_t bdf, uint32_t cap_id);

/**
 * @brief Dynamically connect a PCIe endpoint IRQ to an ISR handler.
 *
 * @param bdf The PCI endpoint to examine.
 * @param irq The IRQ to connect (see pcie_alloc_irq()).
 * @param priority Priority of the IRQ.
 * @param routine The ISR handler to connect to the IRQ.
 * @param parameter The parameter to provide to the handler.
 * @param flags IRQ connection flags.
 *
 * @return True if connected, false otherwise.
 */
bool R_PCIE_ConnectDynamicIRQ(struct st_pcie_host *host, 
                            pcie_bdf_t bdf,
                            unsigned int irq,
                            unsigned int priority,
                            void (*routine)(const void *parameter),
                            const void *parameter,
                            uint32_t flags);

/**
 * @brief Function called to allocate a memory region subset for an endpoint Base Address Register.
 *
 * When enumerating PCIe Endpoints, Type0 endpoints can require up to 6 memory zones
 * via the Base Address Registers from I/O or Memory types.
 *
 * This call allocates such zone in the PCI Express Controller memory regions if
 * such region is available and space is still available.
 * 
 * @param[in] ctrl PCI Express Controller device pointer.
 * @param[in] bdf PCI(e) endpoint.
 * @param[in] mem True if the BAR is of memory type.
 * @param[in] mem64 True if the BAR is of 64bit memory type.
 * @param[in] bar_size Size in bytes of the Base Address Register as returned by HW.
 * @param[in] bar_bus_addr Bus-centric address allocated to be written in the BAR register.
 */
void R_PCIE_RegionAllocate(struct st_pcie_host *host, pcie_bdf_t bdf, bool mem, bool mem64, size_t bar_size, uint32_t *bar_bus_addr);

/**
 * @brief Function called to get the current allocation base of a memory region subset
 * for an endpoint Base Address Register.
 *
 * When enumerating PCIe Endpoints, Type1 bridge endpoints requires a range of memory
 * allocated by all endpoints in the bridged bus.
 *
 * @param[in] ctrl PCI Express Controller device pointer.
 * @param[in] bdf PCI(e) endpoint.
 * @param[in] mem True if the BAR is of memory type.
 * @param[in] mem64 True if the BAR is of 64bit memory type.
 * @param[in] align Size to take in account for alignment.
 * @param[out] bar_base_addr Bus-centric address allocation base.
 *
 * @return True if allocation was possible, False if allocation failed.
 */
bool R_PCIE_RegionGetAllocateBase(struct st_pcie_host *host, pcie_bdf_t bdf, bool mem, bool mem64, size_t align, uint32_t *bar_base_addr);

/**
 * @brief Function called to translate an endpoint Base Address Register bus-centric address
 * into Physical address.
 *
 * When enumerating PCIe Endpoints, Type0 endpoints can require up to 6 memory zones
 * via the Base Address Registers from I/O or Memory types.
 *
 * The bus-centric address set in this BAR register is not necessarily accessible from the CPU,
 * thus must be translated by using the PCI Express Controller memory regions translation
 * ranges to permit mapping from the CPU.
 *
 * @param[in] ctrl PCI Express Controller device pointer.
 * @param[in] bdf PCI(e) endpoint.
 * @param[in] mem True if the BAR is of memory type.
 * @param[in] mem64 True if the BAR is of 64bit memory type.
 * @param[in] bar_bus_addr Bus-centric address written in the BAR register.
 * @param[out] bar_addr CPU-centric address translated from the bus-centric address.
 *
 * @return True if translation was possible, False if translation failed.
 */
bool R_PCIE_RegionTranslate(struct st_pcie_host *host, pcie_bdf_t bdf, bool mem, bool mem64, size_t bar_size, uint32_t *bar_addr);

/**
* @brief Performing PCIe Outbound ATU
*/
void R_PCIE_Host_Outbound_ATU(uint16_t channel);
/*---------------------------------------- Configuration bits -----------------------------------------*/

/*
 * Configuration word 13 contains the head of the capabilities list.
 */

/**
 * @brief define
 */

#define PCIE_CONF_CAPPTR    13U    /* capabilities pointer */

/**
 * @brief define pcie
 */
#define PCIE_CONF_CAPPTR_FIRST(w)    (((w) >> 2) & 0x3FU)

/*
 * The first word of every capability contains a capability identifier,
 * and a link to the next capability (or 0) in configuration space.
 */

/**
 * @brief define pcie
 */
#define PCIE_CONF_CAP_ID(w)        ((w) & 0xFFU)

/**
 * @brief define pcie
 */
#define PCIE_CONF_CAP_NEXT(w)        (((w) >> 10) & 0x3FU)

/*
 * The extended PCI Express capabilities lie at the end of the PCI configuration space
 */
/**
 * @brief define pcie
 */
#define PCIE_CONF_EXT_CAPPTR    64U

/*
 * The first word of every capability contains an extended capability identifier,
 * and a link to the next capability (or 0) in the extended configuration space.
 */

/**
 * @brief define pcie
 */
#define PCIE_CONF_EXT_CAP_ID(w)        ((w) & 0xFFFFU)

/**
 * @brief define pcie
 */
#define PCIE_CONF_EXT_CAP_VER(w)    (((w) >> 16) & 0xFU)

/**
 * @brief define pcie
 */
#define PCIE_CONF_EXT_CAP_NEXT(w)    (((w) >> 20) & 0xFFFU)

/*
 * Configuration word 0 aligns directly with pcie_id_t.
 */

/**
 * @brief define pcie
 */
#define PCIE_CONF_ID        0U

/*
 * Configuration word 1 contains command and status bits.
 */

/**
 * @brief define pcie
 */
#define PCIE_CONF_CMDSTAT    1U    /* command/status register */

/**
 * @brief define pcie
 */
#define PCIE_CONF_CMDSTAT_IO        0x00000001U  /* I/O access enable */
/**
 * @brief define pcie
 */
#define PCIE_CONF_CMDSTAT_MEM        0x00000002U  /* mem access enable */
/**
 * @brief define pcie
 */
#define PCIE_CONF_CMDSTAT_MASTER    0x00000004U  /* bus master enable */
/**
 * @brief define pcie
 */
#define PCIE_CONF_CMDSTAT_INTERRUPT    0x00080000U  /* interrupt status */
/**
 * @brief define pcie
 */
#define PCIE_CONF_CMDSTAT_CAPS        0x00100000U  /* capabilities list */

/*
 * Configuration word 2 has additional function identification that
 * we only care about for debug output (PCIe shell commands).
 */
/**
 * @brief define pcie
 */
#define PCIE_CONF_CLASSREV    2U    /* class/revision register */

/**
 * @brief define pcie
 */
#define PCIE_CONF_CLASSREV_CLASS(w)    (((w) >> 24) & 0xFFU)
/**
 * @brief define pcie
 */
#define PCIE_CONF_CLASSREV_SUBCLASS(w)  (((w) >> 16) & 0xFFU)
/**
 * @brief define pcie
 */
#define PCIE_CONF_CLASSREV_PROGIF(w)    (((w) >> 8) & 0xFFU)
/**
 * @brief define pcie
 */
#define PCIE_CONF_CLASSREV_REV(w)    ((w) & 0xFFU)

/*
 * The only part of configuration word 3 that is of interest to us is
 * the header type, as we use it to distinguish functional endpoints
 * from bridges (which are, for our purposes, transparent).
 */

/**
 * @brief define pcie
 */
#define PCIE_CONF_TYPE        3U

/**
 * @brief define pcie
 */
#define PCIE_CONF_MULTIFUNCTION(w)    (((w) & 0x00800000U) != 0U)
/**
 * @brief define pcie
 */
#define PCIE_CONF_TYPE_BRIDGE(w)    (((w) & 0x007F0000U) != 0U)
/**
 * @brief define pcie
 */
#define PCIE_CONF_TYPE_GET(w)        (((w) >> 16) & 0x7F)

/**
 * @brief define pcie
 */
#define PCIE_CONF_TYPE_STANDARD         0x0U
/**
 * @brief define pcie
 */
#define PCIE_CONF_TYPE_PCI_BRIDGE       0x1U
/**
 * @brief define pcie
 */
#define PCIE_CONF_TYPE_CARDBUS_BRIDGE   0x2U

/*
 * Words 4-9 are BARs are I/O or memory decoders. Memory decoders may
 * be 64-bit decoders, in which case the next configuration word holds
 * the high-order bits (and is, thus, not a BAR itself).
 */

/**
 * @brief define pcie
 */
#define PCIE_CONF_BAR0        4U
/**
 * @brief define pcie
 */
#define PCIE_CONF_BAR1        5U
/**
 * @brief define pcie
 */
#define PCIE_CONF_BAR2        6U
/**
 * @brief define pcie
 */
#define PCIE_CONF_BAR3        7U
/**
 * @brief define pcie
 */
#define PCIE_CONF_BAR4        8U
/**
 * @brief define pcie
 */
#define PCIE_CONF_BAR5        9U

/**
 * @brief define pcie
 */
#define PCIE_CONF_BAR_IO(w)        (((w) & 0x00000001U) == 0x00000001U)
/**
 * @brief define pcie
 */
#define PCIE_CONF_BAR_MEM(w)        (((w) & 0x00000001U) != 0x00000001U)
/**
 * @brief define pcie
 */
#define PCIE_CONF_BAR_64(w)        (((w) & 0x00000006U) == 0x00000004U)
/**
 * @brief define pcie
 */
#define PCIE_CONF_BAR_ADDR(w)        ((w) & ~0xfUL)
/**
 * @brief define pcie
 */
#define PCIE_CONF_BAR_IO_ADDR(w)    ((w) & ~0x3UL)
/**
 * @brief define pcie
 */
#define PCIE_CONF_BAR_FLAGS(w)        ((w) & 0xfUL)
/**
 * @brief define pcie
 */
#define PCIE_CONF_BAR_NONE        0U

/**
 * @brief define pcie
 */
#define PCIE_CONF_BAR_INVAL        0xFFFFFFF0U
/**
 * @brief define pcie
 */
#define PCIE_CONF_BAR_INVAL64        0xFFFFFFFFFFFFFFF0UL

/**
 * @brief define pcie
 */
#define PCIE_CONF_BAR_INVAL_FLAGS(w)            \
    ((((w) & 0x00000006U) == 0x00000006U) ||    \
     (((w) & 0x00000006U) == 0x00000002U))

/*
 * Type 1 Header has files related to bus management
 */
/**
 * @brief define pcie
 */
#define PCIE_BUS_NUMBER         6U

/**
 * @brief define pcie
 */
#define PCIE_BUS_PRIMARY_NUMBER(w)      ((w) & 0xffUL)
/**
 * @brief define pcie
 */
#define PCIE_BUS_SECONDARY_NUMBER(w)    (((w) >> 8) & 0xffUL)
/**
 * @brief define pcie
 */
#define PCIE_BUS_SUBORDINATE_NUMBER(w)  (((w) >> 16) & 0xffUL)
/**
 * @brief define pcie
 */
#define PCIE_SECONDARY_LATENCY_TIMER(w) (((w) >> 24) & 0xffUL)

/**
 * @brief
 */
#define PCIE_BUS_NUMBER_VAL(prim, sec, sub, lat) \
    (((prim) & 0xffUL) |             \
     (((sec) & 0xffUL) << 8) |         \
     (((sub) & 0xffUL) << 16) |         \
     (((lat) & 0xffUL) << 24))

/*
 * @brief Type 1 words 7 to 12 setups Bridge Memory base and limits
 */
#define PCIE_IO_SEC_STATUS      7U

/**
 * @brief define pcie
 */
#define PCIE_IO_BASE(w)         ((w) & 0xffUL)
/**
 * @brief define pcie
 */
#define PCIE_IO_LIMIT(w)        (((w) >> 8) & 0xffUL)
/**
 * @brief define pcie
 */
#define PCIE_SEC_STATUS(w)      (((w) >> 16) & 0xffffUL)

/**
 * @brief define pcie
 */
#define PCIE_IO_SEC_STATUS_VAL(iob, iol, sec_status) \
    (((iob) & 0xffUL) |                 \
     (((iol) & 0xffUL) << 8) |             \
     (((sec_status) & 0xffffUL) << 16))

/**
 * @brief define pcie
 */
#define PCIE_MEM_BASE_LIMIT     8U

/**
 * @brief define pcie
 */
#define PCIE_MEM_BASE(w)        ((w) & 0xffffUL)
/**
 * @brief define pcie
 */
#define PCIE_MEM_LIMIT(w)       (((w) >> 16) & 0xffffUL)

/**
 * @brief define pcie
 */
#define PCIE_MEM_BASE_LIMIT_VAL(memb, meml) \
    (((memb) & 0xffffUL) |            \
     (((meml) & 0xffffUL) << 16))

/**
 * @brief define pcie
 */
#define PCIE_PREFETCH_BASE_LIMIT        9U

/**
 * @brief define pcie
 */
#define PCIE_PREFETCH_BASE(w)   ((w) & 0xffffUL)
/**
 * @brief define pcie
 */
#define PCIE_PREFETCH_LIMIT(w)  (((w) >> 16) & 0xffffUL)

/**
 * @brief define pcie
 */
#define PCIE_PREFETCH_BASE_LIMIT_VAL(pmemb, pmeml) \
    (((pmemb) & 0xffffUL) |               \
     (((pmeml) & 0xffffUL) << 16))

/**
 * @brief define pcie
 */
#define PCIE_PREFETCH_BASE_UPPER        10U

/**
 * @brief define pcie
 */
#define PCIE_PREFETCH_LIMIT_UPPER       11U

/**
 * @brief define pcie
 */
#define PCIE_IO_BASE_LIMIT_UPPER        12U

/**
 * @brief define pcie
 */
#define PCIE_IO_BASE_UPPER(w)   ((w) & 0xffffUL)
/**
 * @brief define pcie
 */
#define PCIE_IO_LIMIT_UPPER(w)  (((w) >> 16) & 0xffffUL)

/**
 * @brief define pcie
 */
#define PCIE_IO_BASE_LIMIT_UPPER_VAL(iobu, iolu) \
    (((iobu) & 0xffffUL) |             \
     (((iolu) & 0xffffUL) << 16))

/*
 * Word 15 contains information related to interrupts.
 *
 * We're only interested in the low byte, which is [supposed to be] set by
 * the firmware to indicate which wire IRQ the device interrupt is routed to.
 */

/**
 * @brief define pcie
 */
#define PCIE_CONF_INTR        15U

/**
 * @brief define pcie
 */
#define PCIE_CONF_INTR_IRQ(w)    ((w) & 0xFFU)
/**
 * @brief define pcie
 */
#define PCIE_CONF_INTR_IRQ_NONE    0xFFU  /* no interrupt routed */

/**
 * @brief define pcie
 */
#define PCIE_MAX_BUS  (0xFFFFFFFFU & PCIE_BDF_BUS_MASK)
/**
 * @brief define pcie
 */
#define PCIE_MAX_DEV  (0xFFFFFFFFU & PCIE_BDF_DEV_MASK)
/**
 * @brief define pcie
 */
#define PCIE_MAX_FUNC (0xFFFFFFFFU & PCIE_BDF_FUNC_MASK)

#ifdef __cplusplus
}
#endif

/** @} */ // end of PCIE_Host_Module

#endif
