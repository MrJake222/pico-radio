#include "sclocal.hpp"

#include <icons.hpp>
#include <screenmng.hpp>

enum Action {
    PLAY,
    BACK,
};

int ScLocal::get_action(int x, int y) {
    if (y == last_y()) {
        return BACK;
    }

    return PLAY;
}

void ScLocal::draw_button(int x, int y, bool selected) {

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

Screen* ScLocal::run_action(int action) {
    int i, r;
    ListEntry* ent;

    switch ((Action) action) {
        case PLAY:
            i = get_selected_station_index();
            ent = ll.get_entry(i);

            switch (ent->type) {
                case ListEntry::le_type_local:
                    // play local file
                    r = ll.check_entry_url(i);
                    if (r < 0) {
                        show_error("Błąd: nie można otworzyć strumienia");
                        return nullptr;
                    }

                    // <i> equals position on search list (not fav list)
                    sc_play.begin(ent, -1, this);
                    return &sc_play;

                case ListEntry::le_type_dir:
                    // open folder recursively
                    r = ll.go(ent->get_url());
                    if (r < 0) {
                        show_error("Błąd: nie można otworzyć strumienia");
                        return nullptr;
                    }

                    set_reload();
                    set_fav_pos(0);
                    show();
                    break;

                // radio is impossible here
                case ListEntry::le_type_radio: break;
            }

            break;

        case BACK:
            ll.load_abort();

            r = ll.up();
            if (r < 0)
                // already top-level
                // close and go to fav screen
                return &sc_fav;

            // path updated
            set_reload();
            set_fav_pos(0);
            show();
    }

    return nullptr;
}

void ScLocal::begin(const char* path_) {
    ll.begin(path_);
    ScreenList::begin();

    // request reload of favourites screen
    // this is important because here we are reusing
    // the same buffers for stations as the fav list
    sc_fav.set_reload();
}
