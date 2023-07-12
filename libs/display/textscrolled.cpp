#include "textscrolled.hpp"

#include <config.hpp>
#include <cstring>

void TextScrolled::begin(int text_x_, int text_y_, const char* str_, const struct font* font_, int bg_, int fg_, int max_width_) {
    text_x = text_x_;
    text_y = text_y_;
    max_width = max_width_;

    strncpy(str, str_, LCD_SCROLLED_TEXTS_LEN_MAX);
    font = font_;

    bg = bg_;
    fg = fg_;

    offset_x = 0;
    offset_second = (strlen_utf8(str) + 4) * font->W;
}

void TextScrolled::update(int time_passed_ms) {
    const float dt = (float)time_passed_ms / 1000.0f; // [s]
    const float v = 20.0f; // [px/s]
    offset_x += dt * v;
}

void TextScrolled::draw() {
    display.write_text(text_x - (int)offset_x, text_y,
                       str, font, bg, fg,
                       text_x, text_x + max_width);

    display.write_text(text_x - (int)offset_x + offset_second, text_y,
                       str, font, bg, fg,
                       text_x, text_x + max_width);

    if ((int)offset_x == offset_second) {
        offset_x = 0;
    }
}