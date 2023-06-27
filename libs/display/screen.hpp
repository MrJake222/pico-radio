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
    inline void limit_x();
    void iny();
    void dey();

    // grid is (0, f(y)) x (0, y)
    // can be irregular
    virtual int max_x(int y) = 0;
    virtual int max_y() = 0;
    virtual int default_x() { return 0; }
    virtual int default_y() { return 0; }

    virtual void draw_button(int x, int y, bool selected) = 0;

    virtual int get_action(int x, int y) = 0;
    virtual void run_action(int action) = 0;

protected:
    ST7735S& display;

    int get_current_x() { return current_x; }
    int get_current_y() { return current_y; }

    void set_btn_bg(bool selected, bool default_dark);

public:
    Screen(ST7735S& display_)
        : display(display_)
        { }

    // called once when the screen is created
    // virtual void begin() { }

    // called every time the screen is shown
    virtual void show();

    // called on input button pressed
    void input(ButtonEnum btn);
};
