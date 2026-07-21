/*
* Copyright (c) 2025 Renesas Electronics Corporation
*
* SPDX-License-Identifier: MIT
*/

#include "pcie/r_pcie_ep.h"
#include "ucie.h"
#include <string.h>
#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"

#define MSI_LOCAL_ADDR   ((volatile uint32_t *)0x60000000)
#define MSI_MSG_DATA     0x0000

/* Implement functions for Endpoint */

typedef struct ucie_epf_test_reg {
    uint32_t	magic;
    uint32_t	command;
    uint32_t	status;
    uint32_t	src_addr_lower;
    uint32_t    src_addr_upper;
    uint32_t	dst_addr_lower;
    uint32_t    dst_addr_upper;
    uint32_t	size;
    uint32_t	checksum;
    uint32_t	irq_type;
    uint32_t	irq_number;
    uint32_t	flags;
} ucie_epf_test_reg_t ;

int ucie_epf_test_read(struct st_pcie_ep *ep, ucie_epf_test_reg_t *bar0_reg)
{
    uint32_t ret;
    void *buf;
    void *src_addr = (void *)UCIE_ADDR_SPACE;
    uint32_t size = bar0_reg->size;
    uint32_t free_win = 2;
    uint32_t index, bit_pos;
    uint64_t remote_rc;
    uint32_t channel = 1; //EP

    if (size == 0 || size > MAX_TRANSFER_SIZE) {
        printf_delay("Invalid transfer size: %d\n", size);
        return -EINVAL;
    }

    buf = malloc(size);
    if (!buf) {
	printf_delay("Fail to allocate the buffer\n");
	return -ENOMEM;
    }

    remote_rc = ((uint64_t)bar0_reg->src_addr_upper << 32) | bar0_reg->src_addr_lower;

    /* Map the EP UCIe/CXL address with the remote RC physical address */
    R_PCIE_Outbound_ATU(channel, (uint64_t)(uintptr_t)src_addr, remote_rc);

    index = free_win / 32;
    bit_pos = free_win % 32;
    ep->ob_window_map[index] |= (1U << bit_pos);

    printf_delay("[%s] Executing memcpy\n",__func__);
    vTaskDelay(2);
    memcpy(buf, (void *)src_addr, size);
    printf_delay("[%s] Memcpy done to 0x%x, size %d\n",__func__, (unsigned long long)(uintptr_t)src_addr, size);

    free(buf);

    return ret;
}

int ucie_epf_test_write(struct st_pcie_ep *ep, ucie_epf_test_reg_t *bar0_reg)
{
    uint32_t ret, i;
    void *dst_addr = (void *)UCIE_ADDR_SPACE;
    void *buf;
    uint32_t size = bar0_reg->size;
    uint32_t free_win = 1;
    uint32_t index, bit_pos;
    uint64_t remote_rc;
    uint32_t channel = 1; //EP

    if (size == 0 || size > MAX_TRANSFER_SIZE) {
        printf_delay("Invalid transfer size: %d\n", size);
        return -EINVAL;
    }

    remote_rc = ((uint64_t)bar0_reg->dst_addr_upper << 32) | bar0_reg->dst_addr_lower;

    /* Map the EP UCIe/CXL address with the remote RC physical address */
    R_PCIE_Outbound_ATU(channel, (uint64_t)(uintptr_t)dst_addr, remote_rc);

    index = free_win / 32;
    bit_pos = free_win % 32;
    ep->ob_window_map[index] |= (1U << bit_pos);

    buf = malloc(size);
    if (!buf) {
	printf_delay("Failed to allocate buffer\n");
	return -ENOMEM;
    }

    /* Create the test data */
    for (i = 0; i < size; i++) {
        ((uint8_t *)buf)[i] = i & 0xFF;
    }

    printf_delay("[%s] Executing memcpy\n",__func__);
    vTaskDelay(2);
    memcpy((void *)dst_addr, buf, size);
    printf_delay("[%s] Memcpy done to 0x%x, size %d\n",__func__, (unsigned long long)(uintptr_t)dst_addr, size);

    free(buf);

    return ret;
}

