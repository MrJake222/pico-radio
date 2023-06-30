#include "radiosearch.hpp"

#include <listm3u.hpp>
#include <listpls.hpp>

static const char* urls[] = {
        "http://de1.api.radio-browser.info/m3u/stations/search?codec=mp3&name=%s"
};

static const int url_count = sizeof(urls) / sizeof(char*);

void rs_raw_buf_write_cb(void* arg, unsigned int bytes) {
    // called directly from lwip callback
    auto task = ((RadioSearch*) arg)->search_task;
    if (!task) {
        puts("rs: no task to notify");
        return;
    }

    xTaskNotifyGive(task);
}

static List* query_url(HttpClientPico& client, const char* url, volatile CircularBuffer* raw_buf, struct station* stations, int max_stations) {
    raw_buf->reset_only_data();
    int r = client.get(url);
    if (r) {
        printf("querying url %s failed", url);
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

    list->begin(raw_buf,
                stations,
                max_stations);

    while (raw_buf->read_bytes_total() < client.get_content_length()) {
        // loop until all content data has been read
        
        ListError lr = list->try_consume();

        if (lr == ListError::NO_DATA) {
            // buffer underflow -> wait for more data
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
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

    for (int i=0; i<url_count; i++) {
        if (rs->should_abort)
            break;

        snprintf(rs->url_buf, SEARCH_URL_BUF_LEN, urls[i], rs->query);
        
        List* list = query_url(rs->client, rs->url_buf, rs->raw_buf,
                               rs->stations + rs->stations_offset,
                               MAX_STATIONS - rs->stations_offset);

        if (!list)
            continue;

        // printf("done loading url, loaded %d stations\n", list->stations_found);
        rs->stations_offset += list->stations_found;

        if (rs->stations_offset == MAX_STATIONS) {
            puts("maxed out stations, done");
            break;
        }
    }

    printf("done loading all, loaded %d stations\n", rs->stations_offset);
    // for (int i=0; i<rs->stations_offset; i++) {
    //     printf("uuid %s name %32s url %s\n", rs->stations[i].uuid, rs->stations[i].name, rs->stations[i].url);
    // }

    if (rs->all_loaded_cb && !rs->should_abort)
        rs->all_loaded_cb(rs->cb_arg);

    rs->search_task = nullptr;
    vTaskDelete(nullptr);
}

void RadioSearch::begin(volatile CircularBuffer* raw_buf_, const char* query_) {
    raw_buf = raw_buf_;
    query = query_;

    raw_buf->reset_with_cb();
    raw_buf->set_write_ack_callback(this, rs_raw_buf_write_cb);

    client.begin(raw_buf);
    should_abort = false;

    stations_offset = 0;
    all_loaded_cb = nullptr;
}

void RadioSearch::load_stations() {
    xTaskCreate(rs_search_task,
                "search",
                configMINIMAL_STACK_SIZE * 4,
                this,
                1,
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
        List* list = query_url(client, url, raw_buf, stations_pls, MAX_STATIONS_PLS);
        if (!list)
            return nullptr;

        // printf("done loading pls, loaded %d stations\n", list->stations_found);
        list->select_random(&stations[i]);
    }

    return stations[i].url;
}
