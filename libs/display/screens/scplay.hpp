#pragma once

#include <config.hpp>
#include <screen.hpp>
#include <cstring>
#include <decodebase.hpp>
#include <list.hpp>

class ScPlay : public Screen {

    const char* get_title() override { return "Radio"; }

    int size_x(int y) override;
    int size_y() override;

    void draw_star(bool selected);
    void draw_button(int x, int y, bool selected) override;

    int get_action(int x, int y) override;
    Screen* run_action(int action) override;

    ListEntry ent;
    int fav_index;
    Screen* prev;

    // metadata scrolled text index
    int meta_idx;

    // true if user pressed a button to request player stop
    // used by <player_finished_callback> to close the player screen when playback
    // stops by itself
    bool player_stop_requested;

    friend void player_finished_callback(void* arg, bool failed);
    friend void player_update_callback(void* arg, DecodeBase* dec);

public:
    using Screen::Screen;

    void begin(const ListEntry* ent_, int fav_index_, Screen* prev_);
    void show() override;
};
