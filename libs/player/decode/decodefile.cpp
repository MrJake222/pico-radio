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
    int r = load_buffer(cbuf.size);
    if (r < 0)
        return -1;

    // no wait required as we just loaded whole buffer

    return DecodeBase::play();
}

void DecodeFile::end() {
    if (file_open) {
        fr = f_close(&fp);
        if (fr != FR_OK) {
            fs_err(fr, "f_close");
        }
    }

    DecodeBase::end();
}

int DecodeFile::load_buffer(int bytes) {
    // printf("loading %d bytes, offset %ld  load_at %ld\n", bytes, cbuf.get_read_offset(), cbuf.get_write_offset());

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

    // printf("loaded,  offset %ld  load_at %ld\n", cbuf.get_read_offset(), cbuf.get_write_offset());
    return read;
}

int DecodeFile::check_buffer() {
    if (eof) {
        // no more data to read
        notify_stop();
        return 0;
    }

    if (cbuf.health() > load_max_health)
        // data chunk too small
        return 0;


    int data_len = cbuf.space_left();

    // this is kind of thread-safe because space_left_continuous is either
    // constant or only growing (then its limited by data_len)
    for (int i=0; i<2 && data_len>0; i++) {
        long write = MIN(data_len, cbuf.space_left_continuous());
        int r = load_buffer(write);
        if (r < 0)
            return -1; // load_buffer errored

        data_len -= write;
    }

    return data_len;
}

void DecodeFile::ack_bytes(uint16_t bytes) {
    int r = check_buffer();
    if (r) {
        printf("check_buffer failed, ending playback");
        notify_playback_end(true);
    }
}