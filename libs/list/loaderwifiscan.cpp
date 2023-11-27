#include "loaderwifiscan.hpp"

#include <FreeRTOS.h>
#include <task.h>

int scan_res_cb(void* arg, const cyw43_ev_scan_result_t* res) {
    auto ld = (LoaderWifiScan*) arg;

    printf("  wifi: scanned %s @ ch%d\n", res->ssid, res->channel);

    ld->set_result((const char*) res->ssid);

    return 0;
}

void LoaderWifiScan::set_result(const char* ssid) {
    if (entries_offset == entries_max)
        return;

    // scan for duplicates
    for (int i=0; i<entries_offset; i++) {
        if (strncmp(entries[i].get_name(), ssid, ENT_NAME_LEN) == 0)
            // duplicate
            return;
    }

    ListEntry* entry = &entries[entries_offset];

    entry->set_name(ssid);
    entry->set_url("");

    entries_offset++;
}

int LoaderWifiScan::scan_networks(bool dry_run) {
    int r;
    cyw43_wifi_scan_options_t scan_options = {0};

    puts("wifi: scan start");

    r = cyw43_wifi_scan(
            &cyw43_state,
            &scan_options,
            this,
            scan_res_cb);

    if (r)
        return r;

    int retries = 100; // 10s max
    while (cyw43_wifi_scan_active(&cyw43_state) && retries--)
        vTaskDelay(pdMS_TO_TICKS(100));

    puts("wifi: scan end");

    return 0;
}

int LoaderWifiScan::get_entry_count_whole() {
    return -1;
}

void LoaderWifiScan::task() {
    int to_skip = page * entries_max;

    scan_networks(false);

    call_all_loaded(false);
}
