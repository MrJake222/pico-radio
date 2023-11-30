#pragma once

#include <loaderm3u.hpp>

typedef void(*ld_fav_update_cb)(void* arg, const char* info);

class LoaderFav : public LoaderM3U {

    void task() override;
    void setup_entry(ListEntry *ent) override;

public:
    LoaderFav(ListEntry* entries_, int entries_max_,
              LfsAccess& acc_)
        : LoaderM3U(entries_, entries_max_,
                    acc_, PATH_FAVOURITES)
    { }
};
