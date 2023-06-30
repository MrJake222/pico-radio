#pragma once

#include <st7735s.hpp>
#include <buttons.hpp>

#define COLOR_BG             0xCCCCCC
#define COLOR_BG_SEL         0x66CC66
#define COLOR_BG_DARK        0xa1af9f
#define COLOR_BG_DARK_SEL    0x55AA55
#define COLOR_BG_DARK_ACC1   0x0077AC
#define COLOR_BG_DARK_ACC2   0x803E33
#define COLOR_FG             0x0

class Screen {

    virtual const char* get_title() = 0;

    // called on input (CENTER button)
    virtual Screen* run_action(int action) = 0;

protected:
    ST7735S& display;

    // grid is (0, f(y)) x (0, y)
    // can be irregular
    virtual int size_x(int y) = 0;
    virtual int size_y() = 0;
    int last_x(int y) { return size_x(y) - 1; }
    int last_y() { return size_y() - 1; }
    virtual int default_x() { return 0; }
    virtual int default_y() { return 0; }

    // currently selected key
    int current_x;
    int current_y;
    // increment/decrement x/y
    // with wrapping and limiting
    void inx();
    void dex();
    virtual void iny();
    virtual void dey();

    // passed to <run_action> but can be used by subclasses for other purposes
    virtual int get_action(int x, int y) = 0;

    virtual void draw_button(int x, int y, bool selected) = 0;
    void draw_buttons();

    // this can be overridden to provide snapping to other target
    // returns new x value
    virtual int adjust_x(int old_x, int old_y, int new_y);
    virtual void x_changed() { }

    void set_btn_bg(bool selected, bool default_dark);

public:
    Screen(ST7735S& display_)
        : display(display_)
        { }

    // called when the screen is entered the first time
    // don't call on "back" button enter
    virtual void begin();

    // called every time the screen is shown
    // replaces standard <start> method
    virtual void show();

    // called on input button pressed
    Screen* input(ButtonEnum btn);
};
