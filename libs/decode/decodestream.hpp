#pragma once


#include <decodebase.hpp>
#include <httpclientpico.hpp>

class DecodeStream : public DecodeBase {

    HttpClientPico client;

public:
    void begin(const char* path_, Format* format_) override;
    void end() override;

    void raw_buf_read_cb(unsigned int bytes) override;

    DecodeStream(uint32_t* const audio_pcm_, int audio_pcm_size_words_, volatile bool& a_done_irq_, volatile bool& b_done_irq_, CircularBuffer& http_buf)
        : DecodeBase(audio_pcm_, audio_pcm_size_words_, a_done_irq_, b_done_irq_)
        , client(http_buf)
        { }
};
