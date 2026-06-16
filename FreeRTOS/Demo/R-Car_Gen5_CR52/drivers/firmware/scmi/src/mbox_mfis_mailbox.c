/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: Apache-2.0
 */
/*
 *
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * This file is based on the
 * https://github.com/zephyrproject-rtos/zephyr/blob/main/drivers/mbox/mbox_nxp_mailbox.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Wrapper of Renesas Mailbox driver for Zephyr's MBOX model.
 */

#include "cmsis_rcar_gen5.h"
#include "interrupts.h"
#include "scmi/inc/rcar_scmi_common.h"
#include "scmi/inc/mbox.h"

#define MAILBOX_MAX_CHANNELS 4
#define MAILBOX_MBOX_SIZE    3

/* MFIS Write Protection Control Register */
#define MFIS_SCP_REG_MFISWPCNTR(base) (*((volatile uint32_t *)(base + 0x900U)))

/*
 * MFIS CPU communication message register Realtime core[m](m=0-11)
 * to SCP core.
 */
#define MFIS_SCP_REG_MFISRSEMBR(base, m) \
    (*(volatile uint32_t *)(size_t)(base + (0x1000U * (m)) + 0x44U + 0x20000U))

/*
 * MFIS CPU communication control register Realtime core[m]
 * to SCP Core(m=0-11).
 */
#define MFIS_SCP_REG_MFISRSEICR(base, m) \
    (*(volatile uint32_t *)(size_t)(base + (0x1000U * (m)) + 0x04U + 0x20000U))

/* MFIS CPU communication control register SCP core
 * to Realtime core[m](m=0-11).
 */
#define MFIS_SCP_REG_MFISRSIICR(base, m) \
    (*(volatile uint32_t *)(size_t)(base + (0x1000U * (m)) + 0x00U + 0x20000U))

static struct scmi_dev mfis_dev;

struct mfis_mailbox_data {
	mbox_callback_t cb[MAILBOX_MAX_CHANNELS];
	void *user_data[MAILBOX_MAX_CHANNELS];
	bool channel_enable[MAILBOX_MAX_CHANNELS];
	//uint32_t received_data;
};

struct mfis_mailbox_config {
	uintptr_t base;
};

static void mfis_mailbox_isr(Context_t *context)
{
	struct scmi_dev *dev = (struct scmi_dev *)context->ctx;
	struct mfis_mailbox_data *data = dev->data;
	const struct mfis_mailbox_config *cfg = dev->config;
	uint32_t mfis_rtcore_num = CURRENT_CORE_IDX;
	uint32_t mfis_irq = X5H_MFIS_SCP_IRQ_REG_SOURCE(0U) |
						X5H_MFIS_SCP_IRQ_REG_INT(0U);

	MFIS_SCP_REG_MFISRSIICR(cfg->base, mfis_rtcore_num) = 
									mfis_irq & X5H_MFIS_SCP_IRQ_REG_MASK;

	for (int i = 0; i < MAILBOX_MAX_CHANNELS; ++i) {
		/* Continue to next channel if channel is not enabled */
		if (!data->channel_enable) {
			continue;
		}

		if (data->cb[i] && data->user_data[i]) {
			data->cb[i](NULL, 0, data->user_data[i], NULL);
		}
	}
}

static int mfis_mailbox_send(const struct scmi_dev *dev, uint32_t channel,
							const struct mbox_msg *msg)
{
	const struct mfis_mailbox_config *cfg = dev->config;
	uint32_t mfis_rtcore_num = CURRENT_CORE_IDX;
	uint32_t mfis_msg = 0U;
	uint32_t mfis_irq = X5H_MFIS_SCP_IRQ_REG_SOURCE(0U) |
						X5H_MFIS_SCP_IRQ_REG_INT(1U);

	if (channel >= MAILBOX_MAX_CHANNELS) {
		return -EINVAL;
	}

	if (msg == NULL) {
		MFIS_SCP_REG_MFISRSEMBR(cfg->base, mfis_rtcore_num) = mfis_msg;
		MFIS_SCP_REG_MFISRSEICR(cfg->base, mfis_rtcore_num) =
									mfis_irq & X5H_MFIS_SCP_IRQ_REG_MASK;
		return 0;
	}

	if (msg->size != MAILBOX_MBOX_SIZE) {
		return -EMSGSIZE;
	}

	/* No code here */
	return 0;
}

static int mfis_mailbox_register_callback(const struct scmi_dev *dev, uint32_t channel,
					 mbox_callback_t cb, void *user_data)
{
	struct mfis_mailbox_data *data = dev->data;

	if (channel >= MAILBOX_MAX_CHANNELS) {
		return -EINVAL;
	}

	data->cb[channel] = cb;
	data->user_data[channel] = user_data;

	return 0;
}

static int mfis_mailbox_mtu_get(const struct scmi_dev *dev)
{
	(void)(dev);

	return MAILBOX_MBOX_SIZE;
}

static uint32_t mfis_mailbox_max_channels_get(const struct scmi_dev *dev)
{
	(void)(dev);
	return MAILBOX_MAX_CHANNELS;
}

static int mfis_mailbox_set_enabled(const struct scmi_dev *dev, uint32_t channel, bool enable)
{
	struct mfis_mailbox_data *data = dev->data;

	if (channel >= MAILBOX_MAX_CHANNELS) {
		return -EINVAL;
	}

	data->channel_enable[channel] = enable;

	return 0;
}

static const struct mbox_driver_api mfis_mailbox_driver_api = {
	.send = mfis_mailbox_send,
	.register_callback = mfis_mailbox_register_callback,
	.mtu_get = mfis_mailbox_mtu_get,
	.max_channels_get = mfis_mailbox_max_channels_get,
	.set_enabled = mfis_mailbox_set_enabled,
};

static const struct mfis_mailbox_config config = {
	.base = X5H_MFIS_SCP_BASE,
};

static struct mfis_mailbox_data data;

Context_t mfis_mailbox_cxt = {
	.ctx = (void *)&mfis_dev,
};

int mfis_mailbox_init(struct mbox_spec *spec)
{
	struct scmi_dev *dev = &mfis_dev;
	int irq_id = CURRENT_CORE_IDX + SCP2CR_INT_BASE_ID;

	if (!spec) {
		return -EINVAL;
	}

	/* Set Handler for Irq */
	Irq_SetupEntry(irq_id, (IrqHandlerFn)mfis_mailbox_isr, &mfis_mailbox_cxt);
	/* Set priority for Irq */
	Irq_SetPriority(irq_id, IPRIORITY(1));
	/* Enable Irq */
	Irq_Enable(irq_id);

	dev->api = (void*) &mfis_mailbox_driver_api;
	dev->config = (void*) &config;
	dev->data = (void*) &data;

	spec->dev = dev;

	return 0;
}