void ucie_epf_test_raise_irq(struct st_pcie_ep *ep, ucie_epf_test_reg_t *bar0_reg)
{
    (void)ep;
    uint32_t channel = 1;
    bar0_reg->status |= STATUS_IRQ_RAISED;
    void *msi_mem = (void *)0xa645000;

    /* Raise MSI per the PCI Local Bus Specification Revision 3.0, 6.8.1. */
    R_PCIE_Outbound_ATU(channel, 0x60000000, 0x437e8000);
    *MSI_LOCAL_ADDR = MSI_MSG_DATA;
}

void R_PCIE_EPF_Test_CmdHandler(struct st_pcie_ep *ep)
{
    int ret;
    uint32_t command;
    ucie_epf_test_reg_t *bar0_reg = (ucie_epf_test_reg_t *)0x45142000;

    while(1) {
	command = bar0_reg->command;
    if (command == 0) {
	    vTaskDelay(2);
    }

	bar0_reg->command = 0;
	bar0_reg->status = 0;

	vTaskDelay(10);
	if (command & COMMAND_READ) {
	    printf_delay("[%s]: Received Read Request\n",__func__);
	    ret = ucie_epf_test_read(ep, bar0_reg);
        if (!ret) {
		bar0_reg->status |= STATUS_READ_SUCCESS;
        } else {
		bar0_reg->status |= STATUS_READ_FAIL;
        }
	    ucie_epf_test_raise_irq(ep, bar0_reg);
	    vTaskDelay(2);

	} else if (command & COMMAND_WRITE) {
	    printf_delay("[%s]: Received Write Request\n",__func__);
	    ret = ucie_epf_test_write(ep, bar0_reg);
        if (!ret) {
		bar0_reg->status |= STATUS_WRITE_SUCCESS;
        } else {
		bar0_reg->status |= STATUS_WRITE_FAIL;
        }
	    ucie_epf_test_raise_irq(ep, bar0_reg);
	    vTaskDelay(2);
	}
    }
}

static void rcar_ucie_ep_hw_enable(uint16_t channel)
{
    /* Configure as Endpoint */
    rcar_ucie_reg_write32(channel, false, true, IMP_CORECONFIG_CONFIG0, UCIECTL_DEF_EP_EN);
    rcar_ucie_controller_enable(channel);

    //rcar_ucie_phy_enable(channel);
}

static void rcar_ucie_ep_header(uint16_t channel)
{
    R_UCIE_RegWrite16(channel, UCIE_VENDOR_ID, 0x16c3);
    R_UCIE_RegWrite16(channel, UCIE_DEVICE_ID, 0xeddau);
    //R_UCIE_RegWrite16(channel, 0x3d, 0x1);
}

static int rcar_ucie_ep_init(struct st_pcie_ep *ep, uint16_t channel)
{
    uint8_t hdr_type;
    uint32_t reg;
    uint32_t flags = 0;

    /* Allocate and initialize the ib/ob window map
       assuming number of ib/out window = 32 */
    ep->ib_window_map = pvPortMalloc(32 * sizeof(uint32_t));
    if (ep->ib_window_map == NULL) {
	return -ENOMEM;
    }
    memset(ep->ib_window_map, 0, 32 * sizeof(uint32_t));

    ep->ob_window_map = pvPortMalloc(32 * sizeof(uint32_t));
    if (ep->ob_window_map == NULL) {
        return -ENOMEM;
    }
    memset(ep->ob_window_map, 0, 32 * sizeof(uint32_t));

    R_UCIE_RegWrite16(channel, UCIE_COMMAND, 0);

    /* Setting for BAR0 */
    R_UCIE_RegWrite32(channel, UCIE_DBI2_BASE_BAR0, 0);
    R_UCIE_RegWrite32(channel, UCIE_BASE_BAR0, 0x0);

    R_UCIE_RegWrite32(channel, UCIE_DBI2_BASE_BAR0 + 4, 0);
    R_UCIE_RegWrite32(channel, UCIE_BASE_BAR0 + 4, 0);

    R_UCIE_RegWrite32(channel, UCIE_EXT_REBAR + UCIE_REBAR_CAP, 0x10);

    hdr_type = R_UCIE_RegRead8(channel, UCIE_HEADER_TYPE) & UCIE_HEADER_TYPE_MASK;
    if (hdr_type != UCIE_HEADER_TYPE_NORMAL) {
	printf_delay("UCIe controller is not set to EP mode (hdr_type:0x%x)!\n", hdr_type);
	return -EIO;
    } else {
	printf_delay("UCIe controller is set to EP mode\n");
	return 0;
    }
}

