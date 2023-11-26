#include "screenkb.hpp"
#include <screenmng.hpp>

#include <icons.hpp>
#include <ubuntu_mono.hpp>

#define KB_BNT_H   18
#define KB_BNT_W   14

#define ROWS             6
#define ROWS_KEYBOARD    4

static const char* letters[2][ROWS_KEYBOARD] = {
        { // letters[0] -- no shift
            "1234567890",
            "qwertyuiop",
            "asdfghjkl",
            "zxcvbnm",
        },

        { // letters[1] -- with shift
            "!@#$%^&*()",
            "QWERTYUIOP",
            "ASDFGHJKL",
            "ZXCVBNM",
        },
};

// offsets of rows
static const int8_t xoff[ROWS_KEYBOARD] = {5, 5, 8, 16};
// number of character in rows
static const int8_t xlen[ROWS] = {1, 10, 10, 9, 7+1, 3}; // +1 shift

int ScreenKb::size_x(int y) {
    return xlen[y];
}

int ScreenKb::size_y() {
    return ROWS;
}

enum Action {
    BACKSPACE,
    SHIFT,
    SPACE,
    BACK,
    SEARCH,
    KB,
};

void ScreenKb::prev_x_clear() {
    prev_x_first_to_kb = -1;
    prev_x_first_to_last = -1;
    prev_x_last_to_kb = -1;
    prev_x_kb_to_last = -1;
}

// adjusts x to match ux
// remembers last position when entering row with fewer elements
int ScreenKb::adjust_x(int old_x, int old_y, int new_y) {

    int8_t new_x = -1;

    // forward
    // <clear>
    // back

    if (new_y == 0) {
        // to top
        // forward: always 0

        prev_x_clear();

        if (old_y == 1) {
            // back: first to kb
            prev_x_first_to_kb = (int8_t) old_x;
        }

        else {
            // old_y == last_y
            // back: first to last
            prev_x_first_to_last = (int8_t) old_x;
        }
    }

    else if (old_y == 0) {
        // from top

        if (new_y == 1) {
            // forward: first to kb
            new_x = prev_x_first_to_kb;
            prev_x_first_to_kb = -1;
        }

        else {
            // new_y == last_y
            // forward: first to last
            new_x = prev_x_first_to_last;
            prev_x_first_to_last = -1;
        }

        if (new_x == -1)
            new_x = (int8_t) last_x(new_y);

        prev_x_clear();
        // back: always 0
    }

        // between last <-> kb
    else if (new_y == last_y() && old_y == (last_y() - 1)) {
        // forward: kb to last
        new_x = prev_x_kb_to_last;
        prev_x_kb_to_last = -1;

        prev_x_clear();

        // back: last to kb
        prev_x_last_to_kb = (int8_t) old_x;

        if (new_x == -1)
            new_x = (old_x == 0) ? 0 : 1; // Z -> search / rest -> space
    }

    else if (new_y == (last_y() - 1) && old_y == last_y()) {
        // forward: last to kb
        new_x = prev_x_last_to_kb;
        prev_x_last_to_kb = -1;

        prev_x_clear();

        // back: kb to last
        prev_x_kb_to_last = (int8_t) old_x;

        if (new_x == -1) {
            if (old_x == 1) // space
                new_x = 3; // V
            else if (old_x == last_x(old_y)) // search
                new_x = (int8_t) last_x(new_y); // M
        }
    }

    else {
        prev_x_clear();
    }

    if (new_x != -1)
        return new_x;

    return Screen::adjust_x(old_x, old_y, new_y);
}

void ScreenKb::x_changed() {
    prev_x_clear();
}

int ScreenKb::get_action(int x, int y) {
    if (y == 0) {
        return BACKSPACE;
    }

    else if ((y == (size_y() - 2)) && (x == (size_x(y) - 1))) {
        return SHIFT;
    }

    else if (y == (size_y() - 1)) {
        if (x == 0)
            return BACK;
        else if (x == 1)
            return SPACE;
        else
            return SEARCH;
    }

    else {
        return KB;
    }
}

