#include "mp3.hpp"

#include <pico/stdlib.h>
#include <cstdio>
#include <cstring>
#include "f_util.h"
#include "helixmp3/pub/mp3common.h"

const uint PIN_DBG_MP3 = 12;
#define DBG_MP3_ON() gpio_put(PIN_DBG_MP3, true)
#define DBG_MP3_OFF() gpio_put(PIN_DBG_MP3, false)

static void fs_err(FRESULT fr, const char* tag) {
    panic("%s: %s (id=%d)\n", tag, FRESULT_str(fr), fr);
}

void MP3::open() {
    puts("open base");
    fr = f_open(&fp, filepath, FA_READ);
    if (fr != FR_OK) {
        fs_err(fr, "f_open");
    }
}

void MP3::close() {
    f_close(&fp);
}

bool MP3::low_on_data() {
    return mp3_buf.data_left() < BUF_MP3_SIZE_BYTES / 2;
}

void MP3::load_buffer(int bytes) {
    // printf("loading, offset %ld  load_at %ld  ", mp3_buf.get_read_offset(), mp3_buf.get_write_offset());

    uint read;
    fr = f_read(&fp, mp3_buf.write_ptr(), bytes, &read);
    if (fr != FR_OK) {
        fs_err(fr, "f_read");
    }

    if (read < bytes) {
        printf("EOF\n");
        eof = true;
    }

    // printf("loaded %d bytes\n", read);

    mp3_buf.write_ack(read);

    // printf("loaded,  offset %ld  load_at %ld\n", mp3_buf.get_read_offset(), mp3_buf.get_write_offset());
}

void MP3::preload_pcm_buffer() {
    decode_exactly_n_frames((int16_t*) audio_pcm, BUF_PCM_SIZE_FRAMES);
}

void MP3::align_buffer() {
    uint8_t* orig_read_ptr = mp3_buf.read_ptr();

    int matched;
    do {
        if (mp3_buf.data_left() < MP3_HEADER_SIZE)
            continue;

        // printf("read at %ld  avail %ld  ", mp3_buf.get_read_offset(), mp3_buf.data_left());
        int sync_word_offset = MP3FindSyncWord(mp3_buf.read_ptr(), mp3_buf.data_left_continuous());
        // printf("sync_word_offset: %d   read %ld   len %ld\n", sync_word_offset, mp3_buf.get_read_offset(), mp3_buf.data_left_continuous());
        if (sync_word_offset < 0) {
            // failed

            // save potential last header & wrap
            mp3_buf.set_read_ptr_end(MP3_HEADER_SIZE);
            if (mp3_buf.can_wrap_buffer())
                mp3_buf.wrap_buffer();
            //load_buffer(BUF_MP3_SIZE_BYTES);
        }
        else {
            printf("offset %ld  avail %ld  ", mp3_buf.get_read_offset(), mp3_buf.data_left());

            mp3_buf.read_ack(sync_word_offset);

            matched = MP3CheckSyncWordRepeated(hMP3Decoder, mp3_buf.read_ptr(), mp3_buf.data_left_continuous());
            if (matched == 0) {
                // failed
                mp3_buf.read_ack(1);
            }

            printf("matched: %d\n", matched);
        }
    } while (matched < 1);



    calculate_stats();
}

void MP3::prepare() {
    mp3_buf.reset();

    init_dbg();
    open();

    if (!hMP3Decoder)
        hMP3Decoder = MP3InitDecoder();
    else
        MP3ClearBuffers(hMP3Decoder);

    eof = false;
    eop = false;

    // preload_pcm_buffer file buffer
    /*puts("preload_pcm_buffer");
    load_buffer(BUF_MP3_SIZE_BYTES);
    puts("align");
    align_buffer();
    puts("align done");*/

    // preload_pcm_buffer pcm buffer
    /*for (int i=0; i<BUF_PCM_SIZE_FRAMES; i++) {
        decode_up_to_one_frame((int16_t *)(audio_pcm + i * MP3_SAMPLES_PER_FRAME));
        watch_file_buffer();
    }*/

    decode_finished_by = NoAbort;
    sum_frames_decoded = 0;
    took_ms = 0;
    seconds = 0;
    last_seconds = -1;
}

void MP3::init_dbg() {
    gpio_init(PIN_DBG_MP3);
    gpio_set_dir(PIN_DBG_MP3, GPIO_OUT);
    DBG_MP3_OFF();
}

void MP3::calculate_stats() {
    int ret = MP3GetNextFrameInfo(hMP3Decoder, &frame_info, mp3_buf.read_ptr());
    if (ret) {
        printf("failed to decode frame information\n");
        return;
    }

    sec_per_frame = (float)frame_info.outputSamps / frame_info.nChans / frame_info.samprate;
    duration = ((int)f_size(&fp) - mp3_buf.get_read_offset()) * 8 / frame_info.bitrate;
    bit_freq = frame_info.bitsPerSample * frame_info.nChans * frame_info.samprate;

    printf("\toutput samples: %d, samplerate: %d, bitrate: %d\n",
           frame_info.outputSamps,
           frame_info.samprate,
           frame_info.bitrate);

    printf("\tms_per_frame: %f, bit freq: %ld\n",
           sec_per_frame * 1000,
           bit_freq);
}

