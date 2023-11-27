#pragma once

#include <listentry.hpp>
#include <lfs.h>

// in-place operators on M3U lists

namespace m3u {

int create(const char* path);
// returns index of the new station or -1 on failure
int add(const char* path, const ListEntry* st);
// returns 0 on success
int remove(const char* path, int index);

}