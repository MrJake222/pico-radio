#pragma once

#include <config.hpp>
#include <cstdint>
#include <circularbuffer.hpp>

#include <FreeRTOS.h>
#include <semphr.h>

typedef void(*icy_new_cb)();

#define ICY_BUF_LEN   (PLAYER_META_BUF_LEN)

class ICY {

    char buf[ICY_BUF_LEN];

    // holds how many bytes should be read to hit next icy
    b_type next;
    // meta-int, how separated are icy fragments
    int step;

    SemaphoreHandle_t buf_mutex;

    bool started;

public:
    void begin();

    int start(int hdr_len, int metaint);

    // returns ICY chunk length on successful read
    // -1 on no chunk detected
    int read(volatile CircularBuffer& cbuf);

    int get_stream_title(char* title, int title_len) const;
};
