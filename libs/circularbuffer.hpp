#pragma once

#include <cstdint>

typedef unsigned long long b_type;
using rw_callback_fn = void(*)(void*, unsigned int);

class CircularBuffer {

    uint8_t* const buffer_hidden;
    uint8_t* const buffer;

    // can't do read pos + avail
    // available data would be modified from 2 cores
    // read/write keep that separate

    // where to read/write data from/to
    // used to calculate effective addresses (must be signed for wrapping)
    int read_at;
    int write_at;

    // how many bytes was read from/written to this buffer
    // used to calculate bytes currently in the buffer
    // solves issues with undefined behavior with read_at == write_at
    b_type read_bytes;
    b_type written_bytes;

    // callbacks
    void* read_ack_arg;
    volatile rw_callback_fn read_ack_callback;
    void* write_ack_arg;
    volatile rw_callback_fn write_ack_callback;

    // wraps buffer unconditionally
    void wrap_buffer() volatile;

public:
    const int size_hidden;
    const int size;

    /**
     *
     * @param size_ size of the visible buffer
     * @param size_hidden_ size of the area before visible buffer (for wrapping)
     * @param buf_ static buffer at least (size + size_hidden) bytes
     */
    CircularBuffer(int size_, int size_hidden_, uint8_t* buf_)
        : size(size_)
        , size_hidden(size_hidden_)
        , buffer_hidden(buf_)
        , buffer(buffer_hidden + size_hidden_)
        {
            reset_with_cb();
        }

    void reset_only_data() volatile {
        read_at = 0;
        write_at = 0;
        read_bytes = 0;
        written_bytes = 0;
    }

    void reset_with_cb() volatile {
        reset_only_data();
        read_ack_callback = nullptr;
        write_ack_callback = nullptr;
    }

    int get_read_offset()  volatile const { return read_at; }
    int get_write_offset() volatile const { return write_at; }

    int data_left()                volatile const;
    int data_left_continuous()     volatile const;
    int space_left()               volatile const;
    int space_left_continuous()    volatile const;

    int health() volatile const { return data_left() * 100 / size; }


    uint8_t* read_ptr()  volatile const;
    uint8_t* write_ptr() volatile const;

    uint8_t* ptr_at(int o)  volatile const;

    void read_ack(unsigned int bytes)  volatile;
    void write_ack(unsigned int bytes) volatile;

    b_type read_bytes_total()    volatile const { return read_bytes; }
    b_type written_bytes_total() volatile const { return written_bytes; }

    void read_reverse(unsigned int bytes) volatile;


    // wrapping
    // 'can' returns whether remaining data will fit into
    // the hidden section of the buffer
    bool can_wrap_buffer()     volatile const;

    // 'should' returns whether the overall available data
    // is bigger than continuous data
    bool should_wrap_buffer()  volatile const;

    // wraps buffer, but only after both checks pass (can & should)
    // if <should> is false, returns -2 (no data to copy)
    // if <can> is false, returns -1 (error, data won't fit, enlarge "hidden" section)
    // on success returns 0
    int try_wrap_buffer()         volatile;


    // helper functions
    // move all data from here to other buffer
    void move_to(volatile CircularBuffer& other) volatile;

    // write at write_ptr and ack
    void write(const uint8_t* data, int data_len) volatile;

    // read at arbitrary offset, used for example to extract metadata
    // no ack of any kind, unsafe, doesn't check data availability
    void read_arb(int o, uint8_t* data, int data_len) volatile const;

    // cut out bytes at arbitrary offset (close behind the write_ptr)
    // moves remaining bytes in <o, write_ptr> to the left
    // modifies write_ptr and written_bytes
    void remove_written(int o, int len) volatile;


    // callbacks
    // register a callback when data was just consumed
    // (more free space available)
    // only one callback supported
    void set_read_ack_callback(void* arg, rw_callback_fn callback) volatile;
    bool is_read_ack_callback_set() volatile;

    // register a callback when data was just written
    // (more data available)
    void set_write_ack_callback(void* arg, rw_callback_fn callback) volatile;
    bool is_write_ack_callback_set() volatile;


    // print read debug info
    void debug_read(int bytes, int reverse) volatile;
};
