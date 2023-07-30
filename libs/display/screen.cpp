#include "screen.hpp"

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
    if (is_err_displayed) {
        show(); // redraw normal screen
        is_err_displayed = false;
        return nullptr;
    }

    if (btn == CENTER) {
        return run_action(get_action(current_x, current_y));
    }

    button_pre_selection_change();

    draw_button(current_x, current_y, false);

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

    draw_button(current_x, current_y, true);
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
            draw_button(x, y, s);
        }
    }
}

void Screen::add_normal_text(int text_x, int text_y, const char* str, const struct font* font, int bg, int fg,
                             int max_width) {
    display.write_text(text_x, text_y, str, font, bg, fg,
                       text_x, text_x + max_width);
}

void Screen::add_normal_text_ljust(int text_x_r, int text_y, const char* str, const struct font* font, int bg, int fg) {

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


void Screen::tick() {
    xSemaphoreTake(mutex_ticker, portMAX_DELAY);

    for (int i=0; i<texts_index; i++) {
        texts[i].update(LCD_TICK_INTERVAL_MS);
        texts[i].draw();
    }

    tick_sec_counter++;
    if (tick_sec_counter == 1000 / LCD_TICK_INTERVAL_MS) {
        tick_sec_counter = 0;
        tick_sec();
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

void Screen::draw_progress_bar(int x, int y, int percent, int bg, int fg) {

    const int filled_bar_width = percent * 64 / 100;

    // filled bar
    display.fill_rect(x, y, filled_bar_width, 4, fg);

    // empty bar
    display.fill_rect(x + filled_bar_width, y, 64 - filled_bar_width, 4, bg);

    // percent
    char buf[5];
    sprintf(buf, "%02d%%", percent);
    add_normal_text(
            x + 64 + 4, y - 5, buf,
            ubuntu_font_get_size(UbuntuFontSize::FONT_12),
            COLOR_BG, COLOR_FG,
            display.W);
}

void Screen::begin() {
    current_x = default_x();
    current_y = default_y();
    is_err_displayed = false;
}

void Screen::show() {
    reset_scrolled_texts();

    display.clear_screen(COLOR_BG);
    add_normal_text(2, 0, get_title(),
                    ubuntu_font_get_size(UbuntuFontSize::FONT_16),
                    COLOR_BG, COLOR_FG,
                    display.W - 2*2);

    draw_buttons();
    tick_sec_counter = 0;
    tick_sec(); // first tick
}

void Screen::hide() {
    reset_scrolled_texts();
}

void Screen::show_error(const char* err) {
    reset_scrolled_texts();

    display.clear_screen(COLOR_BG_ERR);
    display.write_text_wrap(2, 0, err,
                            ubuntu_font_get_size(UbuntuFontSize::FONT_16),
                            COLOR_BG_ERR, COLOR_FG);

    is_err_displayed = true;
}