int R_PCIE_EP_TransferDataDMA(struct st_pcie_ep *ep, uint64_t pcie_addr,
                          uintptr_t *local_addr, uint32_t size,
                          enum pcie_ob_mem_type ob_mem_type,
                          enum xfer_direction dir)
{
    (void)ep;
    (void)ob_mem_type;
    uint32_t i;
    uint16_t channel = 1;
    uintptr_t addr = (uintptr_t)pcie_addr;
    uint8_t *pcie_data = (uint8_t *)addr;

    if (dir == DEVICE_TO_HOST) {
	/* DMA Write channel 0 enable */
	R_UCIE_RegWrite32(channel, UCIE_DMA_WR_EN, 0x1);

	/* DMA write channel interrupt setup */
	R_UCIE_RegWrite32(channel, UCIE_DMA_WR_INT_SET, 0x50);

	/* DMA write channel control setting 1 */
	R_UCIE_RegWrite32(channel, UCIE_DMA_WR_CTL, 0x2);

	/* DMA write channel function number */
	R_UCIE_RegWrite32(channel, UCIE_DMA_WR_FUNC_NUM, 0x0);

	/* DMA write channel QOS setting */
	R_UCIE_RegWrite32(channel, UCIE_DMA_WR_QOS, 0x0);

	/* DMA write channel transfer size to 1KB */
	R_UCIE_RegWrite32(channel, UCIE_DMA_WR_SIZE, 0x00000400);

	/* DMA write channel SAR low */
	R_UCIE_RegWrite32(channel, UCIE_DMA_WR_SAR_LOW, (uint32_t)local_addr & 0xFFFFFFFF);

	/* DMA write channel SAR high */
	R_UCIE_RegWrite32(channel, UCIE_DMA_WR_SAR_HIGH, 0x00000000);

	/* DMA write channel DAR low */
	R_UCIE_RegWrite32(channel, UCIE_DMA_WR_DAR_LOW, UCIE_D2D_CH0_LOWER);

	/* DMA write channel DAR high */
	R_UCIE_RegWrite32(channel, UCIE_DMA_WR_DAR_HIGH, UCIE_D2D_CH0_UPPER);

	/* DMA write channel 0 doorbell */
	R_UCIE_RegWrite32(channel, UCIE_DMA_WR_DOORBELL, 0x00000001);

	printf_delay("WAIT_INT_WRITE\n");
	while (R_UCIE_RegRead32(channel, UCIE_DMA_WR_INT_STT) && 0x1 == 0x0) {
		i++;
		if (i == 10) {
			printf_delay("TIMEOUT\n");
			break;
		}
	}
	/* Clear interrupt */
	R_UCIE_RegWrite32(channel, UCIE_DMA_WR_INT_CLR, 0x1);

    } else {
	/* DMA Read channel 0 enable */
	R_UCIE_RegWrite32(channel, UCIE_DMA_RD_EN, 0x1);

	/* DMA read channel interrupt setup */
        R_UCIE_RegWrite32(channel, UCIE_DMA_RD_INT_SET, 0x50);

	/* DMA read channel control setting 1 */
        R_UCIE_RegWrite32(channel, UCIE_DMA_RD_CTL, 0x2);

	/* DMA read channel function number */
        R_UCIE_RegWrite32(channel, UCIE_DMA_RD_FUNC_NUM, 0x0);

	/* DMA read channel QOS setting */
        R_UCIE_RegWrite32(channel, UCIE_DMA_RD_QOS, 0x0);

	/* DMA read channel transfer size */
        R_UCIE_RegWrite32(channel, UCIE_DMA_RD_SIZE, 0x00000400);

	/* DMA read channel SAR low */
        R_UCIE_RegWrite32(channel, UCIE_DMA_RD_SAR_LOW, UCIE_D2D_CH0_LOWER);

	/* DMA read channel SAR high  */
        R_UCIE_RegWrite32(channel, UCIE_DMA_RD_SAR_HIGH, UCIE_D2D_CH0_UPPER);

	/* DMA read channel DAR low */
        R_UCIE_RegWrite32(channel, UCIE_DMA_RD_DAR_LOW, (uint32_t)local_addr & 0xFFFFFFFF);

	/* DMA read channel DAR high */
        R_UCIE_RegWrite32(channel, UCIE_DMA_RD_DAR_HIGH, 0x00000000);

	/* DMA read channel 0 doorbell */
        R_UCIE_RegWrite32(channel, UCIE_DMA_RD_DOORBELL, 0x00000001);

	printf_delay("WAIT_INT_READ\n");
	while (R_UCIE_RegRead32(channel, UCIE_DMA_RD_INT_STT) && 0x1 == 0x0) {
                i++;
                if (i == 10) {
                        printf_delay("TIMEOUT\n");
                        break;
		}
        }
	/* Clear interrupt */
        R_UCIE_RegWrite32(channel, UCIE_DMA_RD_INT_CLR, 0x1);
    }

