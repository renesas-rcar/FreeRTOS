/*
 *
 * Copyright (c) 2025 Renesas Electronics Corporation
 * Copyright (c) 2021 Carlo Caione <ccaione@baylibre.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#ifndef MBOX_H
#define MBOX_H

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief MBOX Interface
 * @defgroup mbox_interface MBOX Interface
 * @since 1.0
 * @version 0.1.0
 * @ingroup io_interfaces
 * @{
 *
 * @code{.unparsed}
 *
 *                      CPU #1        |
 * +----------+                       |        +----------+
 * |          +---TX9----+   +--------+--RX8---+          |
 * |  dev A   |          |   |        |        |  CPU #2  |
 * |          <---RX8--+ |   | +------+--TX9--->          |
 * +----------+        | |   | |      |        +----------+
 *                  +--+-v---v-+--+   |
 *                  |             |   |
 *                  |   MBOX dev  |   |
 *                  |             |   |
 *                  +--+-^---^--+-+   |
 * +----------+        | |   |  |     |        +----------+
 * |          <---RX2--+ |   |  +-----+--TX3--->          |
 * |  dev B   |          |   |        |        |  CPU #3  |
 * |          +---TX3----+   +--------+--RX2---+          |
 * +----------+                       |        +----------+
 *                                    |
 *
 * @endcode
 *
 * An MBOX device is a peripheral capable of passing signals (and data depending
 * on the peripheral) between CPUs and clusters in the system. Each MBOX
 * instance is providing one or more channels, each one targeting one other CPU
 * cluster (multiple channels can target the same cluster).
 *
 * For example in the plot the device 'dev A' is using the TX channel 9 to
 * signal (or send data to) the CPU #2 and it's expecting data or signals on
 * the RX channel 8. Thus it can send the message through the channel 9, and it
 * can register a callback on the channel 8 of the MBOX device.
 *
 * This API supports two modes: signalling mode and data transfer mode.
 *
 * In signalling mode:
 *  - mbox_mtu_get() must return 0
 *  - mbox_send() must have (msg == NULL)
 *  - the callback must be called with (data == NULL)
 *
 * In data transfer mode:
 *  - mbox_mtu_get() must return a (value != 0)
 *  - mbox_send() must have (msg != NULL)
 *  - the callback must be called with (data != NULL)
 *  - The msg content must be the same between sender and receiver
 *
 */

/** @brief Type for MBOX channel identifiers */
typedef uint32_t mbox_channel_id_t;

/** @brief Message struct (to hold data and its size). */
struct mbox_msg {
	/** Pointer to the data sent in the message. */
	const void *data;
	/** Size of the data. */
	size_t size;
};

/** @brief MBOX specification */
struct mbox_spec {
	/** MBOX device pointer. */
	const struct scmi_dev *dev;
	/** Channel ID. */
	mbox_channel_id_t channel_id;
};

/** @cond INTERNAL_HIDDEN */

/**
 * @brief Callback API for incoming MBOX messages
 *
 * These callbacks execute in interrupt context. Therefore, use only
 * interrupt-safe APIs. Registration of callbacks is done via
 * mbox_register_callback()
 *
 * The data parameter must be NULL in signalling mode.
 *
 * @param dev MBOX device instance
 * @param channel_id Channel ID
 * @param user_data Pointer to some private data provided at registration time
 * @param data Message struct
 */
typedef void (*mbox_callback_t)(const struct scmi_dev *dev,
				mbox_channel_id_t channel_id, void *user_data,
				struct mbox_msg *data);

/**
 * @brief Callback API to send MBOX messages
 *
 * @param dev MBOX device instance
 * @param channel_id Channel ID
 * @param msg Message struct
 *
 * @return See the return values for mbox_send()
 * @see mbox_send()
 */
typedef int (*mbox_send_t)(const struct scmi_dev *dev,
			   mbox_channel_id_t channel_id,
			   const struct mbox_msg *msg);

/**
 * @brief Callback API to get maximum data size
 *
 * @param dev MBOX device instance
 *
 * @return See the return values for mbox_mtu_get()
 * @see mbox_mtu_get()
 */
typedef int (*mbox_mtu_get_t)(const struct scmi_dev *dev);

/**
 * @brief Callback API upon registration
 *
 * @param dev MBOX device instance
 * @param channel_id Channel ID
 * @param cb Callback function to execute on incoming message interrupts.
 * @param user_data Application-specific data pointer which will be passed to
 *                  the callback function when executed.
 *
 * @return See return values for mbox_register_callback()
 * @see mbox_register_callback()
 */
typedef int (*mbox_register_callback_t)(const struct scmi_dev *dev,
					mbox_channel_id_t channel_id,
					mbox_callback_t cb, void *user_data);

/**
 * @brief Callback API upon enablement of interrupts
 *
 * @param dev MBOX device instance
 * @param channel_id Channel ID
 * @param enabled Set to 0 to disable and to nonzero to enable.
 *
 * @return See return values for mbox_set_enabled()
 * @see mbox_set_enabled()
 */
typedef int (*mbox_set_enabled_t)(const struct scmi_dev *dev,
				  mbox_channel_id_t channel_id, bool enabled);

/**
 * @brief Callback API to get maximum number of channels
 *
 * @param dev MBOX device instance
 *
 * @return See return values for mbox_max_channels_get()
 * @see mbox_max_channels_get()
 */
typedef uint32_t (*mbox_max_channels_get_t)(const struct scmi_dev *dev);

