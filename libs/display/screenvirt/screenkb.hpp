#pragma once

#include <config.hpp>
#include <screen.hpp>
#include <cstring>

class ScreenKb : public Screen {

    // navigation ux
    int8_t prev_x_first_to_kb;
    // * to first -- doesn't matter (only one icon)
    int8_t prev_x_first_to_last;
    int8_t prev_x_last_to_kb;
    int8_t prev_x_kb_to_last;
    void prev_x_clear();

    // 2nd row, letter Q
    int default_y() override { return 2; }

    int size_x(int y) override;
    int size_y() override;
    int adjust_x(int old_x, int old_y, int new_y) override;
    void x_changed() override;

    void draw_button(int x, int y, bool selected) override;

    int get_action(int x, int y) override;
    Screen* run_action(int action) override;

    // text input field
    size_t pi;
    char prompt[MAX_PROMPT_LEN + 2]; // cursor + null
    void draw_prompt_field();

public:
    using Screen::Screen;

    void begin() override;
    void show() override;
};
