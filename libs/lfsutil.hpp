#pragma once

#include <lfs.h>

namespace lfsutil {

int skip_line(lfs_t* lfs, lfs_file_t* file);
int skip_lines(lfs_t* lfs, lfs_file_t* file, int n);

}