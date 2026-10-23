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
#include "scmi/rcar_scmi_private.h"
#include "scmi/inc/mbox.h"
#include "mfis/mfis.h"

#define MAILBOX_MAX_CHANNELS 4
#define MAILBOX_MBOX_SIZE    3

static struct scmi_dev mfis_dev;

static Context_t mfis_mailbox_cxt = {
	.ctx = (void *)&mfis_dev,
};
struct mfis_mailbox_data {
	mbox_callback_t cb[MAILBOX_MAX_CHANNELS];
	void *user_data[MAILBOX_MAX_CHANNELS];
	bool channel_enable[MAILBOX_MAX_CHANNELS];
};

struct mfis_mailbox_config {
	uintptr_t base;
};

static struct mfis_channel mfis_mailbox_ch = {
    .type = MFIS_TYPE_CR_SCP,
    .cb_function = NULL,
    .arg = &mfis_mailbox_cxt
};

static int mfis_mailbox_send(const struct scmi_dev *dev, uint32_t channel,
							const struct mbox_msg *msg)
{
    int ret = 0;
	(void) dev;

    if (channel >= (uint32_t) MAILBOX_MAX_CHANNELS) {
        ret = -EINVAL;
    }

    if ((ret == 0) && (msg == NULL)) {
        ret = mfis_trigger_interrupt(&mfis_mailbox_ch, 0U);
        goto out;
    }

    if ((ret == 0) && (msg->size != (size_t)MAILBOX_MBOX_SIZE)) {
        ret = -EMSGSIZE;
    }

out:
    return ret;
}

static int mfis_mailbox_register_callback(const struct scmi_dev *dev, uint32_t channel,
					 mbox_callback_t cb, void *user_data)
{
	struct mfis_mailbox_data *data = dev->data;

	if (channel >= (uint32_t) MAILBOX_MAX_CHANNELS) {
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

	if (channel >= (uint32_t) MAILBOX_MAX_CHANNELS) {
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

static const struct mfis_mailbox_config config;

static struct mfis_mailbox_data data;

int mfis_mailbox_init(struct mbox_spec *spec)
{
	struct scmi_dev *dev = &mfis_dev;
    int ret = 0;

    ret = mfis_init(&mfis_mailbox_ch);

	dev->api = (void*) &mfis_mailbox_driver_api;
	dev->config = (void*) &config;
	dev->data = (void*) &data;

	spec->dev = dev;

	return ret;
}