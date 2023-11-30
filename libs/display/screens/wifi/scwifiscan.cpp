#include "scwifiscan.hpp"

#include <screenmng.hpp>
#include <icons.hpp>

#include <screenmng.hpp>

enum Action {
    PLAY,
    BACK,
};

int ScWifiScan::get_action(int x, int y) {
    if (y == last_y()) {
        return BACK;
    }

    return PLAY;
}

void ScWifiScan::draw_button(int x, int y, bool selected, bool was_selected) {

    auto action = (Action) get_action(x, y);
    int bg;

    switch (action) {
        case PLAY:
            draw_button_entry(y, selected, was_selected);
            break;

        case BACK:
            bg = get_btn_bg(selected, false);
            display.fill_rect(1, 114, 13, 13, bg);
            display.draw_icon(2, 115, &icon_back, bg, COLOR_FG);
            break;
    }
}

Screen* ScWifiScan::run_action(int action) {
    int i, r;
    ListEntry* ent;

    switch ((Action) action) {
        case PLAY:
            i = get_selected_station_index();
            ent = ll.get_entry(i);

            break;

        case BACK:
            ll.load_abort();
            return &sc_wifi_saved;
    }

    return nullptr;
}

void ScWifiScan::show() {
    ll.begin(le_type_wifi);
    ScreenList::show();
}