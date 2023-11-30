#include "loaderwifiscan.hpp"

#include <FreeRTOS.h>
#include <task.h>

#include <lfsorter.hpp>
#include <util.hpp>

static int cmp_ssid(const char* e1, const char* e2) {
    // skip 3-digit percent value + space
    return strcasecmp(e1 + 4, e2 + 4);
}

static int cmp_percent(const char* e1, const char* e2) {
    if (e1[0] == 0x00 || e2[0] == 0xff)
        return -1; //    e1 is smaller than everything
                   // or e2 is larger than everything

    if (e1[0] == 0xff || e2[0] == 0x00)
        return 1; //    e1 is larger than everything
                  // or e2 is smaller than everything

    // parse percent value
    // take advantage of atoi stopping at first invalid character
    int r = atoi(e2) - atoi(e1);
    if (r != 0)
        return r;

    // if percent value didn't sort
    return cmp_ssid(e1, e2);
}

int scan_res_cb(void* arg, const cyw43_ev_scan_result_t* res) {
    auto ld = (LoaderWifiScan*) arg;
    char buf[LFSS_BUF_SIZE];

    int p = rssi_to_percent(res->rssi);
    snprintf(buf, LFSS_BUF_SIZE, "%03d %s", p, res->ssid);
    bool dup = lfsorter::is_duplicate(ld->acc, cmp_ssid, buf);

    printf("  wifi: %c ch%2d rssi%3d (%2d%%) %s\n", (dup ? 'D' : ' '), res->channel, res->rssi, p, res->ssid);

    if (!dup) {
        lfsorter::write(ld->acc, 1, buf);
    }

    return 0;
}

void lwifi_sorter_cb(void* arg, const char* res) {
    auto ld = (LoaderWifiScan*) arg;

    const char* p    = res;     // 3-digit percent value + space
    const char* ssid = res + 4; // skip percent

    ld->set_result(ssid, atoi(p));
}

void LoaderWifiScan::set_result(const char* ssid, int p) {
    ListEntry* entry = &entries[entries_offset];

    entry->set_name(ssid);

    entries_offset++;
}

int LoaderWifiScan::scan_networks() {
    int r;
    bool errored = false;
    cyw43_wifi_scan_options_t scan_options = {0};

    r = lfsorter::open_create_truncate(acc);
    if (r) {
        errored = true;
        goto end_noclose;
    }

    puts("wifi: scan start");

    r = cyw43_wifi_scan(
            &cyw43_state,
            &scan_options,
            this,
            scan_res_cb);

    if (r) {
        errored = true;
        goto end;
    }

    for (int i=0; i<100 && cyw43_wifi_scan_active(&cyw43_state); i++)
        vTaskDelay(pdMS_TO_TICKS(100));

    puts("wifi: scan end");

end:
    acc.close();

end_noclose:
    return errored ? -1 : 0;
}

int LoaderWifiScan::read_networks() {
    int r;
    bool errored = false;

    r = lfsorter::open(acc);
    if (r) {
        errored = true;
        goto end_noclose;
    }

    lfsorter::get_smallest_n_skip_k(acc, entries_max, entries_max * page,cmp_percent,
                                    this, lwifi_sorter_cb);

end:
    acc.close();

end_noclose:
    return errored ? -1 : 0;
}

void LoaderWifiScan::task() {
    int r;
    bool errored = false;

    if (!can_use_cache) {
        r = scan_networks();
        if (r) {
            errored = true;
            goto end;
        }
    }

    r = read_networks();
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
