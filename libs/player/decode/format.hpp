#pragma once


#include <cstdint>
#include <circularbuffer.hpp>

class Format {

protected:
    // used to abort playback as fast as possible
    bool abort;

    volatile CircularBuffer& raw_buf;

public:
    Format(volatile CircularBuffer& raw_buf_)
        : raw_buf(raw_buf_)
        { }

    // used on end-of-file/stream and on user abort
    void set_abort() { abort = true; }

    // call every time before decoding starts
    virtual void begin() {
        abort = false;
    }

    // tries to wrap buffer but on data underflow waits for
    // incoming data (busy waiting). Returns immediately on user abort.
    // Returns 0 on success -1 on failure (not enough "hidden" section in CircularBuffer), -2 on user abort
    int wrap_buffer_wait_for_data();

    // returns number of units to decode to fill whole PCM buffer (size passed down)
    // (wav bytes or mp3 frames)
    // halved passed as n to decode_up_to_n
    virtual int units_to_decode_whole(int audio_pcm_size_words) = 0;
    int units_to_decode_half(int audio_pcm_size_words) { return units_to_decode_whole(audio_pcm_size_words) / 2; }

    // called upon first interaction with the buffer
    // has default implementation as not all formats have headers
    virtual int decode_header() = 0;

    // this return number of units actually decoded
    // "decode" means move from <this->raw_buf> to <audio_pcm_buf> (possibly doing some work)
    // audio_pcm_buf can't be in class because it changes over time (first/second half of the buffer)
    virtual int decode_up_to_n(uint32_t* audio_pcm_buf, int n) = 0;

    virtual long bit_freq() = 0;
    virtual float ms_per_unit() = 0;
    virtual int bytes_to_sec(b_type bytes) = 0;

    virtual int bitrate_in() = 0;
};
