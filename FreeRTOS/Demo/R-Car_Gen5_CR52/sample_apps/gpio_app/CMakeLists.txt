#
# Copyright (c) 2025 Renesas Electronics Corporation
#
# SPDX-License-Identifier: MIT
#

set(app_name "gpio_app")

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

if(BOARD STREQUAL "mdp_x5h_hil")
    # For AIACC0
    set(app_name "mdp_x5h_test_aiacc_pins")
    
    add_executable(${app_name}
       mdp_x5h_test_aiacc_pins.c 
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
