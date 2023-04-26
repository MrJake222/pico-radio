#include "mp3.hpp"

#include <pico/stdlib.h>
#include <cstdio>
#include "f_util.h"

#define MINIMP3_IMPLEMENTATION
#include "minimp3/minimp3.h"

const uint PIN_DBG_MP3 = 12;
#define DBG_MP3_ON() gpio_put(PIN_DBG_MP3, true)
#define DBG_MP3_OFF() gpio_put(PIN_DBG_MP3, false)

static void fs_err(FRESULT fr, const char* tag) {
    panic("%s: %s (id=%d)\n", tag, FRESULT_str(fr), fr);
}

// unused, but required by functions
static int ffbytes;
static int frame_bytes;

void MP3::prepare() {
    gpio_init(PIN_DBG_MP3);
    gpio_set_dir(PIN_DBG_MP3, GPIO_OUT);
    DBG_MP3_OFF();

    mp3dec_init(&mp3dec);

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
    offset = mp3d_find_frame(mp3_buf, BUF_MP3_SIZE_BYTES, &ffbytes, &frame_bytes);
    printf("offset: %ld\n", offset);

    load_at = 0;
    end = false;
}

void MP3::decode_one_frame(int16_t* audio_pcm_buf) {
    ulong dataleft = load_at <= offset
                     ? (BUF_MP3_SIZE_BYTES - offset + load_at)
                     : (load_at - offset);

    if (dataleft < BUF_MP3_SIZE_BYTES / 2 && !end) {
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

    // continuous buffer left
    int cont_buf_left = load_at <= offset
                        ? (BUF_MP3_SIZE_BYTES - offset)   // no + load_at (because it is beyond wrapping)
                        : (load_at - offset);


    int o = mp3d_find_frame(mp3_buf + offset, cont_buf_left, &ffbytes, &frame_bytes);
    // printf("o %d  ", o);

    // TODO this could execute only the memcopy
    // TODO fix to always load 1 frame

    if (o == cont_buf_left) {
        if (end) {
            printf("end of buffer\n");
            // break;
        }

        memcpy(mp3_buf - cont_buf_left, mp3_buf + offset, cont_buf_left);
        offset = -cont_buf_left;
        // printf("wrap  offset %ld\n", offset);
    }
    else if (o > BUF_MP3_SIZE_BYTES/2) {
        printf("buffer underflow\n");
        mp3d_find_frame(mp3_buf + offset, cont_buf_left, &ffbytes, &frame_bytes);
        printf("STOP\n");
        // break;
    }
    else if (o != 0) {
        printf("frame misalign\n");
        printf("STOP\n");
        // break;
    }
    else {
        mp3dec_frame_info_t info;

        DBG_MP3_ON();
        int pcm_samples = mp3dec_decode_frame(
                &mp3dec,
                mp3_buf + offset,
                BUF_MP3_SIZE_BYTES - offset,
                audio_pcm_buf,
                &info);
        DBG_MP3_OFF();

        offset += info.frame_bytes;

        /*printf("pcm samples %d  ", pcm_samples);
        printf("used up %4d  ", info.frame_bytes);
        printf("frame off %5d  ", info.frame_offset);
        printf("off %5ld  ", offset);
        puts("");*/
    }
}


void MP3::decode_n_frames(int16_t* audio_pcm_buf, int n) {
    long offset = 0;
    for (int i=0; i<n; i++) {
        // printf("%2d loading data at %5ld  ", i+1, offset);
        decode_one_frame(audio_pcm_buf + offset);
        offset += MP3_SAMPLES_PER_FRAME * 2;
    }
}