#include "screenmng.hpp"

#include <config.hpp>
#include <st7735s.hpp>

#include <FreeRTOS.h>
#include <task.h>
#include <loadersearch.hpp>
#include <static.hpp>
#include <util.hpp>
#include <loaderfav.hpp>

// protects screen from mutual usage
static SemaphoreHandle_t mutex_display;

static ST7735S display(
        160, 128,
        1, 2,
        LCD_SPI_ID ? spi1 : spi0,
        LCD_SCK, LCD_TX, LCD_CS,
        LCD_RST, LCD_DC, LCD_BL,
        mutex_display);

static Screen* current_screen;

// protects ticker from running when we add/remove/modify anything a ticker can access
// important for ex. when we want to remove a text and redraw something in its place
// (ticker can preempt the input task)
static SemaphoreHandle_t mutex_ticker;

// list loaders
static LoaderSearch sl(
        get_entries(), MAX_ENTRIES,
        get_entries_pls(), MAX_ENTRIES_PLS,
        get_http_client());

static LoaderFav favl(
        get_entries(), MAX_ENTRIES,
        get_lfs());

static LoaderLocal locall(
        get_entries(), MAX_ENTRIES);

ScFavourites sc_fav(display, mutex_ticker, favl);
ScSearch sc_search(display, mutex_ticker);
ScSearchRes sc_search_res(display, mutex_ticker, sl);
ScPlay sc_play(display, mutex_ticker);
ScBattery sc_bat(display, mutex_ticker);
ScLocal sc_local(display, mutex_ticker, locall);
ScWifiPwd sc_wifi_pwd(display, mutex_ticker);

[[noreturn]] void screen_tick_task(void* arg) {
    TickType_t last_wake;
    last_wake = xTaskGetTickCount();

    while (true) {
        if (current_screen)
            current_screen->tick();

        xTaskDelayUntil(&last_wake,
                        LCD_TICK_INTERVAL_MS / portTICK_PERIOD_MS);
    }
}

void screenmng_init() {
    display.init();
    current_screen = &sc_fav; // first screen to open
    // current_screen = &sc_wifi_pwd; // test screen0

    create_mutex_give(mutex_display);
    create_mutex_give(mutex_ticker);

    // this takes time, but we want to show the user
    // readable screen as fast as possible
    current_screen->begin();
    current_screen->show();

    xTaskCreate(
            screen_tick_task,
            "scr tick",
            STACK_DISPLAY_TICKER,
            nullptr,
            PRI_DISPLAY_TICKER,
            nullptr);
}

void screenmng_open(Screen* new_screen) {
    current_screen->hide();

    // disable ticker on old screen
    current_screen = nullptr;

    new_screen->show();

    // enable ticker on new screen
    current_screen = new_screen;
}

void screenmng_input(ButtonEnum input) {
    Screen* new_screen = current_screen->input(input);
    if (new_screen) {
        screenmng_open(new_screen);
    }
}

void screenmng_backlight(bool on) {
    display.bl_set(on);
}

void screenmng_show_error(const char* err) {
    current_screen->show_error(err);
}
