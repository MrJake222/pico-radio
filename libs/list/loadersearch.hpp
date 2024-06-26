#pragma once

#include <config.hpp>
#include <httpclientpico.hpp>
#include <listentry.hpp>
#include <loader.hpp>
#include <listm3u.hpp>
#include <listpls.hpp>

typedef void(*ld_search_update_cb)(void* arg, int provider_idx, int server_idx, int max_servers);

class LoaderSearch : public Loader {

    HttpClientPico& client;

    char query_enc[PROMPT_URL_BUF_LEN];
    char url_buf[SEARCH_URL_BUF_LEN];

    ListEntry* const entries_pls;
    const int entries_pls_max;

    ListM3U& list_m3u;
    ListPLS& list_pls;

    void task() override;
    void setup_entry(ListEntry *ent) override { ent->type = le_type_radio; }

    volatile bool client_errored;
    void client_begin_set_callback();
    friend void client_err_cb(void* arg, int err);

    ld_search_update_cb upd_cb;
    void update(int provider_idx, int server_idx, int max_servers);

    int get_entry_count_whole() override { return -1; }

public:
    LoaderSearch(ListEntry* entries_, int entries_max_,
                 ListEntry* entries_pls_, int entries_pls_max_,
                 HttpClientPico& client_,
                 ListM3U& list_m3u_, ListPLS& list_pls_)
        : Loader(entries_, entries_max_)
        , client(client_)
        , entries_pls(entries_pls_)
        , entries_pls_max(entries_pls_max_)
        , list_m3u(list_m3u_)
        , list_pls(list_pls_)
        { }

    void begin(const char* query_);
    void load_abort() override;

    int check_entry_url(int i) override;

    void set_update_cb(ld_search_update_cb cb) { upd_cb = cb; }
    int get_provider_count();
    int get_station_count_per_provider();
};
