#
# Copyright (c) 2026 Renesas Electronics Corporation
#
# SPDX-License-Identifier: MIT
#

# rsc table linker scripts, one per link (shared memory placement).
set(RSC_LD_LOCAL ${BSP_DIR}/common/linker/lscript_rsc_table_rcar_rpmsg.ld)          # 0x96600000
if(BOARD STREQUAL "x5h_rfs2" OR BOARD STREQUAL "mdp_aiacc_rfs2")
    # RFS2: the x5h memory map has no MPU region at 0x9E600000.
    set(RSC_LD_UCIE0 ${BSP_DIR}/common/linker/lscript_rsc_table_rcar_rpmsg_ucie0_rfs.ld)  # 0x96600000
else()
    set(RSC_LD_UCIE0 ${BSP_DIR}/common/linker/lscript_rsc_table_rcar_rpmsg_ucie0.ld)      # 0x9E600000
endif()

# Link one rcar_rpmsg sample. Empty ldfile: default load region.
# Extra arguments: rsc table linker scripts, one per link the app uses
# (default: local one only).
function(add_rcar_rpmsg_app app_name source ldfile)
    add_executable(${app_name}
        ${source}
    )

    target_include_directories(${app_name}
        PRIVATE
        ${BSP_DIR}
        ${BSP_DIR}/CMSIS_5
        ${BSP_DIR}/CMSIS_5/CMSIS/Core_R/Include
    )

    target_link_libraries(${app_name}
        freertos_bsp
        openamp
        libmetal
        m
    )

    if(NOT "${ldfile}" STREQUAL "")
        set_target_properties(${app_name} PROPERTIES
            LINK_OPTIONS ""  # Clear all link options
            LINK_FLAGS "-lnosys --specs=nosys.specs -L${BSP_DIR}/common/linker/ -T ${ldfile} -Wl,--no-warn-rwx-segments"
        )
    endif()

    set(rsc_ldfiles ${ARGN})
    if(NOT rsc_ldfiles)
        set(rsc_ldfiles ${RSC_LD_LOCAL})
    endif()
    foreach(rsc_ldfile IN LISTS rsc_ldfiles)
        target_link_options(${app_name}
            PRIVATE
            -T${rsc_ldfile}
        )
    endforeach()

    target_link_options(${app_name} BEFORE
        PRIVATE
        -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/${app_name}.map
    )

    convert_bin_to_srec(${app_name})
endfunction()

# One ELF per core, each in its own memory region (as rpmsg_sample).
foreach(CORE 0 1)
    if(BOARD STREQUAL "mdp_aiacc_hil" OR BOARD STREQUAL "mdp_aiacc_rfs2")
        if(CORE EQUAL 0)
            set(TEMP_LDFILE ${BSP_DIR}/common/linker/lscript_dram3.ld)
        else()
            set(TEMP_LDFILE ${BSP_DIR}/common/linker/lscript_dram.ld)
        endif()
    else()
        if(CORE EQUAL 0)
            set(TEMP_LDFILE ${BSP_DIR}/common/linker/lscript_vram1.ld)
        else()
            set(TEMP_LDFILE ${BSP_DIR}/common/linker/lscript_vram2.ld)
        endif()
    endif()

    add_rcar_rpmsg_app(rcar_rpmsg_core${CORE}          rcar_rpmsg_sample.c   ${TEMP_LDFILE})
    add_rcar_rpmsg_app(rcar_rpmsg_api_test_core${CORE} rcar_rpmsg_api_test.c ${TEMP_LDFILE})
endforeach()

# UCIe CH0, one source: RC (FE) on the X5H chip, EP (BE) on the AIACC chip.
# UCIe link only (local channels: rcar_rpmsg_core0/1). Shared memory:
# 0x9E600000 (UCIE_SHARE) on HIL, 0x96600000 on RFS2.
if(BOARD STREQUAL "mdp_x5h_hil" OR BOARD STREQUAL "x5h_rfs2")
    add_rcar_rpmsg_app(rcar_rpmsg_ucie_rc rcar_rpmsg_ucie_sample.c "" ${RSC_LD_UCIE0})
elseif(BOARD STREQUAL "mdp_aiacc_hil" OR BOARD STREQUAL "mdp_aiacc_rfs2")
    add_rcar_rpmsg_app(rcar_rpmsg_ucie_ep rcar_rpmsg_ucie_sample.c "" ${RSC_LD_UCIE0})
endif()

# Echo: core 1 only, core 0 runs rpmsg-echo (rpmsg_sample). X5H only.
if(NOT (BOARD STREQUAL "mdp_aiacc_hil" OR BOARD STREQUAL "mdp_aiacc_rfs2"))
    add_rcar_rpmsg_app(rcar_rpmsg_echo_core1 rcar_rpmsg_echo_sample.c
                       ${BSP_DIR}/common/linker/lscript_vram2.ld)
endif()
