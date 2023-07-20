#pragma once

#include <listloader.hpp>
#include <lfsreader.hpp>
#include <lfs.h>

class LoaderFav : public ListLoader {

    LfsReader rd;

    void task() override;

public:
    LoaderFav(struct station* stations_, int stations_count_,
              lfs_t* lfs)
        : ListLoader(stations_, stations_count_)
        , rd(lfs)
        { }
};
