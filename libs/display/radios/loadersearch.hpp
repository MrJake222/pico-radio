#pragma once

#include <config.hpp>
#include <httpclientpico.hpp>
#include <list.hpp>
#include <listloader.hpp>

#include <FreeRTOS.h>
#include <task.h>

typedef void(*ld_search_update_cb)(void* arg, int provider_idx, int server_idx, int max_servers);

class LoaderSearch : public ListLoader {

    HttpClientPico& client;

    char query_enc[PROMPT_URL_BUF_LEN];
    char url_buf[SEARCH_URL_BUF_LEN];

    struct station* const stations_pls;
    const int stations_pls_count;

    void task() override;

    volatile bool client_errored;
    void client_begin_set_callback();
    friend void client_err_cb(void* arg, int err);

    ld_search_update_cb upd_cb;
    void update(int provider_idx, int server_idx, int max_servers);

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

    int check_station_url(int i) override;

    void set_update_cb(ld_search_update_cb cb) { upd_cb = cb; }
    int get_provider_count();
    int get_station_count_per_provider();
};
