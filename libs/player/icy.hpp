#pragma once

#include <config.hpp>
#include <cstdint>
#include <circularbuffer.hpp>

class ICY {

    char buf[ICY_BUF_LEN];

    // holds how many bytes should be read to hit next icy
    b_type next;
    // meta-int, how separated are icy fragments
    int step;

    void handle_icy(int len);

public:
    void begin(int hdr_len, int metaint);

    // returns 0 when read of ICY chunk successful
    // -1 on no chunk detected
    int read(volatile CircularBuffer& cbuf);
};
