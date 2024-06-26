#include "format.hpp"
#include <config.hpp>
#include <cstdio>

Format::Error Format::wrap_buffer_wait_for_data() {
    // fixes data underflow
    // called only on data underflow (no calling on every frame decoding)
    // printf("wrap health %2d eof %d\n", raw_buf.health(), eof());
    int r;

    if (raw_buf.health() < BUF_HEALTH_UNDERFLOW) {
        // real underflow (no more data in the buffer)

        if (eof()) {
            // no more data in the stream
            return Error::ENDOFSTREAM;
        }

        r = raw_buf.wait_for_health_2p(BUF_HEALTH_MIN, *abort_, *eof_);
        if (r < 0)
            return Error::ABORT;
    }

    if (raw_buf.can_wrap_buffer()) {
        int ret = raw_buf.try_wrap_buffer();
        if (ret < 0)
            return Error::FAILED;
    }

    return Error::OK;
}

void Format::mono_to_stereo(uint32_t* buf, int buf_size_words) {
    // mono audio
    // data is LLL... should be LR LR LR...
    // need to explode one channel into two

    // Indexing with 32bit elements we get 2-channel view of the buffer (as hardware supports it).
    // Indexing with 16bit elements we get mono samples (as software outputted it).

    // Going in reverse not to overwrite anything
    for (int i=buf_size_words-1; i>=0; i--) {
        uint32_t s = ((uint16_t*) buf)[i];

        buf[i]  = s; // left channel
        // replace missing channel with same data
        buf[i] |= s << 16; // right channel
    }
}