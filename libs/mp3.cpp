#include "mp3.hpp"

#include <pico/stdlib.h>
#include <cstdio>
#include <cstring>
#include "f_util.h"

const uint PIN_DBG_MP3 = 12;
#define DBG_MP3_ON() gpio_put(PIN_DBG_MP3, true)
#define DBG_MP3_OFF() gpio_put(PIN_DBG_MP3, false)

static void fs_err(FRESULT fr, const char* tag) {
    panic("%s: %s (id=%d)\n", tag, FRESULT_str(fr), fr);
}

long MP3::buffer_left() {
    return load_at <= offset
            ? (BUF_MP3_SIZE_BYTES - offset + load_at)
            : (load_at - offset);
}

long MP3::buffer_left_continuous() {
    return load_at <= offset
            ? (BUF_MP3_SIZE_BYTES - offset)   // no + load_at (because it is beyond wrapping)
            : (load_at - offset);
}

void MP3::prepare() {
    gpio_init(PIN_DBG_MP3);
    gpio_set_dir(PIN_DBG_MP3, GPIO_OUT);
    DBG_MP3_OFF();

    hMP3Decoder = MP3InitDecoder();

    fr = f_open(&fp, filepath, FA_READ);
    if (fr != FR_OK) {
        fs_err(fr, "f_open");
    }

    // preload
    uint read;
    fr = f_read(&fp, mp3_buf, BUF_MP3_SIZE_BYTES, &read);
    if (fr != FR_OK) {
        fs_err(fr, "f_read preload");
    }

    // find offset
    offset = MP3FindSyncWord(mp3_buf, BUF_MP3_SIZE_BYTES);
    printf("offset: %ld\n", offset);

    load_at = 0;
    end = false;
}

void MP3::wrap_buffer() {
    long buf_left = buffer_left_continuous();

    memcpy(mp3_buf - buf_left, mp3_buf + offset, buf_left);
    offset = -buf_left;
}

void MP3::decode_one_frame(int16_t* audio_pcm_buf) {
    if (buffer_left() < BUF_MP3_SIZE_BYTES / 2 && !end) {
        // printf("loading, offset %ld  load_at %ld\n", offset, load_at);

        uint read;
        fr = f_read(&fp, mp3_buf + load_at, BUF_MP3_SIZE_BYTES/2, &read);
        if (fr != FR_OK) {
            fs_err(fr, "f_read preload");
        }

        if (read == 0) {
            printf("EOF\n");
            end = true;
        }

        load_at += read;
        load_at %= BUF_MP3_SIZE_BYTES;

        // printf("loaded,  offset %ld  load_at %ld\n", offset, load_at);
    }

    DBG_MP3_ON();

    uint8_t* dptr_orig;
    uint8_t* dptr;
    int b_orig;
    int b;

    int res;
    do {
        dptr_orig = mp3_buf + offset;
        dptr = dptr_orig;
        b_orig = BUF_MP3_SIZE_BYTES - offset;
        b = b_orig;

        res = MP3Decode(
                hMP3Decoder,
                &dptr,
                &b,
                audio_pcm_buf,
                0);

        switch (res) {
            case ERR_MP3_NONE:
                break;

            case ERR_MP3_INDATA_UNDERFLOW:
                wrap_buffer();
                break;

            case ERR_MP3_INVALID_FRAMEHEADER:
                printf("wrong sync-word\n");
                offset += 1;
                offset += MP3FindSyncWord(mp3_buf + offset, BUF_MP3_SIZE_BYTES);
                break;

            default:
                printf("\nunknown error code=%d\n", res);
        }

    } while (res == ERR_MP3_INDATA_UNDERFLOW);

    DBG_MP3_OFF();

    offset += dptr - dptr_orig;

    // printf("  ")
    //
    // printf("res %d  ", res);
    // printf("ddiff %d  ", dptr - dptr_orig);
    // printf("bdiff %d  ", b - b_orig);
    // printf("used up %4d  ", info.frame_bytes);
    // printf("frame off %5d  ", info.frame_offset);
    // printf("off %5ld  ", offset);
    // puts("");

}


void MP3::decode_n_frames(int16_t* audio_pcm_buf, int n) {
    long offset = 0;
    for (int i=0; i<n; i++) {
        // printf("%2d loading data at %5ld  ", i+1, offset);
        decode_one_frame(audio_pcm_buf + offset);
        offset += MP3_SAMPLES_PER_FRAME * 2;
    }
}