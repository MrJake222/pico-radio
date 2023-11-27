#pragma once

#include <config.hpp>
#include <datasource.hpp>
#include <lfs.h>

class LfsAccess : public DataSource {

    lfs_t* lfs;
    lfs_file_t file;
    lfs_file_config file_cfg;
    char file_buf[LITTLEFS_CACHES];

    char path[LFS_NAME_MAX];

    int bytes_read;

    int open(int flags);

public:
    LfsAccess(lfs_t* lfs_)
        : lfs(lfs_)
        { }

    void begin(const char* path_);
    int open_r()  { return open(LFS_O_RDONLY); }
    int open_w()  { return open(LFS_O_WRONLY); }
    int open_rw() { return open(LFS_O_RDWR); }
    int open_w_create() { return open(LFS_O_WRONLY | LFS_O_CREAT); }
    int close();

    // DataSource interface
    int read_char(char *chr) override;
    bool more_content() override;

    // skipping lines interface
    // returns line length without termination or -1 on failure
    int skip_line();
    // returns sum of skipped line lengths or -1 on failure
    int skip_lines(int n);
    // returns number of skipped lines or -1 on failure
    int skip_all_lines();

    // writing interface
    int write_str(const char* str);

    // passthrough interface (raw access to lfs)
    // file pointer
    inline int tell() { return lfs_file_tell(lfs, &file); }
    inline int seek(int off, int whence) { return lfs_file_seek(lfs, &file, off, whence); }
    inline int truncate(int size) { return lfs_file_truncate(lfs, &file, size); }
    // read/write
    inline int read_raw(char* buf, int buflen) { return lfs_file_read(lfs, &file, buf, buflen); }
    inline int write_raw(const char* buf, int buflen) { return lfs_file_write(lfs, &file, buf, buflen); }
};
