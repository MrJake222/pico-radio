#include "icy.hpp"

#include <cstdio>
#include <util.hpp>

void ICY::begin(int hdr_len, int metaint) {
    printf("ICY: hdr len %d metaint %d\n", hdr_len, metaint);
    next = hdr_len + metaint;
    step = metaint;
}

int ICY::read(volatile CircularBuffer& cbuf) {
    if (next > cbuf.written_bytes_total()) {
        // icy outside the region
        // just wait for next chunk
        return -1;
    }

    int o = (int)(next % cbuf.size);
    uint8_t* icy = cbuf.ptr_at(o);
    uint8_t icy_len = 1 + *icy * 16; // including size byte

    if (next + icy_len > cbuf.written_bytes_total()) {
        // icy not fully in the region
        // wait for next chunk (don't move next)
        return -1;
    }

    if (icy_len <= ICY_BUF_LEN) {
        cbuf.read_arb(o, (uint8_t*)buf, icy_len);
        handle_icy(icy_len);
    }
    else {
        puts("ICY: too big");
    }

    cbuf.remove_written(o, icy_len);
    next += step;
    return 0;
}

void ICY::handle_icy(int len) {
    if (len > 0) {
        puts("\nICY:");
        debug_print((uint8_t*) buf, 0, len, 0);
        puts("");
    }
}
