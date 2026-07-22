/*
 *
 * Copyright (c) 2025 Renesas Electronics Corporation
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>
#include "scmi/inc/protocol.h"
#include "scmi/inc/shmem.h"
#include "scmi/inc/util.h"
#include "scmi/inc/common.h"
#include "scmi/rcar_scmi_common.h"
#include "board.h"

struct scmi_shmem_config {
	uintptr_t phys_addr;
	uint32_t size;
};

struct scmi_shmem_data {
	uintptr_t regmap[MAX_SHMEM_REGION];
};

struct scmi_shmem_layout {
	volatile uint32_t res0;
	volatile uint32_t chan_status;
	volatile uint32_t res1[2];
	volatile uint32_t chan_flags;
	volatile uint32_t len;
	volatile uint32_t msg_hdr;
};

int scmi_shmem_get_channel_status(const struct scmi_dev *dev, uint32_t *status,
								  bool is_notification)
{
	struct scmi_shmem_data *data;
	struct scmi_shmem_layout *layout;

	data = dev->data;
	if (is_notification) {
		layout = (struct scmi_shmem_layout *)data->regmap[1];
	} else {
		layout = (struct scmi_shmem_layout *)data->regmap[0];
	}

	*status = layout->chan_status;

	return 0;
}

static void scmi_shmem_memcpy(uintptr_t dst, const uintptr_t src,
							  uint32_t bytes)
{
	int i;

	for (i = 0; i < bytes; i++) {
		((uint8_t*)dst)[i] = ((const uint8_t*)src)[i];
	}
}

int scmi_shmem_read_message(const struct scmi_dev *shmem,
							struct scmi_message *msg,
							bool is_notification)
{
	struct scmi_shmem_layout *layout;
	struct scmi_shmem_data *data;
	const struct scmi_shmem_config *cfg;
	uintptr_t regmap;

	data = shmem->data;
	if (is_notification) {
		regmap = data->regmap[1];
		cfg = (struct scmi_shmem_config *)shmem->config + 1;
	} else {
		regmap = data->regmap[0];
		cfg = (struct scmi_shmem_config *)shmem->config;
	}
	layout = (struct scmi_shmem_layout *)regmap;

	/* some sanity checks first */
	if (!msg) {
		return -EINVAL;
	}

	if ((!msg->content && msg->len) || (msg->content && !msg->len)) {
		return -EINVAL;
	}

	if (cfg->size < msg->len) {
		SCMI_LOG_ERR("message doesn't fit in shmem area");
		return -EINVAL;
	}

	/* reply buffer is smaller than the actual payload size? */
	if (msg->len < (layout->len - sizeof(layout->msg_hdr))) {
		SCMI_LOG_ERR("reply buffer too small. Provided 0x%x, required 0x%x",
			msg->len,
			(uint32_t)(layout->len - sizeof(layout->msg_hdr)));
		return -EINVAL;
	}

	msg->len = (uint32_t)(layout->len - sizeof(layout->msg_hdr));

	/* header match? */
	if (layout->msg_hdr != msg->hdr) {
		SCMI_LOG_ERR("bad message header. Expected 0x%x, got 0x%x",
			msg->hdr, layout->msg_hdr);
		return -EINVAL;
	}

	if (msg->content) {
		scmi_shmem_memcpy(POINTER_TO_UINT(msg->content),
				  regmap + sizeof(*layout), msg->len);
	}

	return 0;
}


int scmi_shmem_write_message(const struct scmi_dev *shmem,
							 struct scmi_message *msg,
							 bool is_notification)
{
	struct scmi_shmem_layout *layout;
	struct scmi_shmem_data *data;
	const struct scmi_shmem_config *cfg;
	uintptr_t regmap;

	data = shmem->data;
	if (is_notification) {
		regmap = data->regmap[1];
		cfg = (struct scmi_shmem_config *)shmem->config + 1;
	} else {
		regmap = data->regmap[0];
		cfg = (struct scmi_shmem_config *)shmem->config;
	}
	layout = (struct scmi_shmem_layout *)regmap;

	/* some sanity checks first */
	if (!msg) {
		return -EINVAL;
	}

	if ((!msg->content && msg->len) || (msg->content && !msg->len)) {
		return -EINVAL;
	}

	if (cfg->size < (sizeof(*layout) + msg->len)) {
		return -EINVAL;
	}

	if (!(layout->chan_status & SCMI_SHMEM_CHAN_STATUS_FREE_BIT)) {
		return -EBUSY;
	}

	layout->len = sizeof(layout->msg_hdr) + msg->len;
	layout->msg_hdr = msg->hdr;

	if (msg->content) {
		scmi_shmem_memcpy(regmap + sizeof(*layout),
				  POINTER_TO_UINT(msg->content), msg->len);
	}

	/* done, mark channel as busy and proceed */
	layout->chan_status &= ~SCMI_SHMEM_CHAN_STATUS_FREE_BIT;

	return 0;
}

