#include "loaderwifisaved.hpp"

void LoaderWifiSaved::setup_entry(ListEntry* ent) {
    ent->type = le_type_wifi;
    ent->lwifi.quality = -1; // saved wifi's don't have quality level
}

void LoaderWifiSaved::task() {
    int cnt = load_m3u();
    if (cnt < 0) {
        call_all_loaded(1);
        return;
    }

    set_next_entry(cnt);
    call_all_loaded(0);
}
