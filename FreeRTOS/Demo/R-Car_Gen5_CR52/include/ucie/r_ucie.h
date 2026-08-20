/*
 *
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

/**
 * @defgroup UCIe_Driver UCIe Driver Interface
 * @brief UCIe setup, HDMA transfer, and iATU configuration APIs.
 *
 * This group contains APIs, enums, and structures to interact with the UCIe protocol.
 * @{
 */

#ifndef R_UCIE_H
#define R_UCIE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

/**
 * @brief Enum representing different UCIe channel.
 *
 * This enum includes a list of UCIe channel using for UCIe operations.
 */
typedef enum e_ucie_ch {
    UCIE_CH0,       /**< UCIe Channel 0 */
    UCIE_CH1,       /**< UCIe Channel 1 */
    UCIE_CH_MAX     /**< End of Channel */
} e_ucie_ch_t;

/**
 * @brief Enum representing UCIe mode.
 *
 * This enum includes a list of UCIe mode using for UCIe operations.
 */
typedef enum e_ucie_mode {
    UCIE_MODE_EP,   /**< Endpoint mode */
    UCIE_MODE_RC    /**< Root Complex mode */
} e_ucie_mode_t;

/**
 * @brief Enum representing UCIe link speed.
 *
 * This enum includes a list of UCIe link speed using for UCIe operations.
 */
typedef enum e_ucie_linkspeed {
    LINKSPEED_4GTPS,
    LINKSPEED_8GTPS,
    LINKSPEED_12GTPS,
    LINKSPEED_16GTPS,
} e_ucie_linkspeed_t;

/**
 * @brief Enum representing UCIe linkup status.
 *
 * This enum includes a list of UCIe linkup status using for UCIe operations.
 */
typedef enum e_ucie_linkup_status {
    LINKUP_SUCCESS,
    LINKUP_TIMEOUT,
    LINKUP_ERROR
} e_ucie_linkup_status_t; 

/**
 * @brief Enum representing different UCIe HDMA channel.
 *
 * This enum includes a list of HDMA channel using for HDMA operations.
 */
typedef enum e_ucie_hdma_ch {
    HDMA_CH0,
    HDMA_CH1,
    HDMA_CH2,
    HDMA_CH3,
    HDMA_CH4,
    HDMA_CH5,
    HDMA_CH6,
    HDMA_CH7,
    HDMA_CH8,
    HDMA_CH9,
    HDMA_CH10,
    HDMA_CH11,
    HDMA_CH12,
    HDMA_CH13,
    HDMA_CH14,
    HDMA_CH15,
    HDMA_CH16,
    HDMA_CH17,
    HDMA_CH18,
    HDMA_CH19,
    HDMA_CH20,
    HDMA_CH21,
    HDMA_CH22,
    HDMA_CH23,
    HDMA_CH24,
    HDMA_CH25,
    HDMA_CH26,
    HDMA_CH27,
    HDMA_CH28,
    HDMA_CH29,
    HDMA_CH30,
    HDMA_CH31
} e_ucie_hdma_ch_t;

/** 
 * @brief Enum representing different UCIe HDMA transfer mode.
 *
 * This enum includes a list of HDMA transfer mode using for HDMA operations.
 */
typedef enum e_ucie_hdma_mode {
    HDMA_WRITE,
    HDMA_READ
} e_ucie_hdma_mode_t;

/**
 * @brief Enum representing different iATU region.
 *
 * This enum includes a list of iATU region using for INBOUND/OUTBOUND setting.
 */
typedef enum e_ucie_iatu_region {
    IATU_RGN0,
    IATU_RGN1,
    IATU_RGN2,
    IATU_RGN3,
    IATU_RGN4,
    IATU_RGN5,
    IATU_RGN6,
    IATU_RGN7,
    IATU_RGN8,
    IATU_RGN9,
    IATU_RGN10,
    IATU_RGN11,
    IATU_RGN12,
    IATU_RGN13,
    IATU_RGN14,
    IATU_RGN15,
    IATU_RGN16,
    IATU_RGN17,
    IATU_RGN18,
    IATU_RGN19,
    IATU_RGN20,
    IATU_RGN21,
    IATU_RGN22,
    IATU_RGN23,
    IATU_RGN24,
    IATU_RGN25,
    IATU_RGN26,
    IATU_RGN27,
    IATU_RGN28,
    IATU_RGN29,
    IATU_RGN30,
    IATU_RGN31
} e_ucie_iatu_region_t;

/**
* @brief Direction type for iATU (Internal Address Translation Unit) configuration.
*
* This enum defines the direction of address translation for iATU.
*/
typedef enum e_ucie_iatu_type {
    IATU_OUTBOUND,  /**< Translate local address to remote */
    IATU_INBOUND    /**< Translate remote address to local */
} e_ucie_iatu_type_t;

/**
 * @brief Structure to hold configuration for UCIe HDMA controller.
 */
