#pragma once

#include "ff.h"
#include "../config.hpp"

#include "helixmp3/pub/mp3dec.h"

enum FinishReason {
    NoAbort,
    UnderflowChanA,
    UnderflowChanB,
    User
};

static HMP3Decoder hMP3Decoder = nullptr;
// one extra wrapped frame
static uint8_t mp3_buf_hidden[BUF_MP3_SIZE_BYTES + BUF_HIDDEN_MP3_SIZE_BYTES];
static uint8_t* const mp3_buf = mp3_buf_hidden + BUF_HIDDEN_MP3_SIZE_BYTES; // pointer is const, not data

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
    // finish reason
    FinishReason decode_finished_by;

    long buffer_left();
    long buffer_left_continuous();

    void wrap_buffer();
    void load_buffer();
    void align_buffer();

    void init_dbg();
    void prepare();

    // stats (constant after prepare() calls calculate_stats())
    float sec_per_frame;
    int duration;
    long bit_freq;
    void calculate_stats();

    // stats updated on decoding finished (on request by DMA)
    // used by watch_timer()
    int sum_frames_decoded;
    float took_ms;
    int seconds;
    int last_seconds;
    void decode_done(int decoded_frames, uint64_t took_us, FinishReason channel);

    // these return number of frames actually decoded
    int decode_up_to_one_frame(int16_t* audio_pcm_buf);
    int decode_up_to_n_frames(int16_t* audio_pcm_buf, int n);


public:
    MP3(const char *filepath_, uint32_t* const audio_pcm_)
        : filepath(filepath_)
        , audio_pcm(audio_pcm_)
            { prepare(); init_dbg(); }

    ~MP3() {
        f_close(&fp);
    }

    // statistics
    int frames_to_sec(int frames) { return sec_per_frame * (float)frames; }
    int get_duration() { return duration; }
    float get_ms_per_frame() { return sec_per_frame * 1000; }
    long get_bit_freq() { return bit_freq; }

    // called in loop on core0
    void watch_file_buffer();
    void watch_timer();

    // sustains the loop on core0
    bool get_eof() { return eof; }

    // called to gracefully finish core0 thread
    // gates core0 not to exit before decoding thread is done
    // (and aborting DMA channels)
    // sustains core1 loop
    bool get_decode_finished()  { return decode_finished_by != NoAbort; }
    bool decode_finished_by_A() { return decode_finished_by == UnderflowChanA; }
    bool decode_finished_by_B() { return decode_finished_by == UnderflowChanB; }

    // called from core0 to start core1
    bool needs_core1() { return true; }

    // called from core1
    void watch_decode(volatile bool& a_done_irq, volatile bool& b_done_irq);

    // called when user aborts playback
    void user_abort() {
        eof = true;
        decode_finished_by = User;
    }
};