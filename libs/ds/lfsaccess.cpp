#include "lfsaccess.hpp"

int LfsAccess::open(const char* path, int flags) {
    // read-only, create file if it doesn't exist (empty list)
    int r;

    r = lfs_file_open(lfs, &file, path, flags);
    if (r < 0) {
        printf("littlefs: failed to open code %d\n", r);
        return r;
    }

    bytes_read = 0;

    return 0;
}

int LfsAccess::open_read_create(const char* path) {
    bytes_read = 0;

    // read-only, create file if it doesn't exist (empty list)
    return open(path, LFS_O_RDONLY | LFS_O_CREAT);
}

int LfsAccess::open_write_trunc(const char* path) {
    // write-only, start at the beginning
    return open(path, LFS_O_WRONLY | LFS_O_TRUNC);
}

int LfsAccess::close() {
    int r;

    r = lfs_file_close(lfs, &file);
    if (r < 0) {
        printf("littlefs: failed to close code %d\n", r);
        return r;
    }

    return 0;
}

int LfsAccess::read_char(char* chr) {
    int r = lfs_file_read(lfs, &file, chr, 1);
    if (r < 0) {
        printf("littlefs: failed to read in read_char code %d\n", r);
        return r;
    }

    bytes_read += 1;

    return 0;
}

bool LfsAccess::more_content() {
    return bytes_read < lfs_file_size(lfs, &file);
}

int LfsAccess::write_all(const char* buf, int buflen) {
    int r = lfs_file_write(lfs, &file, buf, buflen);
    if (r < 0) {
        printf("littlefs: failed to write in write_all code %d\n", r);
        return r;
    }

    return 0;
}
