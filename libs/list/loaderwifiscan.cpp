#include "loaderwifiscan.hpp"

#include <wifiscan.hpp>

void lwifi_sorter_cb(void* arg, const char* res) {
    auto ld = (LoaderWifiScan*) arg;

    ld->set_result(
            wifi::lfs::format_decode_ssid(res),
            wifi::lfs::format_decode_quality(res));
}

void LoaderWifiScan::set_result(const char* ssid, int p) {
    ListEntry* entry = get_current_entry();

    entry->set_name(ssid);
    entry->lwifi.quality = p;

    set_next_entry(1);
}

void LoaderWifiScan::task() {
    int r;
    bool errored = false;

    if (!can_use_cache) {
        r = wifi::lfs::scan(acc);
        if (r) {
            errored = true;
            goto end;
        }
    }

    r = wifi::lfs::read(acc, entries_max, page * entries_max,
                       this, lwifi_sorter_cb);

    if (r < 0) {
        errored = true;
        goto end;
    }

end:
    call_all_loaded(errored);
}

int LoaderWifiScan::get_entry_count_whole() {
    return wifi::lfs::count(acc);
}
