#
# Copyright (c) 2025 Renesas Electronics Corporation
#
# SPDX-License-Identifier: MIT
#

function(add_wwdt_app app_name main_c)
    add_executable(${app_name} ${main_c})

    target_compile_definitions(${app_name} PUBLIC)

    target_link_libraries(${app_name}
        freertos_bsp m c
    )

    target_link_options(${app_name}
        PUBLIC -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/${app_name}.map
    )

    convert_bin_to_srec(${app_name})
endfunction()

foreach(num RANGE 1 8)
    add_wwdt_app(wwdt_tc${num}_app main_tc${num}.c)
endforeach()
