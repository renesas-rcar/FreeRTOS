/*
 *
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 */

/*********************************************/
/* Include */
/*********************************************/
#include <string.h>
#include "scmi/inc/protocol.h"
#include "scmi/inc/util.h"
#include "scmi/inc/sensor.h"
#include "scmi/inc/common.h"
#include "scmi/rcar_scmi_common.h"
#include "FreeRTOS.h"

SCMI_PROTOCOL_DEFINE_NODEV(SCMI_PROTOCOL_SENSOR, NULL);

/*********************************************/
/* Macro */
/*********************************************/
/* Definitions for SENSOR_TRIP_POINT_CONFIG */
#define SCMI_SENSOR_TP_ID_SHIFT   4u /* Position of trip_point_id */
#define SCMI_SENSOR_TP_ID_MASK    (0xFFu << SCMI_SENSOR_TP_ID_SHIFT) /* Mask of trip_point_id */
#define SCMI_SENSOR_TP_EV_MASK    0x3u                               /* Mask of event control for the trip-point */

/* Definitions for SENSOR_CONFIG_GET */
#define SCMI_SENSOR_CONFIG_UPD_SHIFT         11u /* Position of sensor_update_interval */
#define SCMI_SENSOR_CONFIG_UPD_MASK          (0xFFFFFu << SCMI_SENSOR_CONFIG_UPD_SHIFT) /* Mask of sensor_update_interval */
#define SCMI_SENSOR_CONFIG_ENABLE_BIT        (1u << 0)  /* Position of Sensor State */
#define SCMI_SENSOR_CONFIG_TIMESTAMP_BIT     (1u << 1)  /* Position of Timestamp reporting */

/* Definitions for SENSOR_CONFIG_SET */
#define SCMI_SENSOR_CONFIG_RESERVED_MASK     (0x7Fu << 2) /* Mask of reserved bits in sensor_config */

/* Definitions for SENSOR_READING_GET */
#define SCMI_SENSOR_READING_GET_ASYNC_BIT   (1u << 0) /* Position of Async flag */

/* Definitions for SENSOR_ DESCRIPTION_GET */
#define SCMI_SENS_ATTR_LOW_EXT_ATTRS_BIT      (1u << 8) /* Position of Extended attributes support in sensor_attributes_low */
#define SCMI_SENS_ATTR_HIGH_AXIS_SUPP_BIT     (1u << 8) /* Position of Axis support in sensor_attributes_high */

/*********************************************/
/* Struct */
/*********************************************/
/* Message response structure, only contains status */
struct scmi_msg_resp_status_only {
    int32_t status;
};

/* Message response structure of PROTOCOL_ATTRIBUTES */
struct scmi_msg_resp_sensor_protocol_attributes {
    int32_t  status;
    uint32_t attributes;
    uint32_t sensor_reg_address_low;
    uint32_t sensor_reg_address_high;
    uint32_t sensor_reg_len;
};

/* Message response structure of PROTOCOL_MESSAGE_ATTRIBUTES */
struct scmi_msg_resp_protocol_message_attributes {
    int32_t  status;
    uint32_t attributes;
};

/* Message response structure of SENSOR_ DESCRIPTION_GET */
struct scmi_msg_resp_sensor_description_get_hdr {
    int32_t  status;
    uint32_t num_sensor_flags;
    /* Followed by SENSOR_DESC desc[N] */
};

/* Message response structure of SENSOR_CONFIG_GET */
struct scmi_msg_resp_sensor_config_get {
    int32_t  status;
    uint32_t sensor_config;
};

/* Message response structure of SENSOR_READING_GET */
struct scmi_msg_resp_sensor_reading_get_hdr {
    int32_t status;
    /* Followed by SENSOR_READING readings[N] */
};

/*********************************************/
/* Local declaration */
/*********************************************/
/* Get uint32_t value as little-endian at given address */
static inline uint32_t scmi_get_le32(const void *p);

/* Check Extended attributes support in SENSOR_ DESCRIPTION_GET */
static inline uint8_t scmi_sensor_desc_ext_attrs_supported(uint32_t attr_low);

/* Check Axis support in SENSOR_ DESCRIPTION_GET */
static inline uint8_t scmi_sensor_desc_axis_supported(uint32_t attr_high);

/* Decode sensor_update_interval in SENSOR_CONFIG_GET*/
static inline struct scmi_sensor_update_interval scmi_sensor_interval_decode(uint32_t raw);

