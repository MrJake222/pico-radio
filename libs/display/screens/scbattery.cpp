#include <icons.hpp>
#include <screenmng.hpp>
#include <ubuntu_mono.hpp>
#include <analog.hpp>
#include "scbattery.hpp"


int ScBattery::size_x(int y) {
    return 1;
}

int ScBattery::size_y() {
    return 1;
}

enum Action {
    BACK
};

void ScBattery::draw_button(int x, int y, bool selected) {

    auto action = (Action) get_action(x, y);
    int bg;
    const int fg = COLOR_FG;

    switch (action) {
        case BACK:
            bg = get_btn_bg(selected, false);
            break;
    }

    switch (action) {
        case BACK:
            display.fill_rect(1, 114, 13, 13, bg);
            display.draw_icon(2, 115, icon_back, bg, fg);
            break;
    }
}

int ScBattery::get_action(int x, int y) {
    return BACK;
}

Screen* ScBattery::run_action(int action) {
    switch ((Action) action) {
        case BACK:
            return &sc_settings;
    }

    return nullptr;
}

void ScBattery::show() {
    Screen::show();

    add_normal_text(2, 20, "VCC:",
                    ubuntu_font_get_size(UbuntuFontSize::FONT_12),
                    COLOR_BG, COLOR_FG,
                    display.W - 2);

    add_normal_text(2, 34, "VBATT:",
                    ubuntu_font_get_size(UbuntuFontSize::FONT_12),
                    COLOR_BG, COLOR_FG,
                    display.W - 2);
}

void ScBattery::tick_sec() {
    Screen::tick_sec();
    char buf[6];

    sprintf(buf, "%1.3f", analog::vcc_voltage());
    add_normal_text(50, 20, buf,
                    ubuntu_font_get_size(UbuntuFontSize::FONT_12),
                    COLOR_BG, COLOR_FG,
                    display.W - 2);

    sprintf(buf, "%1.3f", analog::battery_voltage());
    add_normal_text(50, 34, buf,
                    ubuntu_font_get_size(UbuntuFontSize::FONT_12),
                    COLOR_BG, COLOR_FG,
                    display.W - 2);

    sprintf(buf, "%2d%%", analog::battery_percentage());
    add_normal_text(90, 34, buf,
                    ubuntu_font_get_size(UbuntuFontSize::FONT_12),
                    COLOR_BG, COLOR_FG,
                    display.W - 2);
}
