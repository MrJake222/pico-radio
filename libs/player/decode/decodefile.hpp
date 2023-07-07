#pragma once

#include <decodebase.hpp>

#include <ff.h>
#include <format.hpp>

class DecodeFile : public DecodeBase {

    // maximum buffer health when the load will take place
    static const int load_max_health = 80;

    FRESULT fr;
    FIL fp;
    bool file_open;

    // returns number of bytes loaded or -1 on error
    int load_buffer(int bytes);

    bool eof;
    int check_buffer();

    int source_size_bytes() override { return f_size(&fp); }

public:
    void begin(const char* path_, Format* format_) override;
    int play() override;
    void end() override;

    // Do not call directly
    // used by callbacks

    // callback from fifo data (on core0)
    void ack_bytes(uint16_t bytes) override;

    using DecodeBase::DecodeBase;
};
