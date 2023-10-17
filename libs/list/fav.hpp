#pragma once

#include <listentry.hpp>
#include <lfs.h>

// in-place operators on favourites list
// M3U format

namespace fav {

int create();
// returns index of the new station or -1 on failure
int add(const ListEntry* st);
int remove(int index);

}