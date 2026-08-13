#
# Copyright (c) 2025 Renesas Electronics Corporation
#
# SPDX-License-Identifier: MIT
#

cmake_minimum_required(VERSION 3.21.0)

project(rcar-freertos-bsp VERSION 1.0.0)

enable_language(C ASM)

file(REAL_PATH ${CMAKE_SOURCE_DIR}/../../.. FREERTOS_DIR)
set(BSP_DIR ${CMAKE_SOURCE_DIR})
set(KERNEL_DIR ${FREERTOS_DIR}/FreeRTOS/Source)
set(LOGGING_DIR ${FREERTOS_DIR}/FreeRTOS-Plus/Source/Utilities/logging)
set(LINKER_DIR ${BSP_DIR}/common/linker)

message(STATUS ${BSP_DIR})
message(STATUS ${KERNEL_DIR})
message(STATUS ${LOGGING_DIR})

if(NOT DEFINED BOARD)
    set(BOARD "x5h_ironhide")
endif()

set(ALLOWED_RAM_REGION "1;2;3;4;5;6")
if(NOT DEFINED RAM_REGION)
    set(RAM_REGION 1)
endif()
set(LDFILE ${BSP_DIR}/common/linker/lscript_sram.ld)
if(BOARD STREQUAL "x5h_vdk" OR BOARD STREQUAL "x5h_ironhide" OR BOARD STREQUAL "x5h_rfs2" OR BOARD STREQUAL "mdp_aiacc_rfs2" OR BOARD STREQUAL "mdp_aiacc_hil" OR BOARD STREQUAL "mdp_x5h_hil")
    if(BOARD STREQUAL "x5h_vdk")
        set(BOARD_ID 1)
        set(PLATFORM_DEVICE "x5h")
        set(RPMSG_BIN_NAME
            rpmsg_mfis0_cluster0_core0
            rpmsg_mfis1_cluster0_core1
        )
    elseif(BOARD STREQUAL "x5h_ironhide")
        set(BOARD_ID 3)
        set(PLATFORM_DEVICE "x5h")
        set(RPMSG_BIN_NAME
            rpmsg_mfis0_cluster0_core0
            rpmsg_mfis1_cluster0_core1
            rpmsg_perf_mfis0_cluster0_core0
            rpmsg_perf_mfis1_cluster0_core1
        )
    elseif(BOARD STREQUAL "x5h_rfs2")
        set(BOARD_ID 4)
        set(PLATFORM_DEVICE "x5h")
        set(RPMSG_BIN_NAME
            rpmsg_mfis0_cluster0_core0
            rpmsg_mfis1_cluster0_core1
            rpmsg_perf_mfis0_cluster0_core0
            rpmsg_perf_mfis1_cluster0_core1
        )
    elseif(BOARD STREQUAL "mdp_aiacc_rfs2")
        set(BOARD_ID 2)
        set(PLATFORM_DEVICE "aiacc")
        set(RPMSG_BIN_NAME
            rpmsg_mfis0_cluster0_core0
            rpmsg_mfis1_cluster0_core1
        )
    elseif(BOARD STREQUAL "mdp_aiacc_hil")
        set(BOARD_ID 5)
        set(PLATFORM_DEVICE "aiacc")
        set(RPMSG_BIN_NAME
            rpmsg_mfis0_cluster0_core0
            rpmsg_mfis1_cluster0_core1
        )
    elseif(BOARD STREQUAL "mdp_x5h_hil")
        set(BOARD_ID 6)
        set(PLATFORM_DEVICE "mdp_x5h")
        set(RPMSG_BIN_NAME
            rpmsg_mfis0_cluster0_core0
            rpmsg_mfis1_cluster0_core1
            rpmsg_perf_mfis0_cluster0_core0
            rpmsg_perf_mfis1_cluster0_core1
        )
    else()
        message(FATAL_ERROR "Invalid BOARD: ${BOARD}.")
    endif()
    list(FIND ALLOWED_RAM_REGION "${RAM_REGION}" INDEX)
    if(INDEX GREATER -1)
        message(STATUS "RAM REGION ${RAM_REGION} is using now.")
        if(RAM_REGION EQUAL 4)
            message(STATUS "DRAM REGION is using now.")
            set(LDFILE ${BSP_DIR}/common/linker/lscript_dram.ld)
        elseif(RAM_REGION EQUAL 5)
            message(STATUS "DRAM REGION is using now.")
            set(LDFILE ${BSP_DIR}/common/linker/lscript_dram2.ld)
        elseif(RAM_REGION EQUAL 6)
            message(STATUS "DRAM REGION is using now.")
            set(LDFILE ${BSP_DIR}/common/linker/lscript_dram3.ld)      
        else()
            set(LDFILE ${BSP_DIR}/common/linker/lscript_vram${RAM_REGION}.ld)
        endif()
    else()
        message(FATAL_ERROR "Insert RAM REGION ${RAM_REGION} is not allowed.")
    endif()
