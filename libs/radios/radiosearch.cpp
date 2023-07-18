#include "radiosearch.hpp"

#include <listm3u.hpp>
#include <listpls.hpp>

static const char* urls[] = {
        "http://npc.k21a.mrwski.eu:8080/search",
        "http://at1.api.radio-browser.info/m3u/stations/search?codec=mp3&limit=64&offset=0&name=%s"
};

static const int url_count = sizeof(urls) / sizeof(char*);

static List* query_url(HttpClientPico& client, const char* url, struct station* stations, int max_stations,
        volatile bool& should_abort) {
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

        if (client.is_err()) {
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

void rs_search_task(void* arg) {

    auto rs = (RadioSearch*) arg;

    // load stations from all URLs

    int errored = 0;
    for (int i=0; i<url_count; i++) {
        if (rs->should_abort)
            break;

        snprintf(rs->url_buf, SEARCH_URL_BUF_LEN, urls[i], rs->query);
        
        List* list = query_url(rs->client, rs->url_buf,
                               rs->stations + rs->stations_offset,
                               MAX_STATIONS - rs->stations_offset,
                               rs->should_abort);

        if (!list) {
            errored++;
            continue;
        }

        // printf("done loading url, loaded %d stations\n", list->stations_found);
        rs->stations_offset += list->get_stations_found();

        if (rs->stations_offset == MAX_STATIONS) {
            puts("rs: maxed out stations, done");
            break;
        }
    }

    printf("done loading all, loaded %d stations, %d providers errored\n", rs->stations_offset, errored);
    // for (int i=0; i<rs->stations_offset; i++) {
    //     printf("uuid %s name %32s url %s\n", rs->stations[i].uuid, rs->stations[i].name, rs->stations[i].url);
    // }

    if (rs->all_loaded_cb && !rs->should_abort)
        rs->all_loaded_cb(rs->cb_arg, errored);

    uint32_t min_free_stack = uxTaskGetStackHighWaterMark(nullptr);
    printf("radiosearch unused stack: %ld\n", min_free_stack);

    vTaskDelete(nullptr);
}

void RadioSearch::begin(const char* query_) {
    query = query_;

    client.begin();
    // client.set_err_cb(client_err_cb, this);

    should_abort = false;

    stations_offset = 0;
    all_loaded_cb = nullptr;
}

void RadioSearch::load_stations() {
    xTaskCreate(rs_search_task,
                "search",
                STACK_RADIO_SEARCH,
                this,
                PRI_RADIO_SEARCH,
                nullptr);
}

void RadioSearch::load_abort() {
    should_abort = true;
    client.try_abort();
}

void RadioSearch::set_all_loaded_cb(void* arg, all_ld_cb_fn cb) {
    cb_arg = arg;
    all_loaded_cb = cb;
}

const char* RadioSearch::get_station_url(int i) {
    // handle playlists
    // some of the stations are in *.pls format (playlist, a couple of different streams)
    // we need to load this files and choose random stream from them

    const char* url = stations[i].url;
    const char* ext = url + strlen(url) - 4;
    if (strcmp(ext, ".pls") == 0) {
        List* list = query_url(client, url, stations_pls, MAX_STATIONS_PLS, should_abort);
        if (!list)
            return nullptr;

        // printf("done loading pls, loaded %d stations\n", list->stations_found);
        list->select_random(&stations[i]);
    }

    return stations[i].url;
}