/* Pack trip_point_ev_ctrl in SENSOR_TRIP_POINT_CONFIG */
static inline uint32_t scmi_sensor_tp_ev_ctrl_pack(uint8_t trip_point_id,
                           enum scmi_sensor_trip_point_event_ctrl ctrl);

/* Decode sensor_config in SENSOR_CONFIG_GET */
static inline void scmi_sensor_config_decode(uint32_t raw, struct scmi_sensor_config *out);

/*********************************************/
/* Public function */
/*********************************************/
int scmi_sensor_protocol_version_get(uint32_t *version)
{
    struct scmi_protocol *proto = &SCMI_PROTOCOL_NAME(SCMI_PROTOCOL_SENSOR);
    struct scmi_msg_resp_version reply_buffer;
    struct scmi_message msg, reply;
    int ret = RET_OK;

    /* Sanity checks */
    if (!version) {
        ret = -EINVAL;
    }

    if (ret == RET_OK) {
        if (proto->id != SCMI_PROTOCOL_SENSOR) {
            ret = -EINVAL;
        }
    }

    if (ret == RET_OK) {
        msg.hdr = SCMI_MESSAGE_HDR_MAKE(SCMI_SENSOR_MSG_PROTOCOL_VERSION,
                           SCMI_COMMAND, proto->id, 0x0);
        msg.len = 0;
        msg.content = NULL;

        reply.hdr = msg.hdr;
        reply.len = sizeof(reply_buffer);
        reply.content = &reply_buffer;

        ret = scmi_send_message(proto, &msg, &reply);
        if (ret != RET_OK) {
            /* Do nothing */
        } else {
            if (reply_buffer.status != SCMI_SUCCESS) {
                ret = scmi_status_to_errno(reply_buffer.status);
            } else {
                *version = reply_buffer.version;
            }
        }
    }

    return ret;
}

int scmi_sensor_protocol_attributes_get(struct scmi_sensor_protocol_attributes *attr)
{
    struct scmi_protocol *proto = &SCMI_PROTOCOL_NAME(SCMI_PROTOCOL_SENSOR);
    struct scmi_msg_resp_sensor_protocol_attributes reply_buffer;
    struct scmi_message msg, reply;
    int ret = RET_OK;

    /* Sanity checks */
    if (!attr) {
        ret = -EINVAL;
    }

    if (ret == RET_OK) {
        if (proto->id != SCMI_PROTOCOL_SENSOR) {
            ret = -EINVAL;
        }
    }

    if (ret == RET_OK) {
        msg.hdr = SCMI_MESSAGE_HDR_MAKE(SCMI_SENSOR_MSG_PROTOCOL_ATTRIBUTES,
                           SCMI_COMMAND, proto->id, 0x0);
        msg.len = 0;
        msg.content = NULL;

        reply.hdr = msg.hdr;
        reply.len = sizeof(reply_buffer);
        reply.content = &reply_buffer;

        ret = scmi_send_message(proto, &msg, &reply);
        if (ret != RET_OK) {
            /* Do nothing */
        } else {
            if (reply_buffer.status != SCMI_SUCCESS) {
                ret = scmi_status_to_errno(reply_buffer.status);
            } else {
                attr->num_sensors = (uint16_t)(reply_buffer.attributes & 0xFFFFu);
                attr->max_async   = (uint8_t)((reply_buffer.attributes >> 16) & 0xFFu);

                attr->shared_mem_len = reply_buffer.sensor_reg_len;
                if (attr->shared_mem_len == 0u) {
                    attr->shared_mem_addr = 0u;
                } else {
                    attr->shared_mem_addr =
                        ((uint64_t)reply_buffer.sensor_reg_address_high << 32) |
                        (uint64_t)reply_buffer.sensor_reg_address_low;
                }
            }
        }
    }

    return ret;
}

