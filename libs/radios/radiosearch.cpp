#include "radiosearch.hpp"

#include <listm3u.hpp>
#include <listpls.hpp>

static const char* urls[] = {
        "http://npc.k21a.mrwski.eu:8080/search",
        "http://at1.api.radio-browser.info/m3u/stations/search?codec=mp3&limit=64&offset=0&name=%s"
};

static const int url_count = sizeof(urls) / sizeof(char*);

void client_err_cb(void* arg, int err) {
    printf("rs: client err %d\n", err);
    ((RadioSearch*) arg)->client_errored = true;
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

    list->begin(&client, stations, max_stations);

    while (client.more_content()) {
        // loop until all content data has been read or aborted
        if (should_abort) {
            puts("rs: abort");
            break;
        }

        ListError lr = list->consume();

        if (lr == ListError::ERROR) {
            puts("rs: error");
            client.close();
            return nullptr;
        }

        else if (lr == ListError::ABORT) {
            // buffer maxed out, don't waste more time
            puts("rs: maxed out stations");
            break;
        }

        if (client_errored) {
            puts("rs: client error");
            client.close();
            return nullptr;
        }
    }

    r = client.close();
    if (r) {
        puts("client close failed");
        return nullptr;
    }

    return list;
}

void RadioSearch::task() {
    
    // load stations from all URLs

    int errored = 0;
    for (int i=0; i<url_count; i++) {
        if (should_abort)
            break;

        snprintf(url_buf, SEARCH_URL_BUF_LEN, urls[i], query);
        
        List* list = query_url(client, url_buf,
                               stations + stations_offset,
                               stations_count - stations_offset,
                               should_abort, client_errored);

        if (!list) {
            errored++;
            continue;
        }

        // printf("done loading url, loaded %d stations\n", list->stations_found);
        stations_offset += list->get_stations_found();

        if (stations_offset == stations_count) {
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

void RadioSearch::begin(const char* query_) {
    ListLoader::begin_();
    query = query_;

    client.begin();
    client.set_err_cb(client_err_cb, this);
}

void RadioSearch::load_abort() {
    ListLoader::load_abort();
    client.try_abort();
}

const char* RadioSearch::get_station_url(int i) {
    // handle playlists
    // some of the stations are in *.pls format (playlist, a couple of different streams)
    // we need to load this files and choose random stream from them

    const char* url = stations[i].url;
    const char* ext = url + strlen(url) - 4;
    if (strcmp(ext, ".pls") == 0) {
        List* list = query_url(client, url,
                               stations_pls, stations_pls_count,
                               should_abort, client_errored);

        if (!list)
            return nullptr;

        // printf("done loading pls, loaded %d stations\n", list->stations_found);
        list->select_random(&stations[i]);
    }

    return stations[i].url;
}
