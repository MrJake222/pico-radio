#pragma once

#include <pico/cyw43_arch.h>

#include <loader.hpp>

class LoaderWifiScan : public Loader {

    // if dry running, the results are not added to entries
    bool dry_run;

    int scan_networks();

    int get_entry_count_whole() override;
    void task() override;

    // cyw43 callback function
    friend int scan_res_cb(void* arg, const cyw43_ev_scan_result_t* res);
    // used by callback to set results
    void set_result(const char* ssid);

public:
    LoaderWifiScan(ListEntry* entries_, int entries_max_)
            : Loader(entries_, entries_max_)
    { }
};
