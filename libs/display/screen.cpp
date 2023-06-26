#include "screen.hpp"

void Screen::set_btn_bg(bool selected) {
    display.set_bg(selected ? COLOR_BG_SEL : COLOR_BG_DARK);
}

void Screen::show() {
    display.set_bg_fg(COLOR_BG, COLOR_FG);
    display.clear_screen();
    display.write_text(2, 0, get_title(), 1);
}