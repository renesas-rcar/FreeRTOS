#
# Copyright (c) 2025 Renesas Electronics Corporation
#
# SPDX-License-Identifier: MIT
#
set(CORES 0 1)
foreach(CORE IN LISTS CORES)
    if(CORE EQUAL 0)
        if(BOARD STREQUAL "mdp_aiacc_hil")
            set(TEMP_LDFILE ${BSP_DIR}/common/linker/lscript_dram3.ld)
        else()
            set(TEMP_LDFILE ${BSP_DIR}/common/linker/lscript_vram1.ld)
        endif()
    elseif(CORE EQUAL 1)
        if(BOARD STREQUAL "mdp_aiacc_hil")
            set(TEMP_LDFILE ${BSP_DIR}/common/linker/lscript_dram.ld)
        else()
            set(TEMP_LDFILE ${BSP_DIR}/common/linker/lscript_vram2.ld)
        endif()
    else()
        message(FATAL_ERROR "INVALID CORE: ${CORE}")
    endif()

    #--------------------------------------------------------------------------
    # POWER-MANAGEMENT APP
    #--------------------------------------------------------------------------
    set(app_name "power-management_app_core${CORE}")

    if(BOARD STREQUAL "x5h_ironhide" OR BOARD STREQUAL "mdp_x5h_hil" OR BOARD STREQUAL "x5h_rfs2")
        add_executable(${app_name}
            main_x5h.c
        )
    elseif(BOARD STREQUAL "mdp_aiacc_hil" OR BOARD STREQUAL "mdp_aiacc_rfs2")
        add_executable(${app_name}
            main_aiacc.c
        )
    else()
        message(FATAL_ERROR "INVALID BOARD: ${BOARD}")
    endif()

    target_compile_definitions(${app_name}
        PUBLIC
            APP_CORE_ID=${CORE}
    )
    target_include_directories(${app_name}
        PRIVATE
            ${BSP_DIR}
            ${BSP_DIR}/CMSIS_5
            ${BSP_DIR}/CMSIS_5/CMSIS/Core_R/Include
    )
    # Essential libraries to link with
    target_link_libraries(${app_name}
        freertos_bsp m
    )
    set_target_properties(${app_name} PROPERTIES
        LINK_OPTIONS ""  # Clear all link options
        LINK_FLAGS "-lnosys --specs=nosys.specs \
                    -L${BSP_DIR}/common/linker/ \
                    -T ${TEMP_LDFILE} \
                    -Wl,--no-warn-rwx-segments"
    )
    # Add output program address map file
    target_link_options(${app_name} BEFORE
        PRIVATE
            -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/${app_name}.map
    )
    convert_bin_to_srec(${app_name})

    #--------------------------------------------------------------------------
    # DEEPSTOP TEST
    #--------------------------------------------------------------------------
    set(deepstop_test "deepstop_test_core${CORE}")
    add_executable(${deepstop_test}
        deepstop_test.c
    )
    target_compile_definitions(${deepstop_test}
        PUBLIC
            APP_CORE_ID=${CORE}
    )
    target_include_directories(${deepstop_test}
        PRIVATE
            ${BSP_DIR}
            ${BSP_DIR}/CMSIS_5
            ${BSP_DIR}/CMSIS_5/CMSIS/Core_R/Include
    )
    # Essential libraries to link with
    target_link_libraries(${deepstop_test}
        freertos_bsp m
    )
    set_target_properties(${deepstop_test} PROPERTIES
        LINK_OPTIONS ""  # Clear all link options
        LINK_FLAGS "-lnosys --specs=nosys.specs \
                    -L${BSP_DIR}/common/linker/ \
                    -T ${TEMP_LDFILE} \
                    -Wl,--no-warn-rwx-segments"
    )
    # Add output program address map file
    target_link_options(${deepstop_test} BEFORE
        PRIVATE
            -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/${deepstop_test}.map
    )
    convert_bin_to_srec(${deepstop_test})
endforeach()