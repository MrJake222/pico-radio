#include "scplay.hpp"

#include <screenmng.hpp>
#include <icons.hpp>
#include <ubuntu_mono.hpp>
#include <player.hpp>
#include <cstdio>
#include <fav.hpp>
#include <static.hpp>

#define STATS_Y     80

int ScPlay::size_x(int y) {
    // if from fav don't display fav_back
    return prev == &sc_fav ? 2 : 3;
}

int ScPlay::size_y() {
    return 1;
}

enum Action {
    BACK,
    FAV,
    FAV_BACK // fast fav back
};

void ScPlay::draw_star(bool selected) {
    const int bg = get_btn_bg(selected, false);
    const int fg = COLOR_FG;

    display.fill_rect(14, 111, 17, 17, bg);
    display.draw_icon(15, 112,
                      fav_index < 0 ? icon_star_empty : icon_star_filled,
                      bg, fg);
}

void ScPlay::draw_button(int x, int y, bool selected) {

    auto action = (Action) get_action(x, y);
    int bg;
    const int fg = COLOR_FG;

    switch (action) {
        case BACK:
        case FAV:
        case FAV_BACK:
            bg = get_btn_bg(selected, false);
            break;
    }

    switch (action) {
        case BACK:
            display.fill_rect(1, 114, 13, 13, bg);
            display.draw_icon(2, 115, icon_back, bg, fg);
            break;

        case FAV:
            draw_star(selected);
            break;

        case FAV_BACK:
            display.fill_rect(30, 110, 17, 18, bg);
            display.draw_icon(31, 111, icon_fav_back, bg, fg);
            break;
    }
}

int ScPlay::get_action(int x, int y) {
    if (x == 0)
        return BACK;
    if (x == 1)
        return FAV;
    // if (x == 2)
    return FAV_BACK;
}

Screen* ScPlay::run_action(int action) {
    switch ((Action) action) {
        case BACK:
            player_stop_requested = true;
            player_stop();
            return prev;

        case FAV:
            if (fav_index < 0) {
                // not on fav list
                // add to persistent storage (and get index back)
                fav_index = fav::add(&st);
                // set position to newly added station and request reload
                sc_fav.set_page_pos(fav_index);
                sc_fav.set_reload();
            }
            else {
                // on fav list
                // remove from persistent storage
                fav::remove(fav_index);
                // request reload (page might've disappeared/new stations fill in the gap)
                sc_fav.set_reload();
                // mark removed
                fav_index = -1;
            }

            draw_star(true);
            return nullptr;

        case FAV_BACK:
            player_stop_requested = true;
            player_stop();
            return &sc_fav;

    }

    return nullptr;
}

void player_finished_callback(void* arg, bool failed) {
    // called from player task
    auto sc = (ScPlay*) arg;

    if (failed)
        sc->show_error("odtwarzanie zakończyło się błędem");
    else if (!sc->player_stop_requested)
        // just close the screen (if not closed by the user)
        screenmng_open(sc->prev);
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
            159, 55, buf,
            ubuntu_font_get_size(UbuntuFontSize::FONT_12),
            COLOR_BG, COLOR_ACC1);

    // currently playing song (from metadata)
    int r = dec->get_meta_str(buf, PLAYER_META_BUF_LEN);
    sc->update_scrolled_text(sc->meta_idx,
                             r < 0 ? "brak danych" : buf);

    // current CPU usage
    // sc->draw_progress_bar(74, 76, dec->core0_usage(), COLOR_BG_DARK, COLOR_ACC2);
    sc->draw_progress_bar(74, STATS_Y + 5, dec->core1_usage(), COLOR_BG_DARK, COLOR_ACC2);

    // current buffer health
    sc->draw_progress_bar(74, STATS_Y + 16, dec->buf_health(), COLOR_BG_DARK, COLOR_ACC2);

    // current bitrate
    sprintf(buf, "%d kbps", dec->bitrate() / 1000);
    sc->add_normal_text_ljust(
            160, STATS_Y + 22, buf,
            ubuntu_font_get_size(UbuntuFontSize::FONT_12),
            COLOR_BG, COLOR_ACC2);
}

void ScPlay::show() {
    Screen::show();

    add_scrolled_text_or_normal(
            2, 13, st.name,
            ubuntu_font_get_size(UbuntuFontSize::FONT_24),
            COLOR_BG, COLOR_ACC2,
            display.W - 2*2);

    meta_idx = add_scrolled_text(
            2, 39, "brak danych",
            ubuntu_font_get_size(UbuntuFontSize::FONT_16),
            COLOR_BG, COLOR_ACC1,
            display.W - 2*2);

    add_normal_text_ljust(71, STATS_Y, "CPU",
                          ubuntu_font_get_size(UbuntuFontSize::FONT_12),
                          COLOR_BG, COLOR_FG);

    add_normal_text_ljust(71, STATS_Y + 11, "Bufor",
                          ubuntu_font_get_size(UbuntuFontSize::FONT_12),
                          COLOR_BG, COLOR_FG);

    if (!is_err_displayed) {
        int r;
        r = player_start(st.url,
                         this,
                         player_finished_callback,
                         player_update_callback);

        if (r < 0) {
            show_error("odtwarzanie nie mogło się rozpocząć");
        }
    }
}

void ScPlay::begin(const struct station* st_, int fav_index_, Screen* prev_) {
    st = *st_;
    fav_index = fav_index_;
    prev = prev_;
    player_stop_requested = false;
    Screen::begin();
}