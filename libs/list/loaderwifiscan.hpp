#pragma once

#include <pico/cyw43_arch.h>

#include <loader.hpp>
#include <lfsaccess.hpp>

class LoaderWifiScan : public Loader {

    LfsAccess& acc;

    int get_entry_count_whole() override;
    void task() override;
    void setup_entry(ListEntry *ent) override { ent->type = le_type_wifi; }

    friend void lwifi_sorter_cb(void* arg, const char* res);
    // used by callback to set results
    void set_result(const char* ssid, int p);

public:
    LoaderWifiScan(ListEntry* entries_, int entries_max_,
                   LfsAccess& acc_)
            : Loader(entries_, entries_max_)
            , acc(acc_)
    { }
};
