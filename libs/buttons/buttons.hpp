#pragma once

#include <FreeRTOS.h>
#include <queue.h>

extern QueueHandle_t input_queue;

enum ButtonEnum {
    UP,
    DOWN,
    LEFT,
    RIGHT,
    CENTER
};

void buttons_init();