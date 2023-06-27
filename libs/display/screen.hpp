#pragma once

#include <st7735s.hpp>
#include <buttons.hpp>

#define COLOR_BG             0xCCCCCC
#define COLOR_BG_SEL         0x66CC66
#define COLOR_BG_DARK        0xAAAAAA
#define COLOR_BG_DARK_SEL    0x55AA55
#define COLOR_FG             0x0

class Screen {

    virtual const char* get_title() = 0;

    // ------- Button grid system -------
    // currently selected key
    int current_x;
    int current_y;
    // increment/decrement x/y
    // with wrapping and limiting
    void inx();
    void dex();
    void iny();
    void dey();

    virtual void draw_button(int x, int y, bool selected) = 0;

    // called on input (CENTER button)
    virtual Screen* run_action(int action) = 0;

protected:
    ST7735S& display;

    // grid is (0, f(y)) x (0, y)
    // can be irregular
    virtual int max_x(int y) = 0;
    virtual int max_y() = 0;
    int last_x(int y) { return max_x(y) - 1; }
    int last_y() { return max_y() - 1; }
    virtual int default_x() { return 0; }
    virtual int default_y() { return 0; }

    // passed to <run_action> but can be used by subclasses for other purposes
    virtual int get_action(int x, int y) = 0;

    // this can be overridden to provide snapping to other target
    // returns new x value
    virtual int adjust_x(int old_x, int old_y, int new_y);
    virtual void x_changed() { }

    int get_current_x() { return current_x; }
    int get_current_y() { return current_y; }

    void set_btn_bg(bool selected, bool default_dark);

public:
    Screen(ST7735S& display_)
        : display(display_)
        { }

    // called every time the screen is shown
    virtual void show();

    // called on input button pressed
    Screen* input(ButtonEnum btn);
};
