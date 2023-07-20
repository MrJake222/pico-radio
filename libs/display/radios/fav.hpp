#pragma once

#include <list.hpp>
#include <lfs.h>

// in-place operators on favourites list
// M3U format

namespace fav {

int create(lfs_t* lfs);
int add(lfs_t* lfs, struct station* st);
int remove(lfs_t* lfs, int index);

}