#pragma once

#include <config.hpp>
#include <datasource.hpp>
#include <lfs.h>

class LfsAccess : public DataSource {

    static const int RC_CACHE_MAX_SIZE = LITTLEFS_CACHES;

    lfs_t* lfs;
    lfs_file_t file;
    lfs_file_config file_cfg;
    char file_cache[LITTLEFS_CACHES];

    char path[LFS_NAME_MAX];

    int bytes_read;
    bool is_open_;

    int open(int flags);

    char rc_cache[RC_CACHE_MAX_SIZE];
    // this is separate from RC_CACHE_MAX_SIZE because near the end of file
    // cache can't be topped up to RC_CACHE_MAX_SIZE
    // this stays the same on reads (kind of like an end pointer)
    int rc_cache_size;
    // index into the cache buffer
    // this changes on reads and is set to 0 on writes to cache
    int rc_cache_index;
    inline void rc_cache_clear() { rc_cache_size = 0; rc_cache_index = 0; }
    inline int rc_cache_left() { return rc_cache_size - rc_cache_index; }

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

    // raw access to lfs,
    // but keeps track of cache and <bytes_read>
    int tell();
    int seek(int off, int whence);
    int read_raw(char* buf, int buflen);

    // writing interface
    // doesn't use cache or <bytes_read>
    int write_str(const char* str);
    inline int truncate(int size) { return lfs_file_truncate(lfs, &file, size); }
    inline int write_raw(const char* buf, int buflen) { return lfs_file_write(lfs, &file, buf, buflen); }
};
