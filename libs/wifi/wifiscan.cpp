#include "wifiscan.hpp"

#include <pico/cyw43_arch.h>
#include <util.hpp>

#include <FreeRTOS.h>
#include <task.h>

namespace wifi {

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
    auto& acc = *(LfsAccess*)arg;
    char buf[LFSS_BUF_SIZE];

    // format wifi to a string as "ppp NAME"
    // where ppp is quality in percents is 3-digit format
    // next is one space
    // and NAME is the wifi ssid
    int p = rssi_to_percent(res->rssi);
    snprintf(buf, LFSS_BUF_SIZE, "%03d %s", p, res->ssid);
    bool dup = lfsorter::is_duplicate(acc, cmp_ssid, buf);

    printf("  wifi: %c ch%2d rssi%3d (%2d%%) %s\n", (dup ? 'D' : ' '), res->channel, res->rssi, p, res->ssid);

    if (!dup) {
        lfsorter::write(acc, 1, buf);
    }

    return 0;
}

int scan_lfs(LfsAccess& acc) {
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
            &acc,
            scan_res_cb);

    if (r) {
        errored = true;
        goto end;
    }

    for (int i=0; i<WIFI_SCAN_POLL_MAX_TIMES && cyw43_wifi_scan_active(&cyw43_state); i++)
        vTaskDelay(pdMS_TO_TICKS(WIFI_SCAN_POLL_MS));

    puts("wifi: scan end");

end:
    acc.close();

end_noclose:
    return errored ? -1 : 0;
}

int read_lfs(LfsAccess& acc, int n, int k, void* cb_arg, lfsorter::res_cb_fn cb) {
    int r;
    bool errored = false;

    r = lfsorter::open(acc);
    if (r) {
        errored = true;
        goto end_noclose;
    }

    lfsorter::get_smallest_n_skip_k(acc, n, k, cmp_percent,
                                    cb_arg, cb);

    acc.close();

end_noclose:
    return errored ? -1 : 0;
}

} // namespace