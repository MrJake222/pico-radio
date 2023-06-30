#pragma once

#include <config.hpp>
#include <circularbuffer.hpp>

struct station {
    char uuid[ST_UUID_LEN + 1];
    char name[ST_NAME_LEN + 1];
    char  url[ST_URL_LEN + 1];
};

enum class ListError {
    OK,
    OK_IGNORE,
    OK_ABORT,
    NO_DATA
};

class List {

protected:
    volatile CircularBuffer* buf;

public:
    struct station* stations;
    int stations_len;
    int stations_found;

    virtual void begin(volatile CircularBuffer* buf_, struct station* stations_, int stations_len_) {
        buf = buf_;
        stations = stations_;
        stations_len = stations_len_;
        stations_found = 0;
    }

    // returns -1 when no data was consumed
    virtual ListError try_consume() = 0;
    // returns 1 when init went good, other values need to be propagated
    ListError try_consume_pre(int& eol);
    // always returns 0 (for now)
    void try_consume_post(int eol);

    void set_current_uuid(const char* p);
    void set_current_name(const char* p);
    void set_current_url(const char* p);

    void select_random(struct station* ts);
};
