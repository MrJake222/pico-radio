#pragma once

#include <httpclientpico.hpp>

class RadioSearch {

    volatile CircularBuffer* raw_buf;
    HttpClientPico client;

public:
    RadioSearch(volatile CircularBuffer& http_buf)
        : client(http_buf)
        { }

    void begin(volatile CircularBuffer* raw_buf);

    // used in callback
    void raw_buf_write_cb(unsigned int bytes);
};
