#include "wifiscan.hpp"

#include <pico/cyw43_arch.h>
#include <util.hpp>

#include <FreeRTOS.h>
#include <task.h>

const char* wifiscan_lfs_path = ".tmp_wscan";

void WifiScan::format_encode(char* buf, int buf_size, const char* ssid, int q) {
    // format wifi to a string as "qqq NAME"
    // where qqq is quality in percents is 3-digit format
    // next is one space
    // and NAME is the wifi ssid
    snprintf(buf, buf_size, "%03d %s", q, ssid);
}

const char* WifiScan::format_decode_ssid(const char* buf) {
    // skip 3-digit percent quality + space
    return buf + 4;
}

int WifiScan::format_decode_quality(const char* buf) {
    // directly at the beginning (ends with space)
    return atoi(buf);
}

static int cmp_dup(const char* e1, const char* e2) {
    const int cmp_ssid = strcasecmp(WifiScan::format_decode_ssid(e1), WifiScan::format_decode_ssid(e2));
    if (cmp_ssid != 0) {
        // different ssid
        return cmp_ssid;
    }

    // same ssid
    int r = WifiScan::format_decode_quality(e2) - WifiScan::format_decode_quality(e1);
    if (r > 10)
        // same ssid but quality better by more than 10%
        return r;

    // same ssid and similar or worse quality
    return 0;
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
    int r = WifiScan::format_decode_quality(e2) - WifiScan::format_decode_quality(e1);
    if (r != 0)
        return r;

    // if percent value didn't sort
    return 0;
}

void WifiScan::begin() {
    LfsScan::begin(wifiscan_lfs_path, cmp_percent);
}

int wscan_res_cb(void* arg, const cyw43_ev_scan_result_t* res) {
    auto ws = (WifiScan*) arg;
    char buf[LFSS_BUF_SIZE];

    if (ws->should_abort_cpy)
        return 0;

    if (res->ssid_len == 0)
        // hidden network
        return 0;

    int q = rssi_to_percent(res->rssi);
    WifiScan::format_encode(buf, LFSS_BUF_SIZE, (const char*) res->ssid, q);

    bool dup = ws->is_duplicate(cmp_dup, buf);

    printf("  wifi: %c ch%2d rssi%3d (%2d%%) %s\n", (dup ? 'D' : ' '), res->channel, res->rssi, q, res->ssid);

    if (!dup) {
        ws->write(1, buf);
    }

    return 0;
}

int WifiScan::scan_internal(flagref_t should_abort) {
    int r;
    bool errored = false;
    cyw43_wifi_scan_options_t scan_options = {0};
    should_abort_cpy = should_abort;

    printf("wifi: scan start (active=%d)\n", cyw43_wifi_scan_active(&cyw43_state));

    r = cyw43_wifi_scan(
            &cyw43_state,
            &scan_options,
            this,
            wscan_res_cb);

    if (r) {
        puts("wifi: scan failed to start");
        errored = true;
        goto end;
    }

    for (int i=0; i<WIFI_SCAN_POLL_MAX_TIMES && cyw43_wifi_scan_active(&cyw43_state) && !should_abort; i++) {
        vTaskDelay(pdMS_TO_TICKS(WIFI_SCAN_POLL_MS));

        // update copied value in struct, reference can be destroyed
        // after we leave this function
        should_abort_cpy = should_abort;
    }

    if (should_abort)
        puts("wifi: scan abort");
    else
        puts("wifi: scan end");

end:
    return errored ? -1 : 0;
}