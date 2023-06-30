#include "circularbuffertools.hpp"

#include <cstdio>

int cbt_end_of_line(volatile CircularBuffer& buf) {

    // if the buffer can be wrapped, then wrap it
    // tries to return the line as a continuous memory block
    if (buf.can_wrap_buffer())
        buf.wrap_buffer();

    int consumed = 0;

    while (true) {
        if (consumed >= buf.data_left_continuous()) {
            // end of buffer occurred before end-of-file found
            if (buf.data_left() > buf.data_left_continuous()) {
                // after-wrap data available
                // TODO line is not continuous
                puts("non-continuous line might be available");
            }

            return -1;
        }

        if (*buf.read_ptr_of(consumed) == '\n') {
            // found end-of-line

            if (consumed>0 && *buf.read_ptr_of(consumed - 1) == '\r') {
                return consumed - 1;
            }

            return consumed;
        }

        consumed++;
    }
}
