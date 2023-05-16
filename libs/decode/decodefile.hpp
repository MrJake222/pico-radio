#pragma once

#include "decodebase.hpp"

#include "ff.h"
#include "format.hpp"

class DecodeFile : public DecodeBase {

    FRESULT fr;
    FIL fp;

    bool eof;
    void load_buffer(int bytes);

    bool data_buffer_watch() override;

    void dma_feed_done(int decoded, int took_us, DMAChannel channel) override;

public:
    void begin() override;
    void end() override;

    using DecodeBase::DecodeBase;
};
