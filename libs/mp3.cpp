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

void MP3::wrap_buffer() {
    long buf_left = buffer_left_continuous();

    memcpy(mp3_buf - buf_left, mp3_buf + offset, buf_left);
    offset = -buf_left;
}

void MP3::load_buffer() {
    // printf("loading, offset %ld  load_at %ld\n", offset, load_at);

    uint read;
    fr = f_read(&fp, mp3_buf + load_at, BUF_MP3_SIZE_BYTES/2, &read);
    if (fr != FR_OK) {
        fs_err(fr, "f_read preload");
    }

    if (read < BUF_MP3_SIZE_BYTES/2) {
        printf("EOF\n");
        eof = true;
    }

    load_at += read;
    load_at %= BUF_MP3_SIZE_BYTES;

    // printf("loaded,  offset %ld  load_at %ld\n", offset, load_at);
}

void MP3::align_buffer() {
    int matched;
    do {
        int sync_word_offset = MP3FindSyncWord(mp3_buf + offset, BUF_MP3_SIZE_BYTES - offset);
        if (sync_word_offset < 0) {
            // failed

            offset = BUF_MP3_SIZE_BYTES - MP3_HEADER_SIZE;
            wrap_buffer();
            load_buffer();
            load_buffer();
        }
        else {
            offset += sync_word_offset;

            printf("offset %ld  ", offset);
            matched = MP3CheckSyncWordRepeated(hMP3Decoder, mp3_buf + offset, BUF_MP3_SIZE_BYTES - offset);
            if (matched == 0) {
                // failed
                offset += 1;
            }

            printf("matched: %d\n", matched);
        }
    } while (matched < 1);

    printf("align buffer offset: %ld\n", offset);
}

void MP3::prepare() {

    hMP3Decoder = MP3InitDecoder();

    fr = f_open(&fp, filepath, FA_READ);
    if (fr != FR_OK) {
        fs_err(fr, "f_open");
    }

    eof = false;
    eop = false;

    // preload
    load_at = 0;
    load_buffer();
    load_buffer();

    offset = 0;
    align_buffer();

    int ret = MP3GetNextFrameInfo(hMP3Decoder, &frame_info, mp3_buf + offset);
    if (ret) {
        printf("failed to decode frame information\n");
    } else {
        calculate_stats();
    }
}

void MP3::calculate_stats() {
    sec_per_frame = (float)frame_info.outputSamps / frame_info.nChans / frame_info.samprate;
    duration = ((int)f_size(&fp) - offset) * 8 / frame_info.bitrate;

    printf("output samples: %d, samplerate: %d, bitrate: %d\n",
           frame_info.outputSamps,
           frame_info.samprate,
           frame_info.bitrate);

    printf("ms_per_frame: %f\n",
           sec_per_frame * 1000);
}

int MP3::decode_up_to_one_frame(int16_t* audio_pcm_buf) {
    if (buffer_left() < BUF_MP3_SIZE_BYTES / 2) {
        // low on data

        if (eof)
            // eof, after wrap, set end-of-playback
            eop = true;

        else
            // no eof -> just load more
            load_buffer();
    }

    long bytes_consumed;

    int res;
    do {
        if ((BUF_MP3_SIZE_BYTES - offset) < MP3_HEADER_SIZE) {
            // even the header won't fit in continuous buffer
            wrap_buffer();
        }

        uint8_t* dptr_orig = mp3_buf + offset;
        uint8_t* dptr = dptr_orig;
        int b = BUF_MP3_SIZE_BYTES - offset;

        res = MP3Decode(
                hMP3Decoder,
                &dptr,
                &b,
                audio_pcm_buf,
                0);

        bytes_consumed = dptr - dptr_orig;

        switch (res) {
            case ERR_MP3_NONE:
                break;

            case ERR_MP3_INDATA_UNDERFLOW:
                if (eop)
                    return 0;
                wrap_buffer();
                break;

            case ERR_MP3_INVALID_FRAMEHEADER:
                printf("o %ld  wrong sync-word\n", offset);
                align_buffer();
                break;

            default:
                printf("unknown error code=%d\n", res);
        }

    } while (res == ERR_MP3_INDATA_UNDERFLOW || res == ERR_MP3_INVALID_FRAMEHEADER);

    offset += bytes_consumed;

    /*if (eop) {
        printf("buf left: %ld\n", buffer_left());
    }*/

    // printf("  ")
    //
    // printf("res %d  ", res);
    // printf("ddiff %d  ", dptr - dptr_orig);
    // printf("bdiff %d  ", b - b_orig);
    // printf("used up %4d  ", info.frame_bytes);
    // printf("frame off %5d  ", info.frame_offset);
    // printf("off %5ld  ", offset);
    // puts("");

    return 1;
}

int MP3::decode_up_to_n_frames(int16_t* audio_pcm_buf, int n) {
    long frame_offset = 0;
    int frames_read;

    for (frames_read=0; frames_read < n; frames_read++) {
        // printf("%2d loading data at %5ld  ", i+1, offset);
        int decoded = decode_up_to_one_frame(audio_pcm_buf + frame_offset);
        if (decoded == 0)
            break;

        frame_offset += MP3_SAMPLES_PER_FRAME * 2;
    }

    return frames_read;
}