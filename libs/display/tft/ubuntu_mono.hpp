#pragma once

#include <cstdint>

#define FONT_W  8
#define FONT_H 16

// data is left-to-right, top-to-bottom encoded
// each 8-bit value is a line of 8 pixels
// 16x8bit make a character

extern const uint8_t ascii_data[];

int try_map_ascii(const char* str);


extern const uint8_t utf8_data[];

// returns -1 if failed
// try first
int try_map_utf8(const char* str);

const uint8_t* get_font_data_ptr(const uint8_t data[], int index);