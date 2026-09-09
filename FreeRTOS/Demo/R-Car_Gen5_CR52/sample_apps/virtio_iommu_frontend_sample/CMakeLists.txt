#
# Copyright (c) 2025 Renesas Electronics Corporation
#
# SPDX-License-Identifier: MIT
#

if(NOT DEFINED MFIS_CHAN)
    set(CORES 0 1)
    foreach(CORE IN LISTS CORES)
        if(CORE EQUAL 0)
            if(BOARD STREQUAL "mdp_aiacc_hil" OR BOARD STREQUAL "mdp_aiacc_rfs2")
                set(TEMP_LDFILE ${BSP_DIR}/common/linker/lscript_dram3.ld)
            else()
                set(TEMP_LDFILE ${BSP_DIR}/common/linker/lscript_vram1.ld)
            endif()
            set(MFIS_CHAN 0)
        elseif(CORE EQUAL 1)
            if(BOARD STREQUAL "mdp_aiacc_hil" OR BOARD STREQUAL "mdp_aiacc_rfs2")
                set(TEMP_LDFILE ${BSP_DIR}/common/linker/lscript_dram.ld)
            else()
                set(TEMP_LDFILE ${BSP_DIR}/common/linker/lscript_vram2.ld)
            endif()   
            set(MFIS_CHAN 1)
        else()
            message(FATAL_ERROR "INVALID CORE: ${CORE}")
        endif()

        #message(STATUS "MFIS channel: ${MFIS_CHAN}")
        #message(STATUS "VRAM_REGION: ${VRAM_REGION}")

        set(app_name "virtio_iommu_frontend_app_mfis${MFIS_CHAN}_cluster0_core${CORE}")

        #message(STATUS "TEMP_LDFILE: ${TEMP_LDFILE}")

        add_executable(${app_name}
            virtio-iommu-frontend-app.c
        )

        target_compile_definitions(${app_name}
            PUBLIC MFIS_CHAN=${MFIS_CHAN}
        )

        target_include_directories(${app_name}
            PRIVATE
            ${BSP_DIR}
            ${BSP_DIR}/CMSIS_5
            ${BSP_DIR}/CMSIS_5/CMSIS/Core_R/Include
        )

        # Essential libraries to link with
        target_link_libraries(${app_name}
            freertos_bsp
            openamp
            libmetal
            m
        )

        set_target_properties(${app_name} PROPERTIES
            LINK_OPTIONS ""  # Clear all link options
            LINK_FLAGS "-lnosys --specs=nosys.specs -L${BSP_DIR}/common/linker/ -T ${TEMP_LDFILE} -Wl,--no-warn-rwx-segments"
        )

        target_link_options(${app_name}
            PRIVATE
            -T${BSP_DIR}/common/linker/lscript_rsc_table_virtio.ld
        )

        target_link_options(${app_name} BEFORE
            PRIVATE
            -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/${app_name}.map
        )

        convert_bin_to_srec(${app_name})
    endforeach()
else()
        if(NOT MFIS_CHAN EQUAL 0 AND NOT MFIS_CHAN EQUAL 1 AND NOT MFIS_CHAN EQUAL 2)
            message(FATAL_ERROR "MFIS CHANNEL ${MFIS_CHAN}: RPMSG NOT supported! Re-check with Linux team!")
        endif()

        message(STATUS "USER define: MFIS channel ${MFIS_CHAN}")
        message(STATUS "RAM_REGION: ${RAM_REGION}")

        if(RAM_REGION EQUAL 1)
            set(CORE 0)
        elseif(RAM_REGION EQUAL 2)
            set(CORE 1)
        else()
            set(CORE 0)
        endif()

        message(STATUS "CORE: ${CORE}")

        set(app_name "virtio_iommu_frontend_app_mfis${MFIS_CHAN}_cluster0_core${CORE}")

        add_executable(${app_name}
            virtio-iommu-frontend-app.c
        )

        target_compile_definitions(${app_name}
            PUBLIC MFIS_CHAN=${MFIS_CHAN}
        )

        target_include_directories(${app_name}
            PRIVATE
            ${BSP_DIR}
            ${BSP_DIR}/CMSIS_5
            ${BSP_DIR}/CMSIS_5/CMSIS/Core_R/Include
        )

        # Essential libraries to link with
        target_link_libraries(${app_name}
            freertos_bsp
            openamp
            libmetal
            m
        )

        target_link_options(${app_name}
            PRIVATE
            -T${BSP_DIR}/common/linker/lscript_rsc_table_virtio.ld
        )

        target_link_options(${app_name} BEFORE
            PRIVATE
            -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/${app_name}.map
        )

        convert_bin_to_srec(${app_name})
endif()
