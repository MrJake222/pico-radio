#pragma once

#include <decodebase.hpp>

#include <ff.h>
#include <format.hpp>

class DecodeFile : public DecodeBase {
    FRESULT fr;
    FIL fp;
    bool file_open;

    // returns number of bytes loaded or -1 on error
    int load_buffer(int bytes);

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

    // TODO implement ID3
    int get_meta_str(char *meta, int meta_len) override { return -1; }
};
