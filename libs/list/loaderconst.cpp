#include "loaderconst.hpp"

void LoaderConst::task() {
    int to_skip = page * entries_max;

    for (; entries_offset<entries_max; entries_offset++) {
        const int const_entries_offset = to_skip + entries_offset;
        if (const_entries_offset >= const_entries_count)
            break;

        if (should_abort)
            break;

        ListEntry* entry = &entries[entries_offset];
        entry->set_name(const_entries[const_entries_offset]);
    }

    call_all_loaded(false);
}
