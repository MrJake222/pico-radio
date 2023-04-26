#pragma once

#include "ff.h"
#include "../config.hpp"

#include "helixmp3/pub/mp3dec.h"

class MP3 {

    const char* filepath;
    FRESULT fr;
    FIL fp;

    // MP3
    HMP3Decoder hMP3Decoder;
    // one extra wrapped frame
    uint8_t mp3_buf_hidden[BUF_MP3_SIZE_BYTES + BUF_MP3_SIZE_BYTES_PER_FRAME];
    uint8_t* const mp3_buf = mp3_buf_hidden + BUF_MP3_SIZE_BYTES_PER_FRAME; // pointer is const, not data

    long offset;
    long load_at;
    bool end;

    long buffer_left();
    long buffer_left_continuous();

    void prepare();
    void wrap_buffer();

public:
    MP3(const char *filepath_)
        : filepath(filepath_)
            { prepare(); }

    void decode_one_frame(int16_t* audio_pcm_buf);
    void decode_n_frames(int16_t* audio_pcm_buf, int n);
};