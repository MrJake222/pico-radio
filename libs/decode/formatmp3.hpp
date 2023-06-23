#pragma once

#include <format.hpp>
#include <circularbuffer.hpp>
#include <config.hpp>

#include <mp3dec.h>

class FormatMP3 : public Format {

    static const int MP3_HEADER_SIZE = 4;

    HMP3Decoder hMP3Decoder;

    // set by decode
    // used to limit buffer reversing
    int bytes_consumed_last;

    // set by <calculate_stats>
    // used to calculate all the info about the file
    MP3FrameInfo frame_info;
    bool stats_print;
    void calculate_stats();

    void align_buffer(uint8_t *orig_read_ptr);
    int decode_up_to_one_frame(uint32_t* audio_pcm_buf);

public:
    FormatMP3(volatile CircularBuffer& raw_buf_, HMP3Decoder hMP3Decoder_)
        : Format(raw_buf_)
        , hMP3Decoder(hMP3Decoder_)
        { }

    void init() override {
        Format::init();

        MP3ClearBuffers(hMP3Decoder);
        bytes_consumed_last = 0;
        stats_print = true;
        bitrate_sum = 0;
        bitrate_count = 0;
    }

    int units_to_decode_whole() override { return BUF_PCM_SIZE_FRAMES; }

    int decode_up_to_n(uint32_t *audio_pcm_buf, int n) override;
    void decode_exactly_n(uint32_t *audio_pcm_buf, int n) override;

    long bit_freq() override;
    float ms_per_unit() override;
    int units_to_sec(int units) override;
    int duration_sec(int file_size_bytes) override;

    int bitrate_sum;
    int bitrate_count;
    int avg_bitrate() { return bitrate_sum / bitrate_count; }
};
