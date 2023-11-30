#include "sclocal.hpp"

#include <icons.hpp>
#include <screenmng.hpp>

const char* ScLocal::get_title() {
    const char* leaf = path.leaf();
    if (!*leaf) {
        // empty string -> root folder
        return "Pliki lokalne";
    }

    // It'd be nice to strip trailing slash,
    // but this would require copying
    return leaf;
}

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

            // this is local playback screen
            assert(ent->type == le_type_local);

            if (!ent->llocal.is_dir) {
                // play local file
                r = ll.check_entry_url(i);
                if (r < 0) {
                    show_error("Błąd: nie można otworzyć strumienia");
                    return nullptr;
                }

                // player screen can't mess up the data
                // don't load it again on re-entry
                set_preserve();

                // <i> equals position on search list (not fav list)
                sc_play.begin(ent, -1, this);
                return &sc_play;
            }

            else {
                // open folder recursively
                r = path.go(ent->get_url());
                if (r < 0) {
                    show_error("Błąd: nie można otworzyć katalogu");
                    return nullptr;
                }

                draw_title(); // title changes as we update path

                set_abs_pos(0);
                load_page(lp_src_dir);
            }

            break;

        case BACK:
            ll.load_abort();

            r = path.up();
            if (r < 0)
                // already top-level
                // close and go to fav screen
                return &sc_fav;

            draw_title(); // title changes as we update path

            set_abs_pos(0);
            load_page(lp_src_dir);
            break;
    }

    return nullptr;
}

void ScLocal::begin(const char* path_) {
    path.begin(path_);
    ScreenList::begin();
}

void ScLocal::show() {
    ll.begin(le_type_local, &path);
    ScreenList::show();
}
