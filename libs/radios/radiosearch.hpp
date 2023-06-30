#pragma once

#include <config.hpp>
#include <httpclientpico.hpp>
#include <list.hpp>

#include <FreeRTOS.h>
#include <task.h>

class RadioSearch {

    volatile CircularBuffer* raw_buf;

    HttpClientPico client;
    TaskHandle_t search_task;

    const char* query;
    char url_buf[SEARCH_URL_BUF];

    struct station stations[MAX_STATIONS];
    int stations_offset;

    struct station stations_pls[MAX_STATIONS_PLS];

public:
    RadioSearch(volatile CircularBuffer& http_buf)
        : client(http_buf)
        { }

    void begin(volatile CircularBuffer* raw_buf_, const char* query_);

    friend void rs_raw_buf_write_cb(void* arg, unsigned int bytes);
    friend void rs_search_task(void* arg);
};
