#pragma once

#include <cstdint>

struct icon {
    uint8_t w;
    uint8_t h;
    const uint8_t* data;
};

extern const struct icon icon_backspace;