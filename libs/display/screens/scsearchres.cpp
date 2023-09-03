#include "scsearchres.hpp"

#include <screenmng.hpp>
#include <loadersearch.hpp>
#include <icons.hpp>
#include <ubuntu_mono.hpp>


int ScSearchRes::rows_above() {
    return 0;
}

int ScSearchRes::rows_below() {
    return 1;
}

int ScSearchRes::size_x(int y) {
    return 1;
}

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
            display.draw_icon(2, 115, icon_back, bg, COLOR_FG);
            break;
    }
}

Screen* ScSearchRes::run_action(int action) {
    int i, r;

    switch ((Action) action) {
        case PLAY:
            i = get_selected_station_index();
            r = ll.check_station_url(i);
            if (r < 0) {
                show_error("Błąd: nie można otworzyć strumienia");
                return nullptr;
            }

            // <i> equals position on search list (not fav list)
            sc_play.begin(ll.get_station(i), -1, this);
            return &sc_play;

        case BACK:
            ll.load_abort();
            sc_search.show_cursor();
            return &sc_search;
    }

    return nullptr;
}

void ScSearchRes::show() {
    // called from input
    ScreenList::show();

    sprintf(subtitle, "\"%s\"", prompt);
    add_normal_text(8, 15, subtitle,
                    ubuntu_font_get_size(UbuntuFontSize::FONT_16),
                    COLOR_BG, COLOR_FG,
                    display.W);
}

void ScSearchRes::begin(const char* prompt_) {
    // called from previous screen (on input)
    prompt = prompt_;
    ((LoaderSearch&) ll).begin(prompt);

    // reset favourites screen state
    // this is important because here we are reusing
    // the same buffers for stations as the fav list
    sc_fav.begin();

    ScreenList::begin();
}