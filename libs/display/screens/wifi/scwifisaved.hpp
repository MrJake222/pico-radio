#pragma once

#include <screenlist.hpp>
#include <loaderm3u.hpp>

class ScWifiSaved : public ScreenList {

    const char* get_title() override { return "Zapisane sieci"; }
    int action_icons() override { return 2; }

    void draw_button(int x, int y, bool selected) override;
    int get_action(int x, int y) override;
    Screen* run_action(int action) override;

    LoaderM3U& ll;
    Loader& get_ll() override { return ll; }

public:
    ScWifiSaved(ST7735S& display_, SemaphoreHandle_t& mutex_ticker_,
                LoaderM3U& ll_)
                : ScreenList(display_, mutex_ticker_,
                3, 25,
                147, 20, 1, 2,
                5, 2)
                , ll(ll_)
                { }

    void begin() override;

};
