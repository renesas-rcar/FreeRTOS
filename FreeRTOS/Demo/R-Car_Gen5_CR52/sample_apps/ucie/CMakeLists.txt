#
# Copyright (c) 2025 Renesas Electronics Corporation
#
# SPDX-License-Identifier: MIT
#

set(app_name "ucie_cr_x5h_rc0")

add_executable(${app_name}
    ucie_rc.c
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

set(app_name "ucie_cr_x5h_rc1")

add_executable(${app_name}
    ucie_rc.c
)

target_compile_definitions(${app_name}
    PUBLIC
    UCIE_CH=1
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

set(app_name "ucie_cr_aiacc0_ep0")

add_executable(${app_name}
    ucie_ep0.c
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

set(app_name "ucie_cr_aiacc1_ep0")

add_executable(${app_name}
    ucie_ep0.c
)

target_compile_definitions(${app_name}
    PUBLIC
    UCIE_PIO_MEM=0x24000000000ULL
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

set(app_name "ucie_cr_aiacc0_rc1")

add_executable(${app_name}
    ucie_rc.c
)

target_compile_definitions(${app_name}
    PUBLIC
    UCIE_CH=1
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

set(app_name "ucie_cr_aiacc1_ep1")

add_executable(${app_name}
    ucie_ep1.c
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

if(BOARD STREQUAL "mdp_aiacc_hil")
    set(app_name "ucie_mdp_aiacc_epf")

    add_executable(${app_name}
        ucie_mdp_aiacc_epf.c
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
