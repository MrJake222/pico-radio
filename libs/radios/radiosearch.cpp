#include "radiosearch.hpp"

#include <listm3u.hpp>
#include <listpls.hpp>

static const char* urls[] = {
        "http://npc.k21a.mrwski.eu:8080/search",
        "http://at1.api.radio-browser.info/m3u/stations/search?codec=mp3&limit=64&offset=0&name=%s"
};

static const int url_count = sizeof(urls) / sizeof(char*);

void rs_raw_buf_write_cb(void* arg, unsigned int bytes) {
    // called directly from lwip callback
    auto rs = (RadioSearch*) arg;
    rs->notify();
}

void client_err_cb(void* arg, int err) {
    // called directly from lwip callback
    auto rs = (RadioSearch*) arg;
    rs->notify();
}

static List* query_url(HttpClientPico& client, const char* url, volatile CircularBuffer& raw_buf, struct station* stations, int max_stations) {
    raw_buf.reset_only_data();
    int r = client.get(url);
    if (r) {
        printf("querying url %s failed\n", url);
        client.close();
        return nullptr;
    }

    List* list;

    if (strcmp(client.get_content_type(), "audio/mpegurl") == 0) {
        // .m3u file
        list = &listm3u;
    }
    else if (strcmp(client.get_content_type(), "audio/scpls") == 0) {
        // .pls file
        list = &listpls;
    }
    else {
        printf("unsupported type of radio listing: %s\n", client.get_content_type());
        client.close();
        return nullptr;
    }

    list->begin(&raw_buf,
                stations,
                max_stations);

    while (raw_buf.read_bytes_total() < client.get_content_length()) {
        // loop until all content data has been read
        
        ListError lr = list->try_consume();

        if (lr == ListError::NO_DATA) {
            // buffer underflow -> wait for more data
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
            if (client.is_err()) {
                client.close();
                puts("receive error");
                return nullptr;
            }
        }

        else if (lr == ListError::OK_ABORT) {
            // buffer maxed out, don't waste more time
            break;
        }

        // rest of the values ignored (like OK and IGNORE)
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
        
        List* list = query_url(rs->client, rs->url_buf, rs->cbuf,
                               rs->stations + rs->stations_offset,
                               MAX_STATIONS - rs->stations_offset);

        if (!list) {
            errored++;
            continue;
        }

        // printf("done loading url, loaded %d stations\n", list->stations_found);
        rs->stations_offset += list->stations_found;

        if (rs->stations_offset == MAX_STATIONS) {
            puts("maxed out stations, done");
            break;
        }
    }

    printf("done loading all, loaded %d stations, %d providers errored\n", rs->stations_offset, errored);
    // for (int i=0; i<rs->stations_offset; i++) {
    //     printf("uuid %s name %32s url %s\n", rs->stations[i].uuid, rs->stations[i].name, rs->stations[i].url);
    // }

    if (rs->all_loaded_cb && !rs->should_abort)
        rs->all_loaded_cb(rs->cb_arg, errored);

    rs->search_task = nullptr;
    vTaskDelete(nullptr);
}

void RadioSearch::notify() {
    if (!search_task) {
        puts("rs: no task to notify");
        return;
    }

    xTaskNotifyGive(search_task);
}

void RadioSearch::begin(const char* query_) {
    query = query_;

    cbuf.reset_with_cb();
    cbuf.set_write_ack_callback(this, rs_raw_buf_write_cb);

    client.begin();
    client.set_err_cb(client_err_cb, this);

    should_abort = false;

    stations_offset = 0;
    all_loaded_cb = nullptr;
}

void RadioSearch::load_stations() {
    xTaskCreate(rs_search_task,
                "search",
                configMINIMAL_STACK_SIZE * 4,
                this,
                PRI_RADIO_SEARCH,
                &search_task);
}

void RadioSearch::load_abort() {
    should_abort = true;
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
        List* list = query_url(client, url, cbuf, stations_pls, MAX_STATIONS_PLS);
        if (!list)
            return nullptr;

        // printf("done loading pls, loaded %d stations\n", list->stations_found);
        list->select_random(&stations[i]);
    }

    return stations[i].url;
}
