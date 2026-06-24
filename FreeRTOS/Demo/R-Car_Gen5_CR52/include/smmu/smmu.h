/*
 *
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef R_SMMU_H_
#define R_SMMU_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stdbool.h"

/**
 * @brief Enum representing different SMMU domain types.
 *
 * This enum includes a list of different domain types that can be used for
 * SMMU operations.
 */
typedef enum e_smmu_domain {
    SMMU_DSP,          /**< DSP domain */
    SMMU_HCN,          /**< HCN domain */
    SMMU_HCS0,         /**< HCS0 domain */
    SMMU_HCS1,         /**< HCS1 domain */
    SMMU_IMN,          /**< IMN domain */
    SMMU_IMS,          /**< IMS domain */
    SMMU_MP,           /**< MP domain */
    SMMU_NPU0,         /**< NPU0 domain */
    SMMU_NPU1,         /**< NPU1 domain */
    SMMU_PERE,         /**< PERE domain */
    SMMU_PERW,         /**< PERW domain */
    SMMU_PV,           /**< PV domain */
    SMMU_RT,           /**< RT domain */
    SMMU_SCP,          /**< SCP domain */
    SMMU_VI0,          /**< VI0 domain */
    SMMU_VI1,          /**< VI1 domain */
    SMMU_VIPN,         /**< VIPN domain */
    SMMU_VIPS,         /**< VIPS domain */
    SMMU_DSP_FMU,      /**< DSP FMU domain */
    SMMU_HCN_FMU,      /**< HCN FMU domain */
    SMMU_HCS0_FMU,     /**< HCS0 FMU domain */
    SMMU_HCS1_FMU,     /**< HCS1 FMU domain */
    SMMU_IMN_FMU,      /**< IMN FMU domain */
    SMMU_IMS_FMU,      /**< IMS FMU domain */
    SMMU_MP_FMU,       /**< MP FMU domain */
    SMMU_NPU0_FMU,     /**< NPU0 FMU domain */
    SMMU_NPU1_FMU,     /**< NPU1 FMU domain */
    SMMU_PERE_FMU,     /**< PERE FMU domain */
    SMMU_PERW_FMU,     /**< PERW FMU domain */
    SMMU_PV_FMU,       /**< PV FMU domain */
    SMMU_RT_FMU,       /**< RT FMU domain */
    SMMU_SCP_FMU,      /**< SCP FMU domain */
    SMMU_VI0_FMU,      /**< VI0 FMU domain */
    SMMU_VI1_FMU,      /**< VI1 FMU domain */
    SMMU_VIPN_FMU,     /**< VIPN FMU domain */
    SMMU_VIPS_FMU      /**< VIPS FMU domain */
} e_smmu_domain_t;

/**
 * @brief Enum representing the enable/disable state of the SMMU.
 */
typedef enum e_smmu_en
{
    DISABLE = 0, /**< SMMU is disabled */
    ENABLE = 1   /**< SMMU is enabled */
} e_smmu_en_t;

/**
 * @brief Block memory attributes.
 */
#define ATTR_DEVICE_NGNRNE_EL1_RW_EL0_RW        (0ULL << 2) | (1ULL << 6) | (1ULL << 10)
#define ATTR_DEVICE_NGNRNE_EL1_RO_EL0_RO        (0ULL << 2) | (3ULL << 6) | (1ULL << 10)

/**
 * @brief Enum of fault codes returned by SMMU memory mapping operations.
 */
typedef enum e_smmu_map_fault_code {
    MAP_SUCCESS,            /** SUCCESS */
    MAP_ERR_INVALID_ADDR,   /** ERROR. VA, PA must be aligned with 4KB. */
    MAP_ERR_INVALID_SIZE,   /** ERROR. Size must be aligned with 4KB. */
    MAP_ERR_DUPLICATE,      /** ERROR. Duplicate VA map region. */
    MAP_ERR_NULL            /** ERROR. Provide NULL param/No HEAP space. */
} e_smmu_map_fault_code_t;

/**
 * @brief Structure representing a command sent to the SMMU.
 *
 * This structure contains the command opcode and specific command parameters
 * depending on the operation to be performed.
 */
