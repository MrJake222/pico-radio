#pragma once

#include "ff.h"
#include "../config.hpp"

#include "pico/float.h"

#define MINIMP3_ONLY_MP3
//#define MINIMP3_ONLY_SIMD
//#define MINIMP3_NO_SIMD
#define MINIMP3_NONSTANDARD_BUT_LOGICAL
//#define MINIMP3_FLOAT_OUTPUT
//#define MINIMP3_IMPLEMENTATION
#include "minimp3/minimp3.h"

#define BUF

class MP3 {

    const char* filepath;
    FRESULT fr;
    FIL fp;

    // MP3
    mp3dec_t mp3dec;
    // one extra wrapped frame
    uint8_t mp3_buf_hidden[BUF_MP3_SIZE_BYTES + BUF_MP3_SIZE_BYTES_PER_FRAME];
    uint8_t* const mp3_buf = mp3_buf_hidden + BUF_MP3_SIZE_BYTES_PER_FRAME; // pointer is const, not data

    long offset;
    long load_at;
    bool end;

    // find first frame
    void prepare();

public:
    MP3(const char *filepath_, uint8_t* audio_pcm_bytes_)
        : filepath(filepath_)
            { prepare(); }

    void decode_one_frame(int16_t* audio_pcm_buf);
    void decode_n_frames(int16_t* audio_pcm_buf, int n);
};