else()
    message(STATUS "DRAM REGION is using now.")
    set(LDFILE ${BSP_DIR}/common/linker/lscript_dram.ld)
    set(BOARD_ID 2)
endif()

# =======================================
# UART_ID configuration
# =======================================
if(DEFINED UART_ID)
    message(STATUS "UART_ID is defined as: ${UART_ID}")
    add_compile_definitions(UART_ID=${UART_ID})
else()
    message(STATUS "UART_ID not defined in build options. Using default from header.")
endif()

if (BOARD STREQUAL "x5h_ironhide" OR BOARD STREQUAL "x5h_rfs2" OR BOARD STREQUAL "mdp_x5h_hil")
    set(UCIE_VER "RCAR_UCIE_V100")
elseif(BOARD STREQUAL "mdp_aiacc_rfs2")
    set(UCIE_VER "RCAR_UCIE_V102")
else()
    set(UCIE_VER "NONE")
endif()



message(STATUS "board: ${BOARD}")
message(STATUS "BOARD_ID: ${BOARD_ID}")

### Set common build flags
set(CACHE 1 CACHE BOOL "Enable cache for CPU CR52")
message(STATUS "CACHE option: ${CACHE}")

if(BOARD STREQUAL "x5h_ironhide" OR BOARD STREQUAL "mdp_x5h_hil")
    set(TCM_ENABLE 1 CACHE BOOL "Enable TCM for CPU CR52")
else()
    set(TCM_ENABLE 0 CACHE BOOL "Disable TCM for CPU CR52")
endif()
message(STATUS "TCM option: ${TCM_ENABLE}")

set(CMAKE_ASM_FLAGS "--specs=nosys.specs -mfloat-abi=hard")
set(CMAKE_C_FLAGS "-std=c99")
add_compile_options(
    -ffreestanding -mfpu=neon-fp-armv8 -mcpu=cortex-r52 -mtune=cortex-r52 -mfloat-abi=softfp
    -Wno-unknown-pragmas -DSCMI_DEBUG -DBOARD=${BOARD_ID} -DRAM_REGION=${RAM_REGION} -g
    -DCACHE=${CACHE}
    -DTCM_ENABLE=${TCM_ENABLE}
    ${global_definition}
    -D${UCIE_VER}=1
    -ffile-prefix-map=${FREERTOS_DIR}/=
)

add_link_options(
    -lnosys --specs=nosys.specs
    -L${BSP_DIR}/common/linker/ -T ${LDFILE} -Wl,--no-warn-rwx-segments
)

#######################################################################################
### Set the common BSP and upstream projects source files
set(kernel_source
    ${KERNEL_DIR}/croutine.c
    ${KERNEL_DIR}/event_groups.c
    ${KERNEL_DIR}/list.c
    ${KERNEL_DIR}/queue.c
    ${KERNEL_DIR}/stream_buffer.c
    ${KERNEL_DIR}/tasks.c
    ${KERNEL_DIR}/timers.c
)

