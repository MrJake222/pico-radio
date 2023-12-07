#include "wificonnect.hpp"

#include <pico/cyw43_arch.h>

#include <config.hpp>

#include <FreeRTOS.h>
#include <task.h>
#include <util.hpp>

namespace wifi {

// TODO rewrite wifi as class

static TaskHandle_t wifi_conn_task_h;

static char ssid[WIFI_SSID_MAX_LEN + 1];
static char pwd[WIFI_PWD_MAX_LEN + 1];
static uint8_t scan_auth;
static int16_t scan_rssi;
static uint8_t scan_bssid[6];

static cb_fns cbs;
static bool should_abort;

static uint32_t connect_auth() {
    if (scan_auth & 0x04)
        return CYW43_AUTH_WPA2_AES_PSK;

    if (scan_auth & 0x02)
        return CYW43_AUTH_WPA_TKIP_PSK;

    // if (sec & 0x01)
    //     // wep
    //     return -1;

    return -1;
}

static bool connected_same() {
    if (!is_connected_ip())
        return false; // not connected at all

    uint8_t bssid_current[6];
    cyw43_wifi_get_bssid(&cyw43_state, bssid_current);
    return memcmp(scan_bssid, bssid_current, 6) == 0;
}

static void connect_prepare(const char* ssid_, const char* pwd_, cb_fns cbs_) {
    strncpy(ssid, ssid_, WIFI_SSID_MAX_LEN);
    strncpy(pwd, pwd_, WIFI_PWD_MAX_LEN);
    cbs = cbs_;
    should_abort = false;
}

static void connect_clear() {
    memset(ssid, 0, WIFI_SSID_MAX_LEN);
    memset(pwd, 0, WIFI_PWD_MAX_LEN);
}

static inline void cb_update(const char* str) {
    if (cbs.upd && !should_abort)
        cbs.upd(cbs.arg, str);
}
static inline void cb_scan(int quality) {
    if (cbs.scan && !should_abort)
        cbs.scan(cbs.arg, quality);
}
static inline void cb_conn() {
    if (cbs.conn && !should_abort)
        cbs.conn(cbs.arg);
}

static int connect_scan_cb(void* arg, const cyw43_ev_scan_result_t* res) {
    scan_auth = res->auth_mode;
    scan_rssi = res->rssi;
    memcpy(scan_bssid, res->bssid, 6);

    if (wifi_conn_task_h)
        xTaskNotifyGive(wifi_conn_task_h);

    return 0;
}

static int do_connect() {
    int r;
    wifi_conn_task_h = xTaskGetCurrentTaskHandle();

    // start scan to find out this wifi auth type
    cyw43_wifi_scan_options_t scan_options = {0};
    strncpy((char*) scan_options.ssid, ssid, 31);
    scan_options.ssid_len = strlen(ssid);

    int conn_error = PICO_ERROR_TIMEOUT;
    char buf_upd[48];

    cb_update("Szukanie...");

    r = cyw43_wifi_scan(
            &cyw43_state,
            &scan_options,
            nullptr,
            connect_scan_cb);

    if (r) {
        puts("wifi: cb_conn failed (cb_scan failed)");
        cb_update("Nie znaleziono sieci");
        goto end;
    }

    r = ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(WIFI_CONN_SCAN_TIMEOUT_MS));
    if (r == pdFALSE) {
        puts("wifi: cb_conn failed (cb_scan timeout)");
        cb_update("Nie znaleziono sieci");
        goto end;
    }

    cb_scan(rssi_to_percent(scan_rssi));

    if (connected_same()) {
        conn_error = PICO_ERROR_NONE;
        goto already_connected;
    }

    cb_update("Łączenie...");

    for (int i=0; i<WIFI_CONN_TRIES; i++) {
        if (should_abort)
            goto end;

        r = cyw43_arch_wifi_connect_timeout_ms(
                ssid,
                pwd,
                connect_auth(),
                WIFI_CONN_TRY_TIMEOUT_MS);

        if (r == 0) {
            conn_error = PICO_ERROR_NONE;
            break;
        }
        else {
            printf("wifi: cb_conn try=%d/%d failed reason=%d\n", i+1, WIFI_CONN_TRIES, r);

            sprintf(buf_upd, "Próba %d/%d", i+1, WIFI_CONN_TRIES);
            cb_update(buf_upd);

            if (r != PICO_ERROR_TIMEOUT)
                conn_error = r; // save only "real" errors
                                // (value of conn_error defaults to timeout anyway)
        }
    }

    if (conn_error != PICO_ERROR_NONE) {
        puts("wifi: cb_conn giving up");

        sprintf(buf_upd, "Połaczenie nieudane: %s (kod %d)", err_to_string(conn_error), conn_error);
        cb_update(buf_upd);
        connect_clear();

        goto end;
    }

already_connected:
    cb_update("Połączono");
    cb_conn();

end:
    wifi_conn_task_h = nullptr;
    return conn_error;
}

int connect_blocking(const char* ssid_, const char* pwd_, cb_fns cbs_) {
    connect_prepare(ssid_, pwd_, cbs_);
    return do_connect();
}

static void connect_task(void* arg) {
    do_connect();
    printf("wifi cb_conn unused stack: %ld\n", uxTaskGetStackHighWaterMark(nullptr));
    vTaskDelete(nullptr);
}

void connect_async(const char* ssid_, const char* pwd_, cb_fns cbs_) {
    assert(wifi_conn_task_h == nullptr);

    connect_prepare(ssid_, pwd_, cbs_);

    xTaskCreate(
            connect_task,
            "wifi conn",
            STACK_WIFI_CONN,
            nullptr,
            PRI_WIFI_CONN,
            &wifi_conn_task_h);
}

void abort() {
    if (!wifi_conn_task_h)
        return;

    should_abort = true;
    xTaskNotifyGive(wifi_conn_task_h);
}

void init() {
    int r;
    r = cyw43_arch_init();
    assert(r == 0);

    cyw43_arch_enable_sta_mode();

    wifi_conn_task_h = nullptr;
}

bool is_in_progress() {
    return wifi_conn_task_h != nullptr;
}

bool is_connected_link() {
    return cyw43_wifi_link_status(&cyw43_state, CYW43_ITF_STA) == CYW43_LINK_JOIN;
}

bool is_connected_ip() {
    return cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA) == CYW43_LINK_UP;
}

bool is_connected_to(const char* ssid_) {
    return strncmp(ssid, ssid_, WIFI_SSID_MAX_LEN) == 0;
}

int connected_quality() {
    int32_t rssi;
    cyw43_wifi_get_rssi(&cyw43_state, &rssi);
    return rssi_to_percent(rssi);
}

const char* err_to_string(int error) {
    switch (error) {
        case PICO_ERROR_TIMEOUT:
            return "timeout";

        case PICO_ERROR_BADAUTH:
            return "złe hasło";

        default:
            return "nieznany błąd";
    }
}

const struct icon* quality_to_icon(int quality) {
    if (quality < 0)
        return nullptr;
    if (quality >= 100)
        return icon_wifi[3];

    return icon_wifi[quality / 25];
}

} // namespace