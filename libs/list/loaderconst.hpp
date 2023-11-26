#pragma once

#include <loader.hpp>

struct const_entry {
    int idx;
    const char* display_name;
};

class LoaderConst : public Loader {

    const struct const_entry* const_entries;
    int const_entries_count;

    int get_entry_count_whole() override { return const_entries_count; };
    void task() override;

public:
    LoaderConst(ListEntry* entries_, int entries_max_,
                const struct const_entry* const_entries_, int const_entries_count_)
            : Loader(entries_, entries_max_)
            , const_entries(const_entries_)
            , const_entries_count(const_entries_count_)
    { }
};
