#include "lfsreader.hpp"

void LfsReader::begin(const char* path_) {
    strcpy(path, path_);
}

int LfsReader::open() {
    // read-only, create file if it doesn't exist (empty list)
    int r;

    r = lfs_file_open(lfs, &file, path, LFS_O_RDONLY | LFS_O_CREAT);
    if (r < 0) {
        printf("littlefs: failed to open code %d\n", r);
        return r;
    }

    bytes_read = 0;

    return 0;
}

int LfsReader::close() {
    int r;

    r = lfs_file_close(lfs, &file);
    if (r < 0) {
        printf("littlefs: failed to close code %d\n", r);
        return r;
    }

    return 0;
}

int LfsReader::read_char(char* chr) {
    int r = lfs_file_read(lfs, &file, chr, 1);
    if (r < 0) {
        printf("littlefs: failed to read in read_char code %d\n", r);
        return r;
    }

    bytes_read += 1;

    return 0;
}

bool LfsReader::more_content() {
    return bytes_read < lfs_file_size(lfs, &file);
}
