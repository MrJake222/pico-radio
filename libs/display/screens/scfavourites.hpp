#pragma once

#include <screenlist.hpp>
#include <loaderm3u.hpp>

class ScFavourites : public ScreenList {

    const char* get_title() override { return "Ulubione stacje"; }
    int action_icons() override { return 3; }

    void draw_button(int x, int y, bool selected, bool was_selected) override;

    int get_action(int x, int y) override;
    Screen* run_action(int action) override;

    LoaderM3U& ll;
    Loader & get_ll() override { return ll; }

    friend void fav_update_cb(void* arg, const char* info);

public:
    ScFavourites(ST7735S& display_, SemaphoreHandle_t& mutex_ticker_,
                 LoaderM3U& ll_)
                 : ScreenList(display_, mutex_ticker_,
                              3, 25,
                              147, 20, 1, 2,
                              5, 2,
                              false, false)
                 , ll(ll_)
                 { }

    void show() override;
};
