/*
 *
 * Copyright (c) 2025 Renesas Electronics Corporation
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdbool.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "scmi/inc/protocol.h"
#include "scmi/inc/mailbox.h"
#include "scmi/inc/transport.h"
#include "scmi/inc/common.h"
#include "scmi/inc/system.h"

#define SCMI_CHAN_LOCK_TIMEOUT_USEC 500
#define SCMI_CHAN_SEM_TIMEOUT_USEC 500

//#define S2R_DRAFT_FLOW

struct scmi_dev transport_dev;
static scmi_notifier_callback_t cb_list[MAX_SCMI_PROTOCOLS];
SemaphoreHandle_t hasNotifSemaphore = NULL;
SemaphoreHandle_t chanModeMutex = NULL;
bool hasNotifEvent = false;
SCMI_TRANSPORT_TX_CHAN_DECLARE()

/**< SYSTEM_POWER_STATE_NOTIFIER */
#define SCMI_SYS_POWER_STATE_NOTIFIER (0x000U)

int scmi_status_to_errno(int scmi_status)
{
    switch (scmi_status) {
    case SCMI_SUCCESS:
        return 0;
    case SCMI_NOT_SUPPORTED:
        return -EOPNOTSUPP;
    case SCMI_INVALID_PARAMETERS:
        return -EINVAL;
    case SCMI_DENIED:
        return -EACCES;
    case SCMI_NOT_FOUND:
        return -ENOENT;
    case SCMI_OUT_OF_RANGE:
        return -ERANGE;
    case SCMI_IN_USE:
    case SCMI_BUSY:
        return -EBUSY;
    case SCMI_PROTOCOL_ERROR:
        return -EPROTO;
    case SCMI_COMMS_ERROR:
    case SCMI_GENERIC_ERROR:
    case SCMI_HARDWARE_ERROR:
    default:
        return -EIO;
    }
}

static int scmi_agent_respond(const struct scmi_dev *transport,
							  struct scmi_channel *chan)
{
	int ret;
	struct scmi_message msg;

	/* Set channel is as notification to receive msg */
	if (!chan->is_notification) {
		chan->is_notification = true;
	}

	ret = scmi_transport_channel_free_set(transport, chan);
	if (ret) {
		SCMI_LOG_ERR("Failed to respond.");
		return ret;
	}

	return 0;
}

