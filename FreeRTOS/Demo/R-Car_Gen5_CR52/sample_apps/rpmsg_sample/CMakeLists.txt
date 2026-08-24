#
# Copyright (c) 2025 Renesas Electronics Corporation
#
# SPDX-License-Identifier: MIT
#

macro(add_rpmsg_app APP_NAME SOURCE_FILE MFIS_CHAN VRAM_REGION)
    set(TEMP_LDFILE ${BSP_DIR}/common/linker/lscript_vram${VRAM_REGION}.ld)

    add_executable(${APP_NAME}
        ${SOURCE_FILE}
        platform_rcar.c
        remoteproc_rcar.c
        rsc_table.c
    )
    target_compile_definitions(${APP_NAME}
        PUBLIC MFIS_CHAN=${MFIS_CHAN}
    )
    target_include_directories(${APP_NAME}
        PRIVATE
        ${BSP_DIR}
        ${BSP_DIR}/CMSIS_5
        ${BSP_DIR}/CMSIS_5/CMSIS/Core_R/Include
    )
    target_link_libraries(${APP_NAME}
        freertos_bsp
        openamp
        libmetal
        m
    )
    set_target_properties(${APP_NAME} PROPERTIES
        LINK_OPTIONS ""
        LINK_FLAGS "-lnosys --specs=nosys.specs -L${BSP_DIR}/common/linker/ -T ${TEMP_LDFILE} -Wl,--no-warn-rwx-segments"
    )
    target_link_options(${APP_NAME}
        PRIVATE
        -T${BSP_DIR}/common/linker/lscript_rsc_table_vram${VRAM_REGION}.ld
    )
    target_link_options(${APP_NAME} BEFORE
        PRIVATE
        -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/${APP_NAME}.map
    )
    convert_bin_to_srec(${APP_NAME})
endmacro()

if(NOT DEFINED MFIS_CHAN)
    set(CORES 0 1)
    foreach(CORE IN LISTS CORES)
        if(CORE EQUAL 0)
            set(VRAM_REGION 1)
            set(MFIS_CHAN 0)
        elseif(CORE EQUAL 1)
            set(VRAM_REGION 2)
            set(MFIS_CHAN 1)
        else()
            message(FATAL_ERROR "INVALID CORE: ${CORE}")
        endif()
        if(NOT MFIS_CHAN EQUAL 0 AND NOT MFIS_CHAN EQUAL 1 AND NOT MFIS_CHAN EQUAL 2)
            message(FATAL_ERROR "MFIS CHANNEL ${MFIS_CHAN}: RPMSG NOT supported! Re-check with Linux team!")
        endif()
        #message(STATUS "MFIS channel: ${MFIS_CHAN}")
        #message(STATUS "VRAM_REGION: ${VRAM_REGION}")

        add_rpmsg_app(
            "rpmsg_mfis${MFIS_CHAN}_cluster0_core${CORE}"
            "rpmsg-echo.c"
            ${MFIS_CHAN}
            ${VRAM_REGION}
        )
        add_rpmsg_app(
            "rpmsg_perf_mfis${MFIS_CHAN}_cluster0_core${CORE}"
            "rpmsg-perf.c"
            ${MFIS_CHAN}
            ${VRAM_REGION}
        )
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

        add_rpmsg_app(
            "rpmsg_mfis${MFIS_CHAN}_cluster0_core${CORE}"
            "rpmsg-echo.c"
            ${MFIS_CHAN}
            ${RAM_REGION}
        )
        add_rpmsg_app(
            "rpmsg_perf_mfis${MFIS_CHAN}_cluster0_core${CORE}"
            "rpmsg-perf.c"
            ${MFIS_CHAN}
            ${RAM_REGION}
        )
endif()

set(app_name "rpmsg_echo_aiacc0_0")

add_executable(${app_name}
    rpmsg_echo_aiacc.c
    platform_rcar.c
    remoteproc_rcar.c
    rsc_table.c
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
    -T${BSP_DIR}/common/linker/lscript_rsc_table_aiacc0_0.ld
)

target_link_options(${app_name} BEFORE
    PRIVATE
    -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/${app_name}.map
)

convert_bin_to_srec(${app_name})

set(app_name "rpmsg_echo_aiacc0_1")

add_executable(${app_name}
    rpmsg_echo_aiacc.c
    platform_rcar.c
    remoteproc_rcar.c
    rsc_table.c
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
    -T${BSP_DIR}/common/linker/lscript_rsc_table_aiacc0_1.ld
)

target_link_options(${app_name} BEFORE
    PRIVATE
    -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/${app_name}.map
)

convert_bin_to_srec(${app_name})

set(app_name "rpmsg_echo_aiacc1_0")

add_executable(${app_name}
    rpmsg_echo_aiacc.c
    platform_rcar.c
    remoteproc_rcar.c
    rsc_table.c
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
    -T${BSP_DIR}/common/linker/lscript_rsc_table_aiacc1_0.ld
)

target_link_options(${app_name} BEFORE
    PRIVATE
    -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/${app_name}.map
)

convert_bin_to_srec(${app_name})

set(app_name "rpmsg_echo_aiacc1_1")

add_executable(${app_name}
    rpmsg_echo_aiacc.c
    platform_rcar.c
    remoteproc_rcar.c
    rsc_table.c
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
    -T${BSP_DIR}/common/linker/lscript_rsc_table_aiacc1_1.ld
)

