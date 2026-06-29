/*
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef MFIS_LOCK_H
#define MFIS_LOCK_H

#ifdef __cplusplus
extern "C" {
#endif

#include<stdint.h>

/**
 * @brief Enumeration of MFIS lock identifiers.
 */
typedef enum e_mfis_lock_id {
    MFIS_LOCK_ID_0,
    MFIS_LOCK_ID_1,
    MFIS_LOCK_ID_2,
    MFIS_LOCK_ID_3,
    MFIS_LOCK_ID_4,
    MFIS_LOCK_ID_5,
    MFIS_LOCK_ID_6,
    MFIS_LOCK_ID_7,
    MFIS_LOCK_ID_8,
    MFIS_LOCK_ID_9,
    MFIS_LOCK_ID_10,
    MFIS_LOCK_ID_11,
    MFIS_LOCK_ID_12,
    MFIS_LOCK_ID_13,
    MFIS_LOCK_ID_14,
    MFIS_LOCK_ID_15,
    MFIS_LOCK_ID_16,
    MFIS_LOCK_ID_17,
    MFIS_LOCK_ID_18,
    MFIS_LOCK_ID_19,
    MFIS_LOCK_ID_20,
    MFIS_LOCK_ID_21,
    MFIS_LOCK_ID_22,
    MFIS_LOCK_ID_23,
    MFIS_LOCK_ID_24,
    MFIS_LOCK_ID_25,
    MFIS_LOCK_ID_26,
    MFIS_LOCK_ID_27,
    MFIS_LOCK_ID_28,
    MFIS_LOCK_ID_29,
    MFIS_LOCK_ID_30,
    MFIS_LOCK_ID_31,
    MFIS_LOCK_ID_32,
    MFIS_LOCK_ID_33,
    MFIS_LOCK_ID_34,
    MFIS_LOCK_ID_35,
    MFIS_LOCK_ID_36,
    MFIS_LOCK_ID_37,
    MFIS_LOCK_ID_38,
    MFIS_LOCK_ID_39,
    MFIS_LOCK_ID_40,
    MFIS_LOCK_ID_41,
    MFIS_LOCK_ID_42,
    MFIS_LOCK_ID_43,
    MFIS_LOCK_ID_44,
    MFIS_LOCK_ID_45,
    MFIS_LOCK_ID_46,
    MFIS_LOCK_ID_47,
    MFIS_LOCK_ID_48,
    MFIS_LOCK_ID_49,
    MFIS_LOCK_ID_50,
    MFIS_LOCK_ID_51,
    MFIS_LOCK_ID_52,
    MFIS_LOCK_ID_53,
    MFIS_LOCK_ID_54,
    MFIS_LOCK_ID_55,
    MFIS_LOCK_ID_56,
    MFIS_LOCK_ID_57,
    MFIS_LOCK_ID_58,
    MFIS_LOCK_ID_59,
    MFIS_LOCK_ID_60,
    MFIS_LOCK_ID_61,
    MFIS_LOCK_ID_62,
    MFIS_LOCK_ID_63,
    MFIS_LOCK_ID_MAX_NUM
} e_mfis_lock_id_t;

/**
 * @brief Status codes for MFIS lock operations.
 */
typedef enum e_mfis_lock_status {
    MFIS_LOCK_SUCCESS,
    MFIS_LOCK_TIMEOUT,
    MFIS_LOCK_ID_UNSUPPORTED
} e_mfis_lock_status_t;

/**
 * @brief Acquire a MFIS lock.
 *
 * This function attempts to acquire the specified MFIS lock within
 * a given timeout period.
 *
 * @param[in] mfis_id   Lock identifier to acquire.
 * @param[in] timeout   Timeout value (in ms, 0: never timeout).
 *
 * @return e_mfis_lock_status_t
 */
e_mfis_lock_status_t R_MFIS_LockAcquire(e_mfis_lock_id_t mfis_id, uint32_t timeout);

/**
 * @brief Release a MFIS lock.
 *
 * This function releases a previously acquired MFIS lock.
 *
 * @param[in] mfis_id   Lock identifier to release.
 *
 * @return e_mfis_lock_status_t
 */
e_mfis_lock_status_t R_MFIS_LockRelease(e_mfis_lock_id_t mfis_id);

#ifdef __cplusplus
}
#endif

#endif // MFIS_LOCK_H
