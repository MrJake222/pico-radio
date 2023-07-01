#pragma once


#include <decodebase.hpp>
#include <httpclientpico.hpp>

class DecodeStream : public DecodeBase {

    HttpClientPico client;

    friend void lwip_err_cb(void* arg, int err);

public:
    void begin(const char* path_, Format* format_) override;
    int play_() override;
    int stop() override;

    // callback called from core0
    void raw_buf_just_read(unsigned int bytes) override;

    DecodeStream(uint32_t* const audio_pcm_, int audio_pcm_size_words_, volatile bool& a_done_irq_, volatile bool& b_done_irq_, volatile CircularBuffer& http_buf, volatile CircularBuffer& cbuf)
        : DecodeBase(audio_pcm_, audio_pcm_size_words_, a_done_irq_, b_done_irq_)
        , client(http_buf, cbuf)
        { }
};
