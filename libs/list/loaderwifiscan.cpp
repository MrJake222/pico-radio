#include "loaderwifiscan.hpp"

#include <wifiscan.hpp>
#include <lfsorter.hpp>

void lwifi_sorter_cb(void* arg, const char* res) {
    auto ld = (LoaderWifiScan*) arg;

    const char* p    = res;     // 3-digit percent value + space
    const char* ssid = res + 4; // skip percent

    ld->set_result(ssid, atoi(p));
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
        r = wifi::scan_lfs(acc);
        if (r) {
            errored = true;
            goto end;
        }
    }

    r = wifi::read_lfs(acc, entries_max, page * entries_max,
                       this, lwifi_sorter_cb);

    if (r) {
        errored = true;
        goto end;
    }

end:
    call_all_loaded(errored);
}

int LoaderWifiScan::get_entry_count_whole() {

    // this is called from all_loaded callback
    // -> can use cache

    int r;
    bool errored = false;

    r = lfsorter::open(acc);
    if (r) {
        errored = true;
        goto end_noclose;
    }

    r = acc.skip_all_lines();
    if (r < 0) {
        errored = true;
        goto end;
    }

    printf("wifi: scanned %d networks\n", r);

end:
    acc.close();

end_noclose:
    return errored ? -1 : r;
}
