add_subdirectory(external)

add_library(libs
        analog/analog.cpp
        buttons/buttons.cpp

        display/screens/scbattery.cpp
        display/screens/scfavourites.cpp
        display/screens/scplay.cpp
        display/screens/scsearch.cpp
        display/screens/scsearchres.cpp
        display/screens/sclocal.cpp

        display/assets/ubuntu_mono.cpp
        display/assets/font.cpp
        display/assets/icons.cpp

        display/radios/fav.cpp
        display/radios/list.cpp
        display/radios/listentry.cpp
        display/radios/listm3u.cpp
        display/radios/listpls.cpp
        display/radios/loader.cpp
        display/radios/loadersearch.cpp
        display/radios/loaderfav.cpp
        display/radios/loaderlocal.cpp

        display/tft/st7735s.cpp
        display/textscrolled.cpp
        display/screen.cpp
        display/screenlist.cpp
        display/screenmng.cpp

        ds/datasource.cpp

        httpc/httpclient.cpp
        httpc/httpclientpico.cpp

        lfs/lfs.cpp
        lfs/lfsaccess.cpp

        mcorefifo/mcorefifo.cpp

        player/decode/decodebase.cpp
        player/decode/decodefile.cpp
        player/decode/decodestream.cpp
        player/decode/format.cpp
        player/decode/formatmp3.cpp
        player/decode/formatwav.cpp
        player/i2s.pio.cpp
        player/player.cpp
        player/filetype.cpp
        player/icy.cpp
        player/id3.cpp
        player/amp.cpp

        sd/sd.cpp

        static.cpp
        circularbuffer.cpp
        util.cpp
        )

target_include_directories(libs PUBLIC
        analog
        buttons
        httpc
        lfs
        mcorefifo
        player/decode
        player
        sd
        radios
        display
        display/assets
        display/radios
        display/tft
        ds
        .
        )

target_link_libraries(libs
        libs_external

        pico_multicore
        hardware_timer
        hardware_gpio
        hardware_irq
        hardware_adc

        pico_cyw43_arch_lwip_sys_freertos
        )