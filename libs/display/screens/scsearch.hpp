#pragma once

#include <config.hpp>
#include <screen.hpp>
#include <cstring>

class ScSearch : public Screen {

    const char* get_title() override { return "Wyszukaj stację"; }

    // navigation ux
    int8_t prev_x_first_to_kb;
    // * to first -- doesn't matter (only one icon)
    int8_t prev_x_first_to_last;
    int8_t prev_x_last_to_kb;
    int8_t prev_x_kb_to_last;
    void prev_x_clear();

    int max_x(int y) override;
    int max_y() override;
    int default_y() override { return 2; }
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
    void show() override;

    void begin(const char* prompt_);
};
