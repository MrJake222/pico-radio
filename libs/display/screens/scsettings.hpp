#pragma once

#include <screenlist.hpp>
#include <loaderconst.hpp>

class ScSettings : public ScreenList {

    const char* get_title() override { return "Ustawienia"; }
    int action_icons() override { return 1; }

    void draw_button(int x, int y, bool selected, bool was_selected) override;
    int get_action(int x, int y) override;
    Screen* run_action(int action) override;

    LoaderConst& ll;
    Loader& get_ll() override { return ll; }

public:
    ScSettings(ST7735S& display_, SemaphoreHandle_t& mutex_ticker_,
               LoaderConst& ll_)
               : ScreenList(display_, mutex_ticker_,
                            3, 25,
                            147, 20, 1, 2,
                            5, 2,
                            false, false, false)
               , ll(ll_)
               { }

    void show() override;

};
