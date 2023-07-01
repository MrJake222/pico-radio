#pragma once

#include <decodebase.hpp>

#include <ff.h>
#include <format.hpp>

class DecodeFile : public DecodeBase {

    FRESULT fr;
    FIL fp;

    bool eof;
    int load_buffer(int bytes);
    int check_buffer();

    int source_size_bytes() override { return f_size(&fp); }

public:
    void begin(const char* path_, Format* format_) override;
    int play_() override;
    int stop() override;

    // Do not call directly
    // used by callbacks

    // callback from fifo data (on core0)
    void raw_buf_just_read(unsigned int bytes) override;

    using DecodeBase::DecodeBase;
};
