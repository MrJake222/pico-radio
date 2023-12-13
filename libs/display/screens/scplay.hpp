#pragma once

#include <config.hpp>
#include <screen.hpp>
#include <cstring>
#include <decodebase.hpp>
#include <listentry.hpp>
#include <sdscan.hpp>

class ScPlay : public Screen {

    const char* get_title() override;

    int size_x(int y) override;
    int size_y() override;

    void draw_star(bool selected);
    void draw_button(int x, int y, bool selected, bool was_selected) override;

    int get_action(int x, int y) override;
    Screen* run_action(int action) override;

    // used for loading next entry from local playback list
    SDScan& scan;   // scan instance to load cached entries
    int list_index; // index on list from where the entry was loaded
                    // (only valid on local playback from folder, else -1)
    ListEntry ent;  // this is a copy of currently played entry
                    // (might be overwritten on next entry load)
    int fav_index;  // favourite index (-1 if not on the fav list)
    Screen* prev;   // screen to open on back press

    void start_playback();

    // metadata scrolled text index
    int meta_idx;

    // true if user pressed a button to request player stop
    // used by <player_finished_callback> to close the player screen when playback
    // stops by itself
    bool player_stop_requested;

    friend void player_load_next_callback(void* arg, const char* res);
    friend bool player_finished_callback(void* arg, bool failed);
    friend void player_update_callback(void* arg, DecodeBase* dec);

public:
    ScPlay(ST7735S& display_, SemaphoreHandle_t& mutex_ticker_,
           SDScan& scan_)
        : Screen(display_, mutex_ticker_)
        , scan(scan_)
    { }

    void begin(ListEntry* ent_, int list_index_, int fav_index_, Screen* prev_);
    void show() override;
};
