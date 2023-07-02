#include "scsearchres.hpp"

#include <screenmng.hpp>
#include <radiosearch.hpp>
#include <buffers.hpp>
#include <icons.hpp>

void all_loaded_cb(void* arg, int errored);

static RadioSearch rs(get_http_buf(), get_raw_buf());

#define S_RES_W     145
#define S_RES_H      20

#define KB_BUTTONS_MAX    4

int ScSearchRes::size_x(int y) {
    return 1;
}

int ScSearchRes::size_y() {
    return MIN(station_count, 4) + 1;
}

int ScSearchRes::max_base_y() {
    return station_count - kb_buttons();
}

int ScSearchRes::kb_buttons() {
    return MIN(KB_BUTTONS_MAX, station_count);
}

void ScSearchRes::draw_top_buttons() {
    // draw top 3 buttons, bottom button drawn by base class
    for (int y=0; y<kb_buttons()-1; y++) {
        draw_button(0, y, false);
    }
}

void ScSearchRes::draw_bottom_buttons() {
    // draw bottom 3 buttons, top button drawn by base class
    for (int y=1; y<kb_buttons(); y++) {
        draw_button(0, y, false);
    }
}

void ScSearchRes::draw_scroll_bar() {
    if (station_count == 0)
        return;

    display.set_bg(COLOR_BG_DARK);
    display.fill_rect(152, 31, 5, 83, true);

    const int y = 31;
    const int h = 83;
    const float segment = h / (float)station_count;
    const float start   = y + segment * (float)base_y;         // skip base_y segments
    const float height  =     segment * (float)kb_buttons();   // display 4 segments high bar

    display.set_bg(COLOR_BG_DARK_ACC1);
    display.fill_rect(152, (int)start, 5, (int)height, true);
}

void ScSearchRes::iny() {
    if (current_y == (last_y() - 1) && base_y + current_y + 1 < station_count) {
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

void ScSearchRes::dey() {
    if (current_y == 0 && base_y > 0) {
        // hidden station above exists
        base_y--;
        draw_scroll_bar();
        draw_bottom_buttons();
    }
    else if (current_y == last_y()) {
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

enum Action {
    PLAY,
    BACK,
};

int ScSearchRes::get_action(int x, int y) {
    if (y == last_y()) {
        return BACK;
    }

    return PLAY;
}

void ScSearchRes::draw_button(int x, int y, bool selected) {

    auto action = (Action) get_action(x, y);

    switch (action) {
        case BACK:
            set_btn_bg(selected, false);
            break;

        default:
            set_btn_bg(selected, true);
    }

    unsigned char xs;
    unsigned char ys;
    const char* name;

    switch (action) {
        case PLAY:
            xs = 5;
            ys = 31 + (S_RES_H + 1)*y;
            name = rs.get_station_name(base_y + y);

            display.fill_rect(xs, ys, S_RES_W, S_RES_H, true);
            display.write_text_maxlen(xs+3, ys, name, 1, 17); // TODO do scrolling
            break;

        case BACK:
            display.fill_rect(1, 114, 13, 13, true);
            display.draw_icon(2, 115, icon_back);
            break;
    }
}

Screen* ScSearchRes::run_action(int action) {
    int i;
    const char* url;

    switch ((Action) action) {
        case PLAY:
            i = base_y + current_y;
            url = rs.get_station_url(i);
            if (!url) {
                show_error("Błąd: nie można otworzyć strumienia");
                return nullptr;
            }

            sc_play.begin(rs.get_station_name(i), url);
            return &sc_play;

        case BACK:
            rs.load_abort();
            return &sc_search;
    }

    return nullptr;
}

void ScSearchRes::show() {
    // called from input
    Screen::show();

    display.set_bg(COLOR_BG);
    sprintf(subtitle, "\"%s\"", prompt);
    display.write_text(8, 15, subtitle, 1);

    if (first) {
        display.write_text(10, 40, "Ładowanie", 2);

        first = false;
        rs.load_stations();
    }
    else {
        draw_scroll_bar();
    }
}

void ScSearchRes::begin(const char* prompt_) {
    Screen::begin();

    // called from previous screen (on input)
    prompt = prompt_;

    base_y = 0;
    first = true;
    station_count = 0;

    rs.begin(prompt);
    rs.set_all_loaded_cb(this, all_loaded_cb);
}


void all_loaded_cb(void* arg, int errored) {
    // called from RadioSearch task
    auto sc = ((ScSearchRes*) arg);

    // set station count
    sc->station_count = rs.get_station_count();

    // re-draw the screen
    sc->show();
    if (errored > 0) {
        char c[80];
        snprintf(c, 80, "Błąd: nie udało się załadować %d dostawców stacji.", errored);
        sc->show_error(c);
    }
}