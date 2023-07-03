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

void raw_buf_read_msg_static(void* arg, uint32_t data) {
    // called from core0 fifo IRQ
    uint32_t msg = MSG_MAKE(BUF_READ, data);

    xQueueSendFromISR(((DecodeBase*) arg)->queue,
                      &msg,
                      nullptr);
}

void player_wake(void* arg, uint32_t error) {
    // called from core0 fifo IRQ
    uint32_t msg = MSG_MAKE(error ? ERROR : END, 0);

    xQueueSendFromISR(((DecodeBase*) arg)->queue,
                      &msg,
                      nullptr);
}

void DecodeBase::begin(const char* path_, Format* format_) {
    path = path_;
    format = format_;

    format->begin();
    format->raw_buf.reset_with_cb();
    format->raw_buf.set_read_ack_callback(this, raw_buf_read_cb_static);
    fifo_register(RAW_BUF_READ, raw_buf_read_msg_static, this, false);
    fifo_register(PLAYER_WAKE, player_wake, this, false);

    decode_finished_by = FinishReason::NoFinish;
    sum_units_decoded = 0;
    last_seconds = -1;
}

int DecodeBase::play() {

    queue = xQueueCreate(8, sizeof(uint32_t));
    if (!queue) {
        puts("failed to create queue");
        return -1;
    }

    int ret = play_();
    if (ret < 0)
        return -1;

    bool error = false;
    while (true) {
        uint32_t msg;
        xQueueReceive(queue,
                      &msg,
                      portMAX_DELAY);

        auto type = (DecodeMsgType) MSG_TYPE(msg);

        uint32_t min_free_stack = uxTaskGetStackHighWaterMark(nullptr);

        // printf("rtos ram used: %2d%%  max %2d%%  player unused stack: %ld\n",
        //        (configTOTAL_HEAP_SIZE - xPortGetFreeHeapSize()) * 100 / configTOTAL_HEAP_SIZE,
        //        (configTOTAL_HEAP_SIZE - xPortGetMinimumEverFreeHeapSize()) * 100 / configTOTAL_HEAP_SIZE,
        //        min_free_stack);

        if (type == BUF_READ) {
            raw_buf_just_read(MSG_DATA(msg));
        }
        else if (type == ERROR) {
            puts("playback error");
            error = true;
            break;
        }
        else if (type == END) {
            break;
        }
    }

    return error ? -1 : 0;
}

int DecodeBase::stop() {
    vQueueDelete(queue);

    return 0;
}

void DecodeBase::notify_playback_end(bool error) {
    if (get_core_num() == 1) {
        // core1 -- non-local core for RTOS
        fifo_send_with_data(PLAYER_WAKE, error);
    }
    else {
        // core0 -- notify RTOS directly
        uint32_t msg = MSG_MAKE(error ? ERROR : END, 0);
        xQueueSend(queue, &msg, portMAX_DELAY);
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

        notify_playback_end(false);
    }

    sum_units_decoded += decoded; // TODO try to use raw_buf->read_bytes_total();
    int seconds = format->units_to_sec(sum_units_decoded);

    if (seconds != last_seconds) {
        last_seconds = seconds;

        int duration = format->duration_sec(source_size_bytes());
        float took_ms = (float)took_us / 1000.f / (float)format->units_to_decode_half();

        // TODO move this to a callback/task
        // this is on core1, probably easier to just create a task with interval of 1 sec
        // printf("\n\n\n-------------------------------------------------------------------------------------------------------- ");
        printf("%02d:%02d / %02d:%02d   decode %5.2fms %2d%%   health %2d%%\n",
               seconds/60, seconds%60,
               duration/60, duration%60,
               took_ms,
               int(took_ms * 100 / format->ms_per_unit()),
               format->raw_buf.health()
        );
        //
        // stats_display();
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