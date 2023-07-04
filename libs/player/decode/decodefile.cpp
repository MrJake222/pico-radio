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
    file_open = false;
}

int DecodeFile::play() {
    fr = f_open(&fp, path, FA_READ);
    if (fr != FR_OK) {
        fs_err(fr, "f_open");
        return -1;
    }

    file_open = true;

    // preload with file data
    // this must load less than whole buffer
    // because read == write is undefined behavior
    int r = load_buffer(cbuf.size);
    if (r)
        return -1;

    // no wait required as we just loaded whole buffer

    return 0;
}

void DecodeFile::stop() {
    if (file_open) {
        fr = f_close(&fp);
        if (fr != FR_OK) {
            fs_err(fr, "f_close");
        }
    }

    DecodeBase::stop();
}

int DecodeFile::load_buffer(int bytes) {
//     printf("loading, offset %ld  load_at %ld\n", format->raw_buf.get_read_offset(), format->raw_buf.get_write_offset());

    uint read;
    fr = f_read(&fp, cbuf.write_ptr(), bytes, &read);
    if (fr != FR_OK) {
        fs_err(fr, "f_read");
        return -1;
    }

    if (read < bytes) {
        printf("EOF\n");
        eof = true;
    }

    cbuf.write_ack(read);

    // printf("loaded,  offset %ld  load_at %ld\n", format->raw_buf.get_read_offset(), format->raw_buf.get_write_offset());
    return 0;
}

int DecodeFile::check_buffer() {
    if (cbuf.data_left() < cbuf.size / 2) {
        if (eof) {
            // eof, after wrap, set end-of-playback
            set_eop();
        }

        else {
            // no eof -> just load more
            return load_buffer(cbuf.size / 2);
        }
    }

    return 0;
}

void DecodeFile::ack_bytes(uint16_t bytes) {
    // TODO maybe load exactly how many was consumed?
    int r = check_buffer();
    if (r) {
        printf("check_buffer failed, ending playback");
        notify_playback_end(true);
    }
}