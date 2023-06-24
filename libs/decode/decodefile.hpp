#pragma once

#include <decodebase.hpp>

#include <ff.h>
#include <format.hpp>

class DecodeFile : public DecodeBase {

    FRESULT fr;
    FIL fp;

    bool eof;
    void load_buffer(int bytes);
    void check_buffer();

    int source_size_bytes() override { return f_size(&fp); }

public:
    void begin(const char* path_, Format* format_) override;
    void end() override;

    // Do not call directly
    // used by callbacks

    // callback from fifo data (on core0)
    void raw_buf_read_msg(unsigned int bytes) override;

    using DecodeBase::DecodeBase;
};
