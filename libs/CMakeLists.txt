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
        display/screens/scsettings.cpp

        display/screens/wifi/scwifipwd.cpp
        display/screens/wifi/scwifisaved.cpp
        display/screens/wifi/scwifiscan.cpp
        display/screens/wifi/scwificonn.cpp

        display/screenvirt/screenlist.cpp
        display/screenvirt/screenkb.cpp

        display/assets/ubuntu_mono.cpp
        display/assets/font.cpp
        display/assets/icons.cpp

        display/tft/st7735s.cpp
        display/textscrolled.cpp
        display/screen.cpp
        display/screenmng.cpp

        list/m3u.cpp
        list/list.cpp
        list/listentry.cpp
        list/listm3u.cpp
        list/listpls.cpp
        list/loader.cpp
        list/loadersearch.cpp
        list/loaderm3u.cpp
        list/loaderlocal.cpp
        list/loaderconst.cpp
        list/loaderwifiscan.cpp
        list/path.cpp

        ds/datasource.cpp

        httpc/httpclient.cpp
        httpc/httpclientpico.cpp

        lfs/lfs.cpp
        lfs/lfsaccess.cpp
        lfs/lfsorter.cpp

        mcorefifo/mcorefifo.cpp

        player/decode/decodebase.cpp
        player/decode/decodefile.cpp
        player/decode/decodestream.cpp
        player/decode/format.cpp
        player/decode/formatmp3.cpp
        player/decode/formatwav.cpp
        player/metadata/icy.cpp
        player/metadata/id3.cpp
        player/i2s.pio.cpp
        player/player.cpp
        player/filetype.cpp
        player/amp.cpp

        sd/sd.cpp

        wifi/wifi.cpp

        static.cpp
        circularbuffer.cpp
        util.cpp
        settings.cpp
        )

target_include_directories(libs PUBLIC
        analog
        buttons
        httpc
        lfs
        mcorefifo
        player/decode
        player/metadata
        player
        sd
        wifi
        list
        display
        display/screens
        display/screens/wifi
        display/screenvirt
        display/assets
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