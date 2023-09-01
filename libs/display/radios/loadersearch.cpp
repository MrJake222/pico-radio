#include "loadersearch.hpp"

#include <listm3u.hpp>
#include <listpls.hpp>
#include <util.hpp>

static const char* urls[] = {
        // "http://npc.k21a.mrwski.eu:8080/search",
        // nullptr,

        "http://de1.api.radio-browser.info/m3u/stations/search?codec=mp3&limit=64&offset=0&name=%s",
        "http://fr1.api.radio-browser.info/m3u/stations/search?codec=mp3&limit=64&offset=0&name=%s",
        "http://at1.api.radio-browser.info/m3u/stations/search?codec=mp3&limit=64&offset=0&name=%s",
        nullptr,
};

static const int url_count = sizeof(urls) / sizeof(char*);

void client_err_cb(void* arg, int err) {
    printf("rs: client err %d\n", err);
    ((LoaderSearch*) arg)->client_errored = true;
}

static List* query_url(HttpClientPico& client, const char* url, struct station* stations, int max_stations,
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

    list->begin(stations, max_stations);
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

    int errored = 0;
    for (int i=0; i<url_count; i++) {
        if (should_abort)
            break;

        List* list = nullptr;

        for (; urls[i]; i++) {
            if (should_abort)
                break;

            // loop over one server
            // if already good, skip the code, but advance pointer
            if (list)
                continue;

            snprintf(url_buf, SEARCH_URL_BUF_LEN, urls[i], query_enc);

            client_begin_set_callback();
            list = query_url(client, url_buf,
                                   stations + stations_offset,
                                   stations_max - stations_offset,
                                   should_abort, client_errored);
        }

        if (!list) {
            errored++;
            continue;
        }

        // printf("done loading url, loaded %d stations\n", list->stations_found);
        stations_offset += list->get_stations_found();

        if (stations_offset == stations_max) {
            puts("rs: maxed out stations, done");
            break;
        }
    }

    printf("done loading all, loaded %d stations, %d providers errored\n", stations_offset, errored);
    // for (int i=0; i<stations_offset; i++) {
    //     printf("uuid %s name %32s url %s\n", stations[i].uuid, stations[i].name, stations[i].url);
    // }

    if (!should_abort)
        call_all_loaded(errored);

    uint32_t min_free_stack = uxTaskGetStackHighWaterMark(nullptr);
    printf("radiosearch unused stack: %ld\n", min_free_stack);

    vTaskDelete(nullptr);
}

void LoaderSearch::client_begin_set_callback() {
    client.begin();
    client.set_err_cb(client_err_cb, this);
}

void LoaderSearch::begin(const char* query_) {
    ListLoader::begin();
    url_encode_string(query_enc, query_);
}

void LoaderSearch::load_abort() {
    ListLoader::load_abort();
    client.try_abort();
}

int LoaderSearch::check_station_url(int i) {
    // handle playlists
    // some of the stations are in *.pls format (playlist, a couple of different streams)
    // we need to load this files and choose random stream from them

    const char* url = stations[i].url;
    const char* ext = url + strlen(url) - 4;
    if (strcmp(ext, ".pls") == 0) {
        client_begin_set_callback();
        List* list = query_url(client, url,
                               stations_pls, stations_pls_count,
                               should_abort, client_errored);

        if (!list)
            return -1;

        // printf("done loading pls, loaded %d stations\n", list->stations_found);
        list->select_random(&stations[i]);
    }

    return 0;
}
