#include "scplay.hpp"

#include <screenmng.hpp>
#include <icons.hpp>
#include <ubuntu_mono.hpp>
#include <player.hpp>
#include <cstdio>

int ScPlay::size_x(int y) {
    return 1;
}

int ScPlay::size_y() {
    return 1;
}

enum Action {
    BACK
};

void ScPlay::draw_button(int x, int y, bool selected) {

    auto action = (Action) get_action(x, y);
    int bg;
    const int fg = COLOR_FG;

    switch (action) {
        case BACK:
            bg = get_btn_bg(selected, false);
            break;
    }

    switch (action) {
        case BACK:
            display.fill_rect(1, 114, 13, 13, bg);
            display.draw_icon(2, 115, icon_back, bg, fg);
            break;
    }
}

int ScPlay::get_action(int x, int y) {
    return BACK;
}

Screen* ScPlay::run_action(int action) {
    switch ((Action) action) {
        case BACK:
            player_stop();
            player_wait_for_end();
            return &sc_search_res;
    }

    return nullptr;
}

void player_failed_callback(void* arg) {
    // called from player task

    auto sc = (ScPlay*) arg;
    sc->show_error("odtwarzanie zakończyło się błędem");
}

void player_update_callback(void* arg, DecodeBase* dec) {
    // called from player stat task
    // each call is a new current time value
    auto sc = (ScPlay*) arg;

    char buf[PLAYER_META_BUF_LEN];

    // current playback time
    int c = dec->current_time();
    sprintf(buf, "%02d:%02d", c/60, c%60);
    sc->add_normal_text_ljust(
            159, 53, buf,
            ubuntu_font_get_size(UbuntuFontSize::FONT_12),
            COLOR_BG, COLOR_ACC1);

    // currently playing song (from metadata)
    int r = dec->get_meta_str(buf, PLAYER_META_BUF_LEN);
    sc->update_scrolled_text(sc->meta_idx,
                             r < 0 ? "brak danych" : buf);


}

void ScPlay::show() {
    Screen::show();

    add_scrolled_text_or_normal(
            2, 13, radio_name,
            ubuntu_font_get_size(UbuntuFontSize::FONT_24),
            COLOR_BG, COLOR_ACC2,
            display.W - 2*2);

    meta_idx = add_scrolled_text(
            2, 39, "brak danych",
            ubuntu_font_get_size(UbuntuFontSize::FONT_16),
            COLOR_BG, COLOR_ACC1,
            display.W - 2*2);

    if (!is_err_displayed) {
        player_start(radio_url,
                     this,
                     player_failed_callback,
                     player_update_callback);
    }
}

void ScPlay::begin(const char* name_, const char* url_) {
    radio_name = name_;
    radio_url = url_;
    Screen::begin();
}