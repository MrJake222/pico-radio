#pragma once

#include <FreeRTOS.h>
#include <queue.h>

enum ButtonEnum {
    UP,
    DOWN,
    LEFT,
    RIGHT,
    CENTER
};

void buttons_init();