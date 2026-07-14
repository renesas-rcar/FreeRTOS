#
# Copyright (c) 2025 Renesas Electronics Corporation
#
# SPDX-License-Identifier: MIT
#

set(app_name "main_full")

add_executable(${app_name}
    main.c
    main_full.c
    ${FREERTOS_DIR}/FreeRTOS/Demo/Common/Minimal/EventGroupsDemo.c 
    ${FREERTOS_DIR}/FreeRTOS/Demo/Common/Minimal/blocktim.c 
    ${FREERTOS_DIR}/FreeRTOS/Demo/Common/Minimal/recmutex.c 
    ${FREERTOS_DIR}/FreeRTOS/Demo/Common/Minimal/semtest.c 
    ${FREERTOS_DIR}/FreeRTOS/Demo/Common/Minimal/dynamic.c 
    ${FREERTOS_DIR}/FreeRTOS/Demo/Common/Minimal/countsem.c 
    ${FREERTOS_DIR}/FreeRTOS/Demo/Common/Minimal/GenQTest.c 
    ${FREERTOS_DIR}/FreeRTOS/Demo/Common/Minimal/flop.c 
    ${FREERTOS_DIR}/FreeRTOS/Demo/Common/Minimal/IntSemTest.c 
    ${FREERTOS_DIR}/FreeRTOS/Demo/Common/Minimal/TaskNotify.c 
    ${FREERTOS_DIR}/FreeRTOS/Demo/Common/Minimal/StaticAllocation.c 
    ${FREERTOS_DIR}/FreeRTOS/Demo/Common/Minimal/AbortDelay.c 
    ${FREERTOS_DIR}/FreeRTOS/Demo/Common/Minimal/QueueOverwrite.c 
    ${FREERTOS_DIR}/FreeRTOS/Demo/Common/Minimal/TimerDemo.c 
    ${FREERTOS_DIR}/FreeRTOS/Demo/Common/Minimal/QPeek.c 
    ${FREERTOS_DIR}/FreeRTOS/Demo/Common/Minimal/QueueSet.c 
    ${FREERTOS_DIR}/FreeRTOS/Demo/Common/Minimal/StreamBufferDemo.c 
    ${FREERTOS_DIR}/FreeRTOS/Demo/Common/Minimal/death.c 
    ${FREERTOS_DIR}/FreeRTOS/Demo/Common/Minimal/MessageBufferDemo.c 
)

target_include_directories(${app_name}
    PRIVATE
        ${FREERTOS_DIR}/FreeRTOS/Demo/Common/include
        ${FREERTOS_DIR}/FreeRTOS/Demo/Common/include
)

target_compile_definitions(${app_name}
    PRIVATE
        configUSE_TICK_HOOK=1 
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
