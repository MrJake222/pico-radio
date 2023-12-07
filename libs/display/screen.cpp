#include "screen.hpp"
#include "icons.hpp"
#include "analog.hpp"
#include <sd.hpp>
#include <wificonnect.hpp>

#include <ubuntu_mono.hpp>
#include <cstdio>

void Screen::inx() {
    current_x++;
    current_x %= size_x(current_y);
    x_changed();
}

void Screen::dex() {
    current_x--;
    if (current_x < 0)
        current_x = size_x(current_y) - 1;
    x_changed();
}

void Screen::iny() {
    int old_y = current_y;
    current_y++;
    current_y %= size_y();
    current_x = adjust_x(current_x, old_y, current_y);
}

void Screen::dey() {
    int old_y = current_y;
    current_y--;
    if (current_y < 0)
        current_y = size_y() - 1;
    current_x = adjust_x(current_x, old_y, current_y);
}

int Screen::adjust_x(int old_x, int old_y, int new_y) {
    return MIN(current_x, size_x(current_y) - 1);
}

Screen* Screen::input(ButtonEnum btn) {
    if (is_overlay_displayed) {
        show(); // redraw normal screen
        is_overlay_displayed = false;
        return nullptr;
    }

    if (btn == CENTER) {
        return run_action(get_action(current_x, current_y));
    }

    const int old_x = current_x;
    const int old_y = current_y;

    switch (btn) {
        case UP:
            dey();
            break;

        case DOWN:
            iny();
            break;

        case LEFT:
            dex();
            break;

        case RIGHT:
            inx();
            break;
    }

    if (current_x != old_x || current_y != old_y) {
        // selection changed
        // draw_button should handle resetting its own scrolled texts
        // (probably on selected==false and was_selected==true)

        draw_button(old_x, old_y, false, true);
        draw_button(current_x, current_y, true, false);
    }

    return nullptr;
}

int Screen::get_btn_bg(bool selected, bool dark) {
    if (dark)
        return selected ? COLOR_BG_DARK_SEL : COLOR_BG_DARK;
    else
        return selected ? COLOR_BG_SEL : COLOR_BG;
}

void Screen::draw_buttons() {
    for (int y=0; y<size_y(); y++) {
        for (int x=0; x<size_x(y); x++) {
            bool s = (y == current_y && x == current_x);
            draw_button(x, y, s, false);
        }
    }
}

void Screen::add_normal_text(int text_x, int text_y, const char* str, const struct font* font, int bg, int fg,
                             int max_width) {
    display.write_text(text_x, text_y, str, font, bg, fg,
                       text_x, text_x + max_width);
}

void Screen::add_normal_text_rjust(int text_x_r, int text_y, const char* str, const struct font* font, int bg, int fg) {

    const int text_x = text_x_r - strlen_utf8(str) * font->W;

    display.write_text(text_x, text_y, str, font, bg, fg,
                       text_x, text_x + display.W);
}

int Screen::add_scrolled_text(int text_x, int text_y, const char* str, const struct font* font, int bg, int fg, int max_width) {
    assert(texts_index < LCD_SCROLLED_TEXTS_MAX);

    xSemaphoreTake(mutex_ticker, portMAX_DELAY);
    texts[texts_index].begin(text_x, text_y, str, font, bg, fg, max_width);
    texts[texts_index].draw();
    xSemaphoreGive(mutex_ticker);

    return texts_index++;
}

void Screen::update_scrolled_text(int idx, const char *str) {
    if (idx < texts_index) {
        xSemaphoreTake(mutex_ticker, portMAX_DELAY);
        texts[idx].set_str(str);
        xSemaphoreGive(mutex_ticker);
    }
}

void Screen::reset_scrolled_texts() {
    xSemaphoreTake(mutex_ticker, portMAX_DELAY);
    texts_index = 0;
    xSemaphoreGive(mutex_ticker);
}

void Screen::tick_sec_enable() {
    xSemaphoreTake(mutex_ticker, portMAX_DELAY);
    tick_sec_counter = 0;
    tick_sec_enabled = true;
    xSemaphoreGive(mutex_ticker);
}

void Screen::tick_sec_disable() {
    xSemaphoreTake(mutex_ticker, portMAX_DELAY);
    tick_sec_enabled = false;
    xSemaphoreGive(mutex_ticker);
}

