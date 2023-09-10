#pragma once

#include <st7735s.hpp>
#include <buttons.hpp>
#include <textscrolled.hpp>

#include <FreeRTOS.h>
#include <semphr.h>

#define COLOR_BG             0xCCCCCC
#define COLOR_BG_SEL         0x66CC66
#define COLOR_BG_DARK        0xa1af9f
#define COLOR_BG_DARK_SEL    0x55AA55

#define COLOR_BG_GOOD        0x7ACC7A
#define COLOR_BG_WARN        0xCCCC7A
#define COLOR_BG_ERR         0xCC7A7A

#define COLOR_ACC1           0x0077AC
#define COLOR_ACC2           0x803E33
#define COLOR_ACC3           0xD29ED2
#define COLOR_FG             0x0
#define COLOR_FG_GOOD        0x1F641F
#define COLOR_FG_WARN        0x64641F
#define COLOR_FG_ERR         0x641F1F

class Screen {

    virtual const char* get_title() = 0;

    // called on input (CENTER button)
    virtual Screen* run_action(int action) = 0;

    // Scrollable texts
    // some texts need to be scrolled across the screen
    // need to support multiple text scrolled
    TextScrolled texts[LCD_SCROLLED_TEXTS_MAX];
    // first free index in <texts> array
    int texts_index;

    // see screenmng.cpp for details
    // is private because it's not recursive
    SemaphoreHandle_t& mutex_ticker;

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
    virtual void inx();
    virtual void dex();
    virtual void iny();
    virtual void dey();

    // overlay displayed
    // this can (and should) be used in <show>
    // to disable some actions (for ex. player playback)
    bool is_overlay_displayed;

    // passed to <run_action> but can be used by subclasses for other purposes
    virtual int get_action(int x, int y) = 0;

    virtual void button_pre_selection_change() { }
    virtual void draw_button(int x, int y, bool selected) = 0;
    void draw_buttons();

    void show_overlay(int bg, const char* msg);

    // this can be overridden to provide snapping to other target
    // returns new x value
    virtual int adjust_x(int old_x, int old_y, int new_y);
    virtual void x_changed() { }

    static int get_btn_bg(bool selected, bool dark);

    void add_normal_text(int text_x, int text_y, const char *str, const struct font* font, int bg, int fg, int max_width);
    // adds left justified text, takes <text_x_r> (right corner), calculates <str> length
    void add_normal_text_ljust(int text_x_r, int text_y, const char *str, const struct font* font, int bg, int fg);
    int add_scrolled_text(int text_x, int text_y, const char *str, const struct font* font, int bg, int fg, int max_width);
    void update_scrolled_text(int idx, const char *str);
    void reset_scrolled_texts();
    // this adds scrolling text if the text won't fit in max_width
    // can attach additional check (scrolling will only occur when allow_scroll is true)
    // returns index of scrolled text or -1 if normal text
    int add_scrolled_text_or_normal(int text_x, int text_y, const char *str, const struct font* font, int bg, int fg, int max_width, bool allow_scroll=true);

    void draw_progress_bar(int x, int y, int percent, int bg, int fg);

    // slow ticks (1 s)
    bool tick_sec_enabled; // protects from writing over the error screen
    int tick_sec_counter;
    void tick_sec_enable();
    void tick_sec_disable();
    virtual void tick_sec();

public:
    Screen(ST7735S& display_, SemaphoreHandle_t& mutex_ticker_)
        : display(display_)
        , mutex_ticker(mutex_ticker_)
        , texts{display_, display_, display_, display_}
        { }

    // called when the screen is entered the first time (from previous screen usually)
    // don't call on "back" button enter
    virtual void begin();

    // called every time the screen is shown (before enabling ticking)
    // called by screen manager
    virtual void show();

    // called every time the screen is hidden (before disabling ticking)
    // called by screen manager
    virtual void hide();

    inline void show_error(const char* msg) { show_overlay(COLOR_BG_ERR, msg); }
    inline void show_warn(const char* msg)  { show_overlay(COLOR_BG_WARN, msg); }

    // should be called periodically to update screen content
    // default implementation updates scrollable texts
    // called each LCD_TICK_INTERVAL_MS ms
    void tick();

    // called on input button pressed
    Screen* input(ButtonEnum btn);
};
