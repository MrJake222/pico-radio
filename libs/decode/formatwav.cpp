#include <cstring>
#include <cstdio>
#include <pico/platform.h>
#include "formatwav.hpp"

void FormatWAV::decode_header() {
    Format::decode_header();

    while (raw_buf.data_left() < WAVE_HEADER_SIZE);
    memcpy(&header, raw_buf.read_ptr(), WAVE_HEADER_SIZE);
    raw_buf.read_ack(WAVE_HEADER_SIZE);

    printf("file size:   %u (%u MB)\n", header.file_size, header.file_size / (1024*1024));
    printf("channels:    %u\n", header.channels);
    printf("sample rate: %u\n", header.sample_rate);
    printf("bitrate:     %u\n", header.bytes_per_second * 8);
    printf("bit depth:   %u\n", header.bits_per_sample);
    printf("data size:   %u\n", header.data_size);
    puts("");

    printf("duration:    %02u:%02u\n", duration_sec(0)/60, duration_sec(0)%60);
    puts("");
}

int FormatWAV::decode_up_to_n(uint32_t *audio_pcm_buf, int n) {

    int copied = 0;

    while (copied < n) {
        int cpy = raw_buf.data_left_continuous();
        cpy = MIN(cpy, n - copied);

        if (cpy == 0)
            break;

        memcpy(audio_pcm_buf + copied/4, raw_buf.read_ptr(), cpy);
        raw_buf.read_ack(cpy);
        copied += cpy;
    }

    return copied;
}

void FormatWAV::decode_exactly_n(uint32_t *audio_pcm_buf, int n) {
    for (int bytes_read=0; bytes_read < n;) {
        bytes_read += decode_up_to_n(audio_pcm_buf + bytes_read/4, n - bytes_read);
    }
}

long FormatWAV::bit_freq() {
    return header.sample_rate * header.bits_per_sample * header.channels;
}

float FormatWAV::ms_per_unit() {
    return 1000.0f / header.bytes_per_second;
}

int FormatWAV::units_to_sec(int units) {
    return units / header.bytes_per_second;
}

int FormatWAV::duration_sec(int file_size_bytes) {
    return header.data_size / header.bytes_per_second;
}
