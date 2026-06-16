/*
 *
 * Copyright (c) 2025 Renesas Electronics Corporation
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "scmi/inc/mailbox.h"
#include "scmi/inc/common.h"
#include "scmi/inc/mbox_mfis_mailbox.h"

struct scmi_dev shmem_dev;

static void scmi_mbox_cb(const struct scmi_dev *mbox,
						 mbox_channel_id_t channel_id,
						 void *user_data,
						 struct mbox_msg *data)
{
	struct scmi_channel *scmi_chan = user_data;

	if (scmi_chan->cb) {
		scmi_chan->cb(scmi_chan);
	}
}

static int scmi_mbox_send_message(const struct scmi_dev *transport,
								  struct scmi_channel *chan,
								  struct scmi_message *msg)
{
	struct scmi_mbox_channel *mbox_chan;
	int ret;

	mbox_chan = chan->data;

	ret = scmi_shmem_write_message(mbox_chan->shmem, msg, chan->is_notification);
	if (ret < 0) {
		SCMI_LOG_ERR("failed to write message to shmem: %d", ret);
		return ret;
	}

	ret = mbox_send(&mbox_chan->tx, NULL);
	if (ret < 0) {
		SCMI_LOG_ERR("failed to ring doorbell: %d", ret);
		return ret;
	}

	return 0;
}

static int scmi_mbox_read_message(const struct scmi_dev *transport,
								  struct scmi_channel *chan,
								  struct scmi_message *msg)
{
	struct scmi_mbox_channel *mbox_chan;

	mbox_chan = chan->data;

	return scmi_shmem_read_message(mbox_chan->shmem, msg, chan->is_notification);
}

static bool scmi_mbox_channel_is_free(const struct scmi_dev *transport,
									  struct scmi_channel *chan)
{
	struct scmi_mbox_channel *mbox_chan = chan->data;

	return scmi_shmem_channel_status(mbox_chan->shmem, chan->is_notification) &
		SCMI_SHMEM_CHAN_STATUS_FREE_BIT;
}

static int scmi_mbox_channel_free_set(const struct scmi_dev *transport,
									  struct scmi_channel *chan)
{
	struct scmi_mbox_channel *mbox_chan = chan->data;

	return scmi_shmem_channel_free_set(mbox_chan->shmem, chan->is_notification);
}

static int scmi_mbox_setup_chan(const struct scmi_dev *transport,
				struct scmi_channel *chan,
				bool tx)
{
	int ret;
	struct scmi_mbox_channel *mbox_chan;
	struct mbox_spec *mbox_tx;

	mbox_chan = chan->data;

	if (!tx) {
		return -ENOTSUP;
	}

	mbox_tx = &mbox_chan->tx;

	ret = mbox_register_callback(mbox_tx, scmi_mbox_cb, chan);
	if (ret < 0) {
		SCMI_LOG_ERR("failed to register tx cb");
		return ret;
	}

	ret = mbox_set_enabled(mbox_tx, true);
	if (ret < 0) {
		SCMI_LOG_ERR("failed to enable tx dbell");
	}

	/* enable interrupt-based communication */
	scmi_shmem_update_flags(mbox_chan->shmem,
				SCMI_SHMEM_CHAN_FLAG_IRQ_BIT,
				SCMI_SHMEM_CHAN_FLAG_IRQ_BIT, chan->is_notification);

	return 0;
}

static const struct scmi_transport_api scmi_mbox_api = {
	.setup_chan = scmi_mbox_setup_chan,
	.send_message = scmi_mbox_send_message,
	.read_message = scmi_mbox_read_message,
	.channel_is_free = scmi_mbox_channel_is_free,
	.channel_free_set = scmi_mbox_channel_free_set,
};

struct scmi_mbox_channel scmi_channel_16_0_priv = {
	.shmem = &shmem_dev,
	//.tx = tx,
};

//struct scmi_channel SCMI_TRANSPORT_CHAN_NAME(SCMI_PROTOCOL_BASE, 0) = {
struct scmi_channel scmi_channel_16_0 = {
	.data = &scmi_channel_16_0_priv,
};

int scmi_mbox_init(struct scmi_dev *transport)
{
	int ret;

	if (!transport) {
		return -EINVAL;
	}

	/* Setup shmem */
	ret = scmi_shmem_init(&shmem_dev);
	if (ret) {
		return -EINVAL;
	}

	/* Setup mbox spec */
	ret = mfis_mailbox_init(&scmi_channel_16_0_priv.tx);
	scmi_channel_16_0_priv.tx.channel_id = 0;

	/* Setup transport api */
	transport->api = (void *)&scmi_mbox_api;

	return 0;
}

