#include <formatmp3.hpp>
#include <config.hpp>

#include <cstdio>
#include <pico/platform.h>

int FormatMP3::align_buffer(uint8_t* orig_read_ptr) {

    const int max_check_len = 1024;
    int tries_left = raw_buf.size / max_check_len;
    int matched;
    do {
        if (raw_buf.data_left() < MP3_HEADER_SIZE) {
            int ret = wrap_buffer_wait_for_data();
            if (ret == -1)
                // failed
                return -1; // error

            if (ret == -2)
                return -2; // abort

            continue;
        }

        int check_len = MIN(max_check_len, raw_buf.data_left_continuous());

        // printf("read at %ld  avail %ld  check %d  ", raw_buf.get_read_offset(), raw_buf.data_left(), check_len);
        int sync_word_offset = MP3FindSyncWord(raw_buf.read_ptr(), check_len);
        // printf("sync_word_offset: %d\n", sync_word_offset);
        if (sync_word_offset < 0) {
            // failed (read all <check_len>, and no sync word)
            raw_buf.read_ack(check_len);
        }
        else {
            // success after <sync_word_offset> bytes
            raw_buf.read_ack(sync_word_offset);

            printf("offset %5d (%4d)  avail %d  ", raw_buf.get_read_offset(), raw_buf.read_ptr() - orig_read_ptr, raw_buf.data_left());

            matched = MP3CheckSyncWordRepeated(hMP3Decoder, raw_buf.read_ptr(), raw_buf.data_left_continuous());
            if (matched == 0) {
                // failed
                raw_buf.read_ack(1);
            }

            printf("matched: %d\n", matched);
        }

        tries_left--;
    } while (matched < 1 && tries_left > 0);

    if (matched == 0)
        return -1;

    return 0;
}

int FormatMP3::decode_up_to_one_frame(uint32_t* audio_pcm_buf) {
    if (abort)
        return 0;

    // printf("decode o %ld  avail %ld\n", raw_buf.get_read_offset(), raw_buf.data_left_continuous());

    long bytes_consumed;
    int ret;

    bool again = false;
    do {
        if (again && raw_buf.get_read_offset() > 0) {
            // again & not after wrap
            printf("decode o %5d   health %2d%%\n", raw_buf.get_read_offset(), raw_buf.health());
        }

        if (raw_buf.data_left_continuous() < MP3_HEADER_SIZE) {
            // even the header won't fit in continuous buffer
            ret = wrap_buffer_wait_for_data();
            if (ret == -1)
                // failed
                return -1; // error

            if (ret == -2)
                // abort
                return 0; // no frames decoded
        }

        uint8_t* dptr_orig = raw_buf.read_ptr();
        uint8_t* dptr = dptr_orig;
        int b = raw_buf.data_left_continuous();

        int res = MP3Decode(
                hMP3Decoder,
                &dptr,
                &b,
                (short*)audio_pcm_buf,
                0);

        bytes_consumed = dptr - dptr_orig;

        uint8_t* orig_read;

        again = false;
        switch (res) {
            case ERR_MP3_NONE:
                calculate_stats();
                break;

            case ERR_MP3_INDATA_UNDERFLOW:
                ret = wrap_buffer_wait_for_data();
                if (ret == -1)
                    // failed
                    return -1; // error

                if (ret == -2)
                    // abort
                    return 0; // no frames decoded

                again = true;
                break;

            case ERR_MP3_INVALID_FRAMEHEADER:
                printf("o %d  wrong sync-word  health %2d%%\n", raw_buf.get_read_offset(), raw_buf.health());

                orig_read = raw_buf.read_ptr();
#if BUF_REVERSE
                rev = MAX(0, bytes_consumed_last - MP3_HEADER_SIZE);
                // mp3_buf.debug_read(256, rev);
                printf("reversing buffer by %d: %ld -> %ld\n", rev, raw_buf.get_read_offset(), raw_buf.get_read_offset() - rev);
                raw_buf.read_reverse(rev);
#endif
                ret = align_buffer(orig_read);
                if (ret == -1) {
                    puts("align_buffer failed");
                    return -1;
                }
                if (ret == -2) {
                    // abort
                    return 0;
                }

                again = true;
                break;

            case ERR_MP3_MAINDATA_UNDERFLOW:
                puts("maindata underflow (middle of stream)");
                again = true;
                break;

            case ERR_MP3_INVALID_SIDEINFO:
            case ERR_MP3_INVALID_SCALEFACT:
            case ERR_MP3_INVALID_HUFFCODES:
            case ERR_MP3_INVALID_DEQUANTIZE:
            case ERR_MP3_INVALID_IMDCT:
            case ERR_MP3_INVALID_SUBBAND:
                printf("o %d  frame didn't decode properly (code=%d), trying again\n", raw_buf.get_read_offset(), res);
                raw_buf.read_ack(bytes_consumed);
                again = true;
                break;

            default:
                printf("unknown error code=%d (again %d)\n", res, again);
        }

    } while (again);

    raw_buf.read_ack(bytes_consumed);

    return 1;
}

int FormatMP3::decode_up_to_n(uint32_t* audio_pcm_buf, int n) {
    long frame_offset = 0;
    int frames_read;

    for (frames_read=0; frames_read < n; frames_read++) {
        int decoded = decode_up_to_one_frame(audio_pcm_buf + frame_offset);
        if (decoded < 0)
            return -1;

        if (decoded == 0)
            break;

        frame_offset += MP3_SAMPLES_PER_FRAME;
    }

    return frames_read;
}

void FormatMP3::calculate_stats() {
    MP3GetLastFrameInfo(hMP3Decoder, &frame_info);

    bitrate_sum += frame_info.bitrate;
    bitrate_count += 1;

    if (stats_print) {
        stats_print = false;

        printf("\toutput samples: %d, samplerate: %d, bitrate: %d\n",
               frame_info.outputSamps,
               frame_info.samprate,
               frame_info.bitrate);

        printf("\tms_per_frame: %f, bit freq: %ld\n",
               ms_per_unit(),
               bit_freq());
    }
}

long FormatMP3::bit_freq() {
    return (long)frame_info.bitsPerSample * frame_info.nChans * frame_info.samprate;
}

float FormatMP3::ms_per_unit() {
    return (float)frame_info.outputSamps * 1000 / frame_info.nChans / frame_info.samprate;
}

int FormatMP3::bytes_to_sec(b_type bytes) {
    if (bitrate_count == 0)
        return 0;

    return (int)(bytes / byterate_in());
}

int FormatMP3::bitrate_in() {
    if (bitrate_count == 0)
        return 0;

    return (int)(bitrate_sum / bitrate_count);
}
