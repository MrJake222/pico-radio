#pragma once

#include "ff.h"

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

class WaveHeader {

    struct wave_header hdr;
    uint duration_sec;

public:
    FRESULT read(FIL* fp, uint* read);

    uint get_data_size() { return hdr.data_size; }
    uint get_duration() { return duration_sec; }
    uint get_bit_freq() { return hdr.sample_rate * hdr.bits_per_sample * hdr.channels; }
    uint byte_to_sec(uint byte) { return byte / hdr.bytes_per_second; }

    int check();
    void print();
};