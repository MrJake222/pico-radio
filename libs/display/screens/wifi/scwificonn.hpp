#pragma once

#include <screen.hpp>
#include <listentry.hpp>

class ScWifiConn : public Screen {

    const char * get_title() override { return "Sieć Wi-Fi"; }

    int size_x(int y) override { return 1; }
    int size_y() override { return 1; }

    int get_action(int x, int y) override;
    Screen * run_action(int action) override;
    void draw_button(int x, int y, bool selected, bool was_selected) override;

    ListEntry* net;

    // signal quality bar
    static const int q_text_x_r = 36 + 10;      // quality text x r
    static const int q_text_y = 85;             // quality text y
    static const int q_bar_x = q_text_x_r + 3;  // quality bar x
    static const int q_bar_y = q_text_y + 5;    // quality bar y
    const int q_bar_w;                          // quality bar width

    int meta_idx;
    friend void scwifi_update(void* arg, const char* str);
    friend void scwifi_scan(void* arg, int quality);
    friend void scwifi_conn(void* arg);

public:
    ScWifiConn(ST7735S& display_, SemaphoreHandle_t& mutex_ticker_)
        : Screen(display_, mutex_ticker_)
        , q_bar_w(display.W - q_bar_x - 13 - 22)
        { }

    void begin(ListEntry* net_);

    void show() override;
};
