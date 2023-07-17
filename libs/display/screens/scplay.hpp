#pragma once

#include <config.hpp>
#include <screen.hpp>
#include <cstring>
#include <decodebase.hpp>

class ScPlay : public Screen {

    const char* get_title() override { return "Radio"; }

    int size_x(int y) override;
    int size_y() override;

    void draw_button(int x, int y, bool selected) override;

    int get_action(int x, int y) override;
    Screen* run_action(int action) override;

    const char* radio_name;
    const char* radio_url;

    // metadata text index
    int meta_idx;

    friend void player_failed_callback(void* arg);
    friend void player_update_callback(void* arg, DecodeBase* dec);

public:
    using Screen::Screen;

    void begin(const char* name_, const char* url_);
    void show() override;
};
