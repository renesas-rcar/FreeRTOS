#
# Copyright (c) 2025 Renesas Electronics Corporation
#
# SPDX-License-Identifier: MIT
#

set(app_name "smmu_app")

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

# Add output program address map file
target_link_options(${app_name}
    PUBLIC
        -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/${app_name}.map
)

convert_bin_to_srec(${app_name})

#------------------------------------------------------------------------------
# SMMU RT CORE
#------------------------------------------------------------------------------

set(CORES 0 1)

foreach(CORE IN LISTS CORES)

    if(CORE EQUAL 0)
        set(app_name "smmu_rt_core_app")
        set(TEMP_LDFILE ${BSP_DIR}/common/linker/lscript_vram1.ld)
    elseif(CORE EQUAL 1)
        set(app_name "smmu_rt_core_app_core1")
        set(TEMP_LDFILE ${BSP_DIR}/common/linker/lscript_vram2.ld)
    endif()

    add_executable(${app_name}
        smmu_core.c
    )

    target_compile_definitions(${app_name}
        PUBLIC
            APP_CORE_ID=${CORE}
    )

    target_link_libraries(${app_name}
        freertos_bsp
        m
    )

    set_target_properties(${app_name} PROPERTIES
        LINK_OPTIONS ""
        LINK_FLAGS "-lnosys --specs=nosys.specs \
                    -L${BSP_DIR}/common/linker/ \
                    -T ${TEMP_LDFILE} \
                    -Wl,--no-warn-rwx-segments"
    )

    target_link_options(${app_name} BEFORE
        PRIVATE
            -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/${app_name}.map
    )

    convert_bin_to_srec(${app_name})

endforeach()

set(app_name "smmu_rt_dmac_app")

add_executable(${app_name}
    rtdmac_main.c
)

target_compile_definitions(${app_name}
    PUBLIC

)

# Essential libraries to link with
target_link_libraries(${app_name}
    freertos_bsp m
)

# Add output program address map file
target_link_options(${app_name}
    PUBLIC
        -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/${app_name}.map
)

convert_bin_to_srec(${app_name})
