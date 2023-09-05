#pragma once

#include <listloader.hpp>
#include <lfsaccess.hpp>
#include <lfs.h>

typedef void(*ld_fav_update_cb)(void* arg, const char* info);

class LoaderFav : public ListLoader {

    lfs_t* lfs;
    LfsAccess rd;

    void task() override;

    ld_fav_update_cb upd_cb;
    void update(const char* info);

public:
    LoaderFav(struct station* stations_, int stations_count_,
              lfs_t* lfs_)
        : ListLoader(stations_, stations_count_)
        , lfs(lfs_), rd(lfs_)
        { }

    void set_update_cb(ld_fav_update_cb cb) { upd_cb = cb; }

    // warning: resource hungry
    int get_page_count() override;
};
