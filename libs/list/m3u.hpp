#pragma once

#include <listentry.hpp>
#include <lfs.h>

// in-place operators on M3U lists

namespace m3u {

// creates file with 0 entries
int create(const char* path);

// returns index of the new station or -1 on failure
int add(const char* path, const ListEntry* st);
// returns 0 on success
int remove(const char* path, int index);

// tries to find entry of name <name> and return it's url in <url> parameter
// returns 0 on success, -1 on failure
int get(const char* path, const char* name, char* url, int url_max_len);

}