void Screen::tick_sec(int sec) {
    // update top-of-the-screen status icons

    // important for drawing title
    icon_x = 149;
    const int y = 2;

    // battery
    const int p = analog::battery_percentage();
    if (p >= 60)
        display.draw_icon(icon_x, y, &icon_battery_100, COLOR_BG, COLOR_FG_GOOD);
    else if (p >= 20)
        display.draw_icon(icon_x, y, &icon_battery_50, COLOR_BG, COLOR_FG_WARN);
    else
        display.draw_icon(icon_x, y, &icon_battery_0, COLOR_BG, COLOR_FG_ERR);

    // sd card
    icon_x -= 10; // width=8 + margin=2
    display.draw_icon(icon_x, y,
                      sd::is_card_mounted() ? &icon_sd : &icon_sd_disabled,
                      COLOR_BG, COLOR_FG);

    // wifi
    icon_x -= 13; // width=11 + margin=2
    display.draw_icon(icon_x, y,
                      wifi::is_in_progress() ? icon_wifi[sec % 4] // animate
                      : wifi::is_connected_ip() ? wifi::quality_to_icon(wifi::connected_quality()) // current signal
                      : &icon_wifi_3_x, // disabled
                      COLOR_BG, COLOR_FG);
}

void Screen::tick() {
    xSemaphoreTake(mutex_ticker, portMAX_DELAY);

    for (int i=0; i<texts_index; i++) {
        texts[i].update(LCD_TICK_INTERVAL_MS);
        texts[i].draw();
    }

    if (tick_sec_enabled) {
        tick_sec_counter++;
        if (tick_sec_counter == 1000 / LCD_TICK_INTERVAL_MS) {
            tick_sec_counter = 0;
            tick_sec_call();
        }
    }

    xSemaphoreGive(mutex_ticker);
}

int Screen::add_scrolled_text_or_normal(int text_x, int text_y, const char* str, const struct font* font,
                                         int bg, int fg, int max_width, bool allow_scroll) {

    if (strlen_utf8(str) * font->W > max_width && allow_scroll) {
        // display can't fit name -> do scrolling
        return add_scrolled_text(text_x, text_y, str, font,
                          bg, fg, max_width);
    }

    else {
        // can fit -> do normal text
        add_normal_text(text_x, text_y, str, font,
                        bg, fg, max_width);

        return -1;
    }
}

void Screen::draw_progress_bar(int x, int y, int w, int percent, int bg, int fg, bool draw_text) {

    // not to overflow the bar
    percent = MIN(percent, 100);
    const int filled_bar_width = percent * w / 100;

    // no to overflow the display
    percent = MIN(percent, 99);

    // filled bar
    display.fill_rect(x, y, filled_bar_width, 4, fg);

    // empty bar
    display.fill_rect(x + filled_bar_width, y, w - filled_bar_width, 4, bg);

    if (draw_text) {
        // percent
        char buf[5];
        sprintf(buf, "%02d%%", percent);
        add_normal_text(
                x + w + 4, y - 5, buf,
                ubuntu_font_get_size(UbuntuFontSize::FONT_12),
                COLOR_BG, COLOR_FG,
                display.W - (x + w + 4));
    }
}

void Screen::draw_title() {
    const struct font* font = ubuntu_font_get_size(UbuntuFontSize::FONT_16);
    const int max_width = icon_x - 2*2;

    display.fill_rect(2, 0,
                      max_width, font->H,
                      COLOR_BG);

    add_scrolled_text_or_normal(
            2, 0, get_title(), font,
            COLOR_BG, COLOR_FG,
            max_width, false); // TODO enable scroll when remove_scrolled_text() implemented
}

void Screen::begin() {
    current_x = default_x();
    current_y = default_y();
    is_overlay_displayed = false;
}

void Screen::show() {
    display.clear_screen(COLOR_BG);

    draw_buttons();
    tick_sec_enable();
    tick_sec_call(); // first tick

    // after tick_sec() because it sets icon_x
    draw_title();
}

void Screen::hide() {
    reset_scrolled_texts();
}

void Screen::show_overlay(int bg, const char* msg) {
    reset_scrolled_texts();
    tick_sec_disable();

    display.clear_screen(bg);
    display.write_text_wrap(2, 0, msg,
                            ubuntu_font_get_size(UbuntuFontSize::FONT_16),
                            bg, COLOR_FG);

    is_overlay_displayed = true;
}