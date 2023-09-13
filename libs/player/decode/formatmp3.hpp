#pragma once

#include <format.hpp>
#include <circularbuffer.hpp>
#include <config.hpp>

#include <mp3dec.h>
#include <id3.hpp>

class FormatMP3 : public Format {

    static const int MP3_HEADER_SIZE = 4;

    HMP3Decoder hMP3Decoder;

    // set by <calculate_stats>
    // used to calculate all the info about the file
    MP3FrameInfo frame_info;
    bool stats_print;
    void calculate_stats();

    FormatMP3::Error align_buffer(uint8_t *orig_read_ptr);
    int decode_up_to_one_frame(uint32_t* audio_pcm_buf);

    unsigned long long bitrate_sum;
    unsigned long long bitrate_count;

    ID3 id3;

public:
    FormatMP3(volatile CircularBuffer& raw_buf_, HMP3Decoder hMP3Decoder_)
        : Format(raw_buf_)
        , hMP3Decoder(hMP3Decoder_)
        , id3(raw_buf_)
        { }

    void begin(bool* abort_, bool* eof_) override {
        Format::begin(abort_, eof_);

        MP3ClearBuffers(hMP3Decoder);
        stats_print = true;
        bitrate_sum = 0;
        bitrate_count = 0;

        id3.begin();
    }

    int decode_up_to_n(uint32_t *audio_pcm_buf, int n) override;

    // call before using any of below functions
    int decode_header() override;

    int units_to_decode_whole(int audio_pcm_size_words) override;
    long bit_freq() override;
    float ms_per_unit() override;
    int bytes_to_sec(b_type bytes) override;
    int samps_per_channel();

    int bitrate_in() override;
    int byterate_in() { return bitrate_in() / 8; }

    int get_meta_str(char *meta, int meta_len) override;
};

