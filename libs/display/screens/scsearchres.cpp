#include "scsearchres.hpp"

#include <screenmng.hpp>
#include <loadersearch.hpp>
#include <icons.hpp>
#include <ubuntu_mono.hpp>

#define STATUS_X    60
#define STATUS_Y    70

enum Action {
    PLAY,
    BACK,
};

int ScSearchRes::get_action(int x, int y) {
    if (y == last_y()) {
        return BACK;
    }

    return PLAY;
}

void ScSearchRes::draw_button(int x, int y, bool selected) {

    auto action = (Action) get_action(x, y);
    int bg;

    switch (action) {
        case PLAY:
            draw_button_entry(y, selected);
            break;

        case BACK:
            bg = get_btn_bg(selected, false);
            display.fill_rect(1, 114, 13, 13, bg);
            display.draw_icon(2, 115, &icon_back, bg, COLOR_FG);
            break;
    }
}

Screen* ScSearchRes::run_action(int action) {
    int i, r;

    switch ((Action) action) {
        case PLAY:
            i = get_selected_station_index();
            r = ll.check_entry_url(i);
            if (r < 0) {
                show_error("Błąd: nie można otworzyć strumienia");
                return nullptr;
            }

            // <i> equals position on search list (not fav list)
            sc_play.begin(ll.get_entry(i), -1, this);
            return &sc_play;

        case BACK:
            ll.load_abort();
            // sc_search.show_cursor();
            return &sc_search;
    }

    return nullptr;
}

void search_update_cb(void* arg, int provider_idx, int server_idx, int max_servers);

void ScSearchRes::show() {

    sprintf(subtitle, "\"%s\"", prompt);
    add_normal_text(8, 15, subtitle,
                    ubuntu_font_get_size(UbuntuFontSize::FONT_16),
                    COLOR_BG, COLOR_FG,
                    display.W);

    add_normal_text_ljust(STATUS_X, STATUS_Y, "Dostawca",
                          ubuntu_font_get_size(UbuntuFontSize::FONT_12),
                          COLOR_BG, COLOR_FG);

    add_normal_text_ljust(STATUS_X, STATUS_Y+11, "Serwer",
                          ubuntu_font_get_size(UbuntuFontSize::FONT_12),
                          COLOR_BG, COLOR_FG);

    // setup list-loader first
    ll.begin(le_type_radio, prompt);
    ll.set_update_cb(search_update_cb);

    ScreenList::show();
}

void ScSearchRes::begin(const char* prompt_) {
    // called from previous screen (on input)
    prompt = prompt_;
    ScreenList::begin();
}

void search_update_cb(void* arg, int provider_idx, int server_idx, int max_servers) {
    auto sc = (ScSearchRes*) arg;

    sc->draw_progress_bar(STATUS_X+3, STATUS_Y+5,    64, provider_idx*100 / sc->ll.get_provider_count(), COLOR_BG_DARK, COLOR_ACC2, true);
    sc->draw_progress_bar(STATUS_X+3, STATUS_Y+11+5, 64, server_idx*100   / max_servers,                 COLOR_BG_DARK, COLOR_ACC2, true);
}