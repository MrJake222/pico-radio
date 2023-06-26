#pragma once

#include <screen.hpp>
#include <cstring>

#define MAX_PROMPT_LEN    16

class ScSearch : public Screen {

    const char * get_title() override { return "Wyszukaj stację"; }

    void draw_backspace();
    void draw_space();
    // used to draw individual buttons
    void draw_kb_btn(int x, int y, bool selected);
    // used to draw whole keyboard (x/y are col/row coordinates)
    void draw_kb_row(int y);

    // currently selected key
    int current_x = 0;
    int current_y = 0;
    // increment/decrement x/y
    void inx();
    void dex();
    inline void limit_x();
    void iny();
    void dey();

    // text input field
    int pi;
    char prompt[MAX_PROMPT_LEN + 2]; // cursor + null
    void draw_prompt_field();

public:
    using Screen::Screen;
    void show() override;
    void input(ButtonEnum btn) override;
};
