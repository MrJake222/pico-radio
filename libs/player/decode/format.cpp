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

        // wait for data (TODO why do it twice? below also waits)
        while (raw_buf.health() < BUF_HEALTH_MIN) {
            if (abort())
                return Error::ABORT;
        }

        r = raw_buf.wait_for_health(BUF_HEALTH_MIN, *abort_);
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
