#pragma once

#include <config.hpp>
#include <cstdint>
#include <circularbuffer.hpp>

typedef unsigned long long int ull;

class ICY {

    char buf[ICY_BUF_LEN];

    // holds how many bytes should be read to hit next icy
    ull next;
    // meta-int, how separated are icy fragments
    int step;

    void handle_icy(int len);

public:
    void begin(int hdr_len, int metaint);

    // returns 0 when read of ICY chunk successful
    // -1 on no chunk detected
    int read(volatile CircularBuffer& cbuf);
};
