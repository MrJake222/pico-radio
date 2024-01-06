#include "lfsaccess.hpp"

#include <pico/platform.h> // for min

void LfsAccess::begin(const char* path_) {
    strcpy(path, path_);
    is_open_ = false;
}

int LfsAccess::open(int flags) {
    int r;

    memset(&file_cfg, 0, sizeof(file_cfg));
    file_cfg.buffer = &file_cache;

    r = lfs_file_opencfg(lfs, &file, path, flags, &file_cfg);
    if (r < 0) {
        printf("littlefs: failed to open code %d\n", r);
        return r;
    }

    bytes_read = 0;
    is_open_ = true;
    rc_cache_clear();

    return 0;
}

int LfsAccess::close() {
    int r;

    r = lfs_file_close(lfs, &file);
    if (r < 0) {
        printf("littlefs: failed to close code %d\n", r);
        return r;
    }

    is_open_ = false;

    return 0;
}

int LfsAccess::read_char(char* chr) {

retry:
    if (rc_cache_left()) {
        *chr = rc_cache[rc_cache_index++];
        bytes_read += 1;
        return 0;
    }

    int r = lfs_file_read(lfs, &file, rc_cache, RC_CACHE_MAX_SIZE);
    if (r < 0) {
        printf("littlefs: failed to read in read_char code %d\n", r);
        return r;
    }
    if (r == 0) {
        puts("littlefs: eof in read_char (shouldn't happen, check <more_content> before calling this)");
        assert(false);
        return -1;
    }

    rc_cache_size = r;
    rc_cache_index = 0;
    goto retry;
}

bool LfsAccess::more_content() {
    // <bytes_read> is used to detect EOF
    // (and cannot be abandoned, it's the only method http server (other subclass) can tell EOF)
    return bytes_read < size();
}

int LfsAccess::skip_line() {
    // <read_line> uses <read_char> which updates <bytes_read> (DataSource interface)
    int line_length;
    int r = read_line(this, nullptr, 0, &line_length);
    if (r == RL_ERROR)
        return -1;

    return line_length;
}

int LfsAccess::skip_lines(int n) {
    int line_lengths = 0;

    while (n--) {
        int r = skip_line();
        if (r < 0)
            return r;

        line_lengths += r;
    }

    return line_lengths;
}

int LfsAccess::skip_all_lines() {
    int n = 0;

    while (more_content()) {
        int r = skip_line();
        if (r < 0)
            return r;

        n += 1;
    }

    return n;
}

int LfsAccess::tell() {
    int pos = lfs_file_tell(lfs, &file);

    if (pos >= 0) {
        // update only if no error
        // <pos> is position where cached read ended
        // retract <pos> by the count of bytes left to be read from cache
        pos -= rc_cache_left();
    }

    return pos;
}

int LfsAccess::seek(int off, int whence) {
    if (whence == LFS_SEEK_CUR) {
        // seek relative to current pointer
        // but lfs read ptr is off by rc_cache_left()

        // true formula is
        // off = -(rc_cache_left() - off)
        // so in case of
        //    forward skip (off>0): file needs to be rewinded by number of bytes in cache (already read) minus the forward skip
        //   backward skip (off<0): file needs to be rewinded by number of bytes in cache (already read) plus the backward skip
        off -= rc_cache_left();
    }

    int pos = lfs_file_seek(lfs, &file, off, whence);

    if (pos >= 0) {
        // update only if no error
        bytes_read = pos;
        rc_cache_clear();
    }

    return pos;
}

// Tried refactoring this one to use caching
// It caused issues with write pointer
// stashed on npc as "lfsaccess read_raw & seek cache support"
int LfsAccess::read_raw(char* buf, int buflen) {
    // first read what's left from cache
    const int read_cache = MIN(rc_cache_left(), buflen);
    memcpy(buf, rc_cache + rc_cache_index, read_cache);
    rc_cache_index += read_cache;
    bytes_read += read_cache;

    if (read_cache == buflen)
        return read_cache;

    const int read_lfs = lfs_file_read(
            lfs, &file,
            buf + read_cache,
            buflen - read_cache);

    if (read_lfs < 0)
        return read_lfs; // return error code

    bytes_read += read_lfs;
    return read_cache + read_lfs;
}

int LfsAccess::write_str(const char* str) {
    int r = lfs_file_write(lfs, &file, str, strlen(str));
    if (r < 0) {
        printf("littlefs: failed to write in write_str code %d\n", r);
        return r;
    }

    assert(r == strlen(str));

    return 0;
}