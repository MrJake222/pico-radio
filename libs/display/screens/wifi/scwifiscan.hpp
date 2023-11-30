#pragma once

#include <screenlist.hpp>
#include <loaderwifiscan.hpp>

class ScWifiScan : public ScreenList {

    const char* get_title() override { return "Znalezione sieci"; }
    int action_icons() override { return 1; }

    void draw_button(int x, int y, bool selected, bool was_selected) override;
    int get_action(int x, int y) override;
    Screen* run_action(int action) override;

    LoaderWifiScan& ll;
    Loader& get_ll() override { return ll; }

public:
    ScWifiScan(ST7735S& display_, SemaphoreHandle_t& mutex_ticker_,
               LoaderWifiScan& ll_)
               : ScreenList(display_, mutex_ticker_,
                            3, 25,
                            147, 20, 1, 2,
                            5, 2, true, true)
               , ll(ll_)
               { }

    void show() override;

};
