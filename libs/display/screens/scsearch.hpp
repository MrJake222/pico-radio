#pragma once

#include <screen.hpp>
#include <cstring>

#define MAX_PROMPT_LEN    16

class ScSearch : public Screen {

    const char * get_title() override { return "Wyszukaj stację"; }

    int max_x(int y) override;
    int max_y() override;
    int default_y() override { return 2; }

    void draw_button(int x, int y, bool selected) override;

    int get_action(int x, int y) override;
    void run_action(int action) override;

    // text input field
    int pi;
    char prompt[MAX_PROMPT_LEN + 2]; // cursor + null
    void draw_prompt_field();

public:
    using Screen::Screen;
    void show() override;
};
