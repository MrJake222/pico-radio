#include "wifiscan.hpp"

#include <pico/cyw43_arch.h>
#include <util.hpp>

#include <FreeRTOS.h>
#include <task.h>

namespace wifi::lfs {

void format_encode(char* buf, int buf_size, const char* ssid, int q) {
    // format wifi to a string as "qqq NAME"
    // where qqq is quality in percents is 3-digit format
    // next is one space
    // and NAME is the wifi ssid
    snprintf(buf, buf_size, "%03d %s", q, ssid);
}

const char* format_decode_ssid(const char* buf) {
    // skip 3-digit percent quality + space
    return buf + 4;
}

int format_decode_quality(const char* buf) {
    // directly at the beginning (ends with space)
    return atoi(buf);
}

static int cmp_ssid(const char* e1, const char* e2) {
    return strcasecmp(format_decode_ssid(e1), format_decode_ssid(e2));
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
    int r = format_decode_quality(e2) - format_decode_quality(e1);
    if (r != 0)
        return r;

    // if percent value didn't sort
    return cmp_ssid(e1, e2);
}

int scan_res_cb(void* arg, const cyw43_ev_scan_result_t* res) {
    auto& acc = *(LfsAccess*)arg;
    char buf[LFSS_BUF_SIZE];

    int q = rssi_to_percent(res->rssi);
    format_encode(buf, LFSS_BUF_SIZE, (const char*) res->ssid, q);

    bool dup = lfsorter::is_duplicate(acc, cmp_ssid, buf);

    printf("  wifi: %c ch%2d rssi%3d (%2d%%) %s\n", (dup ? 'D' : ' '), res->channel, res->rssi, q, res->ssid);

    if (!dup) {
        lfsorter::write(acc, 1, buf);
    }

    return 0;
}

int scan(LfsAccess& acc) {
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

int read(LfsAccess& acc, int n, int k, void* cb_arg, lfsorter::res_cb_fn cb) {
    int r;
    bool errored = false;

    r = lfsorter::open(acc);
    if (r) {
        errored = true;
        goto end_noclose;
    }

    int cnt;
    cnt = lfsorter::get_smallest_n_skip_k(
            acc, n, k, cmp_percent,
            cb_arg, cb);

    acc.close();

end_noclose:
    return errored ? -1 : cnt;
}

int count(LfsAccess& acc) {

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


} // namespace