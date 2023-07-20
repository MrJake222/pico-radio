#include <circularbuffer.hpp>

#include <cstring>
#include <pico/platform.h>
#include <cstdio>
#include "util.hpp"

int CircularBuffer::data_left() volatile const {
    return (int) (written_bytes - read_bytes);
}

int CircularBuffer::data_left_continuous() volatile const {
    return MIN(data_left(), size - read_at);
}

int CircularBuffer::space_left() volatile const {
    return size - data_left();
}

int CircularBuffer::space_left_continuous() const volatile {
    return MIN(space_left(), size - write_at);
}

uint8_t* CircularBuffer::read_ptr() volatile const {
    return buffer + read_at;
}

uint8_t* CircularBuffer::write_ptr() volatile const {
    return buffer + write_at;
}

uint8_t* CircularBuffer::ptr_at(int o) const volatile {
    return buffer + o;
}

void CircularBuffer::read_ack(unsigned int bytes) volatile {
    read_at += (int)bytes;
    while (read_at >= size) read_at -= size; // faster than modulo (%)
    read_bytes += (b_type)bytes;

    if (read_ack_callback)
        read_ack_callback(read_ack_arg, bytes);
}

void CircularBuffer::write_ack(unsigned int bytes) volatile {
    write_at += (int)bytes;
    while (write_at >= size) write_at -= size;
    written_bytes += (b_type)bytes;

    if (write_ack_callback)
        write_ack_callback(write_ack_arg, bytes);
}

void CircularBuffer::read_reverse(unsigned int bytes) volatile {
    read_at -= (int)bytes;
    read_bytes -= (b_type) bytes;
}

bool CircularBuffer::can_wrap_buffer() volatile const {
    return data_left_continuous() <= size_hidden;
}

bool CircularBuffer::should_wrap_buffer() volatile const {
    return data_left_continuous() < data_left();
}

int CircularBuffer::wrap_buffer() volatile {
    int buf_left = data_left_continuous();

    memcpy(buffer - buf_left, read_ptr(), buf_left);
    read_at = -buf_left;

    return buf_left;
}

int CircularBuffer::try_wrap_buffer() volatile {
    if (!should_wrap_buffer())
        // wrapping the buffer won't provide more data
        return 0;

    if (!can_wrap_buffer()) {
        // can't wrap buffer, not enough space
        puts("can't wrap buffer, hidden space too small");
        return -1;
    }

    return wrap_buffer();
}

void CircularBuffer::move_to(volatile CircularBuffer &other) volatile {
    // read may be non-continuous
    // read 2 times, each time to a continuous limit
    for (int i=0; i<2 && data_left_continuous()>0; i++) {
        other.write(read_ptr(), data_left_continuous());
        read_ack(data_left_continuous()); // implicit wrap
    }
}

void CircularBuffer::write(const uint8_t* data, int data_len) volatile {
    // data may be too big to write at once (wrapping)
    // write 3 times to wrap and adjust for the write callback reversing buffer
    for (int i=0; i<3 && data_len>0; i++) {
        int write = MIN(data_len, space_left_continuous());
        memcpy(write_ptr(), data, write);
        write_ack(write); // implicit wrap

        data += write;
        data_len -= write;
    }
}

void CircularBuffer::read_arb(int o, uint8_t* data, int data_len) volatile const {
    for (int i=0; i<2 && data_len>0; i++) {
        // minimum of data length and data to end
        int read = MIN(data_len, size - o);
        memcpy(data, ptr_at(o), read);

        o += read;
        while (o >= size) o -= size;

        data += read;
        data_len -= read;
    }
}

void CircularBuffer::remove_written(int o, const int len) volatile {

    // move buffer to the left
    // copying data from +len to 0
    // across entire buffer space (to write_at pointer)

    int o_src = o + len;
    int copy_len = write_at - o_src;
    if (copy_len < 0)
        copy_len += size;

    while (copy_len--) {
        *ptr_at(o) = *ptr_at(o_src);
        if (++o == size)
            o = 0;
        if (++o_src == size)
            o_src = 0;
    }

    // adjust variables
    write_at -= len;
    if (write_at < 0)
        write_at += size;
    written_bytes -= len;
}

void CircularBuffer::set_read_ack_callback(void* arg, rw_callback_fn callback) volatile {
    read_ack_arg = arg;
    read_ack_callback = callback;
}

bool CircularBuffer::is_read_ack_callback_set() volatile {
    return read_ack_callback != nullptr;
}

void CircularBuffer::set_write_ack_callback(void* arg, rw_callback_fn callback) volatile {
    write_ack_arg = arg;
    write_ack_callback = callback;
}

bool CircularBuffer::is_write_ack_callback_set() volatile {
    return write_ack_callback != nullptr;
}

void CircularBuffer::debug_read(int bytes, int reverse) volatile {
    debug_print(buffer, read_at, bytes, reverse);
}




