#pragma once

#include <loaderconst.hpp>

namespace settings {

enum {
    me_wifi_idx,
    me_bat_idx
};

extern const struct const_entry me_wifi;
extern const struct const_entry me_bat;

extern const struct const_entry menu_entries[];

extern const int menu_entry_count;

} // namespace