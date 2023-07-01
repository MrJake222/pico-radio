#include <decodefile.hpp>

#include <pico/platform.h>
#include <cstdio>
#include <f_util.h>

static void fs_err(FRESULT fr, const char* tag) {
    printf("%s: %s (id=%d)\n", tag, FRESULT_str(fr), fr);
}

void DecodeFile::begin(const char* path_, Format* format_) {
    DecodeBase::begin(path_, format_);

    eof = false;
}

int DecodeFile::play_() {
    fr = f_open(&fp, path, FA_READ);
    if (fr != FR_OK) {
        fs_err(fr, "f_open");
        return -1;
    }

    // preload with file data
    // this must load less than whole buffer
    // because read == write is undefined behavior
    int r = check_buffer();
    if (r)
        return r;

    return 0;
}

int DecodeFile::stop() {
    fr = f_close(&fp);
    if (fr != FR_OK) {
        fs_err(fr, "f_close");
        return -1;
    }

    return DecodeBase::stop();
}

int DecodeFile::load_buffer(int bytes) {
//     printf("loading, offset %ld  load_at %ld\n", format->raw_buf.get_read_offset(), format->raw_buf.get_write_offset());

    uint read;
    fr = f_read(&fp, format->raw_buf.write_ptr(), bytes, &read);
    if (fr != FR_OK) {
        fs_err(fr, "f_read");
        return -1;
    }

    if (read < bytes) {
        printf("EOF\n");
        eof = true;
    }

    format->raw_buf.write_ack(read);

    // printf("loaded,  offset %ld  load_at %ld\n", format->raw_buf.get_read_offset(), format->raw_buf.get_write_offset());
    return 0;
}

int DecodeFile::check_buffer() {
    if (format->raw_buf.data_left() < format->raw_buf.size / 2) {
        if (eof) {
            // eof, after wrap, set end-of-playback
            format->set_eop();
        }

        else {
            // no eof -> just load more
            return load_buffer(format->raw_buf.size/2);
        }
    }

    return 0;
}

void DecodeFile::raw_buf_just_read(unsigned int bytes) {
    DecodeBase::raw_buf_just_read(bytes);
    int r = check_buffer();
    if (r) {
        printf("check_buffer failed, ending playback");
        notify_playback_end(true);
    }
}