    if (memcmp(pcie_data, local_addr, size) == 0) {
	printf_delay("PASS\n");
    } else {
	printf_delay("FAILED\n");
    }
}

void R_PCIE_EP_Inbound_ATU(struct st_pcie_ep *ep, uint16_t channel)
{
    uint32_t free_win = 0;
    uint32_t index, bit_pos;

    /* Inbound ATU BAR0 (BAR match mode) configuration */
    R_UCIE_RegWrite32(channel, UCIE_IB_ATU_LOWER_TARGET, 0x45142000);
    R_UCIE_RegWrite32(channel, UCIE_IB_ATU_UPPER_TARGET, 0);
    R_UCIE_RegWrite32(channel, UCIE_IB_REGION_CTL1, 0);
    R_UCIE_RegWrite32(channel, UCIE_IB_REGION_CTL2, 0xc0080000u);

    index = free_win / 32;
    bit_pos = free_win % 32;
    ep->ib_window_map[index] |= (1U << bit_pos);
}

void R_PCIE_EP_Init(struct st_pcie_ep *ep, uint16_t channel)
{
    /* Init a controller in Endpoint mode */

    /* FIXME: Confirm the used of these registers */
/*  writel(0x00004141, 0xDCE005E8);
    writel(0x00004141, 0xDDE005E8);

    R_UCIE_RegWrite32(channel, 0xF01000, 0x08010000); //MmInitCtrl
    R_UCIE_RegWrite32(channel, 0xF0211C, 0x0003BF85); //AcsmLtmlndex0Var1
    R_UCIE_RegWrite32(channel, 0xF0212C, 0x00043FED); //AcsmLtmlndex0Var5
    R_UCIE_RegWrite32(channel, 0xF0214C, 0x000CF816); //AcsmLtmlndex0Var13

    R_UCIE_RegWrite32(channel, 0x118, 0x0000E1E0);
    R_UCIE_RegWrite32(channel, 0x10118, 0x0000E1E0);
    R_UCIE_RegWrite32(channel, 0, 0xABCD16C3);
    R_UCIE_RegWrite32(channel, 0x1C0, 0x00000200);
    R_UCIE_RegWrite32(channel, 0xC48, 0x00800000);
    R_UCIE_RegWrite32(channel, 0x448, 0x00000007);
    R_UCIE_RegWrite32(channel, 0x70, 0x8002B010);
    R_UCIE_RegWrite32(channel, 0x8BC, 0x040BFF48);
*/
    rcar_ucie_dbi_ro_wr_en(channel, true);

    //rcar_ucie_ep_hw_enable(channel);

    if (rcar_ucie_ep_init(ep, channel)) {
	printf_delay("Failed to initialize UCIe EP!\n");
    }

    rcar_ucie_setup(channel);

    rcar_ucie_ep_header(channel);

    if (ep->msi_cap) {
	R_UCIE_RegWrite16(channel, UCIE_MSI_CAP, 0x8a);
    }

    R_PCIE_EP_Inbound_ATU(ep, channel);

    rcar_ucie_dbi_ro_wr_en(channel, false);

    printf_delay("Wait for request from UCIe RC...\n");
    R_PCIE_EPF_Test_CmdHandler(ep);
}
