#include <cstdio>
#include <cstdlib>
#include <pico/stdlib.h>

#include <hardware/clocks.h>
#include <pico/multicore.h>
#include <cstring>
#include <vector>
#include <string>

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

#include <buttons/buttons.hpp>

#include <i2s.pio.h>

#include <f_util.h>
#include <ff.h>
#include <hw_config.h>

#include <config.hpp>
#include <formatmp3.hpp>
#include <formatwav.hpp>
#include <decodebase.hpp>
#include <decodefile.hpp>
#include <decodestream.hpp>
#include <tft/st7735s.hpp>

// wifi
#include <pico/cyw43_arch.h>
#include <player.hpp>

const uint PIN_DBG = 13;
#define DBG_ON() gpio_put(PIN_DBG, true)
#define DBG_OFF() gpio_put(PIN_DBG, false)

// SPI
ST7735S disp(
        160, 128,
        1, 2,
        LCD_SPI ? spi1 : spi0,
        LCD_SCK, LCD_TX, LCD_CS,
        LCD_RST, LCD_DC, LCD_BL);

void fs_err(FRESULT fr, const char* tag) {
    panic("%s: %s (id=%d)\n", tag, FRESULT_str(fr), fr);
}


FRESULT scan_files(char* path, std::vector<std::string>& files) {
    FRESULT res = FR_OK;

#if SD_ENABLE
    DIR dir;
    UINT i;
    static FILINFO fno;

    res = f_opendir(&dir, path);                       /* Open the directory */
    if (res == FR_OK) {
        for (;;) {
            res = f_readdir(&dir, &fno);               /* Read a directory item */
            if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
            i = strlen(path);
            sprintf(&path[i], "/%s", fno.fname);

            if (fno.fattrib & AM_DIR) {                    /* It is a directory */
                res = scan_files(path, files);             /* Enter the directory */
                if (res != FR_OK) break;
            } else {                                       /* It is a file. */
                // printf("found  %s\n", path);
                files.emplace_back(path);
            }

            path[i] = 0;
        }
        f_closedir(&dir);
    }
#endif

    return res;
}



void print_mem_usage() {
    // int size_pcm = sizeof(audio_pcm);
    // printf("PCM buffer size:     %d\n", size_pcm);
    // int size_raw = sizeof(raw_buf) + raw_buf.size + raw_buf.size_hidden;
    // printf("RAW buffer size:     %d\n", size_raw);
    //
    // int size_mp3 = sizeof(format_mp3);
    // printf("MP3 format size:     %d\n", size_mp3);
    // int size_wav = sizeof(format_wav);
    // printf("WAV format size:     %d\n", size_wav);
    //
    // int size_file = sizeof(dec_file);
    // printf("File decoder size:   %d\n", size_file);
    // int size_stream = sizeof(dec_stream);
    // printf("Stream decoder size: %d\n", size_stream);

    // int size_disp = disp.size();
    // printf("Display driver size: %d\n", size_disp);
    //
    // printf("Total: %d\n",
    //        size_pcm + size_raw +\
    //        size_mp3 + size_wav +\
    //        size_file + size_stream +\
    //        size_disp);
}

void init_hardware() {
    // set_sys_clock_khz(140000, true);
    set_sys_clock_khz(180000, true);

    // UART on USB
    // stdio_usb_init();
    // UART on 0/1 and USB
    stdio_init_all();

    // sleep_ms(2000);
    printf("\n\nHello usb pico-radio!\n");
    printf("sys clock: %lu MHz\n", clock_get_hz(clk_sys)/1000000);
    print_mem_usage();
    puts("");

    // IO
    gpio_init(PIN_DBG);
    gpio_set_dir(PIN_DBG, GPIO_OUT);
    DBG_OFF();

    player_begin();
    puts("player done");

    // Display config
    disp.begin();
    disp.set_bg_fg(0xCCCCCC, 0x0);
    disp.clear_screen();
    disp.write_text(0, 0, "Ulubione stacje", 1);

    const char* tests[] = {
            "RMF FM",
            "Złote przeboje",
            "Radio 357",
            "Radio 2223",
            "Radio Eska",
    };

    for (int i=0; i<5; i++) {
        if (i == 2)
            disp.set_bg(0x55AA55);
        else
            disp.set_bg(0xAAAAAA);

        disp.fill_rect(5, 23 + 23*i, 150, 20, true);
        disp.write_text(9, 25 + 23*i, tests[i], 1);
    }

    // disp.write_text(0, 16, "Wyniki wyszukiwania", 1);
    // disp.write_text(10, 50, "MORŚWIN!", 2);
    puts("Display configuration & init done");

    // FS configuration
#if SD_ENABLE
    FRESULT fr;

    sd_card_t* pSD = sd_get_by_num(0);

    fr = f_mount(&pSD->fatfs, pSD->pcName, 1);
    if (fr != FR_OK) {
        fs_err(fr, "f_mount");
    }

    puts("mount ok");
#endif

    buttons_begin();
    puts("buttons init ok");
}

