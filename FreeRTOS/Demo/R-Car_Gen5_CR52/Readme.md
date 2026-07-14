# R-Car X5H FreeRTOS BSP

---

## Initialize submodules (RENESAS internal only)

This project uses several submodules for external software components. You need to initialize the submodules first:

```bash
git submodule update --init --recursive
```

## Compiler

For the CR52 (arm-gnu) the following toolchain have been used: 
"arm-gnu-toolchain-13.2.rel1-x86_64-arm-none-eabi.tar.xz" downloaded from https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads

Extract tool chain to <your_tool_chain_path>.

## Build

```bash
export PATH=$PATH:<your_tool_chain_path>/bin/
cd FreeRTOS/Demo/R-Car_Gen5_CR52/
mkdir build && cd build
cmake -G "Unix Makefiles" \
  -DCMAKE_TOOLCHAIN_FILE=toolchain_arm_none_eabi.cmake \
  -DCMAKE_INSTALL_PREFIX=<path/to/install/dir> \
  -DBOARD=<TARGET_PLATFORM> \
  -DENABLE_OPENAMP=1 \
  -DUART_ID=1 \
  -DCACHE=1 \
  -DRAM_REGION=1 \
  ..
make
```

To install output binaries and headers to `path/to/install/dir`:

```
make install
```

CMake options for the project:

- `-DCMAKE_INSTALL_PREFIX=<path/to/install/dir>`: Location to output install directory
- `-DENABLE_OPENAMP=1`: Enable libmetal, OpenAMP build
- `-DBOARD`: Choose platform to build:
  - `x5h_vdk`: For X5H on VDK
  - `x5h_rfs2`: For X5H on RFS2
  - `x5h_ironhide`: For X5H Ironhide board
  - `ai_acc`: For AI Accelerator, (**requires `-DRAM_REGION=5`**)

## Output

### X5H BSP

`<install_dir>/lib/libfreertos_bsp.a` The archieve file contains FreeRTOS Kernel and R-Car BSP

`<install_dir>/objects/dummy/common/dummy.c.o` Link your application with this object to suspend some warning: `warning: <symbol> is not implemented and will always fail`

`<install_dir>/include` The public include header directory

### Libraries

`<install_dir>/usr/local/include/metal`, `<install_dir>/usr/local/lib/libmetal.a`: Build output of libmetal

`<install_dir>/usr/local/include/openamp`, `<install_dir>/usr/local/lib/libmopen_amp.a`: Build output of OpenAMP 

### Applications

Applications are used for testing and are not installed. The 

`build/<app_name>/<app_name>.elf` for sample apps

`build/<app_name>/<app_name>.map` for memory map


## Out-of-tree application

Out-of-tree application are the applications use FreeRTOS output build aftifacts to compile and link with. 

Application directory structure:

```
my_sample_application
├── CMakeLists.txt
├── source1.c
├── source2.c
└── header.h
```

Reference CMakeLists.txt content:

```cmake
cmake_minimum_required(VERSION 3.21)
project(my_sample_project)

find_package(freertos_bsp REQUIRED)

set(app_name "my_sample_application")

add_executable(${app_name}
    source1.c
	source2.c
)

# Edit these flags based on actual configuration
set(CMAKE_ASM_FLAGS "-mfpu=neon-fp-armv8 --specs=nosys.specs -mcpu=cortex-r52 -mtune=cortex-r52 -mfloat-abi=hard")
target_compile_options(${app_name} PRIVATE
    -ffreestanding -std=gnu99  -mfpu=neon-fp-armv8 -mcpu=cortex-r52 -mtune=cortex-r52 -mfloat-abi=softfp
    --specs=nosys.specs
)

# Essential libraries to link with
target_link_libraries(${app_name}
    freertos_bsp::freertos_bsp
    freertos_bsp::openamp
    freertos_bsp::libmetal
    m
    freertos_bsp::dummy
)

# Linker options based on actual configuration
target_link_options(${app_name} BEFORE
    PRIVATE
    -lnosys --specs=nosys.specs -Wl,--no-warn-rwx-segments
    -L${freertos_bsp_DIR}/linker/ -T ${freertos_bsp_DIR}/linker/lscript_vram.ld
)
```

Application build command:

```shell
export PATH=$PATH:<your_tool_chain_path>/bin/
cd my_sample_application
mkdir build && cd build
cmake -G "Unix Makefiles" \
    -DCMAKE_TOOLCHAIN_FILE=<path/to>/toolchain_arm_none_eabi.cmake \
    -DCMAKE_PREFIX_PATH=<path/to/freertos/install/dir> \
..
make
```
