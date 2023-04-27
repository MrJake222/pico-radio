#pragma once

#include "ff.h"
#include "../config.hpp"

#include "helixmp3/pub/mp3dec.h"

class MP3 {

    static const int MP3_HEADER_SIZE = 4;

    const char* filepath;
    FRESULT fr;
    FIL fp;

    // MP3
    HMP3Decoder hMP3Decoder;
    MP3FrameInfo frame_info;
    // one extra wrapped frame
    uint8_t mp3_buf_hidden[BUF_MP3_SIZE_BYTES + BUF_MP3_SIZE_BYTES_PER_FRAME];
    uint8_t* const mp3_buf = mp3_buf_hidden + BUF_MP3_SIZE_BYTES_PER_FRAME; // pointer is const, not data

    long offset;
    long load_at;

    // end of input file
    bool eof;
    // end of playback
    bool eop;

    long buffer_left();
    long buffer_left_continuous();

    void wrap_buffer();
    void load_buffer();
    void align_buffer();

    void prepare();

    // stats
    float sec_per_frame;
    int duration;
    void calculate_stats();

public:
    MP3(const char *filepath_)
        : filepath(filepath_)
            { prepare(); }

    // these return number of frames actually decoded
    int decode_up_to_one_frame(int16_t* audio_pcm_buf);
    int decode_up_to_n_frames(int16_t* audio_pcm_buf, int n);

    // statistics
    int frames_to_sec(int frames) { return sec_per_frame * (float)frames; }
    int get_duration() { return duration; }
    float get_ms_per_frame() { return sec_per_frame * 1000; }
};