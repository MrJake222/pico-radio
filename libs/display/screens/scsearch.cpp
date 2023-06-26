#include "scsearch.hpp"

#include <icons.hpp>
#include <stack>

#define KB_BNT_H   18
#define KB_BNT_W   14

#define ROWS             6
#define ROWS_CHARACTERS  4    // actual characters A-Z, 0-9
#define ROW_BACKSPACE    0
#define ROW_SPACE        5

static const char* letters[ROWS_CHARACTERS] = {
        "1234567890",
        "QWERTYUIOP",
        "ASDFGHJKL",
        "ZXCVBNM",
};

// offsets of rows
static const char xoff[ROWS_CHARACTERS] = {5, 5, 8, 16};
// number of character in rows
static const char xlen[ROWS] = {1, 10, 10, 9, 7, 1};

static char get_letter(int y, int x) {
    return letters[y - 1][x]; // backspace offset
}

static char get_xoff(int y) {
    return xoff[y - 1]; // backspace offset
}

void ScSearch::draw_backspace() {
    display.fill_rect(138, 20, 15, 15, true);
    display.draw_icon(138, 20, icon_backspace);
}

void ScSearch::draw_space() {
    display.fill_rect(41, 120, 75, 6, true);
}

void ScSearch::draw_kb_btn(int x, int y, bool selected) {

    set_btn_bg(selected);

    if (y == ROW_BACKSPACE)
        draw_backspace();

    else if (y == ROW_SPACE)
        draw_space();

    else {
        // screen coordinates
        int xs = get_xoff(y) + (KB_BNT_W + 1)*x;
        int ys = 42 + (KB_BNT_H + 1)*(y-1);
        char letter = get_letter(y, x);

        display.fill_rect(xs, ys, KB_BNT_W, KB_BNT_H, true);
        display.write_char(xs + 3, ys, &letter, 1);
    }
}

void ScSearch::draw_kb_row(int y) {
    for (int x=0; x<xlen[y]; x++) {
        draw_kb_btn(x, y, false);
    }
}

void ScSearch::inx() {
    current_x++;
    current_x %= xlen[current_y];
}

void ScSearch::dex() {
    current_x--;
    if (current_x < 0)
        current_x = xlen[current_y] - 1;
}

void ScSearch::limit_x() {
    current_x = MIN(current_x, xlen[current_y] - 1);
}

void ScSearch::iny() {
    current_y++;
    current_y %= ROWS;
    limit_x();
}

void ScSearch::dey() {
    current_y--;
    if (current_y < 0)
        current_y = ROWS - 1;
    limit_x();
}

void ScSearch::draw_prompt_field() {
    display.set_bg(COLOR_BG_DARK);
    display.fill_rect(5, 18, 150, 20, true);
    display.write_text_maxlen(5 + 3, 18, prompt, MAX_PROMPT_LEN, 1);

    set_btn_bg(current_x == 0 && current_y == ROW_BACKSPACE);
    draw_backspace();
}

void ScSearch::show() {
    Screen::show();

    pi = 0;
    prompt[pi] = '_';
    prompt[pi+1] = '\0';
    draw_prompt_field();

    for (int y=0; y<ROWS; y++)
        draw_kb_row(y);

    draw_kb_btn(current_x, current_y, true);
}

void ScSearch::input(ButtonEnum btn) {
    if (btn == CENTER) {
        if (current_y == ROW_BACKSPACE) {
            if (pi > 0)
                pi--;
        }

        else if (current_y == ROW_SPACE) {
            if (pi < MAX_PROMPT_LEN)
                prompt[pi++] = ' ';
        }

        else {
            if (pi < MAX_PROMPT_LEN)
                prompt[pi++] = get_letter(current_y, current_x);
        }

        prompt[pi] = '_';
        prompt[pi+1] = '\0';
        draw_prompt_field();
        return;
    }

    draw_kb_btn(current_x, current_y, false);

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

    draw_kb_btn(current_x, current_y, true);
}
