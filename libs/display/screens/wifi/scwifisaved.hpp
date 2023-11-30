#pragma once

#include <screenlist.hpp>
#include <loaderwifisaved.hpp>

class ScWifiSaved : public ScreenList {

    const char* get_title() override { return "Zapisane sieci"; }
    int action_icons() override { return 2; }

    void draw_button(int x, int y, bool selected, bool was_selected) override;
    int get_action(int x, int y) override;
    Screen* run_action(int action) override;

    LoaderWifiSaved& ll;
    Loader& get_ll() override { return ll; }

public:
    ScWifiSaved(ST7735S& display_, SemaphoreHandle_t& mutex_ticker_,
                LoaderWifiSaved& ll_)
                : ScreenList(display_, mutex_ticker_,
                             3, 25,
                             147, 20, 1, 2,
                             5, 2, false, false)
                , ll(ll_)
                { }

    void show() override;

};
