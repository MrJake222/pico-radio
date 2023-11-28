#include "loaderwifiscan.hpp"

#include <FreeRTOS.h>
#include <task.h>

#include <util.hpp>

int scan_res_cb(void* arg, const cyw43_ev_scan_result_t* res) {
    auto ld = (LoaderWifiScan*) arg;

    printf("  wifi: scanned %s @ ch%d\n", res->ssid, res->channel);
    ld->set_result((const char*) res->ssid);

    return 0;
}

static bool before(const ListEntry* e1, const ListEntry* e2) {
    // case insensitive comparison
    // sorts A-Z
    return strcasecmp(e1->get_name(), e2->get_name()) < 0;
}

void LoaderWifiScan::set_result(const char* ssid) {
    // scan for duplicates
    for (int i=0; i<entries_offset; i++) {
        if (strncmp(entries[i].get_name(), ssid, ENT_NAME_LEN) == 0)
            // duplicate
            return;
    }

    ListEntry e{};
    e.set_name(ssid);

    // entries are kept in order

    if (entries_offset == entries_max) {
        // entries maxed out
        if (before(&entries[entries_offset - 1], &e))
            // last entry before <e>
            // discard <e>
            return;

        // <e> before last entry
        // discard last entry
        // (and insert <e> below)
        entries_offset--;
    }

    insert_in_order(entries, entries_offset, &e, before);
    entries_offset++;
}

int LoaderWifiScan::scan_networks() {
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

// TODO LoaderWifiScan implement pagination
// TODO LoaderWifiScan entry counting

void LoaderWifiScan::task() {
    scan_networks();
    call_all_loaded(false);
}

int LoaderWifiScan::get_entry_count_whole() {
    return -1;
}
