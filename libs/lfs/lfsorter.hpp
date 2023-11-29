#pragma once

// LFS sorter
// Saves to a temporary file results of some sort (Wi-Fi scan or FatFS listings)
// and can get k-th smallest element from them and return a sorted representation from said element

#include <lfsaccess.hpp>

namespace lfsorter {

typedef int(*cmp_fn)(const char* e1, const char* e2);
typedef void(*res_cb_fn)(void* arg, const char* res);

// creates and opens temporary file
int open_create_truncate(LfsAccess& acc);
// opens previously created temporary file
int open(LfsAccess& acc);
// writes to access a list of char pointers
int write(LfsAccess& acc, int n, ...);
// skips k smallest elements and returns n smallest elements of the rest
int get_smallest_n_skip_k(LfsAccess& acc, int n, int k, cmp_fn cmp, void* res_cb_arg, res_cb_fn res_cb);

} // namespace