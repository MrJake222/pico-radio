#include "waveheader.hpp"

#include <cstdio>

FRESULT WaveHeader::read(FIL* fp, uint* read) {
    FRESULT fr = f_read(fp, (uint8_t*)&hdr, WAVE_HEADER_SIZE, read);

    duration_sec = hdr.data_size / hdr.bytes_per_second;

    return fr;
}

int WaveHeader::check() {
    // TODO implement
    return 0;
}

void WaveHeader::print() {
    printf("file size:   %u (%u MB)\n", hdr.file_size, hdr.file_size / (1024*1024));
    printf("channels:    %u\n", hdr.channels);
    printf("sample rate: %u\n", hdr.sample_rate);
    printf("bitrate:     %u\n", hdr.bytes_per_second * 8);
    printf("bit depth:   %u\n", hdr.bits_per_sample);
    printf("data size:   %u\n", hdr.data_size);
    puts("");

    printf("duration:    %02u:%02u\n", get_duration()/60, get_duration()%60);
    puts("");
}