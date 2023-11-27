#pragma once

#include <loaderconst.hpp>

namespace settings {

enum {
    me_wifi_idx,
    me_bat_idx
};

const struct const_entry* get_menu_entries();
int get_menu_entry_count();

} // namespace