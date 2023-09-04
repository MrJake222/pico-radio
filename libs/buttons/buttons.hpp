#pragma once

enum ButtonEnum {
    UP,
    DOWN,
    LEFT,
    RIGHT,
    CENTER
};

#define BUTTONS     5

void buttons_init();
void buttons_repeat_left_right(bool enable);
void buttons_repeat_center(bool enable);