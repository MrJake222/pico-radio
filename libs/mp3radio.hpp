#pragma once

#include "mp3.hpp"

class MP3Radio : public MP3 {
    void open() override;
    void close() override;
    bool low_on_data() override;
    void load_buffer(int bytes) override;

/*public:
    MP3Radio(uint32_t* const audio_pcm_)
        : MP3("", audio_pcm_)
        { }*/

    using MP3::MP3;
};
