#pragma once

#include <config.hpp>
#include <screen.hpp>
#include <screenlist.hpp>
#include <loadersearch.hpp>

class ScSearchRes : public ScreenList {

    const char* get_title() override { return "Wyniki"; }
    int action_icons() override { return 1; }

    void draw_button(int x, int y, bool selected, bool was_selected) override;

    int get_action(int x, int y) override;
    Screen* run_action(int action) override;

    const char* prompt;
    char subtitle[10 + MAX_PROMPT_LEN + 1]; // null

    LoaderSearch& ll;
    Loader & get_ll() override { return ll; }

    friend void search_update_cb(void* arg, int provider_idx, int server_idx, int max_servers);

public:
    ScSearchRes(ST7735S& display_, SemaphoreHandle_t& mutex_ticker_,
                LoaderSearch& ll_)
                : ScreenList(display_, mutex_ticker_,
                             3, 31,
                             147, 20, 1, 2,
                             5, 2,
                             true, true)
                , ll(ll_)
                { }

    void begin(const char* prompt_);
    void show() override;
};
