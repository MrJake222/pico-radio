#pragma once

#include <cstdint>

class CircularBuffer {

    uint8_t* const buffer_hidden;
    uint8_t* const buffer;

    // can't do read pos + avail
    // available data would be modified from 2 cores
    // read/write keep that separate

    // where to read/write data from/to
    long read_at;
    long write_at;

    // how many bytes was read from/written to this buffer
    long read_bytes;
    long written_bytes;

    using rw_callback_fn = void(*)(void*, unsigned int);
    void* read_ack_arg;
    volatile rw_callback_fn read_ack_callback;
    void* write_ack_arg;
    volatile rw_callback_fn write_ack_callback;

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

    long get_read_offset()  volatile const { return read_at; }
    long get_write_offset() volatile const { return write_at; }

    long data_left()                volatile const;
    long data_left_continuous()     volatile const;
    long space_left()               volatile const;
    long space_left_continuous()    volatile const;

    int health() volatile const { return data_left() * 100 / size; }


    uint8_t* read_ptr()  volatile const; // TODO make this return const, requires helixmp3 adjustments
    uint8_t* write_ptr() volatile const;

    uint8_t* read_ptr_of(int of)  volatile const; // TODO make this return const, requires helixmp3 adjustments
    uint8_t* write_ptr_of(int of) volatile const;

    void read_ack(unsigned int bytes)  volatile;
    void write_ack(unsigned int bytes) volatile;

    long read_bytes_total()    volatile;
    long written_bytes_total() volatile;

    void read_reverse(unsigned int bytes) volatile { read_at -= (long)bytes; }


    // wrapping
    // 'can' returns whether remaining data will fit into
    // the hidden section of the buffer
    bool can_wrap_buffer()     volatile const;

    // 'should' returns whether the read buffer is near
    // the end of the buffer
    bool should_wrap_buffer()  volatile const;

    void wrap_buffer()         volatile;


    // set read pointer almost at the end of continuous buffer chunk.
    // This may involve write pointer (marks the end of available data)
    void set_read_ptr_end(unsigned int from_end) volatile;


    // helper functions
    // move all data from here to other buffer
    void move_to(volatile CircularBuffer& other) volatile;

    // write at write_ptr and ack
    void write(const uint8_t* data, long data_len) volatile;


    // register a callback when data was just consumed
    // (more free space available)
    // only one callback supported
    void set_read_ack_callback(void* arg, rw_callback_fn callback) volatile;

    // register a callback when data was just written
    // (more data available)
    void set_write_ack_callback(void* arg, rw_callback_fn callback) volatile;


    // print read debug info
    void debug_read(int bytes, int prepend) volatile;
};
