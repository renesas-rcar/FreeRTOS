#
# Copyright (c) 2026 Renesas Electronics Corporation
#
# SPDX-License-Identifier: MIT
#

set(CORES 0 1)

foreach(CORE IN LISTS CORES)
    if (CORE EQUAL 0)
        set(app_name "mfis_cr_cr")
    else()
        set(app_name "mfis_cr_cr_core${CORE}")
    endif()

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

    add_executable(${app_name}
        main.c
    )

    target_compile_definitions(${app_name}
        PUBLIC
    )

    # Essential libraries to link with
    target_link_libraries(${app_name}
        freertos_bsp m
    )

    set_target_properties(${app_name} PROPERTIES
        LINK_OPTIONS ""  # Clear all link options
        LINK_FLAGS "-lnosys --specs=nosys.specs -L${BSP_DIR}/common/linker/ -T ${TEMP_LDFILE} -Wl,--no-warn-rwx-segments"
    )

    # Add output program address map file
    target_link_options(${app_name} BEFORE
        PRIVATE
        -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/${app_name}.map
    )

    convert_bin_to_srec(${app_name})
endforeach()