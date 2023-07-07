#include <hardware/timer.h>
#include <decodebase.hpp>
#include <mcorefifo.hpp>

#include <cstdio>
#include <lwip/stats.h>
#include <pico/multicore.h>

void cbuf_read_cb(void* arg, unsigned int bytes) {
    auto b16 = (uint16_t) bytes;
    assert(b16 == bytes);

    ((DecodeBase*) arg)->notify_ack(b16);
}

void player_msg(void* arg, uint32_t msg) {
    // called from core0 fifo IRQ

    xQueueSendFromISR(((DecodeBase*) arg)->queue,
                      &msg,
                      nullptr);
}

void DecodeBase::begin(const char* path_, Format* format_) {
    path = path_;
    format = format_;

    format->begin();
    cbuf.reset_with_cb();
    // callback set was moved to play()
    // (it pushes to queue and nothing reads from it here)
    fifo_register(PLAYER, player_msg, this, false);

    decode_finished_by = FinishReason::NoFinish;
    sum_units_decoded = 0;
    last_seconds = -1;
}

int DecodeBase::setup() {
    queue = xQueueCreate(8, sizeof(uint32_t));
    if (!queue) {
        puts("failed to create queue");
        return -1;
    }

    return 0;
}

int DecodeBase::play() {

    // redirect ACKs through fifo
    cbuf.set_read_ack_callback(this, cbuf_read_cb);
    // start decoding on core1
    multicore_launch_core1(core1_entry);

    // handle messages
    bool error = false;
    while (true) {
        uint32_t msg;
        xQueueReceive(queue,
                      &msg,
                      portMAX_DELAY);

        auto type = (DecodeMsgType) MSG_TYPE(DECODE_MSG_BITS, DECODE_MSG_TYPE_BITS, msg);
        auto data = (uint16_t)      MSG_DATA(DECODE_MSG_BITS, DECODE_MSG_TYPE_BITS, msg);

        // uint32_t min_free_stack = uxTaskGetStackHighWaterMark(nullptr);
        // printf("rtos ram used: %2d%%  max %2d%%  player unused stack: %ld\n",
        //        (configTOTAL_HEAP_SIZE - xPortGetFreeHeapSize()) * 100 / configTOTAL_HEAP_SIZE,
        //        (configTOTAL_HEAP_SIZE - xPortGetMinimumEverFreeHeapSize()) * 100 / configTOTAL_HEAP_SIZE,
        //        min_free_stack);

        if (type == BUF_READ) {
            ack_bytes(data);
        }
        else if (type == END) {
            puts("playback end");

            if (data) {
                puts("playback error");
                error = true;
            }

            break;
        }
    }

    multicore_reset_core1();

    return error ? -1 : 0;
}

void DecodeBase::end() {
    if (queue)
        vQueueDelete(queue);
}

void DecodeBase::notify(uint8_t type, uint16_t data) {
    uint32_t msg = MSG_MAKE(DECODE_MSG_BITS, DECODE_MSG_TYPE_BITS, type, data);

    if (get_core_num() == 1) {
        // core1 -- non-local core for RTOS
        fifo_send_with_data(PLAYER, msg);
    }
    else {
        // core0 -- notify RTOS directly
        xQueueSend(queue, &msg, portMAX_DELAY);
    }
}

void DecodeBase::dma_feed_done(int decoded, int took_us, DMAChannel channel) {
    if (decoded < 0) {
        // indicates an error
        puts("dma feed failed");
        notify_playback_end(true);
    }

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
               cbuf.health()
        );
        //
        // stats_display();
    }
}

void DecodeBase::dma_preload() {

    // wait for data in buffer
    puts("core1: waiting for data");
    while (cbuf.health() < min_health);

    puts("core1: data loaded");
    cbuf.debug_read(32, 0);

    // TODO move decode header to core1 (along with any other metadata decoding)
    // or maybe not, it'd require reads of possibly mp3 stream if no header
    // or implement peek() function
    format->decode_header();
    // TODO remove "exactly_n" functions
    // they were used before while health
    format->decode_exactly_n(audio_pcm, format->units_to_decode_whole());
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