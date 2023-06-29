#include <hardware/gpio.h>
#include <cstdio>
#include <buttons.hpp>
#include <config.hpp>

QueueHandle_t input_queue;

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

void buttons_init() {
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

    input_queue = xQueueCreate(5, 1);
}
