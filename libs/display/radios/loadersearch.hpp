#pragma once

#include <config.hpp>
#include <httpclientpico.hpp>
#include <list.hpp>
#include <listloader.hpp>

#include <FreeRTOS.h>
#include <task.h>

typedef void(*all_ld_cb_fn)(void* arg, int errored);

class LoaderSearch : public ListLoader {

    HttpClientPico& client;

    const char* query;
    char url_buf[SEARCH_URL_BUF_LEN];

    struct station* const stations_pls;
    const int stations_pls_count;

    void task() override;

    volatile bool client_errored;
    friend void client_err_cb(void* arg, int err);

public:
    LoaderSearch(struct station* stations_, int stations_count_,
                 struct station* stations_pls_, int stations_pls_count_,
                 HttpClientPico& client_)
        : ListLoader(stations_, stations_count_)
        , client(client_)
        , stations_pls(stations_pls_)
        , stations_pls_count(stations_pls_count_)
        { }

    void begin(const char* query_);
    void load_abort() override;

    const char* get_station_url(int i) override;
};
