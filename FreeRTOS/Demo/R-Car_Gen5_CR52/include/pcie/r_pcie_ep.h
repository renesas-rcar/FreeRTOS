/*
* Copyright (c) 2025 Renesas Electronics Corporation
*
* SPDX-License-Identifier: MIT
*/

#ifndef PCIE_EP
#define PCIE_EP

/**
 * @defgroup PCIe_EP_Module PCIe Endpoint Module
 * @{
 * @brief This module provides functions to configure and control PCIe EP communication.
 *
 * The PCIe module allows for the configuration and control of PCIe EP communication between devices.
 */

#ifdef __cplusplus
extern "C" {
#endif
/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include"pcie/r_pcie_ctrl.h"


/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/


/***********************************************************************************************************************
 * Enum definitions
 **********************************************************************************************************************/
/**
 * @brief Type of PCIe OB memory.
 */
enum pcie_ob_mem_type {
	PCIE_OB_ANYMEM,  ///< PCIe OB window within any address range.
	PCIE_OB_LOWMEM,  ///< PCIe OB window within 32-bit address range.
	PCIE_OB_HIGHMEM, ///< PCIe OB window above 32-bit address range.
};

/**
 * @brief Type of PCIe EP irq.
 */
enum pci_ep_irq_type {
	PCIE_EP_IRQ_LEGACY,	///< Raise Legacy interrupt.
	PCIE_EP_IRQ_MSI,	///< Raise MSI interrupt.
	PCIE_EP_IRQ_MSIX,	///< Raise MSIX interrupt.
};

/**
 * @brief Transfer direction.
 */
enum xfer_direction {
	HOST_TO_DEVICE,		///< Read from Host.
	DEVICE_TO_HOST,		///< Write to Host.
};

/**
 * @brief Reset PCIe.
 */
enum pcie_reset {
	PCIE_PERST,     ///< Cold reset.
	PCIE_PERST_INB, ///< Inband hot reset.
	PCIE_FLR,       ///< Functional Level Reset.
	PCIE_RESET_MAX  ///< Reset.
};

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/
/**
 * @typedef pcie_ep_reset_callback_t.
 * @brief Callback API for PCIe reset interrupts.
 *
 * These callbacks execute in interrupt context. Therefore, use only
 * interrupt-safe APIS. Registration of callbacks is done via
 * @a R_PCIE_EP_RegisterResetCB.
 *
 * @param[in] arg Pointer provided at registration time, later to be 
 * passed back as argument to callback function.
 */
typedef void (*pcie_ep_reset_callback_t)(void *arg);

/***********************************************************************************************************************
 * Struct definitions
 **********************************************************************************************************************/
/**
 * @brief Endpoint device structure for the driver instance.
 */
struct st_pcie_ep
{
    struct st_pcie_ctrl ctrl; ///< The controller that control this host.
    enum pci_barno	test_reg_bar;
    bool		msi_cap;
    unsigned long	*ib_window_map;
    unsigned long	*ob_window_map;
    pcie_irq_t irq[];         ///< List of interrupt source and its callback function.
};

/***********************************************************************************************************************
 * Public APIs
 **********************************************************************************************************************/
/**
 * @brief Initialize PCIe channel as an Endpoint device.
 *
 * @param ep      Pointer to the device structure for the driver instance.
 * @param channel Channel of PCIe to init.
 */
void R_PCIE_EP_Init(struct st_pcie_ep *ep, uint16_t channel);

/**
 * @brief Map a host memory buffer to PCIe outbound region.
 *
 * @details This API maps a host memory buffer to PCIe outbound region,
 *	    It is left to EP driver to manage multiple mappings through
 *	    multiple PCIe outbound regions if supported by SoC.
 *
 * @param ep          Pointer to the device structure for the driver instance.
 * @param pcie_addr   Host memory buffer address to be mapped.
 * @param mapped_addr Mapped PCIe outbound region address.
 * @param size        Host memory buffer size (bytes).
 * @param ob_mem_type Hint if lowmem/highmem outbound region has to be used,
 *		      this is useful in cases where bus master cannot generate
 *		      more than 32-bit address; it becomes essential to use
 *		      lowmem outbound region.
 *
 * @return Mapped size : If mapped size is less than requested size,
 *	   then requester has to call the same API again to map
 *	   the unmapped host buffer after data transfer is done with
 *	   mapped size. This situation may arise because of the
 *	   mapping alignment requirements.
 *
 * @return Negative errno code if failure.
 */
int32_t R_PCIE_EP_MapAddress(struct st_pcie_ep *ep, uint32_t pcie_addr,
			uint32_t *mapped_addr, uint32_t size,
			enum pcie_ob_mem_type ob_mem_type);


/**
 * @brief Remove mapping to PCIe outbound region.
 *
 * @details This API removes mapping to PCIe outbound region.
 *	    Mapped PCIe outbound region address is given as argument
 *	    to figure out the outbound region to be unmapped.
 *
 * @param ep          Pointer to the device structure for the driver instance.
 * @param mapped_addr PCIe outbound region address.
 */
void R_PCIE_EP_UnmapAddress(struct st_pcie_ep *ep, uint32_t mapped_addr);

/**
 * @brief Raise interrupt to Host.
 *
 * @details This API raises interrupt to Host.
 *
 * @param   dev      Pointer to the device structure for the driver instance.
 * @param   irq_type Type of Interrupt be raised (legacy, MSI or MSI-X).
 * @param   irq_num  MSI or MSI-X interrupt number.
 *
 * @return 0 if successful, negative errno code if failure.
 */
int32_t R_PCIE_EP_RaiseIRQ(struct st_pcie_ep *ep,
				    enum pci_ep_irq_type irq_type,
				    uint32_t irq_num);

/**
 * @brief Register callback function for reset interrupts.
 *
 * @details If reset interrupts are handled by device, this API can be
 *	    used to register callback function, which will be
 *	    executed part of corresponding PCIe reset handler.
 *
 * @param   dev   Pointer to the device structure for the driver instance.
 * @param   reset Reset interrupt type.
 * @param   cb    Callback function being registered.
 * @param   arg   Argument to be passed back to callback function.
 *
 * @return 0 if successful, negative errno code if failure.
 */
int32_t R_PCIE_EP_RegisterResetCB(struct st_pcie_ep *ep,
					    enum pcie_reset reset,
					    pcie_ep_reset_callback_t cb,
					    void *arg);

/**
 * @brief Data transfer between mapped Host memory and device memory with
 *	  "System DMA". The term "System DMA" is used to clarify that we
 *	  are not talking about dedicated "PCIe DMA"; rather the one
 *	  which does not understand PCIe address directly, and
 *	  uses the mapped Host memory.
 *
 * @details If DMA controller is available in the EP device, this API can be
 *	    used to achieve data transfer between mapped Host memory,
 *	    i.e. outbound memory and EP device's local memory with DMA.
 *
 * @param   dev         Pointer to the device structure for the driver instance.
 * @param   mapped_addr Mapped Host memory address.
 * @param   local_addr  Device memory address.
 * @param   size        DMA transfer length (bytes).
 * @param   dir         Direction of DMA transfer.
 *
 * @return 0 if successful, negative errno code if failure.
 */
int32_t R_PCIE_EP_DMATransferConfig(struct st_pcie_ep *ep,
				   uint64_t mapped_addr,
				   uintptr_t local_addr, uint32_t size,
				   const enum xfer_direction dir);


/**
 * @brief Data transfer using memcpy.
 *
 * @details Helper API to achieve data transfer with memcpy
 *          through PCIe outbound memory.
 *
 * @param ep          Pointer to the device structure for the driver instance.
 * @param pcie_addr   Host memory buffer address.
 * @param local_addr  Local memory buffer address.
 * @param size        Data transfer size (bytes).
 * @param ob_mem_type Hint if lowmem/highmem outbound region has to be used
 *                    (PCIE_OB_LOWMEM / PCIE_OB_HIGHMEM / PCIE_OB_ANYMEM),
 *                    should be PCIE_OB_LOWMEM if bus master cannot generate
 *                    more than 32-bit address.
 * @param dir         Data transfer direction (HOST_TO_DEVICE / DEVICE_TO_HOST).
 *
 * @return 0 if successful, negative errno code if failure.
 */
int R_PCIE_EP_TransferDataMemCpy(struct st_pcie_ep *ep, uint64_t pcie_addr,
			     uintptr_t *local_addr, uint32_t size,
			     enum pcie_ob_mem_type ob_mem_type,
			     enum xfer_direction dir);

/**
 * @brief Data transfer using system DMA.
 *
 * @details Helper API to achieve data transfer with system DMA through PCIe
 *          outbound memory, this API is based off R_PCIE_EP_TransferDataMemCpy,
 *          here we use "system dma" instead of memcpy.
 *
 * @param ep          Pointer to the device structure for the driver instance.
 * @param pcie_addr   Host memory buffer address.
 * @param local_addr  Local memory buffer address.
 * @param size        Data transfer size (bytes).
 * @param ob_mem_type Hint if lowmem/highmem outbound region has to be used
 *                    (PCIE_OB_LOWMEM / PCIE_OB_HIGHMEM / PCIE_OB_ANYMEM).
 * @param dir         Data transfer direction (HOST_TO_DEVICE / DEVICE_TO_HOST).
 *
 * @return 0 if successful, negative errno code if failure.
 */
int R_PCIE_EP_TransferDataDMA(struct st_pcie_ep *ep, uint64_t pcie_addr,
			  uintptr_t *local_addr, uint32_t size,
			  enum pcie_ob_mem_type ob_mem_type,
			  enum xfer_direction dir);

/** @} */ // end of PCIE_EP_Module

/**
* @brief Performing PCIe Inbound ATU
*/
void R_PCIE_EP_Inbound_ATU(struct st_pcie_ep *ep, uint16_t channel);
/**
* @brief Receiving test cmd from Host
*/
void R_PCIE_EPF_Test_CmdHandler(struct st_pcie_ep *ep);

#ifdef __cplusplus
}
#endif

#endif
