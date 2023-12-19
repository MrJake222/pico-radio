#pragma once

#include <config.hpp>
#include <datasource.hpp>
#include <lfs.h>

class LfsAccess : public DataSource {

    static const int RC_CACHE_SIZE = LITTLEFS_CACHES;

    lfs_t* lfs;
    lfs_file_t file;
    lfs_file_config file_cfg;
    char file_cache[LITTLEFS_CACHES];

    char path[LFS_NAME_MAX];

    int bytes_read;
    bool is_open_;

    int open(int flags);

    char rc_cache[RC_CACHE_SIZE];
    int rc_cache_index;
    inline void rc_cache_clear() { rc_cache_index = RC_CACHE_SIZE; }
    inline int rc_cache_left() { return RC_CACHE_SIZE - rc_cache_index; }

public:
    LfsAccess(lfs_t* lfs_)
        : lfs(lfs_)
        { }

    void begin(const char* path_);
    int open_r()  { return open(LFS_O_RDONLY); }
    int open_w()  { return open(LFS_O_WRONLY); }
    int open_rw() { return open(LFS_O_RDWR); }
    int open_w_create() { return open(LFS_O_WRONLY | LFS_O_CREAT); }
    int open_rw_create_truncate() { return open(LFS_O_RDWR | LFS_O_CREAT | LFS_O_TRUNC); }
    int close();
    bool is_open() { return is_open_; }

    int size() { return lfs_file_size(lfs, &file); }

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

    // passthrough interface (raw access to lfs), but keeps <bytes_read> up-to-date
    // file pointer
    int tell();
    int seek(int off, int whence);
    inline int truncate(int size) { return lfs_file_truncate(lfs, &file, size); }
    // read/write
    int read_raw(char* buf, int buflen);
    inline int write_raw(const char* buf, int buflen) { return lfs_file_write(lfs, &file, buf, buflen); }
};
