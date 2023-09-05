#include "lfsreader.hpp"

void LfsReader::begin(const char* path_) {
    strcpy(path, path_);
}

int LfsReader::open() {
    // open read-only
    int r;

    r = lfs_file_open(lfs, &file, path, LFS_O_RDONLY);
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

int LfsReader::skip_lines(int n) {
    while (n--) {
        // can't use raw <lfsutil> variant here, because it won't update <bytes_read>
        // this uses <read_char> which updates <bytes_read>
        // <bytes_read> is used to detect EOF (and cannot be abandoned,
        // it's the only method http server (other subclass) can tell EOF)
        int r = read_line(this, nullptr, 0);
        if (r == RL_ERROR)
            return -1;
    }

    return 0;
}
