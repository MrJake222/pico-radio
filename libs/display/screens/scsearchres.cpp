#include "scsearchres.hpp"
#include <screenmng.hpp>
#include <radiosearch.hpp>
#include <listm3u.hpp>
#include <buffers.hpp>

static RadioSearch rs(get_http_buf());

int ScSearchRes::max_x(int y) {
    return 0;
}

int ScSearchRes::max_y() {
    return 0;
}

void ScSearchRes::draw_button(int x, int y, bool selected) {

}

int ScSearchRes::get_action(int x, int y) {
    return 0;
}

Screen* ScSearchRes::run_action(int action) {
    sc_search.begin(prompt);
    return &sc_search;
}

void ScSearchRes::show() {
    Screen::show();
    display.write_text(5, 20, prompt, 1);
}

void ScSearchRes::begin(const char* prompt_) {
    strcpy(prompt, prompt_);
    rs.begin(&get_raw_buf(), prompt);
}
