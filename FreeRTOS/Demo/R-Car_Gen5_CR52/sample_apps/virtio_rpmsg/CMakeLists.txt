#
# Copyright (c) 2025 Renesas Electronics Corporation
#
# SPDX-License-Identifier: MIT
#




# set(VRAM_REGION 1)
# set(MFIS_CHAN 0)

set(app_name "virtio_rpmsg")

# set(TEMP_LDFILE ${BSP_DIR}/common/linker/lscript_dram3.ld)

add_executable(${app_name}
    virtio_rpmsg.c
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
    -T${BSP_DIR}/common/linker/lscript_rsc_table_virtio.ld
)

target_link_options(${app_name} BEFORE
    PRIVATE
    -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/${app_name}.map
)

convert_bin_to_srec(${app_name})

