#pragma once


#include "decodebase.hpp"
#include "httpclientpico.hpp"

class DecodeStream : public DecodeBase {

    HttpClientPico client;

public:
    void begin() override;
    void end() override;

    DecodeStream(uint32_t* const audio_pcm_, int audio_pcm_size_words_, volatile bool& a_done_irq_, volatile bool& b_done_irq_, const char* path_, Format& format_)
        : DecodeBase(audio_pcm_, audio_pcm_size_words_, a_done_irq_, b_done_irq_, path_, format_)
        , client(format_.raw_buf)
        { }
};
