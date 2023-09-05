#pragma once


#include <format.hpp>
#include <config.hpp>

typedef unsigned int uint;
struct wave_header {
    char riff_str[4];
    uint file_size;
    char wave_str[4];
    char fmt_str[4];
    uint format_size;
    unsigned short format_type;
    unsigned short channels;
    uint sample_rate;
    uint bytes_per_second;
    unsigned short bytes_per_sample;
    unsigned short bits_per_sample;
    char data_str[4];
    uint data_size;
};
#define WAVE_HEADER_SIZE sizeof(struct wave_header)

class FormatWAV : public Format {

    struct wave_header header;

public:
    using Format::Format;

    int units_to_decode_whole(int audio_pcm_size_words) override { return audio_pcm_size_words * 4; }

    int decode_header() override;
    int decode_up_to_n(uint32_t *audio_pcm_buf, int n) override;

    long bit_freq() override;
    float ms_per_unit() override;
    int bytes_to_sec(b_type bytes) override;

    int bitrate_in() override;
};
