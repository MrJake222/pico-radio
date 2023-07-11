#pragma once

#include <cstdint>

enum class UbuntuFontSize {
    FONT_16,
    FONT_24
};

const struct font* ubuntu_font_get_size(UbuntuFontSize size);