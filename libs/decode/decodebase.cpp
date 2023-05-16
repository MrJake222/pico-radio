#include <hardware/timer.h>
#include "decodebase.hpp"

void DecodeBase::core0_init() {

}

bool DecodeBase::core0_loop() {
    // core0 watches unprocessed data buffers
    bool more = data_buffer_watch();

    // finish when no more data to load
    return more;
}

void DecodeBase::core0_end() {
    // wait for core1 to finish execution
    while (!decode_finished());
}

void DecodeBase::core1_init() {
    dma_preload();
}

bool DecodeBase::core1_loop() {
    dma_watch();
    return !decode_finished();
}

void DecodeBase::dma_feed_done(int decoded, int took_us, DMAChannel channel) {
    if (decoded < format.units_to_decode_half()) {
        // dma channel wasn't supplied with enough data -> ending playback
        decode_finished_by = channel == DMAChannel::ChanA
                ? FinishReason::UnderflowChanA
                : FinishReason::UnderflowChanB;
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
        decoded = format.decode_up_to_n(audio_pcm, format.units_to_decode_half());
        t_end = time_us_64();
        dma_feed_done(decoded, (int) (t_end - t_start), DMAChannel::ChanA);
    }

    if (b_done_irq) {
        b_done_irq = false;

        // channel B done (second one)
        // reload second half of the buffer
        t_start = time_us_64();
        decoded = format.decode_up_to_n(audio_pcm + (audio_pcm_size_words / 2), format.units_to_decode_half());
        t_end = time_us_64();
        dma_feed_done(decoded, (int) (t_end - t_start), DMAChannel::ChanB);
    }
}

void DecodeBase::dma_preload() {
    // wait for at least some data to be available
    while (format.raw_buf.get_write_offset() == 0);

    format.decode_exactly_n(audio_pcm, format.units_to_decode_whole());
}