void ScreenKb::draw_button(int x, int y, bool selected) {

    auto action = (Action) get_action(x, y);
    int bg;
    const int fg = COLOR_FG;

    switch (action) {
        case BACKSPACE:
        case BACK:
        case SEARCH:
            bg = get_btn_bg(selected, false);
            break;

        default:
            bg = get_btn_bg(selected, true);
    }

    // used in KB/SHIFT
    // adjust for backspace row
    const int ym = y - 1;
    // screen coordinates
    const int xs = xoff[ym] + (KB_BNT_W + 1)*x;
    const int ys = 42 + (KB_BNT_H + 1)*ym;

    switch (action) {
        case BACKSPACE:
            display.fill_rect(138, 18, 20, 20, bg);
            display.draw_icon(140, 20, icon_backspace, bg, fg);
            break;

        case SPACE:
            display.fill_rect(41, 120, 75, 6, bg);
            break;

        case BACK:
            display.fill_rect(1, 114, 13, 13, bg);
            display.draw_icon(2, 115, icon_back, bg, fg);
            break;

        case SEARCH:
            display.fill_rect(143, 111, 15, 15, bg);
            display.draw_icon(144, 112, icon_search, bg, fg);
            break;

        case SHIFT:
            display.fill_rect(xs, ys, KB_BNT_W + 7, KB_BNT_H, bg);
            display.draw_icon(xs + 4, ys + 4,
                              shift ? icon_shift_filled : icon_shift_empty,
                              bg, fg);
            break;

        case KB:
            display.fill_rect(xs, ys, KB_BNT_W, KB_BNT_H, bg);
            display.write_char(xs + 3, ys, &letters[shift][ym][x],
                               ubuntu_font_get_size(UbuntuFontSize::FONT_16),
                               bg, fg, 0, 0);
            break;
    }
}

void ScreenKb::draw_prompt_field() {
    const int width = 131;
    const struct font* font = ubuntu_font_get_size(UbuntuFontSize::FONT_16);

    const int x = 5; // text field position
    const int m = 1 + font->W/2; // margin
    const int text_x = x+m; // text position
    const int text_width = width - 2*m;

    const int chr_fits = text_width / font->W;
    const int chr_skips = MAX(0, ti+1 - chr_fits); // +1 prompt character

    // not overflow
    const bool nov = chr_skips == 0;

    display.fill_rect(x, 18, width, 20, COLOR_BG_DARK);
    display.write_text(nov ? text_x : text_x-font->W,       // overflow: start rendering a letter earlier
                       18,
                       nov ? text : text + chr_skips - 1,   // overflow: start rendering at offset - a letter to show overflow
                       font, COLOR_BG_DARK, COLOR_FG,
                       nov ? text_x : text_x - font->W/2,   // overflow: indicate by showing a bit of the previous letter
                       5 + width - 3);
}

void ScreenKb::show() {
    Screen::show();

    draw_prompt_field();
    buttons_repeat_left_right(true);
    buttons_repeat_center(true);
}

void ScreenKb::hide() {
    buttons_repeat_left_right(false);
    buttons_repeat_center(false);
}

Screen* ScreenKb::run_action(int action) {

    const int ti_old = ti;

    switch ((Action) action) {
        case BACKSPACE:
            if (ti > 0)
                ti--;
            break;

        case SHIFT:
            shift ^= true;  // xor
            draw_buttons(); // redraw keyboard
            break;

        case SPACE:
            if (ti < text_max_len())
                text[ti++] = ' ';
            break;

        case BACK:
            return sc_back();

        case SEARCH:
            text[ti] = '\0';
            return sc_forward(text);

        case KB:
            if (ti < text_max_len())
                text[ti++] =  letters[shift][current_y - 1][current_x];
            break;
    }

    if (ti != ti_old) {
        // prevents flickering
        text[ti] = '_';
        text[ti + 1] = '\0';
        draw_prompt_field();
    }

    return nullptr;
}

void ScreenKb::begin() {
    Screen::begin();

    shift = false;
    text[0] = '\0';
    ti = 0;

    text[ti] = '_';
    text[ti + 1] = '\0';

    prev_x_clear();
}
