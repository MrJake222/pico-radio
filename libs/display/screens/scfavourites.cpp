#include "scfavourites.hpp"

#include <screenmng.hpp>
#include <ubuntu_mono.hpp>
#include <loaderfav.hpp>
#include <icons.hpp>


int ScFavourites::rows_above() {
    return 0;
}

int ScFavourites::rows_below() {
    return 1;
}

int ScFavourites::size_x(int y) {
    return 1;
}

enum Action {
    PLAY,
    SEARCH
};

int ScFavourites::get_action(int x, int y) {
    if (y == last_y()) {
        return SEARCH;
    }

    return PLAY;
}

void ScFavourites::draw_button(int x, int y, bool selected) {

    auto action = (Action) get_action(x, y);
    int bg;
    const int fg = COLOR_FG;

    switch (action) {
        case SEARCH:
            bg = get_btn_bg(selected, false);
            break;

        default:
            bg = get_btn_bg(selected, true);
    }

    switch (action) {
        case PLAY:
            draw_button_entry(y, selected);
            break;

        case SEARCH:
            display.fill_rect(143, 111, 15, 15, bg);
            display.draw_icon(144, 112, icon_search, bg, fg);
            break;
    }
}

Screen* ScFavourites::run_action(int action) {
    int i, r;

    switch ((Action) action) {
        case PLAY:
            i = get_selected_station_index();
            r = ll.check_station_url(i);
            if (r < 0) {
                show_error("Błąd: nie można otworzyć strumienia");
                return nullptr;
            }

            // <i> equals position on the fav list
            sc_play.begin(ll.get_station(i), i);
            return &sc_play;

        case SEARCH:
            sc_search.begin();
            return &sc_search;

        // case BACK:
        //     ll.load_abort();
        //     return &sc_search;
    }

    return nullptr;
}

void ScFavourites::show() {
    // called from input
    ScreenList::show();
}

void ScFavourites::begin() {
    // called from previous screen (on input)
    ((LoaderFav&) ll).begin();

    ScreenList::begin();
}