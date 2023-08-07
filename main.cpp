#include <cstdio>
#include <pico/stdlib.h>

#include <hardware/clocks.h>
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
#include <tft/st7735s.hpp>

// wifi
#include <pico/cyw43_arch.h>

#include <player.hpp>
#include <mcorefifo.hpp>
#include <screen.hpp>
#include <screenmng.hpp>
#include <static.hpp>
#include <lfs.hpp>
#include <analog.hpp>

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

    // Usage:
    // // File scan
    // char path[1024] = "/";
    // std::vector<std::string> files;
    // scan_files(path, files);
    //
    // // radio
    // files.emplace_back("http://stream.rcs.revma.com/an1ugyygzk8uv"); // Radio 357
    // files.emplace_back("http://rmfstream1.interia.pl:8000/rmf_fm");  // RMF FM
    // files.emplace_back("http://zt03.cdn.eurozet.pl/zet-tun.mp3");   // Radio Zet
    // files.emplace_back("http://stream.streambase.ch/radio32/mp3-192/direct");   // Radio 32 Switzerland
}

void init_lowlevel() {
    // set_sys_clock_khz(140000, true);
    set_sys_clock_khz(180000, true);

    // stdio on USB
    stdio_usb_init();
    // stdio on UART
    stdio_uart_init_full(STDIO_UART_ID ? uart1 : uart0, STDIO_BAUD,
                         STDIO_TX, STDIO_RX);

    printf("\n\nHello usb pico-radio!\n");
    printf("sys clock: %lu MHz\n", clock_get_hz(clk_sys) / 1000000);
    puts("");

    buttons_init();
    puts("buttons init ok");

    fifo_init();
    puts("mcorefifo init ok");

    analog::init();
    puts("analog init ok");
}

void init_hardware() {
    // LittleFS config
    pico_lfs_init();
    puts("littlefs: init ok");

    pico_lfs_mount_format();

    // Display config (needs to be after littlefs)
    // tries to read favourites
    // TODO make init screen show first
    screenmng_init();
    puts("Display configuration done");

    player_init();
    puts("player done");

    // FS configuration
    // TODO move mounting to a task (and different file)
#if SD_ENABLE
    FRESULT fr;

    sd_card_t* pSD = sd_get_by_num(0);

    fr = f_mount(&pSD->fatfs, pSD->pcName, 1);
    if (fr != FR_OK) {
        fs_err(fr, "sd: f_mount");
    }

    puts("sd: mount ok");
#endif
}

void task_hardware_startup(void* arg) {
    init_hardware();

    uint32_t min_free_stack = uxTaskGetStackHighWaterMark(nullptr);
    printf("hardware unused stack: %ld\n", min_free_stack);

    vTaskDelete(nullptr);
}

void init_wifi() {
    // WiFi configuration
    int err;
    err = cyw43_arch_init();
    if (err) {
        printf("wifi arch begin error code %d\n", err);
        while (1);
    }

    cyw43_arch_enable_sta_mode();
    // const char* WIFI_SSID = "Bapplejems";
    // const char* WIFI_PASSWORD = "ForThosE4bOut";
    // const char* WIFI_SSID = "NLP";
    // const char* WIFI_SSID = "NPC";
    // const char* WIFI_SSID = "MyNet";
    const char* WIFI_SSID = "BPi";
    // const char* WIFI_SSID = "NPC";
    const char* WIFI_PASSWORD = "bequick77";
    // const char* WIFI_SSID = "NorbertAP";
    // const char* WIFI_PASSWORD = "fearofthedark";

    printf("Connecting to Wi-Fi...\n");
    bool connected = false;

    for (int i=0; i<10; i++) {
        printf("try %d... ", i+1);

        int con_res = cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 5000);
        if (con_res) {
            printf("failed code %d\n", con_res);
        } else {
            printf("Connected.\n");
            connected = true;
            break;
        }
    }

    if (!connected) {
        printf("failed to connect to wifi");
        return;
    }

    cyw43_arch_lwip_begin();
    const ip4_addr_t* addr;
    do {
        addr = netif_ip4_addr(netif_list);
    } while (ip4_addr_isany_val(*addr));

    printf("got ip: %s\n", ip4addr_ntoa(addr));
    cyw43_arch_lwip_end();
}

void task_wifi_startup(void* arg) {
    init_wifi();

    // testing
    // player_start("http://stream.rcs.revma.com/an1ugyygzk8uv");
    // player_start("http://172.17.1.2:8080/audio.mp3");
    // player_start("https://0n-classicrock.radionetz.de/0n-classicrock.mp3", nullptr, nullptr);
    // player_start("http://www.radiorockfm.co.nz:8000/listenlive", nullptr, nullptr); // longest rtt 321 ms
    // player_start("https://stream.rockantenne.de/alternative/stream/mp3", nullptr, nullptr);
    // player_start("/Shrek l/12 Eddie Murphy - I´m A Believer.mp3");
    // sc_search_res.begin("rmf");

    uint32_t min_free_stack = uxTaskGetStackHighWaterMark(nullptr);
    printf("wifi unused stack: %ld\n", min_free_stack);

    vTaskDelete(nullptr);
}

[[noreturn]] void task_input_handle(void* arg) {
    ButtonEnum input;
    int r;

    while (true) {
        r = xQueueReceive(
                input_queue,
                &input,
                portMAX_DELAY);

        if (r != pdTRUE)
            continue;

        uint32_t min_free_stack = uxTaskGetStackHighWaterMark(nullptr);
        printf("input unused stack: %ld\n", min_free_stack);

        screenmng_input(input);
    }
}

int main() {
    init_lowlevel();

    xTaskCreate(
            task_hardware_startup,
            "hw startup",
            STACK_HW_SETUP,
            nullptr,
            PRI_HW_SETUP,
            nullptr);

    xTaskCreate(
            task_wifi_startup,
            "wifi startup",
            STACK_WIFI_SETUP,
            nullptr,
            PRI_WIFI_SETUP,
            nullptr);

    xTaskCreate(
            task_input_handle,
            "input handle",
            STACK_INPUT,
            nullptr,
            PRI_INPUT,
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