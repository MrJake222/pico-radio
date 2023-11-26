#pragma once

#include <screenlist.hpp>
#include <loaderfav.hpp>

class ScFavourites : public ScreenList {

    const char* get_title() override { return "Ulubione stacje"; }
    int action_icons() override { return 3; }

    int default_y() override;

    void draw_button(int x, int y, bool selected) override;

    int get_action(int x, int y) override;
    Screen* run_action(int action) override;

    LoaderFav& ll;
    Loader & get_ll() override { return ll; }

    friend void fav_update_cb(void* arg, const char* info);

public:
    ScFavourites(ST7735S& display_, SemaphoreHandle_t& mutex_ticker_,
                 LoaderFav& ll_)
    : ScreenList(display_, mutex_ticker_,
                 3, 25,
                 147, 20, 1, 2,
                 5, 2)
    , ll(ll_)
    { }

    void begin() override;
    void show() override;
};