target_link_options(${app_name} BEFORE
    PRIVATE
    -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/${app_name}.map
)

convert_bin_to_srec(${app_name})

set(app_name "rpmsg_multi_task")

add_executable(${app_name}
    rpmsg_multi_task.c
    platform_rcar.c
    remoteproc_rcar.c
    rsc_table.c
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
    -T${BSP_DIR}/common/linker/lscript_rsc_table_vram1.ld
)

target_link_options(${app_name} BEFORE
    PRIVATE
    -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/${app_name}.map
)

convert_bin_to_srec(${app_name})

set(app_name "rpmsg_ucie_cr_x5h_rc0")

add_executable(${app_name}
    rpmsg_ucie_cr_rc.c
    platform_rcar.c
    remoteproc_rcar.c
    rsc_table.c
)

target_compile_definitions(${app_name}
    PUBLIC UCIE_CH=0
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
    -T${BSP_DIR}/common/linker/lscript_rsc_table_ucie_0_0.ld
)

target_link_options(${app_name} BEFORE
    PRIVATE
    -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/${app_name}.map
)

convert_bin_to_srec(${app_name})

set(app_name "rpmsg_ucie_cr_aiacc0_ep0")

add_executable(${app_name}
    rpmsg_ucie_cr_ep.c
    platform_rcar.c
    remoteproc_rcar.c
    rsc_table.c
)

target_compile_definitions(${app_name}
    PUBLIC UCIE_CH=0
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
    -T${BSP_DIR}/common/linker/lscript_rsc_table_ucie_0_0.ld
)

target_link_options(${app_name} BEFORE
    PRIVATE
    -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/${app_name}.map
)

convert_bin_to_srec(${app_name})
if(CORE EQUAL 0)
set(app_name "rpmsg_ucie_x5h_ca_aiacc_core_0")

add_executable(${app_name}
    rpmsg_ucie_cr_ep.c
    platform_rcar.c
    remoteproc_rcar.c
    rsc_table.c
)

target_compile_definitions(${app_name}
    PUBLIC UCIE_CH=0
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
    -T${BSP_DIR}/common/linker/lscript_rsc_table_ucie_ca_aiacc0.ld
)

target_link_options(${app_name} BEFORE
    PRIVATE
    -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/${app_name}.map
)

convert_bin_to_srec(${app_name})
endif()
set(app_name "rpmsg_ucie_cr_x5h_rc1")

add_executable(${app_name}
    rpmsg_ucie_cr_rc.c
    platform_rcar.c
    remoteproc_rcar.c
    rsc_table.c
)

target_compile_definitions(${app_name}
    PUBLIC UCIE_CH=1
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
    -T${BSP_DIR}/common/linker/lscript_rsc_table_ucie_1_0.ld
)

target_link_options(${app_name} BEFORE
    PRIVATE
    -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/${app_name}.map
)

convert_bin_to_srec(${app_name})
elseif(CORE EQUAL 1)
set(app_name "rpmsg_ucie_x5h_ca_aiacc_core_1")

set(TEMP_LDFILE ${BSP_DIR}/common/linker/lscript_dram.ld)
add_executable(${app_name}
    rpmsg_ucie_cr_ep.c
    platform_rcar.c
    remoteproc_rcar.c
    rsc_table.c
)

target_compile_definitions(${app_name}
    PUBLIC UCIE_CH=0
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
    -T${BSP_DIR}/common/linker/lscript_rsc_table_ucie_ca_aiacc1.ld
)

target_link_options(${app_name} BEFORE
    PRIVATE
    -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/${app_name}.map
)

convert_bin_to_srec(${app_name})
endif

set(app_name "rpmsg_ucie_cr_aiacc1_ep0")

add_executable(${app_name}
    rpmsg_ucie_cr_ep.c
    platform_rcar.c
    remoteproc_rcar.c
    rsc_table.c
)

target_compile_definitions(${app_name}
    PUBLIC UCIE_CH=0
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
    -T${BSP_DIR}/common/linker/lscript_rsc_table_ucie_1_0.ld
)

target_link_options(${app_name} BEFORE
    PRIVATE
    -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/${app_name}.map
)

convert_bin_to_srec(${app_name})

set(app_name "rpmsg_ucie_cr_aiacc0_rc1")

add_executable(${app_name}
    rpmsg_ucie_cr_rc.c
    platform_rcar.c
    remoteproc_rcar.c
    rsc_table.c
)

target_compile_definitions(${app_name}
    PUBLIC UCIE_CH=1
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
    -T${BSP_DIR}/common/linker/lscript_rsc_table_ucie_1_1.ld
)

target_link_options(${app_name} BEFORE
    PRIVATE
    -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/${app_name}.map
)

convert_bin_to_srec(${app_name})

set(app_name "rpmsg_ucie_cr_aiacc1_ep1")

add_executable(${app_name}
    rpmsg_ucie_cr_ep.c
    platform_rcar.c
    remoteproc_rcar.c
    rsc_table.c
)

target_compile_definitions(${app_name}
    PUBLIC UCIE_CH=1
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
    -T${BSP_DIR}/common/linker/lscript_rsc_table_ucie_1_1.ld
)

target_link_options(${app_name} BEFORE
    PRIVATE
    -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/${app_name}.map
)

convert_bin_to_srec(${app_name})
