#pragma once

// LFS sorter
// Saves to a temporary file results of some sort (Wi-Fi scan or FatFS listings)
// and can get k-th smallest element from them and return a sorted representation from said element

#include <lfsaccess.hpp>
#include <util.hpp>

// entry buffer, preserves the entire entry length
// (important, local playback saves paths here), needed for duplicate detection and loading
// +10 for any metadata before the line (for example local folders get "0" prefix to be sorted first)
#define LFSS_BUF_SIZE    (ENT_URL_LEN + 10)

// separate buffer sizes are more trouble than it's worth
// (for ex. mp3 and wav files are treated as duplicates)

namespace lfsorter {

// compare function, generally inputs are strings but
// 0x00 at the beginning indicates infinitely small value
// 0xff at the beginning indicates infinitely large value
typedef int(*cmp_fn)(const char* e1, const char* e2);
typedef void(*res_cb_fn)(void* arg, const char* res);

// for opening use LFSAccess functions
// <begin> to set path (should be module-unique)
// <open_r> and <open_rw_create_truncate> to open

// checks for duplicates
bool is_duplicate(LfsAccess& acc, cmp_fn cmp, const char* buf);
// writes to access a list of char pointers
int write(LfsAccess& acc, int n, ...);

// skips k smallest elements and returns n smallest elements of the rest
// note: this function returns entries as-is, refer to writing code to strip any extra flags
// this function is not async, after it returns all entries had been loaded and passed to callback
// returns how many entries were actually reported back to caller via callback or -1 on failure
int get_smallest_n_skip_k(LfsAccess& acc, flagref_t should_abort, int n, int k, cmp_fn cmp, void* res_cb_arg, res_cb_fn res_cb);

} // namespace