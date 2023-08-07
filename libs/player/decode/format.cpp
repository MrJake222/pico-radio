#include "format.hpp"
#include <config.hpp>

int Format::wrap_buffer_wait_for_data() {
    // fix_data_underflow

    if (raw_buf.health() < BUF_HEALTH_UNDERFLOW) {
        // real underflow (no more data)

        // wait for data
        while (raw_buf.health() < BUF_HEALTH_MIN) {
            if (abort)
                return -2;
        }
    }

    if (raw_buf.can_wrap_buffer()) {
        int ret = raw_buf.try_wrap_buffer();
        if (ret < 0)
            return -1;
    }

    return 0;
}