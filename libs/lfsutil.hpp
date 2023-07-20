#pragma once

#include <lfs.h>

namespace lfsutil {

// returns -1 on failure, 0 on success, 1 on end-of-file
int skip_line(lfs_t* lfs, lfs_file_t* file);
int skip_lines(lfs_t* lfs, lfs_file_t* file, int n);
// skips all lines and counts them
// returns -1 on failure
int skip_all_lines(lfs_t* lfs, lfs_file_t* file);

}