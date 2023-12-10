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
static bool btn_pressed[BUTTONS] = { false };
static uint32_t bnt_pressed_time_us[BUTTONS] = { 0 };
static bool btn_repeat_allowed[BUTTONS] = { false };

static int gpio_to_btn(uint gpio) {
    switch (gpio) {
        case BTN_UP: return UP;
        case BTN_DOWN: return DOWN;
        case BTN_LEFT: return LEFT;
        case BTN_RIGHT: return RIGHT;
        case BTN_CENTER: return CENTER;
        default: assert(false);
    }
}

static uint btn_to_gpio(ButtonEnum btn) {
    switch (btn) {
        case UP: return BTN_UP;
        case DOWN: return BTN_DOWN;
        case LEFT: return BTN_LEFT;
        case RIGHT: return BTN_RIGHT;
        case CENTER: return BTN_CENTER;
        default: assert(false);
    }
}

void buttons_callback(uint gpio, uint32_t events) {
    int btn = gpio_to_btn(gpio);

    if (events & GPIO_IRQ_EDGE_FALL) {
        // pressed
        btn_pressed[btn] = true;
        bnt_pressed_time_us[btn] = time_us_32();
        xTaskNotifyGive(input_task_h);
    }
}

static inline uint btn_pressed_duration_ms(ButtonEnum btn) {
    if (!btn_pressed[btn])
        return 0;

    return (time_us_32() - bnt_pressed_time_us[btn]) / 1000;
}

static inline uint btn_is_pressed(ButtonEnum btn) {
    return gpio_get(btn_to_gpio(btn)) == false; // low level
}

#define BTN_WAIT (1000 / BTN_REPEAT_PER_SECOND)

[[noreturn]] void input_task(void* arg) {
    for (ButtonEnum btn : buttons) {
        uint gpio = btn_to_gpio(btn);
        gpio_set_dir(gpio, GPIO_IN);
        gpio_pull_up(gpio);
        gpio_set_input_hysteresis_enabled(gpio, true); // schmitt trigger

        gpio_set_irq_enabled(
                gpio,
                GPIO_IRQ_EDGE_FALL, // only press
                true);
    }

    puts("buttons: gpio init ok");

    // only up/down repeat allowed by default
    btn_repeat_allowed[UP] = true;
    btn_repeat_allowed[DOWN] = true;

    bool btn_press_handled[BUTTONS] = { false };
    uint interaction_time_us = time_us_32();

    while (true) {
        bool btn_pressed_any = false;
        bool interaction = false;
        for (ButtonEnum btn : buttons) {
            // check if button still pressed
            // this affects <btn_pressed_duration_ms>
            // which returns 0 if <btn_pressed> is false
            // can only be set to <true> in interrupt which also updates time
            btn_pressed[btn] &= btn_is_pressed(btn);

            if (btn_pressed[btn]) {
                // button is currently pressed
                btn_pressed_any = true;
            }
            else {
                // not currently pressed
                // reset press handled flag
                btn_press_handled[btn] = false;
            }

            if (btn_pressed_duration_ms(btn) > BTN_DEBOUNCE_MS && !btn_press_handled[btn]) {
                // debounce time elapsed & not yet handled
                btn_press_handled[btn] = true;
                interaction = true;

                if (screenmng_backlight_get()) {
                    // only handle input when backlight is on
                    screenmng_input(btn);
                }
            }

            if (btn_pressed_duration_ms(btn) > BTN_REPEAT_MS && btn_repeat_allowed[btn]) {
                // repeat time elapsed
                interaction = true;
                screenmng_input(btn);
            }
        }

        if (interaction) {
            interaction_time_us = time_us_32();

            // if there was no backlight during interaction, turn in on
            screenmng_backlight_set(true);
        }

        if ((time_us_32() - interaction_time_us) >  LCD_BL_TIMEOUT_MS*1000 && player_is_started()) {
            // if the last key was pressed more than LCD_BL_TIMEOUT_MS ms ago, disable the backlight
            // (but only if the player is playing, otherwise the user might forget to turn the radio off)
            screenmng_backlight_set(false);
        }

        if (btn_pressed_any) {
            // any button is held down
            // do regular time step
            vTaskDelay(pdMS_TO_TICKS(BTN_WAIT));
        }
        else if (screenmng_backlight_get()) {
            // no button pressed but backlight on
            // wait to turn it off, or for interrupt
            ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(LCD_BL_TIMEOUT_MS));
        }
        else {
            // no button, no backlight
            // wait for interrupt
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        }
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
    btn_repeat_allowed[LEFT] = enable;
    btn_repeat_allowed[RIGHT] = enable;
}

void buttons_repeat_center(bool enable) {
    btn_repeat_allowed[CENTER] = enable;
}
