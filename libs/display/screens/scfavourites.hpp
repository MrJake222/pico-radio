#pragma once

#include <screenlist.hpp>

class ScFavourites : public ScreenList {

    const char* get_title() override { return "Ulubione stacje"; }

    int rows_above() override;
    int rows_below() override;

    int default_y() override;
    int size_x(int y) override;

    void draw_button(int x, int y, bool selected) override;

    int get_action(int x, int y) override;
    Screen* run_action(int action) override;

public:
    ScFavourites(ST7735S& display_, SemaphoreHandle_t& mutex_ticker_,
            ListLoader& ll_)
    : ScreenList(display_, mutex_ticker_,
                 3, 25,
                 147, 20, 1, 2,
                 5, 2,
                 ll_)
    { }

    void begin() override;
    void show() override;
};
