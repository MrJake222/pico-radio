#include <ubuntu_mono.hpp>
#include <cstdio>
#include "screenlist.hpp"

// #define s_base_x      3
// #define s_base_y     31
//
// #define s_res_w     147
// #define s_res_h      20
// #define s_res_mar     1
// #define s_res_pad     2
//
// #define s_scr_w    5
// #define s_scr_pad  2

int ScreenList::max_base_y() {
    return station_count - kb_buttons();
}

int ScreenList::kb_buttons() {
    return MIN(KB_BUTTONS_MAX, station_count);
}

void ScreenList::draw_top_buttons() {
    // draw top 3 buttons, bottom button drawn by base class
    for (int y=0; y<kb_buttons()-1; y++) {
        draw_button(0, y, false);
    }
}

void ScreenList::draw_bottom_buttons() {
    // draw bottom 3 buttons, top button drawn by base class
    for (int y=1; y<kb_buttons(); y++) {
        draw_button(0, y, false);
    }
}

void ScreenList::draw_scroll_bar() {
    if (station_count == 0)
        return;

    const int x = s_base_x + s_res_w + s_scr_pad;
    const int y = s_base_y;
    const int h = (s_res_h + s_res_mar) * 3 + s_res_h;

    display.fill_rect(x, y, s_scr_w, h, COLOR_BG_DARK);

    const float segment = h / (float)station_count;
    const float start   = y + segment * (float)base_y;         // skip base_y segments
    const float height  =     segment * (float)kb_buttons();   // display 4 segments high bar

    display.fill_rect(x, (int)start, s_scr_w, (int)height, COLOR_ACC1);
}

void ScreenList::iny() {
    if (current_y == last_list_row() && get_selected_station_index() + 1 < station_count) {
        // last kb row
        // hidden station below exists
        base_y++;
        draw_scroll_bar();
        draw_top_buttons();
    }
    else if (current_y == last_y()) {
        // last icon
        // move to top
        base_y = 0;
        draw_scroll_bar();
        Screen::iny();
        draw_bottom_buttons();
    }
    else {
        Screen::iny();
    }
}

void ScreenList::dey() {
    if (current_y == first_list_row() && base_y > 0) {
        // hidden station above exists
        base_y--;
        draw_scroll_bar();
        draw_bottom_buttons();
    }
    else if (current_y == last_list_row() + 1) {
        // last icon
        // move to bottom
        base_y = max_base_y();
        draw_scroll_bar();
        Screen::dey();
        draw_top_buttons();
    }
    else {
        Screen::dey();
    }
}

void ScreenList::button_pre_selection_change() {
    reset_scrolled_texts();
}

void ScreenList::draw_button_entry(int y, bool selected) {

    unsigned char xs;
    unsigned char ys;
    const char* name;
    int bg;

    xs = s_base_x;
    ys = s_base_y + (s_res_h + s_res_mar) * y;
    name = ll.get_station_name(base_y + y);

    bg = get_btn_bg(selected, true);
    display.fill_rect(xs, ys, s_res_w, s_res_h, bg);
    add_scrolled_text_or_normal(xs + s_res_pad, ys + 1, name,
                                ubuntu_font_get_size(UbuntuFontSize::FONT_16),
                                bg, COLOR_FG,
                                s_res_w - s_res_pad * 2, selected);
}

void ScreenList::show() {
    // called from input
    Screen::show();

    if (first) {
        add_normal_text(10, 40, "Ładowanie",
                        ubuntu_font_get_size(UbuntuFontSize::FONT_24),
                        COLOR_BG, COLOR_FG,
                        display.W);

        first = false;
        ll.load_stations();
    }
    else {
        draw_scroll_bar();
    }
}

void all_loaded_cb(void* arg, int errored);

void ScreenList::begin() {
    Screen::begin();

    base_y = 0;
    first = true;
    station_count = 0;

    ll.set_all_loaded_cb(this, all_loaded_cb);
}


void all_loaded_cb(void* arg, int errored) {
    // called from RadioSearch task
    auto sc = ((ScreenList*) arg);

    // set station count
    sc->station_count = sc->ll.get_station_count();

    if (errored > 0) {
        // show error
        char c[80];
        snprintf(c, 80, "Błąd: nie udało się załadować %d dostawców stacji.", errored);
        sc->show_error(c);
    }
    else {
        // re-draw the screen
        sc->show();
    }
}