int scmi_sensor_protocol_message_attributes_get(uint32_t msg_id,
                        uint32_t *attributes)
{
    struct scmi_protocol *proto = &SCMI_PROTOCOL_NAME(SCMI_PROTOCOL_SENSOR);
    struct scmi_msg_resp_protocol_message_attributes reply_buffer;
    struct scmi_message msg, reply;
    uint32_t req;
    int ret = RET_OK;

    /* Sanity checks */
    if (!attributes) {
        ret = -EINVAL;
    }

    if (ret == RET_OK) {
        if (proto->id != SCMI_PROTOCOL_SENSOR) {
            ret = -EINVAL;
        }
    }

    if (ret == RET_OK) {
        req = msg_id;

        msg.hdr = SCMI_MESSAGE_HDR_MAKE(SCMI_SENSOR_MSG_PROTOCOL_MESSAGE_ATTRIBUTES,
                           SCMI_COMMAND, proto->id, 0x0);
        msg.len = sizeof(req);
        msg.content = &req;

        reply.hdr = msg.hdr;
        reply.len = sizeof(reply_buffer);
        reply.content = &reply_buffer;

        ret = scmi_send_message(proto, &msg, &reply);
        if (ret != RET_OK) {
            /* Do nothing */
        } else {
            if (reply_buffer.status != SCMI_SUCCESS) {
                ret = scmi_status_to_errno(reply_buffer.status);
            } else {
                *attributes = reply_buffer.attributes;
            }
        }
    }

    return ret;
}

int scmi_sensor_description_get(uint32_t desc_index,
                                struct scmi_sensor_desc *out, size_t out_cap,
                                struct scmi_sensor_desc_page *page)
{
    struct scmi_protocol *proto = &SCMI_PROTOCOL_NAME(SCMI_PROTOCOL_SENSOR);
    struct scmi_message msg, reply;
    uint32_t req;
    int ret;
    uint8_t desc_size = 52; /* descriptor size: base 28 + power 4 + (resolution 4 + min 8 + max 8) = 52 */
    const uint32_t sizeof_scmi_shmem_layout = 28;
    const uint32_t max_cap = SCMI_SHMEM_SIZE - sizeof_scmi_shmem_layout;

    if (!out || out_cap == 0u || !page) {
        return -EINVAL;
    }
    if (proto->id != SCMI_PROTOCOL_SENSOR) {
        return -EINVAL;
    }

    req = desc_index;

    msg.hdr = SCMI_MESSAGE_HDR_MAKE(SCMI_SENSOR_MSG_SENSOR_DESCRIPTION_GET,
                                    SCMI_COMMAND, proto->id, 0x0);
    msg.len = sizeof(req);
    msg.content = &req;

    size_t rx_len = sizeof(struct scmi_msg_resp_sensor_description_get_hdr) +
                    (desc_size * out_cap);

    if (rx_len > max_cap) {
        rx_len = max_cap;
    }
    uint8_t *rx = (uint8_t *)pvPortMalloc(rx_len);
    if (!rx) {
        return -ENOMEM;
    }
    memset(rx, 0, rx_len);

    reply.hdr = msg.hdr;
    reply.len = rx_len;
    reply.content = rx;

    ret = scmi_send_message(proto, &msg, &reply);
    if (ret != RET_OK) {
        vPortFree(rx);
        return ret;
    }

    const struct scmi_msg_resp_sensor_description_get_hdr *hdr =
        (const struct scmi_msg_resp_sensor_description_get_hdr *)rx;

    if (hdr->status != SCMI_SUCCESS) {
        ret = scmi_status_to_errno(hdr->status);
        vPortFree(rx);
        return ret;
    }

    uint32_t f = hdr->num_sensor_flags;
    uint16_t num_returned  = (uint16_t)(f & 0x0FFFu);
    uint16_t num_remaining = (uint16_t)((f >> 16) & 0xFFFFu);

    page->num_returned  = num_returned;
    page->num_remaining = num_remaining;

    size_t off = sizeof(*hdr);

