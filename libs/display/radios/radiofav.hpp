#pragma once

#include <listloader.hpp>
#include <lfsreader.hpp>
#include <lfs.h>

class RadioFav : public ListLoader {

    LfsReader rd;

    void task() override;

public:
    RadioFav(struct station* stations_, int stations_count_,
            lfs_t* lfs)
        : ListLoader(stations_, stations_count_)
        , rd(lfs)
        { }
};
