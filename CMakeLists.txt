cmake_minimum_required(VERSION 3.13)


# pico sdk
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

# libraries
# add_subdirectory(../pagers-common pagers-common)
add_subdirectory(libs/sd_lib/FatFs_SPI)

# project setup
project(pico-radio C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
add_compile_options(-O2)

pico_sdk_init()

# this project private files files
add_executable(${PROJECT_NAME}
        main.cpp
        sd_hw_config.c
        i2s.pio.cpp
        libs/waveheader.cpp libs/mp3.cpp libs/mp3.hpp)

# pio
pico_generate_pio_header(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/i2s.pio)

pico_set_float_implementation(${PROJECT_NAME} pico)
#pico_set_binary_type(${PROJECT_NAME} no_flash)

pico_add_extra_outputs(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME}
        pico_stdlib
        hardware_dma
        hardware_pio
        FatFs_SPI
        # pagers-common   # common code
        )

# enable only usb stdio functions
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)
