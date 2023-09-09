#include "gpio_irq.hpp"

#include <cstdio>
#include <hardware/gpio.h>
#include <config.hpp>

#include <sd.hpp>
#include <buttons.hpp>

namespace gpio_irq {

static void gpio_irq_cb(uint gpio, uint32_t events) {
    // auto-ack when using <gpio_set_irq_callback>

    switch (gpio) {
        case SD_CD:
            sd::sd_cd_callback(gpio, events);
            break;

        case BTN_UP:
        case BTN_DOWN:
        case BTN_LEFT:
        case BTN_RIGHT:
        case BTN_CENTER:
            buttons_callback(gpio, events);
            break;

        default:
            // printf("unknown gpio interrupt source gpio=%d events=0x%lx\n", gpio, events);
            break;
    }
}

void init() {
    gpio_set_irq_callback(gpio_irq_cb);
}

} // namespace
