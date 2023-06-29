#pragma once


#include <decodebase.hpp>
#include <httpclientpico.hpp>

class DecodeStream : public DecodeBase {

    HttpClientPico client;

public:
    void begin(const char* path_, Format* format_) override;
    int start() override;
    int stop() override;

    // callback called from core0
    void raw_buf_read_msg(unsigned int bytes) override;

    DecodeStream(uint32_t* const audio_pcm_, int audio_pcm_size_words_, volatile bool& a_done_irq_, volatile bool& b_done_irq_, volatile CircularBuffer& http_buf)
        : DecodeBase(audio_pcm_, audio_pcm_size_words_, a_done_irq_, b_done_irq_)
        , client(http_buf)
        { }
};
