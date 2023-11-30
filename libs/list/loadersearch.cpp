#include "loadersearch.hpp"

#include <listm3u.hpp>
#include <listpls.hpp>
#include <util.hpp>

void LoaderSearch::update(int provider_idx, int server_idx, int max_servers) {
    if (should_abort)
        return;

    if (upd_cb)
        upd_cb(get_cb_arg(), provider_idx, server_idx, max_servers);
}

#define MAX_SERVERS     3

struct provider {
    int server_count;
    const char* servers[MAX_SERVERS];
};

// server urls must contain name(%s) limit(%d) and offset(%d) wildcards
static const struct provider providers[] = {
        // {
        //     .server_count = 2,
        //     .servers = {
        //             "http://npc.k21a.mrwski.eu:8080/search",
        //             "http://bpi.k21a.mrwski.eu:8080/search",
        //     }
        // },

        {
            .server_count = 3,
            .servers = {
                    "http://de1.api.radio-browser.info/m3u/stations/search?codec=mp3&name=%s&limit=%d&offset=%d",
                    "http://fr1.api.radio-browser.info/m3u/stations/search?codec=mp3&name=%s&limit=%d&offset=%d",
                    "http://at1.api.radio-browser.info/m3u/stations/search?codec=mp3&name=%s&limit=%d&offset=%d",
            }
        }
};

int LoaderSearch::get_provider_count() {
    return sizeof(providers) / sizeof(struct provider);
}

int LoaderSearch::get_station_count_per_provider() {
    return MAX_ENTRIES / get_provider_count();
}

void client_err_cb(void* arg, int err) {
    printf("rs: client err %d\n", err);
    ((LoaderSearch*) arg)->client_errored = true;
}

static List* query_url(HttpClientPico& client, const char* url, ListEntry* entries, int max_entries,
        volatile bool& should_abort, volatile bool& client_errored) {
    client_errored = false;
    int r = client.get(url);
    if (r) {
        printf("querying failed for url %s\n", url);
        client.close();
        return nullptr;
    }

    List* list;

    if (strcmp(client.get_content_type(), "audio/mpegurl") == 0) {
        // .m3u file
        list = &listm3u;
    }
    else if (strcmp(client.get_content_type(), "audio/scpls") == 0 || strcmp(client.get_content_type(), "audio/x-scpls") == 0) {
        // .pls file
        list = &listpls;
    }
    else {
        printf("unsupported type of radio listing: %s\n", client.get_content_type());
        client.close();
        return nullptr;
    }

    list->begin(entries, max_entries);
    r = list->consume_all(&client, should_abort, client_errored);
    if (r < 0) {
        // failed
        list = nullptr;
    }

    r = client.close();
    if (r) {
        puts("client close failed");
        return nullptr;
    }

    return list;
}

void LoaderSearch::task() {
    
    // load stations from all URLs
    update(0, 0, MAX_SERVERS);

    int errored = 0;
    for (int pi=0; pi<get_provider_count(); pi++) {
        if (should_abort)
            break;

        struct provider provider = providers[pi];
        List* list = nullptr;

        for (int si=0; si<provider.server_count; si++) {
            if (should_abort)
                break;

            // loop over one server
            // if already good, break
            if (list)
                break;

            snprintf(url_buf, SEARCH_URL_BUF_LEN, provider.servers[si],
                     query_enc,                                     // query string
                     get_station_count_per_provider(),              // limit
                     get_station_count_per_provider() * page);      // offset

            client_begin_set_callback();
            list = query_url(client, url_buf,
                             entries + get_entries_offset(),
                             entries_max - get_entries_offset(),
                             should_abort, client_errored);

            // when done server
            update(pi,
                   si+1,
                   provider.server_count);
        }

        // when done provider
        update(pi+1,
               provider.server_count,
               provider.server_count);

        if (!list) {
            errored++;
            continue;
        }

        // printf("done loading url, loaded %d stations\n", list->stations_found);
        set_next_entry(list->get_entries_found());

        if (!can_fit_more_entries()) {
            puts("rs: maxed out stations, done");
            break;
        }
    }

    printf("done loading all, loaded %d stations, %d providers errored\n", get_entries_offset(), errored);
    // for (int i=0; i<entries_offset; i++) {
    //     printf("uuid %s name %32s url %s\n", entries[i].uuid, entries[i].name, entries[i].url);
    // }

    call_all_loaded(errored);
}

void LoaderSearch::client_begin_set_callback() {
    client.begin();
    client.set_err_cb(client_err_cb, this);
}

void LoaderSearch::begin(const char* query_) {
    Loader::begin();
    url_encode_string(query_enc, query_);
}

void LoaderSearch::load_abort() {
    Loader::load_abort();
    client.try_abort();
}

int LoaderSearch::check_entry_url(int i) {
    // handle playlists
    // some of the stations are in *.pls format (playlist, a couple of different streams)
    // we need to load this files and choose random stream from them

    const char* url = entries[i].get_url();
    const char* ext = url + strlen(url) - 4;
    if (strcmp(ext, ".pls") == 0) {
        client_begin_set_callback();
        List* list = query_url(client, url,
                               entries_pls, entries_pls_max,
                               should_abort, client_errored);

        if (!list)
            return -1;

        // printf("done loading pls, loaded %d stations\n", list->stations_found);
        list->select_random(&entries[i]);
    }

    return 0;
}
