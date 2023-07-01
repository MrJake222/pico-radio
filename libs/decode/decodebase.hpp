#pragma once
#include <cstdint>
#include <format.hpp>
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

enum class FinishReason {
    NoFinish,
    UnderflowChanA,
    UnderflowChanB
};

enum class DMAChannel {
    ChanA,
    ChanB
};

enum DecodeMsgType {
    BUF_READ,
    ERROR,
    END
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
    void dma_watch();
    void dma_preload();

    // play stats
    int sum_units_decoded;
    int last_seconds;

    // Main task variables
    xQueueHandle queue;
    friend void raw_buf_read_msg_static(void* arg, uint32_t data);
    friend void player_wake(void* arg, uint32_t error);

protected:
    // generic path to resource
    // (used by implementations to connect open file/stream)
    const char* path;

    // format decoder
    // needs to be protected for data loading
    Format* format;

    // called when <dma_watch> actually loads some data
    virtual void dma_feed_done(int decoded, int took_us, DMAChannel channel);

    // return source medium size in bytes
    virtual int source_size_bytes() { return 0; }

    // for internal usage
    // notifies playback either directly or via fifo
    // to be used when natural content end occurred (dma underflow)
    // or on error on data read
    void notify_playback_end(bool error);

    // Notifies when format reads from the raw buffer
    // Called from core0
    virtual void raw_buf_just_read(unsigned int bytes) { }

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
    // serves as a constructor with changing parameters
    // can't fail
    virtual void begin(const char* path_, Format* format_);

    // does the heavy work (connect/open file)
    // can fail with non-zero return value
    // start blocks calling task or fails
    virtual int play();
    virtual int stop() { return 0; }

    // after core0_end caller needs to wait for DMA
    bool decode_finished_by_A() { return decode_finished_by == FinishReason::UnderflowChanA; }
    bool decode_finished_by_B() { return decode_finished_by == FinishReason::UnderflowChanB; }
    // called on user abort (from core0) to abort core1
    void abort_user() { format->set_user_abort(); }

    // functions called from core1
    void core1_init();
    bool core1_loop();

    long bit_freq() { return format->bit_freq(); }
};