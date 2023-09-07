#include "buttons.hpp"

#include <cstdio>
#include <config.hpp>
#include <screenmng.hpp>
#include <player.hpp>

#include <hardware/gpio.h>
#include <hardware/timer.h>

#include <FreeRTOS.h>
#include <task.h>

static TaskHandle_t input_task_h;
static bool b_pressed[BUTTONS] = { false };
static uint32_t b_pressed_time_us[BUTTONS] = { 0 };
static bool b_repeat_allowed[BUTTONS] = { false };

void buttons_callback(uint gpio, uint32_t events) {
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

    if (events & GPIO_IRQ_EDGE_FALL) {
        // pressed
        b_pressed[val] = true;
        b_pressed_time_us[val] = time_us_32();
    }
    else if (events & GPIO_IRQ_EDGE_RISE) {
        // released
        b_pressed[val] = false;
    }

    xTaskNotifyGive(input_task_h);
}

[[noreturn]] void input_task(void* arg) {
    const int all[] = {BTN_UP, BTN_DOWN, BTN_LEFT, BTN_RIGHT, BTN_CENTER};
    for (int gpio : all) {
        gpio_set_dir(gpio, GPIO_IN);
        gpio_pull_up(gpio);
        gpio_set_input_hysteresis_enabled(gpio, true); // schmitt trigger

        gpio_set_irq_enabled(
                gpio,
                GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE,
                true);
    }

    puts("buttons: gpio init ok");

    // only up/down repeat allowed
    b_repeat_allowed[UP] = true;
    b_repeat_allowed[DOWN] = true;

    bool backlight = true;
    bool b_pressed_local[BUTTONS] = { false };
    TickType_t timeout = portMAX_DELAY;

    while (true) {
        ulTaskNotifyTake(true, timeout);

        if (!backlight) {
            screenmng_backlight(true);
            backlight = true;

            // when user does not see the screen's content, don't perform any other actions
            continue;
        }

        // if any key is pressed or repeating
        bool pressed_any = false;
        bool repeating_any = false;
        int pressed_time_us_last = 0;

        for (int i=0; i<BUTTONS; i++) {
            if (b_pressed[i] && !b_pressed_local[i]) {
                // newly pressed
                screenmng_input((ButtonEnum) i);
            }

            b_pressed_local[i] = b_pressed[i];

            if (b_pressed[i]) {
                pressed_any = true;

                if (b_repeat_allowed[i] && ((time_us_32() - b_pressed_time_us[i]) >  BTN_REPEAT_START_TIMEOUT_MS*1000)) {
                    repeating_any = true;
                    screenmng_input((ButtonEnum) i);
                }
            }

            pressed_time_us_last = MAX(pressed_time_us_last, b_pressed_time_us[i]);
        }

        if ((time_us_32() - pressed_time_us_last) >  LCD_BL_TIMEOUT_MS*1000 && player_is_started()) {
            // if the last key was pressed more than LCD_BL_TIMEOUT_MS ms ago, disable the backlight
            // (but only if the player is playing, otherwise the user might forget to turn the radio off)
            screenmng_backlight(false);
            backlight = false;
        }

        // set timeout for next notification
        // if repeating some keys, run this task very fast
        // if waiting for repetition to begin (some keys pressed), run fast
        // if no repeating and no keys are held down, wait slowly for backlight timeout
        // if screen backlight is off, wait indefinitely
        timeout =
                repeating_any ? (1000 / BTN_REPEAT_PER_SECOND) / portTICK_PERIOD_MS
                : pressed_any ? BTN_REPEAT_START_TIMEOUT_MS / portTICK_PERIOD_MS
                : backlight   ? LCD_BL_TIMEOUT_MS / portTICK_PERIOD_MS
                : portMAX_DELAY;
    }
}

void buttons_init() {
    xTaskCreate(
            input_task,
            "input handle",
            STACK_INPUT,
            nullptr,
            PRI_INPUT,
            &input_task_h);
}

void buttons_repeat_left_right(bool enable) {
    b_repeat_allowed[LEFT] = enable;
    b_repeat_allowed[RIGHT] = enable;
}

void buttons_repeat_center(bool enable) {
    b_repeat_allowed[CENTER] = enable;
}
