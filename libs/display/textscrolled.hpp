#pragma once

#include <font.hpp>
#include <st7735s.hpp>

class TextScrolled {

    ST7735S& display;

    int text_x; // also min_x
    int text_y;
    int max_width;
    int max_x;

    char str[LCD_SCROLLED_TEXTS_LEN_MAX + 1];
    const struct font* font;
    int text_width; // text width         [px]
    int text_gap;   // gap between texts  [px]

    int bg;
    int fg;

    float offset_x;

public:
    TextScrolled(ST7735S& display_)
        : display(display_)
        { }

    // set text to be displayed
    void set_str(const char* str_);

    // set parameters
    void begin(int text_x_, int text_y_, const char* str_, const struct font* font_, int bg_, int fg_, int max_width_);

    // update the text position
    // parameter is ms since last update
    void update(int time_passed_ms);

    // draw the updated text
    void draw();
};
