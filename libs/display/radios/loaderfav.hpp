#pragma once

#include <listloader.hpp>
#include <lfsreader.hpp>
#include <lfs.h>

class LoaderFav : public ListLoader {

    lfs_t* lfs;
    LfsReader rd;

    void task() override;

public:
    LoaderFav(struct station* stations_, int stations_count_,
              lfs_t* lfs_)
        : ListLoader(stations_, stations_count_)
        , lfs(lfs_), rd(lfs_)
        { }
};
