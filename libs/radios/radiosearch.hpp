#pragma once

#include <config.hpp>
#include <httpclientpico.hpp>
#include <list.hpp>

#include <FreeRTOS.h>
#include <task.h>

typedef void(*all_ld_cb_fn)(void*);

class RadioSearch {

    volatile CircularBuffer* raw_buf;

    HttpClientPico client;
    TaskHandle_t search_task;

    const char* query;
    char url_buf[SEARCH_URL_BUF_LEN];

    struct station stations[MAX_STATIONS];
    int stations_offset;

    struct station stations_pls[MAX_STATIONS_PLS];

    volatile bool should_abort;

    void* cb_arg;
    all_ld_cb_fn all_loaded_cb;

public:
    RadioSearch(volatile CircularBuffer& http_buf)
        : client(http_buf)
        { }

    void begin(volatile CircularBuffer* raw_buf_, const char* query_);
    void load_stations();
    void load_abort();

    friend void rs_raw_buf_write_cb(void* arg, unsigned int bytes);
    friend void rs_search_task(void* arg);

    void set_all_loaded_cb(void* arg, all_ld_cb_fn cb);

    int get_station_count() { return stations_offset; }
    struct station* get_station(int i) { return &stations[i]; }
};
