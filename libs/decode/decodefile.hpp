#pragma once

#include <decodebase.hpp>

#include <ff.h>
#include <format.hpp>

class DecodeFile : public DecodeBase {

    FRESULT fr;
    FIL fp;

    bool eof;
    void load_buffer(int bytes);

    int source_size_bytes() override { return f_size(&fp); }

public:
    void begin(const char* path_, Format* format_) override;
    void end() override;

    // Do not call directly
    // used by callbacks

    // direct callback (on core1)
    void raw_buf_read_cb(unsigned int bytes) override;
    // callback from fifo data (on core0)
    void load_buffer_or_eof();

    using DecodeBase::DecodeBase;
};
