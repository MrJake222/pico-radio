#pragma once

#include "mp3.hpp"
#include "httpclientpico.hpp"

class MP3Radio : public MP3 {
    HttpClientPico client;

protected:
    void open() override;
    void close() override;
    bool low_on_data() override;
    void load_buffer(int bytes) override;

public:
    MP3Radio(const char *filepath_, uint32_t* const audio_pcm_)
        : MP3(filepath_, audio_pcm_)
        , client(mp3_buf) // here all decoding takes place
        { }


};
