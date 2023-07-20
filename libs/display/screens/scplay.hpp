#pragma once

#include <config.hpp>
#include <screen.hpp>
#include <cstring>
#include <decodebase.hpp>

class ScPlay : public Screen {

    const char* get_title() override { return "Radio"; }

    int size_x(int y) override;
    int size_y() override;

    void draw_star(bool selected);
    void draw_button(int x, int y, bool selected) override;

    int get_action(int x, int y) override;
    Screen* run_action(int action) override;

    const struct station* st;
    int fav_index;

    // metadata scrolled text index
    int meta_idx;

    friend void player_failed_callback(void* arg);
    friend void player_update_callback(void* arg, DecodeBase* dec);

public:
    using Screen::Screen;

    void begin(const struct station* st_, int fav_index_);
    void show() override;
};
