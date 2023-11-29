#include <cstring>
#include <cstdio>
#include <pico/platform.h>
#include <formatwav.hpp>

int FormatWAV::decode_header() {
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

    // printf("duration:    %02u:%02u\n", duration_sec(0)/60, duration_sec(0)%60);
    // puts("");

    return 0;
}

int FormatWAV::units_to_decode_whole(int audio_pcm_size_words) {
    // returns number of stereo samples (16bit x 2)
    return audio_pcm_size_words;
}

int FormatWAV::decode_up_to_n(uint32_t *audio_pcm_buf, int n) {
    if (abort())
        return 0;

    // <n> is stereo samples left to be decoded
    int n_read = 0;

    // is source stereo
    bool stereo = channels() == 2;

    while (n_read < n) {
        int read = stereo ? (n - n_read) * 4  // source is stereo (4 bytes per sample 16bit x 2)
                          : (n - n_read) * 2; // source is mono   (2 bytes per sample 16bit x 1)

        read = MIN(read, raw_buf.data_left_continuous());
        // always multiple of 4
        read -= read % 4;

        // handles end-of-file
        if (read == 0)
            break;

        memcpy(audio_pcm_buf, raw_buf.read_ptr(), read);
        int written = read;
        if (!stereo) {
            // expected number of bytes is 2 times read bytes
            mono_to_stereo(audio_pcm_buf, (read*2) / 4);
            written *= 2;
        }

        const int n_written = written / 4;
        audio_pcm_buf += n_written;
        n_read += n_written;

        raw_buf.read_ack(read);
        if (raw_buf.can_wrap_buffer() && raw_buf.should_wrap_buffer())
            raw_buf.try_wrap_buffer();
    }

    return n_read;
}

long FormatWAV::bit_freq_per_channel() {
    return header.sample_rate * header.bits_per_sample;
}

float FormatWAV::ms_per_unit() {
    return 1000.0f / header.bytes_per_second;
}

int FormatWAV::channels() {
    return header.channels;
}

int FormatWAV::bytes_to_sec(b_type bytes) {
    return (int)(bytes / header.bytes_per_second);
}

int FormatWAV::bitrate_in() {
    return bit_freq_per_channel() * header.channels;
}