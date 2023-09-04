#pragma once

#include <cstdint>

// data is left-to-right, top-to-bottom encoded
// each 8-bit value is an opacity value (255=black, 0=white)

struct font {
    const int W;
    const int H;
    const uint8_t* ascii_data;
    const uint8_t* utf8_data;
};

// interpret 1x UTF-8 character and return its length (in bytes)
int get_char_width(const char* str);

// interpret whole string (up to \0) and return its length in displayed characters
int strlen_utf8(const char* str);

// returns data pointer of the appropriate font type (utf8, ascii)
// displays unknown characters as '?'
const uint8_t* get_font_data_ptr(const struct font* font, const char* str);