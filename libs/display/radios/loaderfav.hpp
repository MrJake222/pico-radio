#pragma once

#include <loader.hpp>
#include <lfsaccess.hpp>
#include <lfs.h>

typedef void(*ld_fav_update_cb)(void* arg, const char* info);

class LoaderFav : public Loader {

    lfs_t* lfs;
    LfsAccess rd;

    void task() override;

    ld_fav_update_cb upd_cb;
    void update(const char* info);

    int get_entry_count_whole() override;

public:
    LoaderFav(ListEntry* entries_, int entries_count_,
              lfs_t* lfs_)
        : Loader(entries_, entries_count_)
        , lfs(lfs_), rd(lfs_)
        { }

    void set_update_cb(ld_fav_update_cb cb) { upd_cb = cb; }
};