void MP3::watch_file_buffer() {
    if (low_on_data()) {
        // low on data

        if (eof)
            // eof, after wrap, set end-of-playback
            eop = true;

        else
            // no eof -> just load more
            load_buffer(BUF_MP3_SIZE_BYTES/2);
    }
}

void MP3::watch_timer() {
    if (seconds != last_seconds) {
        printf("%02d:%02d / %02d:%02d   decode %5.2fms %2d%%   health %2d%%\n",
               seconds/60, seconds%60,
               get_duration()/60, get_duration()%60,
               took_ms,
               int(took_ms * 100 / get_ms_per_frame()),
               mp3_buf.health()
        );

        last_seconds = seconds;
    }
}

const static int LOAD_FRAMES = BUF_PCM_SIZE_FRAMES / 2;

void MP3::decode_done(int decoded_frames, uint64_t took_us, FinishReason channel) {
    sum_frames_decoded += decoded_frames;
    seconds = frames_to_sec(sum_frames_decoded);
    took_ms = (float)took_us / 1000.f / (float)LOAD_FRAMES;

    if (decoded_frames < LOAD_FRAMES) {
        // dma channel wasn't supplied with enough data -> EOF
        decode_finished_by = channel;
    }
}

void MP3::watch_decode(volatile bool& a_done_irq, volatile bool& b_done_irq) {

    int decoded;
    uint64_t start, end;

    if (a_done_irq) {
        a_done_irq = false;

        // channel A done (first one)
        // reload first half of the buffer
        start = time_us_64();
        DBG_MP3_ON();
        decoded = decode_up_to_n_frames((int16_t *) audio_pcm, LOAD_FRAMES);
        DBG_MP3_OFF();
        end = time_us_64();
        decode_done(decoded, end - start, UnderflowChanA);
    }

    if (b_done_irq) {
        b_done_irq = false;

        // channel B done (second one)
        // reload second half of the buffer
        start = time_us_64();
        DBG_MP3_ON();
        decoded = decode_up_to_n_frames((int16_t *) (audio_pcm + BUF_PCM_HALF_32BIT), LOAD_FRAMES);
        DBG_MP3_OFF();
        end = time_us_64();
        decode_done(decoded, end - start, UnderflowChanB);
    }
}

int MP3::decode_up_to_one_frame(int16_t* audio_pcm_buf) {
    // watch_file_buffer();

    long bytes_consumed;

    bool again;
    do {
        // printf("o %ld\n", offset);
        if (mp3_buf.data_left_continuous() < MP3_HEADER_SIZE) {
            // even the header won't fit in continuous buffer
            mp3_buf.wrap_buffer();
        }

        uint8_t* dptr_orig = mp3_buf.read_ptr();
        uint8_t* dptr = dptr_orig;
        int b = mp3_buf.data_left_continuous();

        int res = MP3Decode(
                hMP3Decoder,
                &dptr,
                &b,
                audio_pcm_buf,
                0);

        bytes_consumed = dptr - dptr_orig;

        /*if (res != ERR_MP3_NONE) {
            printf("err %d\n", res);
        }*/

        again = false;
        switch (res) {
            case ERR_MP3_NONE:
                break;

            case ERR_MP3_INDATA_UNDERFLOW:
                if (eop)
                    return 0;
                if (mp3_buf.can_wrap_buffer())
                    mp3_buf.wrap_buffer();
                again = true;
                break;

            case ERR_MP3_INVALID_FRAMEHEADER:
                printf("o %ld  wrong sync-word\n", mp3_buf.get_read_offset());
                align_buffer();
                again = true;
                break;

            case ERR_MP3_MAINDATA_UNDERFLOW:
                puts("maindata underflow (middle of stream)");
                again = true;
                break;

            default:
                printf("unknown error code=%d\n", res);
        }

    } while (again);

    mp3_buf.read_ack(bytes_consumed);

    return 1;
}

int MP3::decode_up_to_n_frames(int16_t* audio_pcm_buf, int n) {
    long frame_offset = 0;
    int frames_read;

    for (frames_read=0; frames_read < n; frames_read++) {
        int decoded = decode_up_to_one_frame(audio_pcm_buf + frame_offset);
        if (decoded == 0)
            break;

        frame_offset += MP3_SAMPLES_PER_FRAME * 2;
    }

    return frames_read;
}

void MP3::decode_exactly_n_frames(int16_t* audio_pcm_buf, int n) {
    long frame_offset = 0;
    int frames_read;

    for (frames_read=0; frames_read < n;) {
        frames_read += decode_up_to_one_frame(audio_pcm_buf + frame_offset);
        frame_offset += MP3_SAMPLES_PER_FRAME * 2;
    }
}
