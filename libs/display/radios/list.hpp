#pragma once

#include <config.hpp>
#include <circularbuffer.hpp>
#include <datainterface.hpp>

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

    char line[LIST_MAX_LINE_LENGTH + 1];

    struct station* stations;
    int stations_len;
    int stations_found;

    // reads line and passes it to <consume_format>
    ListError consume(DataInterface* di);
    // reads format line-by-line
    virtual ListError consume_format(char* line) = 0;

    // write header of a format
    virtual ListError produce_preamble(DataInterface* di, char* scratch) { return ListError::OK; }
    // writes format station-by-station
    virtual ListError produce_format(DataInterface* di, char* scratch) = 0;

protected:
    void set_current_uuid(const char* p);
    void set_current_name(const char* p);
    void set_current_url(const char* p);
    void set_next_station() { stations_found++; }

    const char* get_current_name();
    const char* get_current_url();
    void set_prev_station() { stations_found--; }

public:
    void begin(struct station* stations_, int stations_len_, int stations_found_ = 0) {
        stations = stations_;
        stations_len = stations_len_;
        stations_found = stations_found_;
    }

    int consume_all(DataInterface* di, volatile bool& abort, volatile bool& error);
    int produce_all(DataInterface* di);

    int get_stations_found() { return stations_found; }
    void select_random(struct station* ts);
};
