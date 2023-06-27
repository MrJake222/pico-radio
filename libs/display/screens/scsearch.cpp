#include "scsearch.hpp"

#include <icons.hpp>

#define KB_BNT_H   18
#define KB_BNT_W   14

#define ROWS             6
#define ROWS_CHARACTERS  4    // actual characters A-Z, 0-9

static const char* letters[ROWS_CHARACTERS] = {
        "1234567890",
        "QWERTYUIOP",
        "ASDFGHJKL",
        "ZXCVBNM",
};

// offsets of rows
static const char xoff[ROWS_CHARACTERS] = {5, 5, 8, 16};
// number of character in rows
static const char xlen[ROWS] = {1, 10, 10, 9, 7, 3};

int ScSearch::max_x(int y) {
    return xlen[y];
}

int ScSearch::max_y() {
    return ROWS;
}

enum Action {
    BACKSPACE,
    SPACE,
    BACK,
    SEARCH,
    KB,
};

int ScSearch::get_action(int x, int y) {
    if (y == 0) {
        return BACKSPACE;
    }

    else if (y == ROWS-1) {
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

void ScSearch::draw_button(int x, int y, bool selected) {

    auto action = (Action) get_action(x, y);
    unsigned char xs, ys;
    char letter;

    switch (action) {
        case BACKSPACE:
        case BACK:
        case SEARCH:
            set_btn_bg(selected, false);
            break;

        default:
            set_btn_bg(selected, true);
    }

    switch (action) {
        case BACKSPACE:
            display.fill_rect(138, 18, 20, 20, true);
            display.draw_icon(140, 20, icon_backspace);
            break;

        case SPACE:
            display.fill_rect(41, 120, 75, 6, true);
            break;

        case BACK:
            display.fill_rect(1, 114, 13, 13, true);
            display.draw_icon(2, 115, icon_back);
            break;

        case SEARCH:
            display.fill_rect(143, 111, 15, 15, true);
            display.draw_icon(144, 112, icon_search);
            break;

        case KB:
            y--; // backspace row

            // screen coordinates
            xs = xoff[y] + (KB_BNT_W + 1)*x;
            ys = 42 + (KB_BNT_H + 1)*y;
            letter = letters[y][x];

            display.fill_rect(xs, ys, KB_BNT_W, KB_BNT_H, true);
            display.write_char(xs + 3, ys, &letter, 1);
            break;
    }
}

void ScSearch::draw_prompt_field() {
    display.set_bg(COLOR_BG_DARK);
    display.fill_rect(5, 18, 131, 20, true);
    display.write_text_maxlen(5 + 3, 18, prompt, MAX_PROMPT_LEN, 1);

    set_btn_bg(get_action(get_current_x(), get_current_y()) == BACKSPACE, false);
}

void ScSearch::show() {
    Screen::show();

    pi = 0;
    prompt[pi] = '_';
    prompt[pi+1] = '\0';
    draw_prompt_field();
}

void ScSearch::run_action(int action) {
    switch ((Action) action) {
        case BACKSPACE:
            if (pi > 0)
                pi--;
            break;

        case SPACE:
            if (pi < MAX_PROMPT_LEN)
                prompt[pi++] = ' ';
            break;

        case BACK:
            break;

        case SEARCH:
            break;

        case KB:
            if (pi < MAX_PROMPT_LEN)
                prompt[pi++] = letters[get_current_y() - 1][get_current_x()];
            break;
    }

    prompt[pi] = '_';
    prompt[pi+1] = '\0';
    draw_prompt_field();
}