    for (size_t i = 0; i < (size_t)num_returned && i < out_cap; i++) {

        /* Base descriptor fixed 28 bytes */
        if (off + 28u > reply.len) {
            ret = -EIO;
            break;
        }

        uint32_t sensor_id  = scmi_get_le32(rx + off + 0u);
        uint32_t attr_low   = scmi_get_le32(rx + off + 4u);
        uint32_t attr_high  = scmi_get_le32(rx + off + 8u);

        out[i].sensor_id = sensor_id;
        out[i].sensor_attributes_low  = attr_low;
        out[i].sensor_attributes_high = attr_high;

        memcpy(out[i].sensor_name, rx + off + 12u, SCMI_SENSOR_NAME_MAX);
        /* ensure null-terminated of a string */
        out[i].sensor_name[SCMI_SENSOR_NAME_MAX - 1u] = '\0';

        out[i].ext_attrs_supported = scmi_sensor_desc_ext_attrs_supported(attr_low);

        /* reset optional fields */
        out[i].sensor_power_uW = 0u;

        out[i].resolution_valid = 0u;
        out[i].resolution_exponent = 0;
        out[i].resolution_res = 0u;

        out[i].range_valid = 0u;
        out[i].min_range = 0;
        out[i].max_range = 0;

        off += 28u;

        /* Extended attributes: present only if attr_low[8] == 1 */
        if (out[i].ext_attrs_supported != 0) {

            /* sensor_power always present when ext attrs supported */
            if (off + 4u > reply.len) {
                ret = -EIO;
                break;
            }
            out[i].sensor_power_uW = scmi_get_le32(rx + off);
            off += 4u;

            /* Remaining ext fields present only if scalar sensor (axis_support == 0) */
            if (scmi_sensor_desc_axis_supported(attr_high) == 0u) {

                /* sensor_resolution */
                if (off + 4u > reply.len) {
                    ret = -EIO;
                    break;
                }
                uint32_t sensor_resolution = scmi_get_le32(rx + off);
                off += 4u;

                uint32_t exp5 = (sensor_resolution >> 27) & 0x1Fu;
                uint32_t res  = (sensor_resolution & 0x07FFFFFFu);

                out[i].resolution_exponent = (uint8_t) exp5;
                out[i].resolution_res = res;
                out[i].resolution_valid = (res != 0u) ? 1u : 0u;

                /* min range low/high */
                if (off + 8u > reply.len) {
                    ret = -EIO;
                    break;
                }
                uint32_t min_low_u  = scmi_get_le32(rx + off + 0u);
                uint32_t min_high_u = scmi_get_le32(rx + off + 4u);
                off += 8u;

                /* max range low/high */
                if (off + 8u > reply.len) {
                    ret = -EIO;
                    break;
                }
                uint32_t max_low_u  = scmi_get_le32(rx + off + 0u);
                uint32_t max_high_u = scmi_get_le32(rx + off + 4u);
                off += 8u;

                int64_t min64 = ((int64_t)(uint64_t)min_high_u << 32) | (uint64_t)min_low_u;
                int64_t max64 = ((int64_t)(uint64_t)max_high_u << 32) | (uint64_t)max_low_u;

                out[i].min_range = min64;
                out[i].max_range = max64;

                /* Sentinel per spec:
                 * min not reported => 0x8000000000000000
                 * max not reported => 0x7FFFFFFFFFFFFFFF
                 */
                if (min64 == (int64_t)0x8000000000000000LL ||
                    max64 == (int64_t)0x7FFFFFFFFFFFFFFFLL) {
                    out[i].range_valid = 0u;
                } else {
                    out[i].range_valid = 1u;
                }
            }
        }
    }

    vPortFree(rx);
    return ret;
}

int scmi_sensor_trip_point_config(uint32_t sensor_id,
                  uint8_t trip_point_id,
                  enum scmi_sensor_trip_point_event_ctrl ctrl,
                  uint32_t trip_point_val_low,
                  uint32_t trip_point_val_high)
{
    struct scmi_protocol *proto = &SCMI_PROTOCOL_NAME(SCMI_PROTOCOL_SENSOR);
    struct scmi_msg_resp_status_only reply_buffer;
    struct scmi_message msg, reply;
    int ret = RET_OK;

    struct {
        uint32_t sensor_id;
        uint32_t trip_point_ev_ctrl;
        uint32_t trip_point_val_low;
        uint32_t trip_point_val_high;
    } req;

    /* Sanity checks */
    if (proto->id != SCMI_PROTOCOL_SENSOR) {
        return -EINVAL;
    }
    if ((uint32_t)ctrl > SCMI_SENSOR_TP_EVENT_EITHER_DIR) {
        return -EINVAL;
    }
    /* trip_point_id is 8-bit in the packed field (bits[11:4]) */
    if (trip_point_id > 0xFFu) {
        return -EINVAL;
    }

    req.sensor_id = sensor_id;
    req.trip_point_ev_ctrl = scmi_sensor_tp_ev_ctrl_pack(trip_point_id, ctrl);
    req.trip_point_val_low = trip_point_val_low;
    req.trip_point_val_high = trip_point_val_high;

    msg.hdr = SCMI_MESSAGE_HDR_MAKE(SCMI_SENSOR_MSG_SENSOR_TRIP_POINT_CONFIG,
                       SCMI_COMMAND, proto->id, 0x0);
    msg.len = sizeof(req);
    msg.content = &req;

    reply.hdr = msg.hdr;
    reply.len = sizeof(reply_buffer);
    reply.content = &reply_buffer;

    ret = scmi_send_message(proto, &msg, &reply);
    if (ret != RET_OK) {
        /* Do nothing */
    } else {
        if (reply_buffer.status != SCMI_SUCCESS) {
            ret = scmi_status_to_errno(reply_buffer.status);
        }
    }

    return ret;
}

