#pragma once

#include <list.hpp>

typedef void(*all_ld_cb_fn)(void* arg, int errored);

class ListLoader {

    void* cb_arg;
    all_ld_cb_fn all_loaded_cb;

    friend void ll_task(void* arg);
    virtual void task() = 0;

protected:
    // this should be called by subclass begin method
    void begin_();

    void call_all_loaded(int errored);

    volatile bool should_abort;

    struct station* const stations;
    const int stations_count;
    int stations_offset;

public:
    ListLoader(struct station* stations_, int stations_count_)
        : stations(stations_)
        , stations_count(stations_count_)
        { }

    // set callback to call when all loading is done
    void set_all_loaded_cb(void* arg, all_ld_cb_fn cb);

    // start loading stations
    virtual void load_stations();
    // stop loading stations
    virtual void load_abort();

    int get_station_count() { return stations_offset; }
    const char* get_station_name(int i) { return stations[i].name; }
    // can be overridden to handle *.pls format
    virtual const char* get_station_url(int i) { return stations[i].url; }
};
