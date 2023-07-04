#pragma once


#include <decodebase.hpp>
#include <httpclientpico.hpp>

class DecodeStream : public DecodeBase {

    HttpClientPico& client;

    friend void lwip_err_cb(void* arg, int err);

public:
    void begin(const char* path_, Format* format_) override;
    int play() override;
    void stop() override;

    // callback called from core0
    void ack_bytes(uint16_t bytes) override;

    DecodeStream(uint32_t* const audio_pcm_, int audio_pcm_size_words_, volatile bool& a_done_irq_, volatile bool& b_done_irq_, volatile CircularBuffer& cbuf, HttpClientPico& client_, no_param_fn core1_start_)
        : DecodeBase(audio_pcm_, audio_pcm_size_words_, a_done_irq_, b_done_irq_, cbuf, core1_start_)
        , client(client_)
        { }
};
