#include <cstdio>

#include <pico/stdlib.h>
#include <hardware/clocks.h>

#include <player.hpp>
#include <mcorefifo.hpp>
#include <screenmng.hpp>
#include <analog.hpp>
#include <sd.hpp>
#include <gpio_irq.hpp>
#include <wifibest.hpp>

void init_lowlevel() {
    set_sys_clock_khz(150000, true);
    // set_sys_clock_khz(180000, true);

#if LIB_PICO_STDIO_USB
    // stdio on USB
    stdio_usb_init();
#endif

#if LIB_PICO_STDIO_UART
    // stdio on UART
    stdio_uart_init_full(STDIO_UART_ID ? uart1 : uart0, STDIO_BAUD,
                         STDIO_TX, STDIO_RX);
#endif

    printf("\n\nHello pico-radio!\n");
    printf("sys clock: %lu MHz\n", clock_get_hz(clk_sys) / 1000000);
    puts("");

    fifo_init();
    puts("mcorefifo init ok");

    analog::init();
    puts("analog init ok");
}

void task_init(void* arg) {
    // LittleFS config
    // (before screen because some try read favourites)
    pico_lfs_init();
    puts("littlefs: init ok");
    pico_lfs_mount_format();

    // Wi-Fi config
    // (before screen because some try to scan)
    int r;
    r = cyw43_arch_init();
    assert(r == 0);
    cyw43_arch_enable_sta_mode();
    puts("wifi init+sta done");
    wifi::connect_best_saved();

    // Display config
    // TODO make init screen show first
    screenmng_init();
    puts("Display configuration done");

    // before all irq-using modules
    // interrupts can fire after modules initialize their GPIO, we need callbacks set
    gpio_irq::init();
    puts("gpio irq init done");

    // all queues/task handlers need to be initialized before the modules enable interrupts

    buttons_init();
    puts("buttons init ok");

    sd::init();
    puts("sd init done");

    player_init();
    puts("player done");

    // print stack usage & end task
    uint32_t min_free_stack = uxTaskGetStackHighWaterMark(nullptr);
    printf("init task unused stack: %ld\n", min_free_stack);
    vTaskDelete(nullptr);
}

    // testing
    // player_start("http://stream.rcs.revma.com/an1ugyygzk8uv");
    // player_start("http://172.17.1.2:8080/audio.mp3");
    // player_start("https://0n-classicrock.radionetz.de/0n-classicrock.mp3", nullptr, nullptr);
    // player_start("http://www.radiorockfm.co.nz:8000/listenlive", nullptr, nullptr); // longest rtt 321 ms
    // player_start("https://stream.rockantenne.de/alternative/stream/mp3", nullptr, nullptr);
    // player_start("http://173.249.21.17:8108/;", nullptr, nullptr, nullptr);   // BluesMen Channel (320kbps)
    // player_start("/Shrek l/12 Eddie Murphy - I´m A Believer.mp3");
    // sc_search_res.begin("rmf");


int main() {
    init_lowlevel();

    xTaskCreate(
            task_init,
            "init",
            STACK_INIT,
            nullptr,
            PRI_INIT,
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