#include "wifi.hpp"

#include <pico/cyw43_arch.h>

#include <config.hpp>

#include <FreeRTOS.h>
#include <task.h>
#include <util.hpp>

namespace wifi {

static TaskHandle_t wifi_conn_task_h;

static char ssid[WIFI_SSID_MAX_LEN + 1];
static char pwd[WIFI_PWD_MAX_LEN + 1];
static uint8_t auth;
static int16_t rssi;
static uint8_t bssid[6];

static uint32_t connect_auth() {
    if (auth & 0x04)
        return CYW43_AUTH_WPA2_AES_PSK;

    if (auth & 0x02)
        return CYW43_AUTH_WPA_TKIP_PSK;

    // if (sec & 0x01)
    //     // wep
    //     return -1;

    return -1;
}

static bool connected_same() {
    int status = cyw43_wifi_link_status(&cyw43_state, CYW43_ITF_STA);
    if (status != CYW43_LINK_JOIN)
        return false; // not connected at all

    uint8_t bssid_current[6];
    cyw43_wifi_get_bssid(&cyw43_state, bssid_current);
    return memcmp(bssid, bssid_current, 6) == 0;
}

static bool should_abort;

static cb_fns cbs;
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
    auth = res->auth_mode;
    rssi = res->rssi;
    memcpy(bssid, res->bssid, 6);

    if (wifi_conn_task_h)
        xTaskNotifyGive(wifi_conn_task_h);

    return 0;
}

static void err_to_string(int error, char* buf) {
    switch (error) {
        case PICO_ERROR_TIMEOUT:
            strcpy(buf, "timeout");
            break;

        case PICO_ERROR_BADAUTH:
            strcpy(buf, "złe hasło");
            break;

        default:
            sprintf(buf, "bład kod %d", error);
            break;
    }
}

static void connect_task(void* arg) {
    int r;

    // start scan to find out this wifi auth type
    cyw43_wifi_scan_options_t scan_options = {0};
    strncpy((char*) scan_options.ssid, ssid, 31);
    scan_options.ssid_len = strlen(ssid);

    bool connected = false;
    char buf_err[16];
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

    cb_scan(rssi_to_percent(rssi));

    if (connected_same())
        goto already_connected;

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
            connected = true;
            break;
        }
        else {
            printf("wifi: cb_conn try=%d/%d failed reason=%d\n", i+1, WIFI_CONN_TRIES, r);

            err_to_string(r, buf_err);
            sprintf(buf_upd, "Próba %d/%d: %s", i+1, WIFI_CONN_TRIES, buf_err);
            cb_update(buf_upd);
        }
    }

    if (!connected) {
        puts("wifi: cb_conn giving up");

        err_to_string(r, buf_err);
        sprintf(buf_upd, "Połaczenie nieudane: %s", buf_err);
        cb_update(buf_upd);

        goto end;
    }

already_connected:
    cb_update("Połączono");
    cb_conn();

end:
    printf("wifi cb_conn unused stack: %ld\n", uxTaskGetStackHighWaterMark(nullptr));
    wifi_conn_task_h = nullptr;
    vTaskDelete(nullptr);
}

void connect(const char* ssid_, const char* pwd_, cb_fns cbs_) {

    strncpy(ssid, ssid_, WIFI_SSID_MAX_LEN);
    strncpy(pwd, pwd_, WIFI_PWD_MAX_LEN);
    cbs = cbs_;
    should_abort = false;

    xTaskCreate(
            connect_task,
            "wifi cb_conn",
            STACK_WIFI,
            nullptr,
            PRI_WIFI,
            &wifi_conn_task_h);
}

void abort() {
    if (!wifi_conn_task_h)
        return;

    should_abort = true;
    xTaskNotifyGive(wifi_conn_task_h);
}

} // namespace