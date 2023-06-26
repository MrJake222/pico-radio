#pragma once

#include <st7735s.hpp>
#include <buttons.hpp>

#define COLOR_BG        0xCCCCCC
#define COLOR_BG_DARK   0xAAAAAA
#define COLOR_BG_SEL    0x55AA55
#define COLOR_FG        0x0

class Screen {

protected:
    ST7735S& display;

    virtual const char* get_title() = 0;

    void set_btn_bg(bool selected);

public:
    Screen(ST7735S& display_)
        : display(display_)
        { }

    // called once when the screen is created
    virtual void begin() { }

    // called every time the screen is shown
    virtual void show();

    virtual void input(ButtonEnum btn) = 0;
};
