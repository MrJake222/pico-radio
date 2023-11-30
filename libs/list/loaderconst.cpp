#include "loaderconst.hpp"

void LoaderConst::task() {
    int to_skip = page * entries_max;

    while (can_fit_more_entries()) {
        const int const_entries_offset = to_skip + get_entries_offset();
        if (const_entries_offset >= const_entries_count)
            break;

        if (should_abort)
            break;

        const struct const_entry* const_entry = &const_entries[const_entries_offset];
        ListEntry* entry = get_current_entry();
        entry->set_name(const_entry->display_name);
        entry->lconst.idx = const_entry->idx;
        set_next_entry(1);
    }

    call_all_loaded(false);
}
