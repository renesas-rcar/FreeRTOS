/*
* Copyright (c) 2025 Renesas Electronics Corporation
*
* SPDX-License-Identifier: MIT
*/
#ifndef PCIE_CTRL
#define PCIE_CTRL

/**
 * @defgroup PCIe_Module PCIe Module
 * @{
 * @brief This module provides functions to configure and control PCIe communication.
 *
 * The PCIe module allows for the configuration and control of PCIe communication between devices.
 */

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/


/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/
/**
 * @brief A unique PCI(e) endpoint (bus, device, function).
 *
 * A PCI(e) endpoint is uniquely identified topologically using a
 * (bus, device, function) tuple. The internal structure is documented
 * in include/dt-bindings/pcie/pcie.h: see PCIE_BDF() and friends, since
 * these tuples are referenced from devicetree.
 */
typedef uint32_t pcie_bdf_t;

/** 
 * @brief Contain entry to an IRQ
 */
typedef struct
{
    uint16_t irq_id;                    ///< INT ID.
    void (*cb_func)(void * context);    ///< Pointer to callback function.
    void * context;                     ///< Pointer to the context of a callback function.
} pcie_irq_t;

/** 
 * @brief Contain PCIe BAR information.
 */

#define PCI_STD_NUM_BARS	6

typedef enum pci_barno
{
    PCIE_NO_BAR = -1,               ///< -1
    PCIE_BAR_0,                     ///< 0
    PCIE_BAR_1,                     ///< 1
    PCIE_BAR_2,                     ///< 2
    PCIE_BAR_3,                     ///< 3
    PCIE_BAR_4,                     ///< 4
    PCIE_BAR_5,                     ///< 5
} pci_barno_t;

/***********************************************************************************************************************
 * Struct definitions
 **********************************************************************************************************************/
/**
 * @brief PCIe Base Address Register
 */
struct pcie_bar
{
    uintptr_t phys_addr;            ///< Physical address of the PCIe BAR.
    void *addr;                     ///< Virtual address of the PCIe BAR.
    size_t size;                    ///< Size of the PCIe BAR region in bytes.
    pci_barno_t barno;              ///< BAR number (index) that identifies the specific BAR in the PCIe device.
};

/**
 * @brief Structure describe the configuration of a PCIe controller device.
 */
struct st_pcie_cfg
{
    uintptr_t cfg_addr;             ///< Configuration space physical address.
    size_t cfg_size;                ///< Configuration space physical size.
    size_t ranges_count;            ///< BAR regions translation ranges count.
    /** 
     * @brief BAR regions translation ranges table.
     */
    struct {
        uint32_t flags;             ///< Flags as defined in the PCI Bus Binding to IEEE Std 1275-1994.
        uintptr_t pcie_bus_addr;    ///< Bus-centric offset from the start of the region.
        uintptr_t host_map_addr;    ///< CPU-centric offset from the start of the region.
        size_t map_length;          ///< Region size.
    } ranges[];
};

// /** Controller API Function mapping struct */
// struct st_pcie_ctrl_api
// {
//     uint32_t (*conf_read)(struct st_pcie_ctrl *ctrl, pcie_bdf_t bdf, uint32_t reg);
//     void     (*conf_write)(struct st_pcie_ctrl *ctrl, pcie_bdf_t bdf, uint32_t reg, uint32_t data);
// };

/** 
 * @brief Struct represent a PCIe channel or instance, do not initialize manually.
 */
struct st_pcie_ctrl
{
    uintptr_t * base_addr;           ///< Base Register address of this channel.
    struct st_pcie_cfg cfg;          ///< Device configuration structure.
    // struct st_pcie_ctrl_api * api;   // API functions mapping
};

/***********************************************************************************************************************
 * Public APIs
 **********************************************************************************************************************/
/**
 * @brief Function to initialize a PCIe Controller Device.
 * 
 * This function is hardware specific, all the controller configurations are stored in driver.
 * 
 * @param[in] channel Channel of PCIe to be initialized.
 * @param[out] ctrl Pointer to a controller that stores the PCIe device parameters.
 */
void R_PCIE_ControllerInit(uint32_t channel, struct st_pcie_ctrl *ctrl);

/** @brief Function called to read a 32-bit word from an endpoint's configuration space. 
 * 
 * Read a 32-bit word from an endpoint's configuration space with the PCI Express Controller
 * configuration space access method (I/O port, memory mapped or custom method).
 *
 * @param[in] ctrl PCI Express Controller device pointer.
 * @param[in] bdf PCI(e) endpoint.
 * @param[in] reg the configuration word index (not address).
 *
 * @return the word read (0xFFFFFFFFU if nonexistent endpoint or word).
 */
uint32_t R_PCIE_ConfigRead(struct st_pcie_ctrl *ctrl, pcie_bdf_t bdf, uint32_t reg);

/**
 * @brief Write a 32-bit word to an endpoint's configuration space.
 * 
 * Write a 32-bit word to an endpoint's configuration space with the PCI Express Controller
 * configuration space access method (I/O port, memory mapped or custom method)
 * 
 * @param[in] ctrl PCI Express Controller device pointer.
 * @param[in] bdf PCI(e) endpoint BDF.
 * @param[in] reg the configuration word index (not address).
 * @param[in] data the value to write.
 */
void R_PCIE_ConfigWrite(struct st_pcie_ctrl *ctrl, pcie_bdf_t bdf, uint32_t reg, uint32_t data);

#ifdef __cplusplus
}
#endif

/** @} */ // end of PCIE_Module

#endif 