int scmi_sensor_config_get(uint32_t sensor_id, struct scmi_sensor_config *out)
{
    struct scmi_protocol *proto = &SCMI_PROTOCOL_NAME(SCMI_PROTOCOL_SENSOR);
    struct scmi_msg_resp_sensor_config_get reply_buffer;
    struct scmi_message msg, reply;
    int ret = RET_OK;

    struct {
        uint32_t sensor_id;
    } req;

    if (!out) {
        ret = -EINVAL;
    }

    if (ret == RET_OK) {
        if (proto->id != SCMI_PROTOCOL_SENSOR) {
            ret = -EINVAL;
        }
    }

    if (ret == RET_OK) {
        req.sensor_id = sensor_id;

        msg.hdr = SCMI_MESSAGE_HDR_MAKE(SCMI_SENSOR_MSG_SENSOR_CONFIG_GET,
                           SCMI_COMMAND, proto->id, 0x0);
        msg.len = sizeof(req);
        msg.content = &req;

        reply.hdr = msg.hdr;
        reply.len = sizeof(reply_buffer);
        reply.content = &reply_buffer;

        ret = scmi_send_message(proto, &msg, &reply);
        if (ret != RET_OK) {
            /* Do nothing */
        } else {
            if (reply_buffer.status != SCMI_SUCCESS) {
                ret = scmi_status_to_errno(reply_buffer.status);
            } else {
                scmi_sensor_config_decode(reply_buffer.sensor_config, out);
            }
        }
    }

    return ret;
}

int scmi_sensor_config_set(uint32_t sensor_id, uint32_t sensor_config)
{
    struct scmi_protocol *proto = &SCMI_PROTOCOL_NAME(SCMI_PROTOCOL_SENSOR);
    struct scmi_msg_resp_status_only reply_buffer;
    struct scmi_message msg, reply;
    int ret = RET_OK;

    struct {
        uint32_t sensor_id;
        uint32_t sensor_config;
    } req;

    /* Sanity checks (match existing style in your file) */
    if (proto->id != SCMI_PROTOCOL_SENSOR) {
        return -EINVAL;
    }

    req.sensor_id = sensor_id;
    req.sensor_config = sensor_config;

    msg.hdr = SCMI_MESSAGE_HDR_MAKE(SCMI_SENSOR_MSG_SENSOR_CONFIG_SET,
                       SCMI_COMMAND, proto->id, 0x0);
    msg.len = sizeof(req);
    msg.content = &req;

    reply.hdr = msg.hdr;
    reply.len = sizeof(reply_buffer);
    reply.content = &reply_buffer;

    ret = scmi_send_message(proto, &msg, &reply);
    if (ret != RET_OK) {
        /* Do nothing */
    } else {
        if (reply_buffer.status != SCMI_SUCCESS) {
            ret = scmi_status_to_errno(reply_buffer.status);
        }
    }

    return ret;
}

int scmi_sensor_reading_get(uint32_t sensor_id,
                uint8_t async_read,
                struct scmi_sensor_reading_desc *out, size_t out_cap)
{
    struct scmi_protocol *proto = &SCMI_PROTOCOL_NAME(SCMI_PROTOCOL_SENSOR);
    struct scmi_message msg, reply;
    int ret = RET_OK;

    if (ret == RET_OK) {
        if (!out || out_cap == 0u) {
            ret = -EINVAL;
        }
    }
    if (ret == RET_OK) {
        if (proto->id != SCMI_PROTOCOL_SENSOR) {
            ret = -EINVAL;
        }
    }

