#pragma once

#include <config.hpp>
#include <screen.hpp>

class ScSearchRes : public Screen {

    const char* get_title() override { return "Wyniki wyszukiwania"; }

    int size_x(int y) override;
    int size_y() override;

    void iny() override;
    void dey() override;

    void draw_button(int x, int y, bool selected) override;
    void draw_top_buttons();
    void draw_bottom_buttons();
    void draw_scroll_bar();

    int get_action(int x, int y) override;
    Screen* run_action(int action) override;

    // first <show> after <begin>
    bool first;

    // this variable holds index of the first
    // currently displayed station
    int base_y;
    int max_base_y();
    int kb_buttons();

    // gated count, only updated after all stations are loaded
    int station_count;

    const char* prompt;
    char subtitle[10 + MAX_PROMPT_LEN + 1]; // null

public:
    using Screen::Screen;
    void begin(const char* prompt_);
    void show() override;



    friend void all_loaded_cb(void* arg);
};
