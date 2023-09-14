#include "scplay.hpp"

#include <screenmng.hpp>
#include <icons.hpp>
#include <ubuntu_mono.hpp>
#include <player.hpp>
#include <cstdio>
#include <fav.hpp>

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
                fav_index = fav::add(ent);
                if (fav_index < 0) {
                    show_error("Nie udało się dodać do ulubionych");
                    return nullptr;
                }

                // set position to newly added station and request reload
                sc_fav.set_fav_pos(fav_index);
                sc_fav.set_reload();
            }
            else {
                // on fav list
                // remove from persistent storage
                int r = fav::remove(fav_index);
                if (r < 0) {
                    show_error("Nie udało się usunąć z ulubionych");
                    return nullptr;
                }

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

    if (sc->is_overlay_displayed)
        // don't update stats if something covers them
        return;

    // current playback time
    const int c = dec->current_time();
    const int d = dec->duration();

    // currently playing song (from metadata)
    int r = dec->get_meta_str(buf, PLAYER_META_BUF_LEN);

    switch (sc->ent->type) {

        case ListEntry::le_type_radio:
            // update bottom text
            sc->update_scrolled_text(sc->meta_idx,
                                     r < 0 ? "brak danych" : buf);

            // station time
            sprintf(buf, "%02d:%02d", c/60, c%60);
            sc->add_normal_text_ljust(
                    159, 55, buf,
                    ubuntu_font_get_size(UbuntuFontSize::FONT_12),
                    COLOR_BG, COLOR_ACC1);

            break;

        case ListEntry::le_type_local:
            // update top text (only if meta available)
            if (r == 0) {
                sc->update_scrolled_text(sc->meta_idx, buf);
                if (sc->ent->no_name()) {
                    sc->ent->set_name(buf);
                }
            }

            // scrolling "progress bar" of the song
            sc->draw_progress_bar(2, 45, sc->display.W - 4,
                                  d == 0 ? 0    // probably beginning of the playback
                                                 : c * 100 / d,
                                  COLOR_BG_DARK, COLOR_ACC1, false);

            // song time / total time
            sprintf(buf, "%02d:%02d / %02d:%02d", c/60, c%60, d/60, d%60);
            sc->add_normal_text_ljust(
                    159, 50, buf,
                    ubuntu_font_get_size(UbuntuFontSize::FONT_12),
                    COLOR_BG, COLOR_ACC1);



            break;

        // impossible
        case ListEntry::le_type_dir: break;
    }

    // current CPU usage
    // sc->draw_progress_bar(74, 76, dec->core0_usage(), COLOR_BG_DARK, COLOR_ACC2);
    sc->draw_progress_bar(74, STATS_Y + 5, 64, dec->core1_usage(), COLOR_BG_DARK, COLOR_ACC2, true);

    // current buffer health
    sc->draw_progress_bar(74, STATS_Y + 16, 64, dec->buf_health(), COLOR_BG_DARK, COLOR_ACC2, true);

    // current bitrate
    sprintf(buf, "%d kbps", dec->bitrate() / 1000);
    sc->add_normal_text_ljust(
            160, STATS_Y + 22, buf,
            ubuntu_font_get_size(UbuntuFontSize::FONT_12),
            COLOR_BG, COLOR_ACC2);
}

void ScPlay::show() {
    Screen::show();

    switch (ent->type) {

        case ListEntry::le_type_radio:
            // top text -- station name: scrolled or normal (not changeable)
            add_scrolled_text_or_normal(
                    2, 13, ent->get_name(),
                    ubuntu_font_get_size(UbuntuFontSize::FONT_24),
                    COLOR_BG, COLOR_ACC2,
                    display.W - 2*2);

            // bottom text -- station song: scrolled (save id to change later from metadata)
            meta_idx = add_scrolled_text(
                    2, 39, "brak danych",
                    ubuntu_font_get_size(UbuntuFontSize::FONT_16),
                    COLOR_BG, COLOR_ACC1,
                    display.W - 2*2);

            break;

        case ListEntry::le_type_local:
            // top text -- song name: scrolled (save id to change later from metadata)
            meta_idx = add_scrolled_text(
                    2, 13, ent->get_name(),
                    ubuntu_font_get_size(UbuntuFontSize::FONT_24),
                    COLOR_BG, COLOR_ACC2,
                    display.W - 2*2);

            // no bottom text

            break;

        // impossible
        case ListEntry::le_type_dir: break;
    }

    add_normal_text_ljust(71, STATS_Y, "CPU",
                          ubuntu_font_get_size(UbuntuFontSize::FONT_12),
                          COLOR_BG, COLOR_FG);

    add_normal_text_ljust(71, STATS_Y + 11, "Bufor",
                          ubuntu_font_get_size(UbuntuFontSize::FONT_12),
                          COLOR_BG, COLOR_FG);

    if (!is_overlay_displayed) {
        int r;
        r = player_start(ent->get_url(),
                         this,
                         player_finished_callback,
                         player_update_callback);

        if (r < 0) {
            show_error("odtwarzanie nie mogło się rozpocząć");
        }
    }
}

void ScPlay::begin(ListEntry* ent_, int fav_index_, Screen* prev_) {
    ent = ent_;
    fav_index = fav_index_;
    prev = prev_;
    player_stop_requested = false;
    Screen::begin();
}