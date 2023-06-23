#include <decodefile.hpp>

#include <pico/platform.h>
#include <cstdio>
#include <f_util.h>
#include <mcorefifo.hpp>

static void fs_err(FRESULT fr, const char* tag) {
    panic("%s: %s (id=%d)\n", tag, FRESULT_str(fr), fr);
}

static void buf_top_up(void* arg, uint32_t _) {
    ((DecodeFile*) arg)->load_buffer_or_eof();
}

void DecodeFile::begin(const char* path_, Format* format_) {
    DecodeBase::begin(path_, format_);

    fr = f_open(&fp, path, FA_READ);
    if (fr != FR_OK) {
        fs_err(fr, "f_open");
    }

    // preload with file data
    load_buffer(format->raw_buf.size);

    eof = false;
    fifo_register(RAW_BUF_TOP_UP, buf_top_up, this, true);
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

void DecodeFile::load_buffer_or_eof() {
    if (eof) {
        // eof, after wrap, set end-of-playback
        format->set_eop();
    }

    else {
        // no eof -> just load more
        load_buffer(format->raw_buf.size/2);
    }
}

void DecodeFile::raw_buf_read_cb(unsigned int bytes) {
    DecodeBase::raw_buf_read_cb(bytes);

    // called from core1
    // Here we need to pass a message to core0, to load file

    if (format->raw_buf.data_left() < format->raw_buf.size / 2) {
        fifo_send(RAW_BUF_TOP_UP);
    }
}