static void scmi_core_isr_cb(struct scmi_channel *chan)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	hasNotifEvent = true;
	xSemaphoreGiveFromISR(hasNotifSemaphore, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

static void scmi_notification_process(void *unused)
{
	/* Remove compiler warning about unused parameter. */
	(void) unused;
#ifdef S2R_DRAFT_FLOW
	int cnt = 0;
#endif

	int ret;
	const struct scmi_dev *transport = &transport_dev;
	struct scmi_channel *chan = SCMI_TRANSPORT_TX_CHAN();
	scmi_syspower_state_notifier_t notifier;
	struct scmi_message msg = {
		.hdr = SCMI_MESSAGE_HDR_MAKE(SCMI_SYS_POWER_STATE_NOTIFIER,
									 SCMI_NOTIFICATION, SCMI_PROTOCOL_SYSTEM, 0x0),
		.len = sizeof(notifier),
		.content = &notifier,
	};
	int protocol_id;
	int message_id;

	for(;;) {
		vTaskDelay(1);
		if (xSemaphoreTake(hasNotifSemaphore, portMAX_DELAY) != pdTRUE) {
			continue;
		}

		if (!hasNotifEvent) {
			continue;
		}

		hasNotifEvent = false;
		xSemaphoreTake(chanModeMutex, portMAX_DELAY);
		chan->is_notification = true;
#ifdef S2R_DRAFT_FLOW
		msg.hdr &= ~(GENMASK(9, 0) << 18);
		msg.hdr |= (cnt << 18);
		SCMI_LOG_DBG("Read notification message cnt: %d, msg.hdr: %#x", cnt, msg.hdr);
		++cnt;
#endif
		ret = scmi_transport_read_message(transport, chan, &msg);
		if (ret < 0) {
			chan->is_notification = false;
			SCMI_LOG_ERR("Failed to read notification message");
			continue;
		}

		protocol_id = SCMI_MESSAGE_PROTO_PARSE(msg.hdr);
		if (MAX_SCMI_PROTOCOLS > protocol_id) {
			SCMI_LOG_ERR("Invalid protocol(%d)!", protocol_id);
			continue;
		}
		message_id = SCMI_MESSAGE_ID_PARSE(msg.hdr);

		/*
		 * Currently callback function is used for system protocol only.
		 */
		if ((SCMI_PROTOCOL_SYSTEM == protocol_id) &&
			(SCMI_SYS_POWER_STATE_NOTIFIER == message_id)) {
			SCMI_LOG_DBG("Respond to platform");
			scmi_agent_respond(transport, chan);
			/* Set channel for command */
			chan->is_notification = false;
			 xSemaphoreGive(chanModeMutex);
			if (cb_list[protocol_id - SCMI_PROTOCOL_BASE]) {
				cb_list[protocol_id - SCMI_PROTOCOL_BASE](&notifier);
			}
		} else {
			scmi_agent_respond(transport, chan);
			/* Set channel for command */
			chan->is_notification = false;
			xSemaphoreGive(chanModeMutex);
		}
	}
}

static int scmi_core_setup_chan(const struct scmi_dev *transport,
                struct scmi_channel *chan, bool tx)
{
    int ret;

    if (!chan) {
        return -EINVAL;
    }

    if (chan->ready) {
        return 0;
    }

    /* no support for RX channels ATM */
    if (!tx) {
        return -ENOTSUP;
    }

	/*
	 * This callback is called from an ISR, so it must be non-blocking
	 * and should not access any shared resources that may cause contention
	 * or delays.
	 */
    chan->cb = scmi_core_isr_cb;

    /* setup transport-related channel data */
    ret = scmi_transport_setup_chan(transport, chan, tx);
    if (ret < 0) {
        SCMI_LOG_ERR("failed to setup channel");
        return ret;
    }

    /* protocols might share a channel. In such cases, this
     * will stop them from being initialized again.
     */
    chan->ready = true;

    return 0;
}

int scmi_send_message(struct scmi_protocol *proto,
                     struct scmi_message *msg,
                     struct scmi_message *reply)
{
    int ret = 0;
	const struct scmi_transport_api *api =
		(const struct scmi_transport_api *)proto->transport->api;

    if (!proto->tx) {
        return -ENODEV;
    }

	xSemaphoreTake(chanModeMutex, portMAX_DELAY);
    ret = scmi_transport_send_message(proto->transport, proto->tx, msg);
    if (ret < 0) {
        SCMI_LOG_ERR("failed to send message");
		return ret;
    }

	/* Wait until the message is read by SCP.
	 * Will add timeout in the future.
	 */
	while (!api->channel_is_free(proto->transport, proto->tx)) {}

    ret = scmi_transport_read_message(proto->transport, proto->tx, reply);
    if (ret < 0) {
        SCMI_LOG_ERR("failed to read reply");
		return ret;
    }
	xSemaphoreGive(chanModeMutex);

    return ret;
}

static int scmi_core_protocol_setup(const struct scmi_dev *transport)
{
    int ret;

	ret = scmi_core_setup_chan(transport,
			SCMI_TRANSPORT_TX_CHAN(), true);
	if (ret < 0) {
		return ret;
	}

    return 0;
}

static int scmi_core_transport_init(struct scmi_dev *transport)
{
    int ret;

	ret = scmi_mbox_init(transport);
    if (ret < 0) {
        return ret;
    }

    ret = scmi_transport_init(transport);
    if (ret < 0) {
        return ret;
    }

    return scmi_core_protocol_setup(transport);
}

void scmi_notifier_callback_register(struct scmi_protocol *proto,
                                     scmi_notifier_callback_t cb)
{
	/* Valid protocol check */
	if (MAX_SCMI_PROTOCOLS > proto->id) {
		SCMI_LOG_ERR("Invalid protocol(%d)!", proto->id);
		return;
	}

	cb_list[proto->id - SCMI_PROTOCOL_BASE] = cb;
}

int scmi_driver_init(void)
{
	int ret;

	ret = scmi_core_transport_init(&transport_dev);
	if (ret) {
		SCMI_LOG_ERR("Failed to init scmi core transport.");
		return ret;
	}

	hasNotifSemaphore = xSemaphoreCreateBinary();
	if (hasNotifSemaphore == NULL) {
		SCMI_LOG_ERR("Failed to create semaphore");
		return -EINVAL;
	} else {
		xTaskCreate(scmi_notification_process, "scmi_notification_process",
					configMINIMAL_STACK_SIZE,
					NULL, configMAX_PRIORITIES - 1, NULL);
	}

	chanModeMutex = xSemaphoreCreateMutex();
	if (chanModeMutex == NULL) {
		SCMI_LOG_ERR("Failed to create semaphore");
		return -EINVAL;
	}

	return 0;
}