struct mbox_driver_api {
	mbox_send_t send;
	mbox_register_callback_t register_callback;
	mbox_mtu_get_t mtu_get;
	mbox_max_channels_get_t max_channels_get;
	mbox_set_enabled_t set_enabled;
};

/** @endcond */

/**
 * @brief Try to send a message over the MBOX device.
 *
 * Send a message over an struct mbox_channel. The msg parameter must be NULL
 * when the driver is used for signalling.
 *
 * If the msg parameter is not NULL, this data is expected to be delivered on
 * the receiving side using the data parameter of the receiving callback.
 *
 * @param dev MBOX device instance
 * @param channel_id MBOX channel identifier
 * @param msg Message
 *
 * @retval 0         On success.
 * @retval -EBUSY    If the remote hasn't yet read the last data sent.
 * @retval -EMSGSIZE If the supplied data size is unsupported by the driver.
 * @retval -EINVAL   If there was a bad parameter, such as: too-large channel
 *		     descriptor or the device isn't an outbound MBOX channel.
 */
static inline int mbox_send(struct mbox_spec *spec,
							const struct mbox_msg *msg)
{
	const struct scmi_dev *dev = spec->dev;
	mbox_channel_id_t channel_id = spec->channel_id;
	const struct mbox_driver_api *api =
		(const struct mbox_driver_api *)dev->api;

	if (api->send == NULL) {
		return -ENOSYS;
	}

	return api->send(dev, channel_id, msg);
}

/**
 * @brief Register a callback function on a channel for incoming messages.
 *
 * This function doesn't assume anything concerning the status of the
 * interrupts. Use mbox_set_enabled() to enable or to disable the interrupts
 * if needed.
 *
 * @param dev MBOX device instance
 * @param channel_id MBOX channel identifier
 * @param cb Callback function to execute on incoming message interrupts.
 * @param user_data Application-specific data pointer which will be passed
 *                  to the callback function when executed.
 *
 * @retval 0      On success.
 * @retval -errno Negative errno on error.
 */
static inline int mbox_register_callback(struct mbox_spec *spec,
					 mbox_callback_t cb,
					 void *user_data)
{
	const struct scmi_dev *dev = spec->dev;
	mbox_channel_id_t channel_id = spec->channel_id;
	const struct mbox_driver_api *api =
		(const struct mbox_driver_api *)dev->api;

	if (api->register_callback == NULL) {
		return -ENOSYS;
	}

	return api->register_callback(dev, channel_id, cb, user_data);
}

/**
 * @brief Return the maximum number of bytes possible in an outbound message.
 *
 * Returns the actual number of bytes that it is possible to send through an
 * outgoing channel.
 *
 * This number can be 0 when the driver only supports signalling or when on the
 * receiving side the content and size of the message must be retrieved in an
 * indirect way (i.e. probing some other peripheral, reading memory regions,
 * etc...).
 *
 * If this function returns 0, the msg parameter in mbox_send() is expected
 * to be NULL.
 *
 * @param dev MBOX device instance.
 *
 * @retval >0     Maximum possible size of a message in bytes
 * @retval 0      Indicates signalling
 * @retval -errno Negative errno on error.
 */
static inline int mbox_mtu_get(const struct scmi_dev *dev)
{
	const struct mbox_driver_api *api =
		(const struct mbox_driver_api *)dev->api;

	if (api->mtu_get == NULL) {
		return -ENOSYS;
	}

	return api->mtu_get(dev);
}

/**
 * @brief Enable (disable) interrupts and callbacks for inbound channels.
 *
 * Enable interrupt for the channel when the parameter 'enable' is set to true.
 * Disable it otherwise.
 *
 * Immediately after calling this function with 'enable' set to true, the
 * channel is considered enabled and ready to receive signal and messages (even
 * already pending), so the user must take care of installing a proper callback
 * (if needed) using mbox_register_callback() on the channel before enabling
 * it. For this reason it is recommended that all the channels are disabled at
 * probe time.
 *
 * Enabling a channel for which there is no installed callback is considered
 * undefined behavior (in general the driver must take care of gracefully
 * handling spurious interrupts with no installed callback).
 *
 * @param dev MBOX device instance
 * @param channel_id MBOX channel identifier
 * @param enabled Enable (true) or disable (false) the channel.
 *
 * @retval 0         On success.
 * @retval -EINVAL   If it isn't an inbound channel.
 * @retval -EALREADY If channel is already @p enabled.
 */
static inline int mbox_set_enabled(struct mbox_spec *spec,
			       				   bool enabled)
{
	const struct scmi_dev *dev = spec->dev;
	mbox_channel_id_t channel_id = spec->channel_id;
	const struct mbox_driver_api *api =
		(const struct mbox_driver_api *)dev->api;

	if (api->set_enabled == NULL) {
		return -ENOSYS;
	}

	return api->set_enabled(dev, channel_id, enabled);
}

/**
 * @brief Return the maximum number of channels.
 *
 * Return the maximum number of channels supported by the hardware.
 *
 * @param dev MBOX device instance.
 *
 * @return >0     Maximum possible number of supported channels on success
 * @return -errno Negative errno on error.
 */
static inline uint32_t mbox_max_channels_get(const struct scmi_dev *dev)
{
	const struct mbox_driver_api *api =
		(const struct mbox_driver_api *)dev->api;

	if (api->max_channels_get == NULL) {
		return -ENOSYS;
	}

	return api->max_channels_get(dev);
}

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* MBOX_H */
