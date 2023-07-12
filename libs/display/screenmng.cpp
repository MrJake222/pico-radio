#include "screenmng.hpp"

#include <config.hpp>
#include <st7735s.hpp>

#include <FreeRTOS.h>
#include <task.h>
#include <cstdio>

// protects screen from mutual usage
static SemaphoreHandle_t mutex_display;

static ST7735S display(
        160, 128,
        1, 2,
        LCD_SPI ? spi1 : spi0,
        LCD_SCK, LCD_TX, LCD_CS,
        LCD_RST, LCD_DC, LCD_BL,
        mutex_display);

static Screen* current_screen;

// protects ticker from running when we add/remove a text
// important when we want to remove a text and redraw something in its place
// (ticker can preempt the input task)
static SemaphoreHandle_t mutex_ticker;

static void create_mutex(SemaphoreHandle_t& mutex) {
    mutex = xSemaphoreCreateMutex();
    assert(mutex != nullptr);
    puts("mutex creation ok");
    xSemaphoreGive(mutex);
}

// static ScFavourites sc_favourites(display, sem_ticker);
ScSearch sc_search(display, mutex_ticker);
ScSearchRes sc_search_res(display, mutex_ticker);
ScPlay sc_play(display, mutex_ticker);

[[noreturn]] void screen_tick_task(void* arg) {
    TickType_t last_wake;
    last_wake = xTaskGetTickCount();

    while (true) {
        current_screen->tick();

        xTaskDelayUntil(&last_wake,
                        LCD_TICK_INTERVAL_MS / portTICK_PERIOD_MS);
    }
}

void screenmng_init() {
    display.init();
    current_screen = &sc_search;

    create_mutex(mutex_display);
    create_mutex(mutex_ticker);

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

void screenmng_input(ButtonEnum input) {
    Screen* screen_new = current_screen->input(input);
    if (screen_new) {
        current_screen = screen_new;
        current_screen->show();
    }
}
