#pragma once

#include <list.hpp>

typedef void(*all_ld_cb_fn)(void* arg, int errored);

class ListLoader {

    void* cb_arg;
    all_ld_cb_fn all_loaded_cb;

    // wrapper to call task() from rtos
    friend void ll_task(void* arg);
    // real task, should respect <should_abort>
    virtual void task() = 0;

protected:
    void* get_cb_arg() { return cb_arg; }
    void call_all_loaded(int errored);

    volatile bool should_abort;

    struct station* const stations;
    const int stations_max;
    int stations_offset;

    int page;

public:
    ListLoader(struct station* stations_, int stations_max_)
        : stations(stations_)
        , stations_max(stations_max_)
        { }

    void begin();

    // set callback to call when all loading is done
    void set_cb_arg(void* arg) { cb_arg = arg; }
    void set_all_loaded_cb(all_ld_cb_fn cb) { all_loaded_cb = cb; }

    // start loading stations
    virtual void load_stations(int page_);
    // stop loading stations
    virtual void load_abort();

    int get_station_count() { return stations_offset; }
    const struct station* get_station(int i) { return &stations[i]; }
    // can be overridden to handle *.pls format
    virtual int check_station_url(int i) { return 0; }

    // should return max pages to switch to
    // return 1 to disable, -1 to infinite pages
    // must be called in all_loaded callback
    virtual int get_page_count() = 0;
};