#pragma pack(1)
typedef struct st_smmu_cmd {
    uint64_t opcode : 8; /**< [7:0] Command opcode */

    union {
        /**
         * @brief Configuration command for SMMU STE (Stream Table Entry).
         */
        #define CMDQ_OP_CFGI_STE        0x03
        struct { // CMD_CFGI_STE
            volatile uint64_t res : 2;    /**< [9:8] Reserved */
            volatile uint64_t ssec : 1;   /**< [10] Secure mode */
            volatile uint64_t res1 : 21;  /**< [31:11] Reserved */
            volatile uint64_t sid : 32;   /**< [63:32] Stream ID */
            volatile uint64_t leaf : 1;   /**< [64] Leaf flag */
            volatile uint64_t res2 : 63;  /**< [127:65] Reserved */
        } st_cmd_cfgi_ste_t;

        /**
         * @brief Configuration command for a range of SMMU STE.
         */
        #define CMDQ_OP_CFGI_STE_RANGE  0x04
        struct { // CMD_CFGI_STE_RANGE
            volatile uint64_t res3 : 2;   /**< [9:8] Reserved */
            volatile uint64_t ssec1 : 1;  /**< [10] Secure mode */
            volatile uint64_t res4 : 21;  /**< [31:11] Reserved */
            volatile uint64_t sid1 : 32;  /**< [63:32] Stream ID */
            volatile uint64_t range : 5;  /**< [68:64] Range */
            volatile uint64_t res5 : 59;  /**< [127:69] Reserved */
        } st_cmd_cfgi_ste_range_t;

        /**
         * @brief Command to synchronize the SMMU operation.
         */
        #define CMDQ_OP_CMD_SYNC   0x46
        struct { // CMD_SYNC
            volatile uint64_t res6 : 4;    /**< [11:8] Reserved */
            volatile uint64_t cs : 2;      /**< [13:12] CS flag */
            volatile uint64_t res7 : 8;    /**< [21:14] Reserved */
            volatile uint64_t msh : 2;     /**< [23:22] MSH flag */
            volatile uint64_t msiattr : 4; /**< [27:24] MSI attributes */
            volatile uint64_t res8 : 4;    /**< [31:28] Reserved */
            volatile uint64_t msidata : 32; /**< [63:32] MSI data */
            volatile uint64_t res9 : 2;    /**< [65:64] Reserved */
            volatile uint64_t msiaddress : 50; /**< [115:66] MSI address */
            volatile uint64_t res10 : 12;  /**< [127:116] Reserved */
        } st_cmd_sync_t;

        /**
         * @brief Command for SMMU TLBI (Translation Lookaside Buffer Invalidate).
         */
        #define CMDQ_OP_TLBI_NSNH_ALL   0x30
        #define CMD_TLBI_NH_ALL         0x10
        struct { // CMD_TLBI_NSNH_ALL
            volatile uint64_t res11 : 64; /**< [71:8] Reserved */
            volatile uint64_t res12 : 56; /**< [127:72] Reserved */
        } st_cmd_tlbi_nsnh_all_t;

        /**
         * @brief Command for configuring the SMMU Context Descriptor (CD).
         */
        #define CMDQ_OP_CMD_CFGI_CD   0x05
        struct { // CMD_CFGI_CD
            volatile uint64_t res3 : 2;    /**< [9:8] Reserved */
            volatile uint64_t ssec1 : 1;   /**< [10] Secure mode */
            volatile uint64_t res4 : 1;    /**< [11] Reserved */
            volatile uint64_t ssid : 20;   /**< [31:12] SSID */
            volatile uint64_t sid : 32;    /**< [63:32] Stream ID */
            volatile uint64_t leaf : 1;    /**< [64] Leaf flag */
            volatile uint64_t res5 : 63;   /**< [127:65] Reserved */
        } st_cmd_cfgi_cd_t;
    };
} st_smmu_cmd_t;
#pragma pack()

/**
 * @brief Structure to hold instance control information for SMMU.
 */
typedef struct st_smmu_streamid_instance_ctrl
{   
    uint32_t stream_id;                /**< The stream id of SMMU domain */
    void *p_context;                   /**< Context */
    const e_smmu_domain_t smmu_domain; /**< The SMMU domain type */
    bool is_secure;                    /**< Stream id type.
                                            True: Secure, False: Non-secure */
} st_smmu_streamid_instance_ctrl_t;