typedef struct st_ucie_hdma_cfg {
    e_ucie_ch_t ucie_ch;        /**< UCIe channel >*/
    e_ucie_hdma_ch_t hdma_ch;   /**< HDMA channel >*/
    uint64_t mSrcAddr;          /**< Source address >*/
    uint64_t mDestAddr;         /**< Destination address >*/
    uint32_t size;              /**< Transfer size >*/
    e_ucie_hdma_mode_t rw;      /**< Read/Write mode >*/
} st_ucie_hdma_cfg_t;

/**
 * @brief Structure to hold configuration for UCIe iATU setting.
 */
typedef struct st_ucie_iatu_cfg {
    e_ucie_ch_t ucie_ch;        /**< UCIe channel >*/
    e_ucie_iatu_region_t rgn;    /**< UCIe iATU region >*/
    e_ucie_iatu_type_t type;    /**< UCIe iATU type >*/
    uint64_t mSrcAddr;          /**< Source address >*/
    uint64_t mDestAddr;         /**< Destination address >*/
    uint32_t size;              /**< Region size >*/
} st_ucie_iatu_cfg_t;

/**
 * @brief Config UCIe protocol.
 *
 * This function using for config UCIe protocol.
 *
 * @param[in] ch                UCIe channel.
 * @param[in] mode              UCIe mode.
 * @param[in] speed             UCIe transfer speed.
 * @param[in] init_with_system  Init and linkup when system available.
 *
 * @return 0 if success, other is error.
 */
uint32_t R_UCIE_Config(e_ucie_ch_t ch, e_ucie_mode_t mode,
                       e_ucie_linkspeed_t speed, bool init_with_system);

/**
 * @brief Setup UCIe protocol.
 *
 * This function using for setup and linkup UCIe protocol.
 *
 * @param[in] ch        UCIe channel.
 * @param[in] mode      UCIe mode.
 * @param[in] speed     UCIe transfer speed.
 *
 * @return Linkup status.
 */
e_ucie_linkup_status_t R_UCIE_Setup(e_ucie_ch_t ch, e_ucie_mode_t mode,
                                                    e_ucie_linkspeed_t speed);

/**
 * @brief Retry linkup UCIe protocol.
 *
 * This function using for retry linkup UCIe protocol
 * when the first time linkup is timeout.
 *
 * @param[in] ch        UCIe channel.
 * @param[in] mode      UCIe mode.
 * @param[in] speed     UCIe transfer speed.
 * @param[in] retry     Maximum number of retries.
 *
 * @return Linkup status.
 */
e_ucie_linkup_status_t R_UCIE_Retry_Linkup(e_ucie_ch_t ch, e_ucie_mode_t mode,
                                           e_ucie_linkspeed_t speed, uint16_t retry);

/**
 * @brief Get UCIe status.
 *
 * This function using for get UCIe status.
 *
 * @param[in] ch        UCIe channel.
 *
 * @return Linkup status.
 */
e_ucie_linkup_status_t R_UCIE_Get_Linkup_Status(e_ucie_ch_t ch);

/**
 * @brief UCIe HDMA set config.
 *
 * This function using for set config to HDMA.
 *
 * @param[in] cfg Pointer to the instance HMDA config structure.
 *
 * @return 0 if success, non-zero if error.
 */
uint32_t R_UCIE_HDMA_SetConfig(st_ucie_hdma_cfg_t *cfg);

/**
 * @brief UCIe HDMA start.
 *
 * This function using for start HDMA transfer.
 *
 * @param[in] cfg Pointer to the instance HMDA config structure.
 *
 * @return 0 if success, non-zero if error.
 */
uint32_t R_UCIE_HDMA_Start(st_ucie_hdma_cfg_t *cfg);

/**
 * @brief UCIe HDMA Wait Stop.
 *
 * This function using for waiting HDMA transfer stop.
 *
 * @param[in] cfg Pointer to the instance HMDA config structure.
 *
 * @return 0 if success, non-zero if timeout.
 */
uint32_t R_UCIE_HDMA_WaitStop(st_ucie_hdma_cfg_t *cfg);

/**
 * @brief UCIe HDMA Stop.
 *
 * This function using for stop HDMA transfer.
 *
 * @param[in] cfg Pointer to the instance HMDA config structure.
 *
 * @return 0 if success, non-zero if error.
 */
uint32_t R_UCIE_HDMA_Stop(st_ucie_hdma_cfg_t *cfg);

/**
 * @brief Set UCIe iATU region.
 *
 * This function using for set iATU region.
 *
 * @param[in] cfg Pointer to the instance iATU config structure.
 *
 * @return 0 if success, non-zero if error.
 */
uint32_t R_UCIE_IATU_SetRegion(st_ucie_iatu_cfg_t *cfg);

/**
 * @brief Unset UCIe iATU region.
 *
 * This function using for unset iATU region.
 *
 * @param[in] cfg Pointer to the instance iATU config structure.
 *
 * @return 0 if success, non-zero if error.
 */
uint32_t R_UCIE_IATU_UnsetRegion(st_ucie_iatu_cfg_t *cfg);

void R_UCIE_Setup_EP_BAR(e_ucie_ch_t ch);

#ifdef __cplusplus
}
#endif

#endif /* R_UCIE_H */
/** @} */  // end of file-level defgroup
