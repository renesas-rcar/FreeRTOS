#
# Copyright (c) 2025 Renesas Electronics Corporation
#
# SPDX-License-Identifier: MIT
#

#
# Generate pm_skip_config.h (per target / per core) from pm_skip_config.json
#

cmake_minimum_required(VERSION 3.19)   # string(JSON ...)

set(PM_SKIP_TEMPLATE ${CMAKE_CURRENT_LIST_DIR}/pm_skip_config.h.in)

# BOARD (CMake) -> SoC key + board key tinrong JSON
# BOARD -> SoC key + board key in JSON
function(pm_resolve_board board out_soc out_key)
    if("${board}" STREQUAL "x5h_ironhide")
        set(soc x5h)
        set(key ironhide)
    elseif("${board}" STREQUAL "mdp_x5h_hil")
        set(soc x5h)
        set(key mdp_x5h)
    elseif("${board}" STREQUAL "x5h_rfs2")
        set(soc x5h)
        set(key x5h_rfs2)
    elseif("${board}" STREQUAL "mdp_aiacc_hil")
        set(soc aiacc)
        set(key mdp_aiacc)
    elseif("${board}" STREQUAL "mdp_aiacc_rfs2")
        set(soc aiacc)
        set(key mdp_aiacc_rfs2)
    else()
        message(
            FATAL_ERROR
            "pm_skip_config: unsupported BOARD '${board}'"
        )
    endif()

    set(${out_soc} "${soc}" PARENT_SCOPE)
    set(${out_key} "${key}" PARENT_SCOPE)
endfunction()

function(pm_json_append_entries out_var json)
    string(JSON type ERROR_VARIABLE err TYPE "${json}" ${ARGN})
    if(err OR NOT type STREQUAL "ARRAY")
        return()
    endif()
    string(JSON len LENGTH "${json}" ${ARGN})
    if(len EQUAL 0)
        return()
    endif()

    set(result "${${out_var}}")
    math(EXPR last "${len} - 1")
    foreach(i RANGE ${last})
        string(JSON item_type TYPE "${json}" ${ARGN} ${i})
        if(item_type STREQUAL "STRING")
            string(JSON from GET "${json}" ${ARGN} ${i})
            set(to "${from}")
        elseif(item_type STREQUAL "OBJECT")
            string(JSON from GET "${json}" ${ARGN} ${i} from)
            string(JSON to   GET "${json}" ${ARGN} ${i} to)
        else()
            message(FATAL_ERROR "pm_skip_config: invalid item at ${ARGN} [${i}]")
        endif()
        string(APPEND result "    { (${from}), (${to}) },\n")
    endforeach()
    set(${out_var} "${result}" PARENT_SCOPE)
endfunction()

# pm_generate_skip_header(<target> <core> <BOARD> <json_file>)
function(pm_generate_skip_header target core board json_file)
    pm_resolve_board("${board}" soc board_key)

    file(READ "${json_file}" json)

    set_property(
        DIRECTORY
        APPEND
        PROPERTY CMAKE_CONFIGURE_DEPENDS "${json_file}"
    )

    foreach(type power power_restore clock clock_off reset)
        string(TOUPPER "${type}" TYPE_UP)

        set(entries "")

        pm_json_append_entries(
            entries "${json}"
            socs "${soc}" "${type}_skip" all
        )

        pm_json_append_entries(
            entries "${json}"
            socs "${soc}" "${type}_skip" "cpu${core}"
        )

        pm_json_append_entries(
            entries "${json}"
            boards "${board_key}" "${type}_skip" all
        )

        pm_json_append_entries(
            entries "${json}"
            boards "${board_key}" "${type}_skip" "cpu${core}"
        )

        set(PM_${TYPE_UP}_SKIP_ENTRIES "${entries}")
    endforeach()

    set(PM_BOARD_NAME "${board}")
    set(PM_CORE_ID "${core}")

    get_filename_component(PM_JSON_NAME "${json_file}" NAME)
    string(CONCAT PM_HEADER_COMMENT
        "/* AUTO-GENERATED from ${PM_JSON_NAME} - DO NOT EDIT\n"
        " * board : ${PM_BOARD_NAME}\n"
        " * core  : ${PM_CORE_ID}\n"
        " */"
    )

    set(
        gen_dir
        "${CMAKE_CURRENT_BINARY_DIR}/generated/core${core}"
    )

    configure_file(
        "${PM_SKIP_TEMPLATE}"
        "${gen_dir}/pm_skip_config.h"
        @ONLY
    )

    target_include_directories(
        "${target}"
        PRIVATE
        "${gen_dir}"
    )

    message(
        STATUS
        "pm_skip_config: ${target} "
        "(board=${board}, soc=${soc}, core=${core})"
    )
endfunction()

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

    pm_generate_skip_header(
        "${app_name}"
        "${CORE}"
        "${BOARD}"
        "${CMAKE_CURRENT_SOURCE_DIR}/pm_skip_config.json"
    )

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