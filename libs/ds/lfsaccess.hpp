#pragma once

#include <datainterface.hpp>
#include <lfs.h>

class LfsAccess : public DataInterface {

    lfs_t* lfs;
    lfs_file_t file;

    int bytes_read;
    int open(const char* path, int flags);

public:
    LfsAccess(lfs_t* lfs_)
        : lfs(lfs_)
        { }

    int open_read_create(const char* path);
    int open_write_trunc(const char* path);
    int close();

    int read_char(char *chr) override;
    bool more_content() override;
    int write_all(const char *buf, int buflen) override;
};