    if (ret == RET_OK) {
        struct {
            uint32_t sensor_id;
            uint32_t flags;
        } req;

        req.sensor_id = sensor_id;
        req.flags = async_read ? SCMI_SENSOR_READING_GET_ASYNC_BIT : 0u;

        msg.hdr = SCMI_MESSAGE_HDR_MAKE(SCMI_SENSOR_MSG_SENSOR_READING_GET,
                           SCMI_COMMAND, proto->id, 0x0);
        msg.len = sizeof(req);
        msg.content = &req;

        /* Response is variable-length: hdr + N * reading_desc */
        size_t rx_len = sizeof(struct scmi_msg_resp_sensor_reading_get_hdr) +
                (out_cap * sizeof(struct scmi_sensor_reading_desc));

        uint8_t *rx = (uint8_t *)pvPortMalloc(rx_len);
        if (!rx) {
            ret = -ENOMEM;
        }

        if (ret == RET_OK) {
            memset(rx, 0, rx_len);

            reply.hdr = msg.hdr;
            reply.len = rx_len;
            reply.content = rx;

            ret = scmi_send_message(proto, &msg, &reply);
            if (ret != RET_OK) {
                /* Do nothing */
            } else {
                const struct scmi_msg_resp_sensor_reading_get_hdr *hdr =
                    (const struct scmi_msg_resp_sensor_reading_get_hdr *)rx;

                if (hdr->status != SCMI_SUCCESS) {
                    ret = scmi_status_to_errno(hdr->status);
                } else {
                    /* For async read, SUCCESS means enqueued; no readings returned now.
                     * Spec: errors during actual read come via delayed response.
                     */
                    if (async_read != 0U) {
                        /* Do nothing */
                    } else {
                        /* Copy up to out_cap entries, as reply doesn't carry N explicitly.
                         * We rely on caller to pass correct out_cap (N) based on sensor desc.
                         */
                        size_t need_min = sizeof(*hdr) + sizeof(struct scmi_sensor_reading_desc);
                        if (reply.len < need_min) {
                            ret = -EIO;
                        } else {
                            size_t payload = reply.len - sizeof(*hdr);
                            size_t n = payload / sizeof(struct scmi_sensor_reading_desc);
                            if (n > out_cap) {
                                n = out_cap;
                            }

                            memcpy(out, rx + sizeof(*hdr),
                                   n * sizeof(struct scmi_sensor_reading_desc));
                        }
                    }
                }
            }

            vPortFree(rx);
        }
    }

    return ret;
}

/*********************************************/
/* Local function */
/*********************************************/
static inline uint32_t scmi_get_le32(const void *p)
{
    const uint8_t *b = (const uint8_t *)p;
    return ((uint32_t)b[0]) |
           ((uint32_t)b[1] << 8) |
           ((uint32_t)b[2] << 16) |
           ((uint32_t)b[3] << 24);
}

static inline uint8_t scmi_sensor_desc_ext_attrs_supported(uint32_t attr_low)
{
    return (attr_low & SCMI_SENS_ATTR_LOW_EXT_ATTRS_BIT) ? 1u : 0u;
}

static inline uint8_t scmi_sensor_desc_axis_supported(uint32_t attr_high)
{
    return (attr_high & SCMI_SENS_ATTR_HIGH_AXIS_SUPP_BIT) ? 1u : 0u;
}

static inline struct scmi_sensor_update_interval scmi_sensor_interval_decode(uint32_t raw)
{
    struct scmi_sensor_update_interval out;
    out.sec = (raw >> 5) & 0xFFFFu;
    out.exponent = raw & 0x001Fu;
    return out;
}

static inline uint32_t scmi_sensor_tp_ev_ctrl_pack(uint8_t trip_point_id,
                           enum scmi_sensor_trip_point_event_ctrl ctrl)
{
    return (((uint32_t)trip_point_id << SCMI_SENSOR_TP_ID_SHIFT) & SCMI_SENSOR_TP_ID_MASK) |
           ((uint32_t)ctrl & SCMI_SENSOR_TP_EV_MASK);
}

static inline void scmi_sensor_config_decode(uint32_t raw, struct scmi_sensor_config *out)
{
    uint32_t upd21; /* 21-bit upper in a 32-bit */

    out->sensor_config = raw;

    out->enabled     = (uint8_t)((raw & SCMI_SENSOR_CONFIG_ENABLE_BIT) ? 1u : 0u);
    out->timestamped = (uint8_t)((raw & SCMI_SENSOR_CONFIG_TIMESTAMP_BIT) ? 1u : 0u);

    upd21 = (raw & SCMI_SENSOR_CONFIG_UPD_MASK) >> SCMI_SENSOR_CONFIG_UPD_SHIFT;

    /* Use existing helper in your source */
    out->update_interval = scmi_sensor_interval_decode(upd21);
}
