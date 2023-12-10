#pragma once

#include <pico/platform.h>

enum ButtonEnum {
    UP,
    DOWN,
    LEFT,
    RIGHT,
    CENTER
};

#define BUTTONS     5
const ButtonEnum buttons[BUTTONS] = { UP, DOWN, LEFT, RIGHT, CENTER };

// called from gpio_irq
void buttons_callback(uint gpio, uint32_t events);

void buttons_init();
void buttons_repeat_left_right(bool enable);
void buttons_repeat_center(bool enable);