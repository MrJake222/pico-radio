#pragma once

#include <datasource.hpp>
#include <lfs.h>

// TODO rename to LfsAccess and merge helpers here
// (from lfsutil)
class LfsReader : public DataSource {

    lfs_t* lfs;
    lfs_file_t file;

    char path[LFS_NAME_MAX];

    int bytes_read;

public:
    LfsReader(lfs_t* lfs_)
        : lfs(lfs_)
        { }

    void begin(const char* path_);
    int open();
    int close();

    int read_char(char *chr) override;
    bool more_content() override;

    int skip_lines(int n);
};
