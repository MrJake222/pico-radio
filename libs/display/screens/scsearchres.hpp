#pragma once

#include <config.hpp>
#include <screen.hpp>

class ScSearchRes : public Screen {

    const char* get_title() override { return "Wyniki wyszukiwania"; }

    int max_x(int y) override;
    int max_y() override;
    int default_y() override { return 0; }

    void draw_button(int x, int y, bool selected) override;

    int get_action(int x, int y) override;
    Screen* run_action(int action) override;

    char prompt[MAX_PROMPT_LEN + 1]; // null

public:
    using Screen::Screen;
    void show() override;

    // called when initialized from other screen
    void begin(const char* prompt_);
};