void init_wifi() {
    // WiFi configuration
    int err;
    err = cyw43_arch_init();
    if (err) {
        printf("wifi arch init error code %d\n", err);
        while (1);
    }

    cyw43_arch_enable_sta_mode();
    const char* WIFI_SSID = "Bapplejems";
    const char* WIFI_PASSWORD = "ForThosE4bOut";
    // const char* WIFI_SSID = "NLP";
    // const char* WIFI_SSID = "NPC";
    // const char* WIFI_SSID = "MyNet";
    // const char* WIFI_SSID = "BPi";
    // const char* WIFI_PASSWORD = "bequick77";
    // const char* WIFI_SSID = "NorbertAP";
    // const char* WIFI_PASSWORD = "fearofthedark";

    printf("Connecting to Wi-Fi...\n");
    int con_res = cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000);
    if (con_res) {
        printf("connection failed code %d\n", con_res);
        while (1);
    } else {
        printf("Connected.\n");
    }

    cyw43_arch_lwip_begin();
    const ip4_addr_t* addr;
    do {
        addr = netif_ip4_addr(netif_list);
    } while (ip4_addr_isany_val(*addr));

    printf("got ip: %s\n", ip4addr_ntoa(addr));
    cyw43_arch_lwip_end();
}

void oldmain() {
    // File scan
    char path[1024] = "/";
    std::vector<std::string> files;
    scan_files(path, files);

    // radio
    files.emplace_back("http://stream.rcs.revma.com/an1ugyygzk8uv"); // Radio 357
    files.emplace_back("http://rmfstream1.interia.pl:8000/rmf_fm");  // RMF FM
    files.emplace_back("http://zt03.cdn.eurozet.pl/zet-tun.mp3");   // Radio Zet
    files.emplace_back("http://stream.streambase.ch/radio32/mp3-192/direct");   // Radio 32 Switzerland

    while (1) {
        for (uint i=0; i<files.size(); i++) {
            printf("[%02d] %s\n", i+1, files[i].c_str());
        }

        // const char* filepath;
        // FileType type;
        //
        // while (1) {
        //     uint choice;
        //     scanf("%d", &choice);
        //     if ((choice < 1) || (choice > files.size())) {
        //         puts("invalid number");
        //     }
        //     else {
        //         filepath = files[choice - 1].c_str();
        //         type = get_file_type(filepath);
        //
        //         if (type == FileType::UNSUPPORTED) {
        //             puts("unsupported format. Supported: wav, wave, mp3, http(s)");
        //         }
        //         else {
        //             // valid & supported
        //             break;
        //         }
        //     }
        // }

        // play(filepath, type);

        printf("\033[2J"); // clear screen
    }
}

// void core1() {
//     int cnt = 1000;
//
//     while (1) {
//         sleep_ms(1000);
//         multicore_fifo_push_blocking(cnt++);
//     }
// }
//
// void sio0() {
//     multicore_fifo_clear_irq();
//     unsigned int v = multicore_fifo_pop_blocking();
//     printf("received: %5d\n", v);
// }

[[noreturn]] void task_input_handle(void* arg) {
    ButtonEnum input;
    int r;
    int cnt = 0;

    while (true) {
        r = xQueueReceive(
                input_queue,
                &input,
                portMAX_DELAY);

        if (r != pdTRUE)
            continue;

        printf("input: %d (cnt %5d)\n", input, cnt++);
        if (input == CENTER) {
            player_start("/4mmc.wav");
        }
    }
}

int main() {
    init_hardware();
    // multicore_launch_core1(core1);
    // irq_set_exclusive_handler(SIO_IRQ_PROC0, sio0);
    // irq_set_enabled(SIO_IRQ_PROC0, true);

    xTaskCreate(
            task_input_handle,
            "input handle",
            configMINIMAL_STACK_SIZE,
            nullptr,
            1,
            nullptr);

    vTaskStartScheduler();
}

/*-----------------------------------------------------------*/
void vApplicationMallocFailedHook( void ) {
    panic("FreeRTOS: malloc failed");
}

void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName) {
    panic("FreeRTOS: stack overflow task %s", pcTaskName);
}