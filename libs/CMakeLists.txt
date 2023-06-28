add_subdirectory(external)

add_library(libs
        buttons/buttons.cpp

        decode/decodebase.cpp
        decode/decodefile.cpp
        decode/decodestream.cpp
        decode/formatmp3.cpp
        decode/formatwav.cpp

        display/screen.cpp
        display/screenmng.cpp
        display/screens/scfavourites.cpp
        display/screens/scsearch.cpp
        display/screens/scsearchres.cpp

        httpc/httpclient.cpp
        httpc/httpclientpico.cpp

        mcorefifo/mcorefifo.cpp

        player/i2s.pio.cpp
        player/player.cpp
        player/filetype.cpp

        tft/st7735s.cpp
        tft/ubuntu_mono.cpp
        tft/icons.cpp

        buffers.cpp
        circularbuffer.cpp
        )

target_include_directories(libs PUBLIC
        buttons
        decode
        httpc
        mcorefifo
        player
        display
        tft
        .
        )

target_link_libraries(libs
        libs_external

        pico_multicore
        hardware_timer
        hardware_gpio
        hardware_irq

        pico_cyw43_arch_lwip_sys_freertos
        )

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
pico_sdk_init()
pico_generate_pio_header(libs ${CMAKE_CURRENT_SOURCE_DIR}/player/i2s.pio)