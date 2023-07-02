#include "screen.hpp"

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

void Screen::set_btn_bg(bool selected, bool dark) {
    if (dark)
        display.set_bg(selected ? COLOR_BG_DARK_SEL : COLOR_BG_DARK);
    else
        display.set_bg(selected ? COLOR_BG_SEL : COLOR_BG);
}

void Screen::draw_buttons() {
    for (int y=0; y<size_y(); y++) {
        for (int x=0; x<size_x(y); x++) {
            bool s = (y == current_y && x == current_x);
            draw_button(x, y, s);
        }
    }
}

void Screen::begin() {
    current_x = default_x();
    current_y = default_y();
    is_err_displayed = false;
}

void Screen::show() {
    display.set_bg_fg(COLOR_BG, COLOR_FG);
    display.clear_screen();
    display.write_text(2, 0, get_title(), 1);

    draw_buttons();
}

void Screen::show_error(const char* err) {
    display.set_bg_fg(COLOR_BG_ERR, COLOR_FG);
    display.clear_screen();
    display.write_text_wrap(2, 0, err, 1);

    is_err_displayed = true;
}