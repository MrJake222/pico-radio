#include "circularbuffer.hpp"

#include <cstring>
#include <pico/platform.h>

long CircularBuffer::data_left() volatile const {
    return write_at < read_at
           ? ((size - read_at) + write_at)  // from reading position to end + wrapped
           : (write_at - read_at);          // from reading to next write pos
}

long CircularBuffer::data_left_continuous() volatile const {
    return MIN(data_left(), size - read_at);
}

long CircularBuffer::space_left() volatile const {
    return size - data_left();
}

long CircularBuffer::space_left_continuous() const volatile {
    return MIN(space_left(), size - write_at);
}

uint8_t* CircularBuffer::read_ptr() volatile const {
    return buffer + read_at;
}

uint8_t* CircularBuffer::write_ptr() volatile const {
    return buffer + write_at;
}

void CircularBuffer::read_ack(unsigned int bytes) volatile {
    read_at += (long)bytes;
    read_at %= size;

    if (read_ack_callback)
        read_ack_callback(read_ack_arg, bytes);
}

void CircularBuffer::write_ack(unsigned int bytes) volatile {
    write_at += (long)bytes;
    write_at %= size;
}

bool CircularBuffer::can_wrap_buffer() volatile const {
    return data_left_continuous() <= size_hidden;
}

void CircularBuffer::wrap_buffer() volatile {
    long buf_left = data_left_continuous();

    memcpy(buffer - buf_left, read_ptr(), buf_left);
    read_at = -buf_left;
}

void CircularBuffer::set_read_ptr_end(unsigned int from_end) volatile {
    read_at = write_at - (long)from_end;
}

void CircularBuffer::move_to(volatile CircularBuffer &other) volatile {
    // read may be non-continuous
    // read 2 times, each time to a continuous limit
    for (int i=0; i<2; i++) {
        other.write(read_ptr(), data_left_continuous());
        read_ack(data_left_continuous());
    }
}

void CircularBuffer::write(const uint8_t* data, long data_len) volatile {
    // data may be too big to write at once (wrapping)
    // write 2 times to wrap
    for (int i=0; i<2; i++) {
        long write = MIN(data_len, space_left_continuous());
        memcpy(write_ptr(), data, write);
        write_ack(write);

        data += write;
        data_len -= write;
    }
}

void CircularBuffer::set_read_ack_callback(void* arg, CircularBuffer::read_callback_fn callback) volatile {
    read_ack_arg = arg;
    read_ack_callback = callback;
}





