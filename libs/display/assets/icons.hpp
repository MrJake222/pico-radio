#pragma once

#include <cstdint>

struct icon {
    uint8_t w;
    uint8_t h;
    const uint8_t* data;
};

extern const struct icon icon_backspace;
extern const struct icon icon_back;
extern const struct icon icon_search;
extern const struct icon icon_star_empty;
extern const struct icon icon_star_filled;
extern const struct icon icon_fav_back;
extern const struct icon icon_battery_0;
extern const struct icon icon_battery_50;
extern const struct icon icon_battery_100;
extern const struct icon icon_sd;
extern const struct icon icon_sd_disabled;
extern const struct icon icon_local;
extern const struct icon icon_folder;
extern const struct icon icon_shift_empty;
extern const struct icon icon_shift_filled;
extern const struct icon icon_shift_sticky;
extern const struct icon icon_settings;
// extern const struct icon icon_wifi_0;
// extern const struct icon icon_wifi_1;
// extern const struct icon icon_wifi_2;
// extern const struct icon icon_wifi_3;
extern const struct icon* icon_wifi[4];
extern const struct icon icon_wifi_3_x;
extern const struct icon icon_delete;
extern const struct icon icon_conn;