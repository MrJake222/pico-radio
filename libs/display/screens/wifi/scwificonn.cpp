#include "scwificonn.hpp"

#include <screenmng.hpp>
#include <icons.hpp>
#include <ubuntu_mono.hpp>
#include <wifi.hpp>
#include <m3u.hpp>

enum Action {
    BACK,
    DELETE
};

int ScWifiConn::get_action(int x, int y) {
    if (x == 0)
        return BACK;

    return DELETE;
}

void ScWifiConn::draw_button(int x, int y, bool selected, bool was_selected) {
    auto action = (Action) get_action(x, y);

    int bg;
    bg = get_btn_bg(selected, false);

    switch (action) {
        case BACK:
            display.fill_rect(1, 114, 13, 13, bg);
            display.draw_icon(2, 115, &icon_back, bg, COLOR_FG);
            break;

        case DELETE:
            break; // TODO implement
    }
}

Screen* ScWifiConn::run_action(int action) {
    switch ((Action) action) {
        case BACK:
            wifi::abort();
            return &sc_wifi_pwd;

        case DELETE:
            break; // TODO implement
    }

    return nullptr;
}

void ScWifiConn::begin(ListEntry* net_) {
    net = net_;
    Screen::begin();
}

void scwifi_update(void* arg, const char* str);
void scwifi_scan(void* arg, int quality);
void scwifi_conn(void* arg);

void ScWifiConn::show() {
    Screen::show();

    // top text -- wifi name: scrolled or normal (not changeable)
    add_scrolled_text_or_normal(
            2, 13, net->get_name(),
            ubuntu_font_get_size(UbuntuFontSize::FONT_24),
            COLOR_BG, COLOR_ACC2,
            display.W - 2*2);

    // bottom text -- "status"
    add_normal_text(
            2, 39, "Status:",
            ubuntu_font_get_size(UbuntuFontSize::FONT_16),
            COLOR_BG, COLOR_ACC1,
            display.W - 2*2);

    // bottom text -- wifi status: scrolled (save id to change later)
    meta_idx = add_scrolled_text(
            2, 39 + 18, "brak danych",
            ubuntu_font_get_size(UbuntuFontSize::FONT_16),
            COLOR_BG, COLOR_ACC1,
            display.W - 2*2);

    // signal strength "progress" bar
    add_normal_text_rjust(q_text_x_r, q_text_y, "Sygnał",
                          ubuntu_font_get_size(UbuntuFontSize::FONT_12),
                          COLOR_BG, COLOR_FG);
    draw_progress_bar(q_bar_x, q_bar_y, q_bar_w, 0, COLOR_BG_DARK, COLOR_ACC2, true);

    wifi::connect(net->get_name(),
                  net->get_url(),
                  {
                      this,
                      scwifi_update,
                      scwifi_scan,
                      scwifi_conn
                  });
}

void scwifi_update(void* arg, const char* str) {
    auto sc = (ScWifiConn*) arg;

    sc->update_scrolled_text(sc->meta_idx, str);
}

void scwifi_scan(void* arg, int quality) {
    auto sc = (ScWifiConn*) arg;

    sc->draw_progress_bar(sc->q_bar_x, sc->q_bar_y, sc->q_bar_w, quality, COLOR_BG_DARK, COLOR_ACC2, true);
}

void scwifi_conn(void* arg) {
    auto sc = (ScWifiConn*) arg;

    m3u::add(PATH_WIFI, sc->net);
}