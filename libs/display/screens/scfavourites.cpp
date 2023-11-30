#include "scfavourites.hpp"

#include <screenmng.hpp>
#include <ubuntu_mono.hpp>
#include <icons.hpp>
#include <sd.hpp>
#include <wifi.hpp>

enum Action {
    LIST,

    LOCAL,
    SETTINGS,
    SEARCH,
};

int ScFavourites::get_action(int x, int y) {
    if (y == last_y()) {
        // action icon row

        switch (x) {
            case 0: return LOCAL;
            case 1: return SETTINGS;
            case 2: return SEARCH;
        }
    }

    return LIST;
}

void ScFavourites::draw_button(int x, int y, bool selected, bool was_selected) {

    auto action = (Action) get_action(x, y);
    int bg;
    const int fg = COLOR_FG;

    switch (action) {
        case LIST:
            bg = get_btn_bg(selected, true);
            break;

        default:
            // all icons
            bg = get_btn_bg(selected, false);
    }

    switch (action) {
        case LIST:
            draw_button_entry(y, selected, was_selected);
            break;

        case LOCAL:
            display.fill_rect(2, 111, 15, 15, bg);
            display.draw_icon(3, 112, &icon_local, bg, fg);
            break;

        case SETTINGS:
            display.fill_rect(2+15, 111, 15, 15, bg);
            display.draw_icon(3+15, 112, &icon_settings, bg, fg);
            break;

        case SEARCH:
            display.fill_rect(143, 111, 15, 15, bg);
            display.draw_icon(144, 112, &icon_search, bg, fg);
            break;
    }
}

Screen* ScFavourites::run_action(int action) {
    int i, r;
    ListEntry* ent;

    switch ((Action) action) {
        case LIST:
            i = get_selected_station_index();
            ent = ll.get_entry(i);

            if (ent->type == le_type_radio && !wifi::is_connected()) {
                show_warn("Uwaga: brak połączenia Wi-Fi");
                return nullptr;
            }

            r = ll.check_entry_url(i);
            if (r < 0) {
                show_error("Błąd: nie można otworzyć strumienia");
                return nullptr;
            }

            // player screen can delete station from favs
            // its easier just not to set preserve here
            // set_preserve();

            // <i> equals position on the fav list (in relation to loaded stations)
            // need to adjust fav_index for current page
            sc_play.begin(ll.get_entry(i), i + get_page() * MAX_ENTRIES, this);
            return &sc_play;

        case LOCAL:
            if (!sd::is_card_mounted()) {
                show_warn("Uwaga: brak karty SD");
                return nullptr;
            }

            sc_local.begin("/");
            return &sc_local;

        case SETTINGS:
            sc_settings.begin();
            return &sc_settings;

        case SEARCH:
            if (!wifi::is_connected()) {
                show_warn("Uwaga: brak połączenia Wi-Fi");
                return nullptr;
            }

            sc_search.begin();
            return &sc_search;
    }

    return nullptr;
}

void fav_update_cb(void* arg, const char* info);

void ScFavourites::show() {
    ll.begin();
    ll.set_update_cb(fav_update_cb);

    // call superclass after setup
    ScreenList::show();
}

void fav_update_cb(void* arg, const char* info) {
    auto sc = (ScFavourites*) arg;

    // it just loads too fast (no point in cluttering with what user cannot read)
    // sc->add_normal_text(10, 60, info,
    //                 ubuntu_font_get_size(UbuntuFontSize::FONT_12),
    //                 COLOR_BG, COLOR_FG,
    //                 sc->display.W - 10);
}