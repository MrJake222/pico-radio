#pragma once

#include <list.hpp>
#include <lfs.h>

// in-place operators on favourites list
// M3U format

namespace fav {

int create(lfs_t* lfs);
// returns index of the new station or -1 on failure
int add(lfs_t* lfs, const struct station* st);
int remove(lfs_t* lfs, int index);

}