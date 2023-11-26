#include "settings.hpp"

namespace settings {

const struct const_entry me_wifi = { .idx = me_wifi_idx, .display_name = "Połączenie Wi-Fi"};
const struct const_entry me_bat =  { .idx = me_bat_idx, .display_name = "Poziom baterii"};

const struct const_entry menu_entries[] = {
        me_wifi,
        me_bat
};

const int menu_entry_count = sizeof(menu_entries) / sizeof(menu_entries[0]);

} // namespace