#include "icy.hpp"

#include <cstdio>
#include <cstring>

void ICY::begin() {
    started = false;
    strcpy(buf, "");
}

int ICY::start(int hdr_len, int metaint) {
    printf("ICY: hdr len %d metaint %d\n", hdr_len, metaint);
    next = hdr_len + metaint;
    step = metaint;

    buf_mutex = xSemaphoreCreateMutex();
    if (!buf_mutex)
        return -1;

    started = true;
    return 0;
}

int ICY::read(volatile CircularBuffer& cbuf) {
    if (next > cbuf.written_bytes_total()) {
        // icy outside the region
        // just wait for next chunk
        return -1;
    }

    int o = (int)(next % cbuf.size);
    uint8_t* icy = cbuf.ptr_at(o);
    int icy_len = 1 + *icy * 16; // including size byte

    if (next + icy_len > cbuf.written_bytes_total()) {
        // icy not fully in the region
        // wait for next chunk (don't move next)
        return -1;
    }

    if (icy_len <= ICY_BUF_LEN) {
        if (icy_len > 1) {
            // copy only content
            xSemaphoreTake(buf_mutex,
                           1000 / portTICK_PERIOD_MS);

            cbuf.read_arb(o + 1, (uint8_t*) buf, icy_len - 1);

            xSemaphoreGive(buf_mutex);
        }

        // silently ignore 0-len ICY tags (but still consume them below)
    }
    else {
        puts("ICY: too big");
    }

    cbuf.remove_written(o, icy_len);
    next += step;
    return 0;
}

int ICY::get_stream_title(char* title, int title_len) const {
    char buf_priv[ICY_BUF_LEN];

    if (!started)
        return -1;

    xSemaphoreTake(buf_mutex, portMAX_DELAY);
    memcpy(buf_priv, buf, ICY_BUF_LEN);
    xSemaphoreGive(buf_mutex);

    // parse
    char* name = buf_priv;
    char* val;
    char* end;

    // search for StreamTitle tag
    while (true) {
        val = strchr(name, '=');
        if (!*val)
            return -1;

        *val++ = '\0'; // replace = with \0 and skip

        end = strchr(val, ';');
        if (!*end)
            return -1;

        *end++ = '\0'; // replace ; with \0 and skip

        if (strcmp(name, "StreamTitle") == 0)
            break;

        name = end;
    }

    // beginning of the tag
    val = strchr(val, '\'');
    if (val) {
        // quote found
        // strip val of '
        val += 1;                     // skip first '
        *strrchr(val, '\'') = '\0';   // trim last '
    }

    strncpy(title, val, title_len);
    return 0;
}