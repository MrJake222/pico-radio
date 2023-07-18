#pragma once

#include <format.hpp>
#include <circularbuffer.hpp>
#include <config.hpp>

#include <mp3dec.h>

class FormatMP3 : public Format {

    static const int MP3_HEADER_SIZE = 4;

    HMP3Decoder hMP3Decoder;

    // set by <calculate_stats>
    // used to calculate all the info about the file
    MP3FrameInfo frame_info;
    bool stats_print;
    void calculate_stats();

    int align_buffer(uint8_t *orig_read_ptr);
    int decode_up_to_one_frame(uint32_t* audio_pcm_buf);

    unsigned long long bitrate_sum;
    unsigned long long bitrate_count;

public:
    FormatMP3(volatile CircularBuffer& raw_buf_, HMP3Decoder hMP3Decoder_)
        : Format(raw_buf_)
        , hMP3Decoder(hMP3Decoder_)
        { }

    void begin() override {
        Format::begin();

        MP3ClearBuffers(hMP3Decoder);
        stats_print = true;
        bitrate_sum = 0;
        bitrate_count = 0;
    }

    int units_to_decode_whole() override { return BUF_PCM_SIZE_FRAMES; }

    int decode_up_to_n(uint32_t *audio_pcm_buf, int n) override;

    long bit_freq() override;
    float ms_per_unit() override;
    int bytes_to_sec(b_type bytes) override;

    int bitrate_in() override;
    int byterate_in() { return bitrate_in() / 8; }
};

