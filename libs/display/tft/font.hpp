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

// can be used to skip bytes
bool is_known_unicode(const char* str);
int strlen_utf8(const char* str);

// returns data pointer of the appropriate font type (utf8, ascii)
// returns number of bytes consumed (2 byte when utf8 character detected or 1 byte) into bytes_consumed
// displays unknown characters as '?'
const uint8_t* get_font_data_ptr(const struct font* font, const char* str, int* bytes_consumed);