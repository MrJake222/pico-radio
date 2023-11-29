#include "scwifisaved.hpp"

#include <screenmng.hpp>
#include <icons.hpp>

#include <screenmng.hpp>

enum Action {
    PLAY,
    BACK,
    SEARCH,
};

int ScWifiSaved::get_action(int x, int y) {
    if (y == last_y()) {
        switch (x) {
            case 0: return BACK;
            case 1: return SEARCH;
        }
    }

    return PLAY;
}

void ScWifiSaved::draw_button(int x, int y, bool selected) {

    auto action = (Action) get_action(x, y);
    int bg = get_btn_bg(selected, false);

    switch (action) {
        case PLAY:
            draw_button_entry(y, selected);
            break;

        case BACK:
            display.fill_rect(1, 114, 13, 13, bg);
            display.draw_icon(2, 115, icon_back, bg, COLOR_FG);
            break;

        case SEARCH:
            display.fill_rect(143, 111, 15, 15, bg);
            display.draw_icon(144, 112, icon_search, bg, COLOR_FG);
            break;
    }
}

Screen* ScWifiSaved::run_action(int action) {
    int i, r;
    ListEntry* ent;

    switch ((Action) action) {
        case PLAY:
            i = get_selected_station_index();
            ent = ll.get_entry(i);

            break;

        case BACK:
            ll.load_abort();
            return &sc_settings;

        case SEARCH:
            sc_wifi_scan.begin();
            return &sc_wifi_scan;

    }

    return nullptr;
}

void ScWifiSaved::show() {
    ll.begin(PATH_WIFI);
    ScreenList::show();
}
