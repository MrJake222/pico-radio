#include "scfavourites.hpp"

#include <screenmng.hpp>
#include <ubuntu_mono.hpp>
#include <radiofav.hpp>


int ScFavourites::rows_above() {
    return 0;
}

int ScFavourites::rows_below() {
    return 0;
}

int ScFavourites::size_x(int y) {
    return 1;
}

int ScFavourites::size_y() {
    return kb_buttons();
}

enum Action {
    PLAY,
};

int ScFavourites::get_action(int x, int y) {
    // if (y == last_y()) {
    //     return BACK;
    // }

    return PLAY;
}

void ScFavourites::draw_button(int x, int y, bool selected) {

    auto action = (Action) get_action(x, y);
    int bg;

    switch (action) {
        case PLAY:
            draw_button_entry(y, selected);
            break;

        // case BACK:
        //     bg = get_btn_bg(selected, false);
        //     display.fill_rect(1, 114, 13, 13, bg);
        //     display.draw_icon(2, 115, icon_back, bg, COLOR_FG);
        //     break;
    }
}

Screen* ScFavourites::run_action(int action) {
    int i;
    const char* url;

    switch ((Action) action) {
        case PLAY:
            i = get_selected_station_index();
            url = ll.get_station_url(i);
            if (!url) {
                show_error("Błąd: nie można otworzyć strumienia");
                return nullptr;
            }

            sc_play.begin(ll.get_station_name(i), url);
            return &sc_play;

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
    ((RadioFav&) ll).begin();

    ScreenList::begin();
}