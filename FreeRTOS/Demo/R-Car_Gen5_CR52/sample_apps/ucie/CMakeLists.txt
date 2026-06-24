#
# Copyright (c) 2025 Renesas Electronics Corporation
#
# SPDX-License-Identifier: MIT
#


set(rc_main "ucie_rc")

add_executable(${rc_main}
    ucie_rc.c
)

convert_bin_to_srec(${rc_main})

set(ep_main "ucie_ep")

add_executable(${ep_main}
    ucie_ep.c
)

convert_bin_to_srec(${ep_main})

target_compile_definitions(${rc_main}
    PUBLIC
)

target_compile_definitions(${ep_main}
    PUBLIC
)

# Essential libraries to link with
target_link_libraries(${rc_main}
    freertos_bsp m c
)

target_link_libraries(${ep_main}
    freertos_bsp m c
)

# Add output program address map file
target_link_options(${rc_main}
    PUBLIC
        -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/${rc_main}.map
)

target_link_options(${ep_main}
    PUBLIC
        -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/${ep_main}.map
)

if(BOARD STREQUAL "x5h_ironhide" OR BOARD STREQUAL "x5h_rfs2" OR BOARD STREQUAL "mdp_x5h_hil")
    set(app_name "ucie_mdp_x5h")
    
    add_executable(${app_name}
        ucie_mdp_x5h.c
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
endif()

if(BOARD STREQUAL "mdp_aiacc_rfs2")
    set(app_name "ucie_mdp_aiacc0")
    
    add_executable(${app_name}
        ucie_mdp_aiacc0.c
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

    set(app_name "ucie_mdp_aiacc1")
    
    add_executable(${app_name}
        ucie_mdp_aiacc1.c
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

    set(app_name "ucie_mdp_x5h_ca_aiaccl")
    
    add_executable(${app_name}
        ucie_mdp_x5h_ca_aiaccl.c
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
endif()
