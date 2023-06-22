add_subdirectory(external)

add_library(libs
        buttons/buttons.cpp
        decode/decodebase.cpp
        decode/decodefile.cpp
        decode/decodestream.cpp
        decode/formatmp3.cpp
        decode/formatwav.cpp
        httpc/httpclient.cpp
        httpc/httpclientpico.cpp
        tft/st7735s.cpp
        tft/ubuntu_mono.cpp
        circularbuffer.cpp
        )

target_include_directories(libs PUBLIC
        buttons
        decode
        httpc
        player
        tft
        .
        )

target_link_libraries(libs
        libs_external
        hardware_timer
        hardware_gpio
        )