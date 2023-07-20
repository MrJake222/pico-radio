add_subdirectory(sd_lib/FatFs_SPI)
add_subdirectory(helixmp3)

# FreeRTOS
add_library(freertos_config INTERFACE)
target_include_directories(freertos_config SYSTEM
        INTERFACE
        ${CMAKE_SOURCE_DIR}/config)
set(FREERTOS_PORT GCC_RP2040)
add_subdirectory(FreeRTOS-Kernel)

add_subdirectory(littlefs)
target_compile_definitions(littlefs PUBLIC LFS_THREADSAFE)

# export library
add_library(libs_external)
target_link_libraries(libs_external
        freertos_kernel
        FatFs_SPI
        helixmp3_static
        littlefs
        )