#include "loaderconst.hpp"

void LoaderConst::task() {
    int to_skip = page * entries_max;

    for (; entries_offset<entries_max; entries_offset++) {
        const int const_entries_offset = to_skip + entries_offset;
        if (const_entries_offset >= const_entries_count)
            break;

        if (should_abort)
            break;

        const struct const_entry* const_entry = &const_entries[const_entries_offset];
        ListEntry* entry = &entries[entries_offset];
        entry->idx = const_entry->idx;
        entry->set_name(const_entry->display_name);
    }

    call_all_loaded(false);
}
