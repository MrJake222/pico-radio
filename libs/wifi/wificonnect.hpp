#pragma once

#include <icons.hpp>

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

void init();

// creates new rtos task to connect to the network, doesn't block the caller
void connect_async(const char* ssid_, const char* pwd_, cb_fns cbs_);
// uses caller rtos task, blocks it until connected or failure
int connect_blocking(const char* ssid_, const char* pwd_, cb_fns cbs_);
void abort();

bool is_in_progress();
bool is_connected_link();
bool is_connected_ip();
bool is_connected_to(const char* ssid_);
int connected_quality();

// "static" functions
const char* err_to_string(int error);
const struct icon* quality_to_icon(int quality);

} // namespace