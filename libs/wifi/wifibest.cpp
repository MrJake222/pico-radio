#include "wifibest.hpp"

#include <wificonnect.hpp>
#include <wifiscan.hpp>
#include <m3u.hpp>

#include <static.hpp>

#include <FreeRTOS.h>
#include <task.h>
#include <screenmng.hpp>
#include <lwip/apps/sntp.h>

namespace wifi {

struct status {
    bool conn_attempted;
    int conn_error;
};

static void best_cb_fn(void* arg, const char* res) {
    auto status = (struct status*) arg;
    char pwd[WIFI_PWD_MAX_LEN + 1];
    int r;

    const char* ssid = WifiScan::format_decode_ssid(res);
    int q = WifiScan::format_decode_quality(res);

    r = m3u::get(
            PATH_WIFI,
            ssid,
            pwd,
            WIFI_PWD_MAX_LEN);

    if (r) {
        printf("wifi-best: no saved wifi named %s\n", ssid);
        return;
    }

    printf("wifi-best: saved match named %s quality %d%%\n", ssid, q);

    r = connect_blocking(
            ssid,
            pwd,
            {nullptr, nullptr, nullptr});

    status->conn_attempted = true;
    status->conn_error = r;
}

static void connect_best_task(void* arg) {
    int r;

    // defined on stack, because this task is really short-lived and
    // will exit after connecting (won't hold memory for too long)
    LfsAccess acc(get_lfs());
    LfsAccess acc2(get_lfs());
    WifiScan scan(acc, acc2);

    printf("connect_best_saved unused stack entry: %ld, lfsaccess: %d frames\n", uxTaskGetStackHighWaterMark(nullptr), sizeof(acc) / sizeof(StackType_t));

    scan.begin();

    r = scan.scan(FLAG_FALSE);
    if (r) {
        puts("wifi-best: scan failed");
        return;
    }

    printf("connect_best_saved unused stack scan: %ld\n", uxTaskGetStackHighWaterMark(nullptr));

    struct status status = { false, 0 };

    for (int skip=0; !is_connected_link(); skip++) {
        r = scan.get_smallest_n_skip_k(FLAG_FALSE, 1, skip, &status, best_cb_fn);
        if (r < 0) {
            puts("wifi-best: read failed");
            return;
        }

        if (r == 0) {
            // no stations loaded
            break;
        }

        // lfs::read returned
        // the callback was called and it also returned
        // the flags are correctly set now
    }

    if (!is_connected_link()) {
        if (status.conn_attempted) {
            // attempted and failed
            char buf[52];
            sprintf(buf, "Błąd połączenia z zapisaną siecią: %s (kod %d)\n",
                    err_to_string(status.conn_error),
                    status.conn_error);

            screenmng_show_error(buf);
        }
    }
    else {
        // has link
        int tries = 100;
        while (!is_connected_ip() && tries--)
            vTaskDelay(pdMS_TO_TICKS(100));

        if (is_connected_ip() && arg) {
            ((conn_cb)arg)();
        }
    }

    printf("connect_best_saved unused stack end: %ld\n", uxTaskGetStackHighWaterMark(nullptr));
    vTaskDelete(nullptr);
}

void connect_best_saved(conn_cb cb) {
    xTaskCreate(
            connect_best_task,
            "wifi best",
            STACK_WIFI_BEST,
            (void*)cb,
            PRI_WIFI_BEST,
            nullptr);
}

} // namespace