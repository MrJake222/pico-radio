#pragma once

#include <loader.hpp>
#include <lfsaccess.hpp>
#include <lfs.h>

typedef void(*ld_fav_update_cb)(void* arg, const char* info);

// This class loads M3U format
// loads EXTINF=dummy,<name> and the following line as <url>
// can be used for favs or wifi
class LoaderM3U : public Loader {

    const char* path;

    lfs_t* lfs;
    LfsAccess rd;

    void task() override;

    ld_fav_update_cb upd_cb;
    void update(const char* info);

    int get_entry_count_whole() override;

public:
    LoaderM3U(ListEntry* entries_, int entries_max_,
              lfs_t* lfs_)
        : Loader(entries_, entries_max_)
        , lfs(lfs_), rd(lfs_)
        { }

    void begin(const char* path_);

    void set_update_cb(ld_fav_update_cb cb) { upd_cb = cb; }
};
