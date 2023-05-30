cmake_minimum_required(VERSION 3.13)

# pico w project
set(PICO_BOARD pico_w)

# pico sdk
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

# libraries
add_subdirectory(libs/sd_lib/FatFs_SPI)
add_subdirectory(libs/helixmp3)
add_subdirectory(libs/httpc)

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
        libs/httpc/httpclientpico.cpp
        libs/circularbuffer.cpp
        libs/decode/decodebase.cpp
        libs/decode/decodefile.cpp
        libs/decode/formatmp3.cpp
        libs/decode/decodestream.cpp
        libs/decode/formatwav.cpp libs/tft/st7735s.cpp libs/tft/st7735s.hpp libs/tft/st7735_init.h libs/tft/ubuntu_mono.cpp libs/tft/ubuntu_mono.hpp)

# pio
pico_generate_pio_header(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/i2s.pio)

pico_add_extra_outputs(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME}
        pico_stdlib
        hardware_dma
        hardware_pio
        pico_multicore
        pico_cyw43_arch_lwip_threadsafe_background
        FatFs_SPI
        helixmp3
        httpc
        )

include_directories(
        ${PROJECT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/libs
        ${PROJECT_SOURCE_DIR}/libs/decode
)

add_compile_definitions(PICO_MAX_SHARED_IRQ_HANDLERS=8u)

# enable only usb stdio functions
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 1)
