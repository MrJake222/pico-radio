#include "textscrolled.hpp"

#include <config.hpp>
#include <cstring>

void TextScrolled::set_str(const char* str_) {
    if (strncmp(str, str_, LCD_SCROLLED_TEXTS_LEN_MAX) == 0)
        // same text
        return;

    strncpy(str, str_, LCD_SCROLLED_TEXTS_LEN_MAX);
    text_width = font->W * strlen_utf8(str);
    text_gap   = font->W * 4;

    offset_x = 0;

    display.fill_rect(text_x, text_y, max_width, font->H, bg);
}

void TextScrolled::begin(int text_x_, int text_y_, const char* str_, const struct font* font_, int bg_, int fg_, int max_width_) {
    text_x = text_x_;
    text_y = text_y_;
    max_width = max_width_;
    max_x = text_x + max_width;

    font = font_;

    bg = bg_;
    fg = fg_;

    set_str(str_);
    offset_x = 0; // force scroll reset (needed here because of reusing same instances of the class)
}

void TextScrolled::update(int time_passed_ms) {
    const float dt = (float)time_passed_ms / 1000.0f; // [s]
    const float v = 20.0f; // [px/s]
    offset_x += dt * v;
}

void TextScrolled::draw() {
    const int start = text_x - (int)offset_x;
    const int step  = text_width + text_gap;

    for (int x=start; x<max_x; x+=step) {
        display.write_text(x, text_y,
                           str, font, bg, fg,
                           text_x, max_x);
    }

    if ((int)offset_x >= text_width) {
        offset_x = (float) -text_gap;
    }
}