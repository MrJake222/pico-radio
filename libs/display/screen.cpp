#include "screen.hpp"

void Screen::inx() {
    current_x++;
    current_x %= max_x(current_y);
}

void Screen::dex() {
    current_x--;
    if (current_x < 0)
        current_x = max_x(current_y) - 1;
}

void Screen::limit_x() {
    current_x = MIN(current_x, max_x(current_y) - 1);
}

void Screen::iny() {
    current_y++;
    current_y %= max_y();
    limit_x();
}

void Screen::dey() {
    current_y--;
    if (current_y < 0)
        current_y = max_y() - 1;
    limit_x();
}

Screen* Screen::input(ButtonEnum btn) {
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

void Screen::show() {
    display.set_bg_fg(COLOR_BG, COLOR_FG);
    display.clear_screen();
    display.write_text(2, 0, get_title(), 1);

    for (int y=0; y<max_y(); y++) {
        for (int x=0; x<max_x(y); x++) {
            draw_button(x, y, false);
        }
    }

    current_x = default_x();
    current_y = default_y();
    draw_button(current_x, current_y, true);
}