#pragma once

#include "decodebase.hpp"

#include <ff.h>
#include "format.hpp"

class DecodeFile : public DecodeBase {

    FRESULT fr;
    FIL fp;

    bool eof;
    void load_buffer(int bytes);

    bool data_buffer_watch() override;
    int source_size_bytes() override { return f_size(&fp); }

public:
    void begin(const char* path_, Format* format_) override;
    void end() override;

    using DecodeBase::DecodeBase;
};
