#pragma once


#include <cstdint>
#include <circularbuffer.hpp>

class Format {

    // stop playback immediately
    bool* abort_;
    // stop playback after reading whole buffer
    bool* eof_;

protected:

    bool abort() { return *abort_; }
    bool eof()   { return *eof_; }

    enum class Error {
        OK,
        FAILED,
        ABORT,
        ENDOFSTREAM
    };

    volatile CircularBuffer& raw_buf;

public:
    Format(volatile CircularBuffer& raw_buf_)
        : raw_buf(raw_buf_)
        { }

    // call every time before decoding starts
    virtual void begin(bool* abort, bool* eof) {
        abort_ = abort;
        eof_ = eof;
    }

    // tries to wrap buffer but on data underflow waits for
    // incoming data (busy waiting). Returns immediately on user abort.
    // Returns Error::OK on success Error::Error on failure, Error::ABORT on user abort, Error:EOF on eof
    Error wrap_buffer_wait_for_data();

    // returns number of units to decode to fill whole PCM buffer (size passed down)
    // (wav bytes or mp3 frames)
    // halved passed as n to decode_up_to_n
    virtual int units_to_decode_whole(int audio_pcm_size_words) = 0;
    int units_to_decode_half(int audio_pcm_size_words) { return units_to_decode_whole(audio_pcm_size_words) / 2; }

    // called upon first interaction with the buffer
    virtual int decode_header() = 0;

    // this return number of units actually decoded
    // "decode" means move from <this->raw_buf> to <audio_pcm_buf> (possibly doing some work)
    // audio_pcm_buf can't be in class because it changes over time (first/second half of the buffer)
    virtual int decode_up_to_n(uint32_t* audio_pcm_buf, int n) = 0;

    virtual long bit_freq() = 0;
    virtual float ms_per_unit() = 0;
    virtual int bytes_to_sec(b_type bytes) = 0;

    virtual int bitrate_in() = 0;

    virtual int get_meta_str(char* meta, int meta_len) = 0;
};

#define CHECK_ERROR(ferr)           \
    switch (ferr) {                 \
        case Error::OK:             \
            break;                  \
        case Error::FAILED:         \
            /* severe error */      \
            /* pass upward */       \
            return -1;              \
        case Error::ABORT:          \
        case Error::ENDOFSTREAM:    \
            /* no frames decoded */ \
            return 0;               \
    }
