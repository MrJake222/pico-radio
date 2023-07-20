#pragma once

#include <listloader.hpp>
#include <lfsaccess.hpp>
#include <lfs.h>

class RadioFav : public ListLoader {

    LfsAccess lfsa;

    void task() override;

public:
    RadioFav(struct station* stations_, int stations_count_,
            lfs_t* lfs)
        : ListLoader(stations_, stations_count_)
        , lfsa(lfs)
        { }
};
