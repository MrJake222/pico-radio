#include "buttons.hpp"

#include <cstdio>
#include <config.hpp>
#include <screenmng.hpp>

#include <hardware/gpio.h>

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

static QueueHandle_t input_queue;

static void gpio_callback(uint gpio, uint32_t events) {
    gpio_acknowledge_irq(gpio, events);
    char val;

    switch (gpio) {
        case BTN_UP:
            val = UP;
            break;

        case BTN_DOWN:
            val = DOWN;
            break;

        case BTN_LEFT:
            val = LEFT;
            break;

        case BTN_RIGHT:
            val = RIGHT;
            break;

        case BTN_CENTER:
            val = CENTER;
            break;

        default:
            return;
    }

    xQueueSendFromISR(input_queue, &val, nullptr);
}

[[noreturn]] void task_input_handle(void* arg) {
    const int all[] = {BTN_UP, BTN_DOWN, BTN_LEFT, BTN_RIGHT, BTN_CENTER};
    for (int gpio : all) {
        gpio_set_dir(gpio, GPIO_IN);
        gpio_pull_up(gpio);
        gpio_set_input_hysteresis_enabled(gpio, true); // schmitt trigger

        gpio_set_irq_enabled_with_callback(
                gpio,
                GPIO_IRQ_EDGE_FALL,
                true,
                gpio_callback);
    }

    puts("buttons: gpio init ok");

    ButtonEnum input;
    int r;
    bool backlight = true;

    while (true) {
        r = xQueueReceive(
                input_queue,
                &input,
                backlight
                    ? LCD_BL_TIMEOUT_MS / portTICK_PERIOD_MS
                    : portMAX_DELAY); // wait timeout if backlight on (to turn it off)
                                      // or indefinitely (for input) if off

        if (r == pdTRUE) {
            // received input

            if (backlight) {
                // process only if display on (user sees what he's doing)
                screenmng_input(input);

                // print stack stats
                uint32_t min_free_stack = uxTaskGetStackHighWaterMark(nullptr);
                printf("input unused stack: %ld\n", min_free_stack);
            }

            // turn on the display
            screenmng_backlight(true);
            backlight = true;
        }
        else {
            // timeout occurred -> turn off the display
            screenmng_backlight(false);
            backlight = false;
        }
    }
}

void buttons_init() {
    input_queue = xQueueCreate(5, 1);
    if (!input_queue) {
        puts("buttons: queue init failed");
    }

    xTaskCreate(
            task_input_handle,
            "input handle",
            STACK_INPUT,
            nullptr,
            PRI_INPUT,
            nullptr);
}
