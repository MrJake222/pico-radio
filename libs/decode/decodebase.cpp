#include <hardware/timer.h>
#include <decodebase.hpp>
#include <mcorefifo.hpp>

#include <cstdio>
#include <lwip/stats.h>

static void raw_buf_read_cb_static(void* arg, unsigned int bytes) {
    // called from core1
    // Here we need to pass a message to core0
    // printf("[%ld us] read %d bytes\n", time_us_32(), bytes);
    fifo_send_with_data(RAW_BUF_READ, bytes);
}

static void raw_buf_read_msg_static(void* arg, uint32_t data) {
    // called from core0 task
    ((DecodeBase*) arg)->raw_buf_read_msg(data);
}

static void player_wake_end(void* arg, uint32_t data) {
    // called from core0 fifo task or directly from core0
    xTaskNotifyGive(((DecodeBase*) arg)->player_task);
}

void DecodeBase::begin(const char* path_, Format* format_) {
    path = path_;
    format = format_;
    player_task = xTaskGetCurrentTaskHandle();

    format->begin();
    format->raw_buf.set_read_ack_callback(this, raw_buf_read_cb_static);
    fifo_register(RAW_BUF_READ, raw_buf_read_msg_static, this, true);
    fifo_register(PLAYER_WAKE_END, player_wake_end, this, true);

    decode_finished_by = FinishReason::NoFinish;
    sum_units_decoded = 0;
    last_seconds = -1;
}

void DecodeBase::notify_playback_end() {
    if (get_core_num() == 1) {
        // core1 -- non-local core for RTOS
        fifo_send(PLAYER_WAKE_END);
    }
    else {
        // core0 -- notify RTOS directly
        player_wake_end(this, 0);
    }
}

void DecodeBase::core1_init() {
    dma_preload();
}

bool DecodeBase::core1_loop() {
    dma_watch();
    return !decode_finished();
}

void DecodeBase::dma_feed_done(int decoded, int took_us, DMAChannel channel) {
    if (decoded < format->units_to_decode_half()) {
        // dma channel wasn't supplied with enough data -> ending playback
        decode_finished_by = channel == DMAChannel::ChanA
                ? FinishReason::UnderflowChanA
                : FinishReason::UnderflowChanB;

        notify_playback_end();
    }

    sum_units_decoded += decoded; // TODO try to use raw_buf->read_bytes_total();
    int seconds = format->units_to_sec(sum_units_decoded);

    if (seconds != last_seconds) {
        last_seconds = seconds;

        int duration = format->duration_sec(source_size_bytes());
        float took_ms = (float)took_us / 1000.f / (float)format->units_to_decode_half();

        // TODO move this to a callback/task
        // this is on core1, probably easier to just create a task with interval of 1 sec
        printf("\n\n\n-------------------------------------------------------------------------------------------------------- ");
        printf("%02d:%02d / %02d:%02d   decode %5.2fms %2d%%   health %2d%%\n",
               seconds/60, seconds%60,
               duration/60, duration%60,
               took_ms,
               int(took_ms * 100 / format->ms_per_unit()),
               format->raw_buf.health()
        );

        stats_display();
    }
}

void DecodeBase::dma_watch() {
    uint64_t t_start, t_end;
    int decoded;

    if (a_done_irq) {
        a_done_irq = false;

        // channel A done (first one)
        // reload first half of the buffer
        t_start = time_us_64();
        decoded = format->decode_up_to_n(audio_pcm, format->units_to_decode_half());
        t_end = time_us_64();
        dma_feed_done(decoded, (int) (t_end - t_start), DMAChannel::ChanA);
    }

    if (b_done_irq) {
        b_done_irq = false;

        // channel B done (second one)
        // reload second half of the buffer
        t_start = time_us_64();
        decoded = format->decode_up_to_n(audio_pcm + (audio_pcm_size_words / 2), format->units_to_decode_half());
        t_end = time_us_64();
        dma_feed_done(decoded, (int) (t_end - t_start), DMAChannel::ChanB);
    }
}

void DecodeBase::dma_preload() {
    // wait for at least some data to be available
    // can't use <data_left_*> methods because read_ptr might be equal to write_ptr (undefined behavior)
    while (format->raw_buf.get_write_offset() == 0);

    format->decode_header();
    format->decode_exactly_n(audio_pcm, format->units_to_decode_whole());
}