#pragma once

#include <screen.hpp>

class ScBattery : public Screen {

    const char* get_title() override { return "Bateria"; }

    int size_x(int y) override;
    int size_y() override;

    void draw_button(int x, int y, bool selected, bool was_selected) override;

    int get_action(int x, int y) override;
    Screen* run_action(int action) override;

    void tick_sec() override;

public:
    using Screen::Screen;

    void show() override;
};
