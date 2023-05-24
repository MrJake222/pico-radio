#include "decodefile.hpp"

#include <pico/platform.h>
#include <cstdio>
#include "f_util.h"
#include "config.hpp"

static void fs_err(FRESULT fr, const char* tag) {
    panic("%s: %s (id=%d)\n", tag, FRESULT_str(fr), fr);
}

void DecodeFile::begin(const char* path_, Format* format_) {
    DecodeBase::begin(path_, format_);

    fr = f_open(&fp, path, FA_READ);
    if (fr != FR_OK) {
        fs_err(fr, "f_open");
    }

    eof = false;
}

void DecodeFile::end() {
    DecodeBase::end();

    f_close(&fp);
}

void DecodeFile::load_buffer(int bytes) {
//     printf("loading, offset %ld  load_at %ld\n", format->raw_buf.get_read_offset(), format->raw_buf.get_write_offset());

    uint read;
    fr = f_read(&fp, format->raw_buf.write_ptr(), bytes, &read);
    if (fr != FR_OK) {
        fs_err(fr, "f_read");
    }

    if (read < bytes) {
        printf("EOF\n");
        eof = true;
    }

    format->raw_buf.write_ack(read);

    // printf("loaded,  offset %ld  load_at %ld\n", format->raw_buf.get_read_offset(), format->raw_buf.get_write_offset());
}

bool DecodeFile::data_buffer_watch() {
    DecodeBase::data_buffer_watch();

    if (format->raw_buf.data_left() < format->raw_buf.size / 2) { // TODO <=
        // low on data

        if (eof) {
            // eof, after wrap, set end-of-playback
            format->set_eop();
            // end loop
            return false;
        }

        else
            // no eof -> just load more
            load_buffer(format->raw_buf.size / 2);
    }

    // loading not occurred
    return true;
}
