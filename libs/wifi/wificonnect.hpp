#pragma once

namespace wifi {

typedef void(*update_fn)(void* arg, const char* str);
typedef void(*scan_fn)(void* arg, int quality);
typedef void(*connected_fn)(void* arg);

struct cb_fns {
    void* arg;
    update_fn upd;
    scan_fn scan;
    connected_fn conn;
};

// creates new rtos task to connect to the network, doesn't block the caller
void connect_async(const char* ssid_, const char* pwd_, cb_fns cbs_);
// uses caller rtos task, blocks it until connected or failure
void connect_blocking(const char* ssid_, const char* pwd_, cb_fns cbs_);
void abort();

bool is_connected();

} // namespace