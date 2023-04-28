#pragma once

#include "ff.h"
#include "../config.hpp"

#include "helixmp3/pub/mp3dec.h"

enum DMAChannel {
    ChanA,
    ChanB
};

static HMP3Decoder hMP3Decoder = nullptr;
// one extra wrapped frame
static uint8_t mp3_buf_hidden[BUF_MP3_SIZE_BYTES + BUF_MP3_SIZE_BYTES_PER_FRAME];
static uint8_t* const mp3_buf = mp3_buf_hidden + BUF_MP3_SIZE_BYTES_PER_FRAME; // pointer is const, not data

class MP3 {

    static const int MP3_HEADER_SIZE = 4;

    const char* filepath;
    FRESULT fr;
    FIL fp;

    uint32_t* const audio_pcm;

    // MP3
    MP3FrameInfo frame_info;

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

    void init_dbg();
    void prepare();

    // stats
    float sec_per_frame;
    int duration;
    long bit_freq;
    void calculate_stats();

    int sum_frames_decoded = 0;
    float took_ms = 0;
    int seconds = 0;
    int last_seconds = -1;

    bool decode_finished = false;
    bool decode_finished_by_user = false;
    DMAChannel decode_finished_by;

public:
    MP3(const char *filepath_, uint32_t* const audio_pcm_)
        : filepath(filepath_)
        , audio_pcm(audio_pcm_)
            { prepare(); init_dbg(); }

    ~MP3() {
        //MP3FreeDecoder(hMP3Decoder);
        f_close(&fp);
    }

    bool get_eof() { return eof; }

    // these return number of frames actually decoded
    int decode_up_to_one_frame(int16_t* audio_pcm_buf);
    int decode_up_to_n_frames(int16_t* audio_pcm_buf, int n);

    // control
    long get_bit_freq() { return bit_freq; }

    // statistics
    int frames_to_sec(int frames) { return sec_per_frame * (float)frames; }
    int get_duration() { return duration; }
    float get_ms_per_frame() { return sec_per_frame * 1000; }


    // called from core0
    void watch_file_buffer();

    // called from core1
    bool needs_core1() { return true; }
    void decode_done(int decoded_frames, uint64_t took_us, DMAChannel channel);
    void watch_decode(volatile bool& a_done_irq, volatile bool& b_done_irq);

    bool get_decode_finished() { return decode_finished; }
    bool decode_finished_by_A() { return decode_finished_by == ChanA && !decode_finished_by_user; }
    bool decode_finished_by_B() { return decode_finished_by == ChanB && !decode_finished_by_user; }

    void user_abort() {
        eof = true;
        decode_finished_by_user = true;
        decode_finished = true;
    }
};