#include "scplay.hpp"

#include <screenmng.hpp>
#include <icons.hpp>
#include <player.hpp>

int ScPlay::size_x(int y) {
    return 1;
}

int ScPlay::size_y() {
    return 1;
}

enum Action {
    BACK
};

void ScPlay::draw_button(int x, int y, bool selected) {

    auto action = (Action) get_action(x, y);

    switch (action) {
        case BACK:
            set_btn_bg(selected, false);
            break;
    }

    switch (action) {
        case BACK:
            display.fill_rect(1, 114, 13, 13, true);
            display.draw_icon(2, 115, icon_back);
            break;
    }
}

int ScPlay::get_action(int x, int y) {
    return BACK;
}

Screen* ScPlay::run_action(int action) {
    switch ((Action) action) {
        case BACK:
            player_stop();
            return &sc_search_res;
    }

    return nullptr;
}

void ScPlay::show() {
    Screen::show();

    display.set_bg_fg(COLOR_BG, COLOR_BG_DARK_ACC2);
    display.write_text_maxlen(15, 24, radio_name, 17, 1);

    player_start(radio_url);
    // TODO when failed, close the screen
    // interesting how to do it, only main.cpp can show screens currently
    // maybe move it to screen manager
}

void ScPlay::begin(const char* name_, const char* url_) {
    radio_name = name_;
    radio_url = url_;
    Screen::begin();
}