/**
 * @brief Initializes the SMMU.
 *
 * This function initializes the SMMU for the specified instance.
 *
 * @param[in] smmu_domain SMMU domain.
 * @param[in] is_secure SMMU type. True: Secure, False: Non-secure.
 *
 * @return 0 if initialization is successful, non-zero if an error occurs.
 */
int R_SMMU_Init(e_smmu_domain_t smmu_domain, bool is_secure);

/**
 * @brief Disables the SMMU.
 *
 * This function disables the SMMU and stops address translation.
 * Release all SMMU resource.
 *
 * @param[in] smmu_domain SMMU domain.
 * @param[in] is_secure SMMU type. True: Secure, False: Non-secure.
 */
void R_SMMU_Deinit(e_smmu_domain_t smmu_domain, bool is_secure);

/**
 * @brief Attach stream id into the specified SMMU domain.
 *
 * This function attaches a stream ID to the SMMU domain for translation.
 *
 * @param[in] p_ctrl Pointer to the instance control structure.
 *
 * @return 0 if successful, non-zero if an error occurs.
 */
int R_SMMU_Attach(st_smmu_streamid_instance_ctrl_t *p_ctrl);

/**
 * @brief Detach stream id from the specified SMMU domain.
 *
 * This function detaches a stream ID from the SMMU domain.
 *
 * @param[in] p_ctrl Pointer to the instance control structure.
 */
void R_SMMU_Detach(st_smmu_streamid_instance_ctrl_t *p_ctrl);

/**
 * @brief Maps a virtual address (VA) to a physical address (PA).
 *
 * This function maps a given VA to a PA for translation by the SMMU.
 *
 * @param[in] p_ctrl Pointer to the instance control structure.
 * @param[in] va Virtual address to map.
 * @param[in] pa Physical address to map.
 * @param[in] size Size of the memory region to map.
 * @param[in] attr Block attributes.
 *
 * @return e_smmu_map_fault_code_t
 */
e_smmu_map_fault_code_t R_SMMU_Map(st_smmu_streamid_instance_ctrl_t *p_ctrl,
                        uint64_t va, uint64_t pa, uint64_t size, uint64_t attr);

/**
 * @brief Unmaps a previously mapped virtual address (VA) from a physical address (PA).
 *
 * This function unmaps a given VA from the corresponding PA.
 *
 * @param[in] p_ctrl Pointer to the instance control structure.
 * @param[in] va Virtual address to unmap.
 * @param[in] pa Physical address to unmap.
 * @param[in] size Size of the memory region to unmap.
 */
void R_SMMU_Unmap(st_smmu_streamid_instance_ctrl_t *p_ctrl, uint64_t va, uint64_t pa, uint64_t size);

/**
 * @brief Reads and processes events from the Event Queue (EVTQ).
 *
 * This function reads and processes the events that are stored in the event queue.
 */
void R_SMMU_ProcessEventQueue(void);

/**
 * @brief Issues a TLB invalidation command.
 *
 * This function invalidates the TLB entries for the specified SMMU instance.
 *
 * @param[in] smmu_domain SMMU domain.
 * @param[in] is_secure SMMU type. True: Secure, False: Non-secure.
 *
 * @return 0 if the TLB invalidation is successful, non-zero otherwise.
 */
int R_SMMU_InvalidateTLB(e_smmu_domain_t smmu_domain, bool is_secure);

/**
 * @brief Issues a command to the command queue.
 *
 * This function issues a command to the SMMU command queue.
 *
 * @param[in] smmu_domain SMMU domain.
 * @param[in] is_secure SMMU type. True: Secure, False: Non-secure.
 * @param[in] p_cmd Pointer to the command to be issued.
 * @param[in] sync Pass false if don't want to sync and true if want to sync. 
 *
 * @return 0 if command is successfully issued, non-zero otherwise.
 */
int R_SMMU_IssueCommand(e_smmu_domain_t smmu_domain, bool is_secure, st_smmu_cmd_t *p_cmd, bool sync);

/**
 * @brief Enables the SMMU.
 *
 * This function enables the SMMU to start performing address translation.
 *
 * @param[in] smmu_domain SMMU domain.
 * @param[in] is_secure SMMU type. True: Secure, False: Non-secure.
 *
 * @return 0 if initialization is successful, non-zero if an error occurs.
 */
int R_SMMU_Enable(e_smmu_domain_t smmu_domain, bool is_secure);

#ifdef __cplusplus
}
#endif

#endif /* R_SMMU_H_ */
