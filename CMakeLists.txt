cmake_minimum_required(VERSION 3.13)

# pico w project
set(PICO_BOARD pico_w)

# pico sdk
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)


# project setup
project(pico-radio C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

add_compile_definitions(PICO_MAX_SHARED_IRQ_HANDLERS=8u)
add_compile_definitions(PICO_DEBUG_MALLOC=1)

# project-wide includes
include_directories(config)

# libraries
add_subdirectory(libs)

pico_sdk_init()

# this project private files files
add_executable(${PROJECT_NAME}
        main.cpp
        config/sd_hw_config.c
        )

pico_add_extra_outputs(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME}
        pico_stdlib
        hardware_dma
        hardware_pio
        pico_multicore
        pico_cyw43_arch_lwip_sys_freertos
        libs
        )

# enable only usb stdio functions
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 1)
