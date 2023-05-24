#pragma once


#include <cstdint>
#include "circularbuffer.hpp"

class Format {

protected:
    // used to end decoding when no more continuous data is present
    bool eop;

public:
    volatile CircularBuffer& raw_buf;

    Format(volatile CircularBuffer& raw_buf_)
        : raw_buf(raw_buf_)
        { }

    void set_eop() { eop = true; }

    // call every time before decoding starts
    virtual void init() {
        raw_buf.reset();
        eop = false;
    }

    // returns number of units to decode to fill whole buffer
    // (wav bytes or mp3 frames)
    // halved passed as n to decode_up_to_n
    virtual int units_to_decode_whole() = 0;
    int units_to_decode_half() { return units_to_decode_whole() / 2; }

    // called upon first interaction with the buffer
    // has default implementation as not all formats have headers
    virtual void decode_header() { }

    // this return number of units actually decoded
    // "decode" means move from <this->raw_buf> to <audio_pcm_buf> (possibly doing some work)
    // audio_pcm_buf can't be in class because it changes over time (first/second half of the buffer)
    virtual int decode_up_to_n(uint32_t* audio_pcm_buf, int n) = 0;
    // won't return before decoding <n> units exactly
    virtual void decode_exactly_n(uint32_t* audio_pcm_buf, int n) = 0;

    virtual long bit_freq() = 0;
    virtual float ms_per_unit() = 0;
    virtual int units_to_sec(int units) = 0;
    virtual int duration_sec(int file_size_bytes) = 0;
};
