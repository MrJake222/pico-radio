#include "scsettings.hpp"

#include <screenmng.hpp>
#include <icons.hpp>
#include <settings.hpp>

enum Action {
    LIST,
    BACK,
};

int ScSettings::get_action(int x, int y) {
    if (y == last_y()) {
        return BACK;
    }

    return LIST;
}

void ScSettings::draw_button(int x, int y, bool selected, bool was_selected) {

    auto action = (Action) get_action(x, y);
    int bg;

    switch (action) {
        case LIST:
            draw_button_entry(y, selected, was_selected);
            break;

        case BACK:
            bg = get_btn_bg(selected, false);
            display.fill_rect(1, 114, 13, 13, bg);
            display.draw_icon(2, 115, &icon_back, bg, COLOR_FG);
            break;
    }
}

Screen* ScSettings::run_action(int action) {
    int i, r;
    ListEntry* ent;

    switch ((Action) action) {
        case LIST:
            i = get_selected_station_index();
            ent = ll.get_entry(i);

            switch (ent->lconst.idx) {
                case settings::me_wifi_idx:
                    sc_wifi_saved.begin();
                    return &sc_wifi_saved;

                case settings::me_bat_idx:
                    sc_bat.begin();
                    return &sc_bat;
            }

            break;

        case BACK:
            ll.load_abort();
            return &sc_fav;
    }

    return nullptr;
}

void ScSettings::show() {
    ll.begin(le_type_const);
    ScreenList::show();
}
