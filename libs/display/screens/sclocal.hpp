#pragma once

#include <screenlist.hpp>
#include <loaderlocal.hpp>

class ScLocal : public ScreenList {

    const char* get_title() override;
    int action_icons() override { return 1; }

    void draw_button(int x, int y, bool selected) override;
    int get_action(int x, int y) override;
    Screen* run_action(int action) override;

    Path path;
    LoaderLocal& ll;
    Loader& get_ll() override { return ll; }

public:
    ScLocal(ST7735S& display_, SemaphoreHandle_t& mutex_ticker_,
            LoaderLocal& ll_)
            : ScreenList(display_, mutex_ticker_,
                         3, 25,
                         147, 20, 1, 2,
                         5, 2,
                         false, false)
                         , ll(ll_)
                         { }

    void begin(const char* path_);
    void show() override;
};