uint32_t scmi_shmem_channel_status(const struct scmi_dev *shmem,
								   bool is_notification)
{
	struct scmi_shmem_layout *layout;
	struct scmi_shmem_data *data;

	data = shmem->data;
	if (is_notification) {
		layout = (struct scmi_shmem_layout *)data->regmap[1];
	} else {
		layout = (struct scmi_shmem_layout *)data->regmap[0];
	}

	return layout->chan_status;
}

int scmi_shmem_channel_free_set(const struct scmi_dev *shmem,
								   bool is_notification)
{
	struct scmi_shmem_layout *layout;
	struct scmi_shmem_data *data;

	data = shmem->data;
	if (is_notification) {
		layout = (struct scmi_shmem_layout *)data->regmap[1];
	} else {
		layout = (struct scmi_shmem_layout *)data->regmap[0];
	}

	if ((layout->chan_status & SCMI_SHMEM_CHAN_STATUS_FREE_BIT) != 0) {
		return SCMI_GENERIC_ERROR;
	}

	layout->chan_status |= SCMI_SHMEM_CHAN_STATUS_FREE_BIT;
	return 0;
}

void scmi_shmem_update_flags(const struct scmi_dev *shmem, uint32_t mask,
							 uint32_t val, bool is_notification)
{
	struct scmi_shmem_layout *layout;
	struct scmi_shmem_data *data;

	data = shmem->data;
	if (is_notification) {
		layout = (struct scmi_shmem_layout *)data->regmap[1];
	} else {
		layout = (struct scmi_shmem_layout *)data->regmap[0];
	}

	layout->chan_flags = (layout->chan_flags & ~mask) | (val & mask);
}

const struct scmi_shmem_config configs_main[MAX_SHMEM_REGION] = {
	{
		.phys_addr = SCMI_SHMEM_PLATFORM_MAIN,
		.size = SCMI_SHMEM_SIZE - sizeof(struct scmi_shmem_layout),
	},
	{
		.phys_addr = SCMI_SHMEM_AGENT_MAIN,
		.size = SCMI_SHMEM_SIZE - sizeof(struct scmi_shmem_layout),
	},
};

const struct scmi_shmem_config configs_2nd[MAX_SHMEM_REGION] = {
	{
		.phys_addr = SCMI_SHMEM_PLATFORM_2ND,
		.size = SCMI_SHMEM_SIZE - sizeof(struct scmi_shmem_layout),
	},
	{
		.phys_addr = SCMI_SHMEM_AGENT_2ND,
		.size = SCMI_SHMEM_SIZE - sizeof(struct scmi_shmem_layout),
	},
};

struct scmi_shmem_data data;

int scmi_shmem_init(struct scmi_dev *dev)
{
	uint8_t cpuid = __get_MPIDR() & 0xFF;

	if (!dev) {
		return -EINVAL;
	}

	for (int i = 0; i < MAX_SHMEM_REGION; ++i)
	{
		if ((configs_main[i].size < sizeof(struct scmi_shmem_layout)) ||
			(configs_2nd[i].size < sizeof(struct scmi_shmem_layout)))
		{
			return -EINVAL;
		}
	}

	dev->data = &data;

	/* No MMU -> map 1:1 */
	if (0 == cpuid) {
		dev->config = &configs_main;
		data.regmap[0] = configs_main[0].phys_addr;
		data.regmap[1] = configs_main[1].phys_addr;
	} else if (1 == cpuid) {
		dev->config = &configs_2nd;
		data.regmap[0] = configs_2nd[0].phys_addr;
		data.regmap[1] = configs_2nd[1].phys_addr;
	} else {
		SCMI_LOG_ERR("Invalid CPU ID (%d)", cpuid);
		return -EINVAL;
	}

	return 0;
}