file(GLOB SCMI_SRC_LIST
    ${BSP_DIR}/drivers/firmware/scmi/src/*.c
    )

if (BOARD STREQUAL "x5h_ironhide" OR BOARD STREQUAL "mdp_x5h_hil")
    set(UCIE_SRC_LIST
        ${BSP_DIR}/drivers/ucie/uciedrv.c
    )
elseif (BOARD STREQUAL "x5h_rfs2" OR BOARD STREQUAL "mdp_aiacc_rfs2")
    set(UCIE_SRC_LIST 
        ${BSP_DIR}/drivers/ucie/ucie_rfs/uciedrv.c
    )
elseif (BOARD STREQUAL "x5h_vdk")
    file(GLOB UCIE_SRC_LIST
         ${BSP_DIR}/drivers/ucie/ucie_vdk/*.c
    )
endif()

if(BOARD STREQUAL "x5h_vdk" OR BOARD STREQUAL "x5h_rfs2" OR BOARD STREQUAL "mdp_aiacc_rfs2")
    set(STATE_MANAGER_SRC
        ${BSP_DIR}/drivers/state-manager/state_manager_dummy.c
    )
else()
    set(STATE_MANAGER_SRC
        ${BSP_DIR}/drivers/state-manager/state_manager.c
    )
endif()

if(BOARD STREQUAL "mdp_aiacc_hil")
    set(MFIS_SRC
        ${BSP_DIR}/drivers/mfis/mfis_v2.c
    )
else()
    set(MFIS_SRC
        ${BSP_DIR}/drivers/mfis/mfis.c
    )
endif()

if(BOARD STREQUAL "x5h_vdk" OR BOARD STREQUAL "x5h_rfs2" OR BOARD STREQUAL "mdp_aiacc_rfs2")
    file(GLOB AUDIO_SRC_LIST
        ${BSP_DIR}/drivers/audio/ak4619_module_dummy.c
        ${BSP_DIR}/drivers/audio/adg.c
        ${BSP_DIR}/drivers/audio/ssi.c
        ${BSP_DIR}/drivers/audio/rcar_audio_api.c
        ${BSP_DIR}/drivers/audio/rcar_audio.c
    )
else()
    file(GLOB AUDIO_SRC_LIST
        ${BSP_DIR}/drivers/audio/ak4619_module.c
        ${BSP_DIR}/drivers/audio/adg.c
        ${BSP_DIR}/drivers/audio/ssi.c
        ${BSP_DIR}/drivers/audio/rcar_audio_api.c
        ${BSP_DIR}/drivers/audio/rcar_audio.c
    )
endif()

set(bsp_source
    ${BSP_DIR}/common/boot.S
    ${BSP_DIR}/common/asm_vectors.S
    ${BSP_DIR}/common/FreeRTOS_tick_config.c
    ${BSP_DIR}/common/system_rcar_gen5.c
    ${BSP_DIR}/common/interrupts.c
    ${BSP_DIR}/common/heap_useNewlib.c
    ${BSP_DIR}/common/idle_task.c
    ${BSP_DIR}/CMSIS_5/irq_ctrl_gic.c
    ${BSP_DIR}/drivers/gic/gic.c
    ${BSP_DIR}/drivers/smmu/smmu.c
    ${BSP_DIR}/drivers/smmu/translation_table.c
    ${BSP_DIR}/drivers/serial/serial.c
    ${BSP_DIR}/drivers/serial/scif.c
    ${BSP_DIR}/drivers/mpu/mpu.c
    ${BSP_DIR}/drivers/tcm/tcm.c
    ${BSP_DIR}/drivers/gpio/r_gpio_api.c
    ${BSP_DIR}/drivers/gpio/r_gpio.c
    ${BSP_DIR}/drivers/pfc/r_pfc_api.c
    ${BSP_DIR}/drivers/dmac/dmac_ctrl.c
    ${BSP_DIR}/drivers/dmac/rdmac_ctrl.c
    ${BSP_DIR}/drivers/dmac/sysdmac_ctrl.c
    ${BSP_DIR}/drivers/dmac/dmac_reg.c
    ${BSP_DIR}/drivers/i2c/r_i2c.c
    ${BSP_DIR}/drivers/i2c/r_i2c_regs.c
    ${BSP_DIR}/drivers/watchdog/r_swdt.c
    ${BSP_DIR}/drivers/watchdog/r_wwdt.c
    ${BSP_DIR}/drivers/watchdog/r_wwdt_reg.c
    ${BSP_DIR}/common/ARM_CR52/port.c
    ${BSP_DIR}/common/rcar_utils.c

    ${MFIS_SRC}
    ${SCMI_SRC_LIST}
    ${STATE_MANAGER_SRC}
    ${BSP_DIR}/drivers/wcrc/r_wcrc.c
    ${BSP_DIR}/drivers/wcrc/r_crc_wrapper.c

    ${AUDIO_SRC_LIST}
    ${BSP_DIR}/drivers/taud/r_taud.c
    ${BSP_DIR}/drivers/taud/r_taud_reg.c
    ${BSP_DIR}/drivers/taud/r_taud_api.c
    ${BSP_DIR}/drivers/ecm/r_ecm.c
    ${UCIE_SRC_LIST}
)

if(BOARD STREQUAL "mdp_aiacc_hil")
    list(APPEND bsp_source
        ${BSP_DIR}/drivers/module_controller/module_controller.c
    )
endif()

if(ENABLE_OPENAMP)
    list(APPEND bsp_source
        ${BSP_DIR}/common/libmetal/metal/cpu.c
        ${BSP_DIR}/drivers/virtio/platform_rcar.c
        ${BSP_DIR}/drivers/virtio/remoteproc_rcar.c
        ${BSP_DIR}/drivers/virtio/rsc_table.c
        ${BSP_DIR}/drivers/virtio/r_virtio.c
        ${BSP_DIR}/drivers/virtio-iommu-backend/r_virtio_iommu_backend.c
        ${BSP_DIR}/drivers/virtio-iommu-frontend/r_virtio_iommu_frontend.c
    )
endif()

add_library(dummy OBJECT
    ${BSP_DIR}/common/dummy.c
)
link_libraries(dummy)

add_library(freertos_bsp STATIC
    ${kernel_source}
    ${bsp_source}
)

target_include_directories(freertos_bsp
    PUBLIC
        $<BUILD_INTERFACE:${KERNEL_DIR}/include>
        $<BUILD_INTERFACE:${BSP_DIR}/include>
        $<BUILD_INTERFACE:${BSP_DIR}/drivers/mpu>
        $<BUILD_INTERFACE:${BSP_DIR}/drivers/serial>
        $<BUILD_INTERFACE:${BSP_DIR}/drivers/timer>
        $<BUILD_INTERFACE:${BSP_DIR}/drivers/gic>
        $<BUILD_INTERFACE:${BSP_DIR}/drivers/mfis>
        $<BUILD_INTERFACE:${BSP_DIR}/drivers/pfc>
        $<BUILD_INTERFACE:${BSP_DIR}/drivers/gpio>
        $<BUILD_INTERFACE:${BSP_DIR}/drivers/audio>
        $<BUILD_INTERFACE:${BSP_DIR}/drivers/i2c>
        $<BUILD_INTERFACE:${BSP_DIR}/drivers/dmac>
        $<BUILD_INTERFACE:${BSP_DIR}/drivers/watchdog>
        $<BUILD_INTERFACE:${BSP_DIR}/drivers/ucie>
        $<BUILD_INTERFACE:${BSP_DIR}/drivers/iommu>
        $<BUILD_INTERFACE:${BSP_DIR}/drivers/firmware>
        $<BUILD_INTERFACE:${BSP_DIR}/drivers/smmu>
        $<BUILD_INTERFACE:${BSP_DIR}/drivers/wcrc>
        $<BUILD_INTERFACE:${BSP_DIR}/drivers/taud>
        $<BUILD_INTERFACE:${BSP_DIR}/drivers/ecm>
        $<BUILD_INTERFACE:${BSP_DIR}/drivers/tcm>
        # LOGGING
        $<BUILD_INTERFACE:${LOGGING_DIR}>
        # COMMON
        $<BUILD_INTERFACE:${BSP_DIR}/common/include>
        $<BUILD_INTERFACE:${BSP_DIR}/common/include/${PLATFORM_DEVICE}>
        
        $<INSTALL_INTERFACE:include/kernel/include>
        $<INSTALL_INTERFACE:include>
        
    PRIVATE 
        $<BUILD_INTERFACE:${BSP_DIR}>
        $<BUILD_INTERFACE:${BSP_DIR}/CMSIS_5>
        $<BUILD_INTERFACE:${BSP_DIR}/CMSIS_5/CMSIS/Core_R/Include>
)

if(BOARD STREQUAL "mdp_aiacc_hil")
    target_include_directories(freertos_bsp
        PUBLIC
            $<BUILD_INTERFACE:${BSP_DIR}/drivers/module_controller>
    )
endif()

if(ENABLE_OPENAMP)
    target_include_directories(freertos_bsp PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/libraries/openamp/open-amp/lib/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/libraries/libmetal/build/lib/include>
        $<BUILD_INTERFACE:${BSP_DIR}/drivers/virtio>
        $<BUILD_INTERFACE:${BSP_DIR}/drivers/virtio-iommu-backend>
    )
endif()

### Installation config
# Kernel
install(
    DIRECTORY ${KERNEL_DIR}/include
    DESTINATION include/kernel
    FILES_MATCHING PATTERN "*.h"
)
# BSP
install(
    TARGETS freertos_bsp dummy
    EXPORT freertos_bspTargets
    ARCHIVE DESTINATION lib
    OBJECTS DESTINATION .
)
install(
    EXPORT freertos_bspTargets
    NAMESPACE freertos_bsp::
    DESTINATION lib
)
install(
    FILES ${CMAKE_SOURCE_DIR}/freertos_bspConfig.cmake
    DESTINATION .
)
install(
    DIRECTORY ${BSP_DIR}/include
    DESTINATION .
)
# Specify device tree header depend on BOARD_ID
install(
    FILES ${BSP_DIR}/common/include/${PLATFORM_DEVICE}/device_tree.h
    DESTINATION include
)

# Specify interrupt id header depend on BOARD_ID
install(
    FILES ${BSP_DIR}/common/include/${PLATFORM_DEVICE}/interrupt_id.h
    DESTINATION include
)

# Specify pfc cfg header depend on BOARD_ID
install(
    FILES ${BSP_DIR}/common/include/${PLATFORM_DEVICE}/r_pfc_cfg.h
    DESTINATION include
)

# Specify pd/clk/reset id header depend on BOARD_ID
install(
    DIRECTORY ${BSP_DIR}/common/include/${PLATFORM_DEVICE}/state-manager/
    DESTINATION include/state-manager
    FILES_MATCHING
        PATTERN "*.h"
)

# Logging
install(
    FILES
        ${LOGGING_DIR}/logging.h
        ${LOGGING_DIR}/logging_levels.h
        ${LOGGING_DIR}/logging_stack.h
    DESTINATION include
)

# Linker scripts
install(
    DIRECTORY ${BSP_DIR}/common/linker
    DESTINATION .
    FILES_MATCHING PATTERN "*.ld"
)

# Install sample app bins
install(
    DIRECTORY ${CMAKE_BINARY_DIR}/sample_apps/
    DESTINATION bins
    FILES_MATCHING
        PATTERN "*.${BIN_TYPE}"
        PATTERN "*.map"
    PATTERN "CMakeFiles" EXCLUDE
    PATTERN "rpmsg_sample" EXCLUDE
)

# Install RPMsg bins
if(ENABLE_OPENAMP)
    foreach(bin IN LISTS RPMSG_BIN_NAME)
        install(
            DIRECTORY ${CMAKE_BINARY_DIR}/sample_apps/rpmsg_sample/
            DESTINATION bins/rpmsg_sample/
            FILES_MATCHING
                PATTERN "${bin}.elf"
            PATTERN "CMakeFiles" EXCLUDE
        )
        if(BOARD STREQUAL x5h_ironhide OR BOARD STREQUAL "mdp_aiacc_hil" OR BOARD STREQUAL "mdp_x5h_hil")
            install(
                DIRECTORY ${CMAKE_BINARY_DIR}/sample_apps/rpmsg_sample/
                DESTINATION bins/rpmsg_sample/
                FILES_MATCHING
                    PATTERN "${bin}.srec"
                PATTERN "CMakeFiles" EXCLUDE
            )
        endif()
    endforeach()
endif()

################ Function to convert output file to .srec #############################

function(convert_bin_to_srec app_name)
    if(NOT CMAKE_EXECUTABLE_SUFFIX)
        set(CMAKE_EXECUTABLE_SUFFIX "")
    endif()

    set(BIN_FILE "${CMAKE_CURRENT_BINARY_DIR}/${app_name}${CMAKE_EXECUTABLE_SUFFIX}")
    set(SREC_FILE "${CMAKE_CURRENT_BINARY_DIR}/${app_name}.srec")

    add_custom_command(
        OUTPUT ${SREC_FILE}
        COMMAND ${CMAKE_OBJCOPY} -O srec --srec-forceS3
                --remove-section .resource_table
                --remove-section .resource_table1
                --remove-section .resource_table2
                ${BIN_FILE} ${SREC_FILE}
        DEPENDS ${app_name}
        COMMENT "Converting ${BIN_FILE} to SREC format"
        VERBATIM
    )

    add_custom_target(${app_name}_srec ALL
        DEPENDS ${SREC_FILE}
    )
endfunction()

function(add_subdirectory_from_list list_app)
    foreach(app IN LISTS ${list_app})
        add_subdirectory(sample_apps/${app})
    endforeach()
endfunction()

#######################################################################################
# Add subdirectory for applications below
if(ENABLE_OPENAMP)
    add_subdirectory(libraries/libmetal)
    add_subdirectory(libraries/openamp)
    add_subdirectory(sample_apps/rpmsg_sample)
    add_subdirectory(sample_apps/virtio_iommu_frontend_sample)
    add_subdirectory(sample_apps/virtio_iommu_backend_sample)
    add_subdirectory(sample_apps/virtio_rpmsg)
    add_custom_target(openamp_full)
    add_dependencies(openamp_full libmetal_build openamp_build)
    add_dependencies(freertos_bsp openamp_full)
endif()

if(ENABLE_FULL_DEMO)
    add_subdirectory(Full_Demo)
endif()

if(BOARD STREQUAL "x5h_vdk")
    set(SAMPLE_APPS
        hello_world
        uart_app
        logging_app
        mpu_app
        gpio_app
        ak4619_app
        cpp_app
        i2c_app
        sys_dmac_app
        rt_dmac_app
        swdt_app
        wwdt_app
        smmu_app
        power-management_app
        wcrc_app
        utils_app
        taud_app
        ecm_app
        sensor_app
        dummy_app
        cxl_app
        audio_app
        gic_app
        generic_timer_app
    )
elseif(BOARD STREQUAL "x5h_ironhide" OR BOARD STREQUAL "mdp_x5h_hil")
    set(SAMPLE_APPS
        hello_world
        uart_app
        logging_app
        mpu_app
        gpio_app
        ak4619_app
        cpp_app
        i2c_app
        sys_dmac_app
        rt_dmac_app
        swdt_app
        wwdt_app
        smmu_app
        power-management_app
        wcrc_app
        utils_app
        taud_app
        ecm_app
        sensor_app
        ucie
        audio_app
        gic_app
        generic_timer_app
    )
elseif(BOARD STREQUAL "x5h_rfs2")
    set(SAMPLE_APPS
        hello_world
        uart_app
        logging_app
        mpu_app
        gpio_app
        cpp_app
        i2c_app
        sys_dmac_app
        rt_dmac_app
        swdt_app
        wwdt_app
        smmu_app
        wcrc_app
        utils_app
        taud_app
        ecm_app
        dummy_app
        ucie
        audio_app
        gic_app
        generic_timer_app
    )
elseif(BOARD STREQUAL "mdp_aiacc_rfs2")
    set(SAMPLE_APPS
        hello_world
        uart_app
        logging_app
        mpu_app
        gpio_app
        cpp_app
        i2c_app
        rt_dmac_app
        swdt_app
        wwdt_app
        smmu_app
        wcrc_app
        utils_app
        ecm_app
        ucie
        dummy_app
        gic_app
        generic_timer_app
    )
elseif(BOARD STREQUAL "mdp_aiacc_hil")
    set(SAMPLE_APPS
        hello_world
        uart_app
        logging_app
        mpu_app
        gpio_app
        cpp_app
        i2c_app
        rt_dmac_app
        swdt_app
        wwdt_app
        smmu_app
        power-management_app
        wcrc_app
        utils_app
        ecm_app
        sensor_app
        gic_app
        generic_timer_app
    )
else()
    message(FATAL_ERROR "Invalid BOARD: ${BOARD}. Cannot add apps!")
endif()

add_subdirectory_from_list(SAMPLE_APPS)

add_subdirectory(Blinky)

# TODO: Delete application makefile
# TODO: Remove porting code in Kernel
# TODO: Remove rcar code in FreeRTOS_POSIX repo
