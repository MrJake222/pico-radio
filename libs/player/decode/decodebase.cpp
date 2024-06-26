#include <hardware/timer.h>
#include <decodebase.hpp>
#include <mcorefifo.hpp>

#include <cstdio>
#include <lwip/stats.h>
#include <cstring>

void cbuf_read_cb(void* arg, unsigned int bytes) {
    auto dec = (DecodeBase*) arg;

    // don't let large reads happen
    // (threads must have time to top up the buffer)
    // wave reads half of buffer
    assert(bytes <= dec->cbuf.size / 2);

    auto b16 = (uint16_t) bytes;
    assert(b16 == bytes);
    dec->notify_ack(b16);
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

    format->begin(&abort, &eof);
    cbuf.reset_with_cb();
    // callback set was moved to play()
    // (it pushes to queue and nothing reads from it here)
    fifo_register(PLAYER, player_msg, this, false);

    decode_finished_by = FinishReason::NoFinish;
    frame_decode_time_ms = 0;
    abort = false;
    eof = false;
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
    // watch out for lfs flash access

    pico_lfs_launch_core1(core1_entry);

    // handle messages
    bool error = false;
    while (true) {
        uint32_t msg;
        xQueueReceive(queue,
                      &msg,
                      portMAX_DELAY);

        auto type = (DecodeMsgType) MSG_TYPE(DECODE_MSG_BITS, DECODE_MSG_TYPE_BITS, msg);
        auto data = (uint16_t)      MSG_DATA(DECODE_MSG_BITS, DECODE_MSG_TYPE_BITS, msg);

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

    pico_lfs_reset_core1();

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
        return;
    }

    if (decoded < format->units_to_decode_half(audio_pcm_size_words)) {
        // dma channel wasn't supplied with enough data -> ending playback
        decode_finished_by = channel == DMAChannel::ChanA
                ? FinishReason::UnderflowChanA
                : FinishReason::UnderflowChanB;

        // TODO configure dma channel to handle less samples than full buffer (requires abstracting dma code out of player.cpp)
        // dma_channel_set_trans_count(dma)
        // format->units_to_bytes(decoded);

        notify_playback_end(false);
    }

    frame_decode_time_ms = (float)took_us / 1000.f / decoded;
}

int DecodeBase::dma_preload() {

    int decoded;
    int whole;
    int r;

    // wait for data in buffer
    puts("core1: waiting for data");
    r = cbuf.wait_for_health(BUF_HEALTH_MIN, abort);
    if (r < 0)
        goto fail;

    puts("core1: data loaded");
    cbuf.debug_read(32, 0);
    puts("");

    r = format->decode_header();
    if (r < 0) {
        // failure, couldn't decode header (possibly required below)
        puts("dma_preload: decode_header failed");
        goto fail;
    }

    whole = format->units_to_decode_whole(audio_pcm_size_words);
    decoded = format->decode_up_to_n(audio_pcm, whole);
    printf("core1: preloaded %d/%d frames\n", decoded, whole);
    if (decoded < whole) {
        // failure, couldn't decode enough frames
        // probably a user abort (buffer has enough data because of loop above)
        puts("dma_preload: decoded less than <whole> units");
        goto fail;
    }

    return 0;

fail:
    notify_playback_end(true);
    return -1;
}

void DecodeBase::dma_watch() {
    uint64_t t_start, t_end;
    int decoded;

    // TODO add wfi here (requires enabling dma irq0 on core1 -> requires abstracting dma code out of player.cpp)
    // __wfi();

    if (a_done_irq) {
        a_done_irq = false;

        // channel A done (first one)
        // reload first half of the buffer
        t_start = time_us_64();
        decoded = format->decode_up_to_n(audio_pcm, format->units_to_decode_half(audio_pcm_size_words));
        t_end = time_us_64();
        dma_feed_done(decoded, (int) (t_end - t_start), DMAChannel::ChanA);
    }

    if (b_done_irq) {
        b_done_irq = false;

        // channel B done (second one)
        // reload second half of the buffer
        t_start = time_us_64();
        decoded = format->decode_up_to_n(audio_pcm + (audio_pcm_size_words / 2), format->units_to_decode_half(audio_pcm_size_words));
        t_end = time_us_64();
        dma_feed_done(decoded, (int) (t_end - t_start), DMAChannel::ChanB);
    }
}