#pragma once

#include <config.hpp>
#include <circularbuffer.hpp>
#include <httpclientpico.hpp>

struct station {
    char uuid[ST_UUID_LEN + 1];
    char name[ST_NAME_LEN + 1];
    char  url[ST_URL_LEN + 1];
};

enum class ListError {
    ERROR,
    OK,
    ABORT,
};

class List {

    HttpClientPico* client;
    struct station* stations;
    int stations_len;
    int stations_found;

    virtual ListError try_consume_format(char* line) = 0;

protected:
    void set_current_uuid(const char* p);
    void set_current_name(const char* p);
    void set_current_url(const char* p);
    void set_next_station() { stations_found++; }

public:
    virtual void begin(HttpClientPico* client_, struct station* stations_, int stations_len_) {
        client = client_;
        stations = stations_;
        stations_len = stations_len_;
        stations_found = 0;
    }

    ListError try_consume();

    int get_stations_found() { return stations_found; }
    void select_random(struct station* ts);
};
