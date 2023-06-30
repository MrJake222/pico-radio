#include "scsearch.hpp"
#include <screenmng.hpp>

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
static const int8_t xoff[ROWS_CHARACTERS] = {5, 5, 8, 16};
// number of character in rows
static const int8_t xlen[ROWS] = {1, 10, 10, 9, 7, 3};

int ScSearch::size_x(int y) {
    return xlen[y];
}

int ScSearch::size_y() {
    return ROWS;
}

enum Action {
    BACKSPACE,
    SPACE,
    BACK,
    SEARCH,
    KB,
};

void ScSearch::prev_x_clear() {
    prev_x_first_to_kb = -1;
    prev_x_first_to_last = -1;
    prev_x_last_to_kb = -1;
    prev_x_kb_to_last = -1;
}

// adjusts x to match ux
// remembers last position when entering row with fewer elements
int ScSearch::adjust_x(int old_x, int old_y, int new_y) {

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

void ScSearch::x_changed() {
    prev_x_clear();
}

int ScSearch::get_action(int x, int y) {
    if (y == 0) {
        return BACKSPACE;
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
    display.write_text_maxlen(5 + 3, 18, prompt, 1, MAX_PROMPT_LEN);

    set_btn_bg(get_action(current_x, current_y) == BACKSPACE, false);
}

void ScSearch::show() {
    Screen::show();

    draw_prompt_field();
}

Screen* ScSearch::run_action(int action) {
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
            prompt[pi] = '\0';
            sc_search_res.begin(prompt);
            return &sc_search_res;

        case KB:
            if (pi < MAX_PROMPT_LEN)
                prompt[pi++] = letters[current_y - 1][current_x];
            break;
    }

    prompt[pi] = '_';
    prompt[pi+1] = '\0';
    draw_prompt_field();

    return nullptr;
}

void ScSearch::begin() {
    Screen::begin();

    strcpy(prompt, "");

    pi = strlen(prompt);
    prompt[pi] = '_';
    prompt[pi+1] = '\0';

    prev_x_clear();
}
