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

#define DECODE_MSG_BITS         24
#define DECODE_MSG_TYPE_BITS     8

enum DecodeMsgType {
    BUF_READ,
    END
};

typedef void(*entry_fn)();

class DecodeBase {

    // set on feeding DMA partially filled data
    // (or on user abort)
    FinishReason decode_finished_by;

    // raw PCM data buffer & size
    uint32_t* const audio_pcm;
    const int audio_pcm_size_words;

    /* ---------- DMA feed handling CORE 1 ---------- */
    // references to DMA finish flags
    // A done -> reload 1st half of audio_pcm
    // B done -> reload 2nd half of audio_pcm
    volatile bool& a_done_irq;
    volatile bool& b_done_irq;

    // called when <dma_watch> actually loads some data
    void dma_feed_done(int decoded, int took_us, DMAChannel channel);

    // TODO refactor
    // return source medium size in bytes
    virtual int source_size_bytes() { return 0; }
    // play stats
    int sum_units_decoded;
    int last_seconds;

    // data format to decode
    Format* format;

    // Main task variables
    xQueueHandle queue;
    friend void cbuf_read_cb(void* arg, unsigned int bytes);
    friend void player_msg(void* arg, uint32_t error);

    // notifies RTOS task either directly (called from core0) or via fifo (called from core1)
    void notify(uint8_t type, uint16_t data);

    // notify RTOS task about buffer being read
    inline void notify_ack(unsigned short bytes) { notify(BUF_READ, bytes); }

    // Called from core 0 RTOS task after receiving a notification sent via read callback
    virtual void ack_bytes(uint16_t bytes) = 0;

    // function to be called to start core1
    // called after filling up at least <min_health>% of buffer filled
    entry_fn core1_entry;

protected:
    static const int min_health = 50;

    // generic path to resource
    // (used by implementations to connect open file/stream)
    const char* path;

    // content buffer
    volatile CircularBuffer& cbuf;

    // notify about natural content end occurred (dma underflow) or error
    inline void notify_playback_end(bool error) { notify(END, error); }

    // called when there is a data underflow
    void set_eop() { format->set_eop(); }

public:
    DecodeBase(uint32_t* const audio_pcm_, int audio_pcm_size_words_, volatile bool& a_done_irq_, volatile bool& b_done_irq_, volatile CircularBuffer& cbuf_, entry_fn core1_entry_)
            : audio_pcm(audio_pcm_)
            , audio_pcm_size_words(audio_pcm_size_words_)
            , a_done_irq(a_done_irq_)
            , b_done_irq(b_done_irq_)
            , cbuf(cbuf_)
            , core1_entry(core1_entry_)
    { }

    virtual ~DecodeBase() = default;

    // called before play
    // "constructor" for changing parameters (can't fail)
    virtual void begin(const char* path_, Format* format_);

    // called before play
    // setups queue (can fail, but needs to be done before play)
    int setup();

    // does the heavy work (connect/open file)
    // blocks calling task or fails
    // subclasses should override and call base class
    // but not before buffer health is at least <min_buffer>%
    virtual int play();

    // called after play
    // cleans up, runs always
    // check what should be closed/destroyed
    virtual void stop();

    // core1 should decode units of data while this is false
    bool decode_finished() { return decode_finished_by != FinishReason::NoFinish; }
    // after play() returns caller needs to wait for DMA
    bool decode_finished_by_A() { return decode_finished_by == FinishReason::UnderflowChanA; }
    bool decode_finished_by_B() { return decode_finished_by == FinishReason::UnderflowChanB; }
    // called on user abort (from core0) to abort core1
    void abort_user() { format->set_user_abort(); }

    /* ---------- DMA feed handling CORE 1 ---------- */
    // uses dma flags to load specific part of the buffer
    void dma_preload();
    void dma_watch();

    // this becomes valid after <dma_preload>
    long bit_freq() { return format->bit_freq(); }
};