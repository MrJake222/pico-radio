#pragma once
#include <cstdint>
#include <format.hpp>

enum class FinishReason {
    NoFinish,
    UnderflowChanA,
    UnderflowChanB
};

enum class DMAChannel {
    ChanA,
    ChanB
};

class DecodeBase {

    // set on feeding DMA partially filled data
    // (or on user abort)
    FinishReason decode_finished_by;

    /* ------------ Private helpers ------------ */
    // called to gracefully finish core0 thread
    // gates core0 not to exit before DMA decodes latest unit of information
    bool decode_finished() { return decode_finished_by != FinishReason::NoFinish; }

    // raw PCM data buffer & size
    uint32_t* const audio_pcm;
    const int audio_pcm_size_words;

    /* ---------- DMA feed handling CORE 1 ---------- */
    // references to DMA finish flags
    // A done -> reload 1st half of audio_pcm
    // B done -> reload 2nd half of audio_pcm
    volatile bool& a_done_irq;
    volatile bool& b_done_irq;
    // uses dma flags to load specific part of the buffer
    // these function are run on <feed_dma_core> result
    void dma_watch();
    void dma_preload();

    // play stats
    int sum_units_decoded;
    int last_seconds;

protected:
    // generic path to resource
    // (used by implementations to connect open file/stream)
    const char* path;

    // format decoder
    // needs to be protected for data loading
    Format* format;

    // designed to watch file buffers
    // (or do nothing in internet streams)
    // returns: true on success, false when there is no more data to load
    virtual bool data_buffer_watch() { return true; }
    // called when <dma_watch> actually loads some data
    virtual void dma_feed_done(int decoded, int took_us, DMAChannel channel);

    // return source medium size in bytes
    virtual int source_size_bytes() { return 0; }

public:
    DecodeBase(uint32_t* const audio_pcm_, int audio_pcm_size_words_, volatile bool& a_done_irq_, volatile bool& b_done_irq_)
            : audio_pcm(audio_pcm_)
            , audio_pcm_size_words(audio_pcm_size_words_)
            , a_done_irq(a_done_irq_)
            , b_done_irq(b_done_irq_)
//            , path(path_)
//            , format(format_)
    { }

    virtual ~DecodeBase() = default;

    // called before and after decoding
    virtual void begin(const char* path_, Format* format_) {
        path = path_;
        format = format_;

        format->init();
        decode_finished_by = FinishReason::NoFinish;
        sum_units_decoded = 0;
        last_seconds = -1;
    }

    virtual void end() { }

    // functions called from core0
    void core0_init();
    bool core0_loop();
    void core0_end();
    // after core0_end caller needs to wait for DMA
    bool decode_finished_by_A() { return decode_finished_by == FinishReason::UnderflowChanA; }
    bool decode_finished_by_B() { return decode_finished_by == FinishReason::UnderflowChanB; }
    // called on user abort (from core0) to abort core1
    void user_abort() { format->set_eop(); }

    // functions called from core1
    void core1_init();
    bool core1_loop();

    long bit_freq() { return format->bit_freq(); }
};
