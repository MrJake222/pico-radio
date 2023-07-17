#include "font.hpp"

static int try_map_ascii(const char* str) {
    if (32 <= *str && *str <= 126) return *str - 32;
    return -1;
}

static int try_map_utf8(const char* str) {
    if ((*str == 0xc4) && (*(str+1) == 0x84)) return 0;
    if ((*str == 0xc4) && (*(str+1) == 0x86)) return 1;
    if ((*str == 0xc4) && (*(str+1) == 0x98)) return 2;
    if ((*str == 0xc5) && (*(str+1) == 0x81)) return 3;
    if ((*str == 0xc5) && (*(str+1) == 0x83)) return 4;
    if ((*str == 0xc3) && (*(str+1) == 0x93)) return 5;
    if ((*str == 0xc5) && (*(str+1) == 0x9a)) return 6;
    if ((*str == 0xc5) && (*(str+1) == 0xb9)) return 7;
    if ((*str == 0xc5) && (*(str+1) == 0xbb)) return 8;
    if ((*str == 0xc4) && (*(str+1) == 0x85)) return 9;
    if ((*str == 0xc4) && (*(str+1) == 0x87)) return 10;
    if ((*str == 0xc4) && (*(str+1) == 0x99)) return 11;
    if ((*str == 0xc5) && (*(str+1) == 0x82)) return 12;
    if ((*str == 0xc5) && (*(str+1) == 0x84)) return 13;
    if ((*str == 0xc3) && (*(str+1) == 0xb3)) return 14;
    if ((*str == 0xc5) && (*(str+1) == 0x9b)) return 15;
    if ((*str == 0xc5) && (*(str+1) == 0xba)) return 16;
    if ((*str == 0xc5) && (*(str+1) == 0xbc)) return 17;
    return -1;
}

bool is_known_unicode(const char* str) {
    return try_map_utf8(str) != -1;
}

int strlen_utf8(const char* str) {
    int displayed = 0;

    while (*str) {
        str += is_known_unicode(str) ? 2 : 1;
        displayed += 1;
    }

    return displayed;
}

const uint8_t* get_font_data_ptr(const struct font* font, const char* str, int* bytes_consumed) {
    int index = try_map_utf8(str);
    if (index != -1) {
        // utf character
        // consume 2 bytes
        *bytes_consumed = 2;
        // return utf8 data pointer
        return font->utf8_data + index * font->H * font->W;
    }

    index = try_map_ascii(str);
    if (index == -1) {
        // unknown character
        // map to ascii question mark
        const char q = '?';
        index = try_map_ascii(&q);
    }

    // ascii character (either valid character or '?')
    // consume 1 byte
    *bytes_consumed = 1;
    // return ascii data pointer
    return font->ascii_data + index * font->H * font->W;
}