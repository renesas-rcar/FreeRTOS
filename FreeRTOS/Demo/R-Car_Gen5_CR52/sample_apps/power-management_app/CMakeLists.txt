#
# Copyright (c) 2025 Renesas Electronics Corporation
#
# SPDX-License-Identifier: MIT
#

set(app_name "power-management_app")

if(BOARD STREQUAL "x5h_ironhide" OR BOARD STREQUAL "mdp_x5h_hil" OR BOARD STREQUAL "x5h_rfs2")
    add_executable(${app_name}
        main_x5h.c
    )
elseif(BOARD STREQUAL "mdp_aiacc_hil" OR BOARD STREQUAL "mdp_aiacc_rfs2")
    add_executable(${app_name}
        main_aiacc.c
    )
endif()

target_compile_definitions(${app_name}
    PUBLIC
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

# Add output program address map file
target_link_options(${app_name}
    PUBLIC
        -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/${app_name}.map
)

convert_bin_to_srec(${app_name})


set(deepstop_test "deepstop_test")
add_executable(${deepstop_test}
    deepstop_test.c
)
target_compile_definitions(${deepstop_test}
    PUBLIC
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
# Add output program address map file
target_link_options(${deepstop_test}
    PUBLIC
        -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/${deepstop_test}.map
)
convert_bin_to_srec(${deepstop_test})
