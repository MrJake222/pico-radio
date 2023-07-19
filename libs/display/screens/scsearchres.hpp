#pragma once

#include <config.hpp>
#include <screen.hpp>
#include <screenlist.hpp>

class ScSearchRes : public ScreenList {

    const char* get_title() override { return "Wyniki wyszukiwania"; }

    int rows_above() override;
    int rows_below() override;

    int size_x(int y) override;
    int size_y() override;

    void draw_button(int x, int y, bool selected) override;

    int get_action(int x, int y) override;
    Screen* run_action(int action) override;

    const char* prompt;
    char subtitle[10 + MAX_PROMPT_LEN + 1]; // null

public:
    ScSearchRes(ST7735S& display_, SemaphoreHandle_t& mutex_ticker_,
                ListLoader& ll_)
        : ScreenList(display_, mutex_ticker_,
                     3, 31,
                     147, 20, 1, 2,
                     5, 2,
                     ll_)
        { }

    void begin(const char* prompt_);
    void show() override;

    friend void all_loaded_cb(void* arg, int errored);
};
