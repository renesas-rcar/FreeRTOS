#
# Copyright (c) 2025 Renesas Electronics Corporation
#
# SPDX-License-Identifier: MIT
#

set(app_name "wcrc_app")

if(BOARD STREQUAL "mdp_aiacc_rfs2" OR BOARD STREQUAL "mdp_aiacc_hil")
    add_executable(${app_name}
        aiacc_main.c
    )
else()
    add_executable(${app_name}
        main.c
    )
endif()

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
