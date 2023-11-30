#include "loaderfav.hpp"

void LoaderFav::setup_entry(ListEntry* ent) {
    ent->type = le_type_radio;
    // TODO check for files
}

void LoaderFav::task() {
    int cnt = load_m3u();
    if (cnt < 0) {
        call_all_loaded(1);
        return;
    }

    set_next_entry(cnt);
    call_all_loaded(0);
}