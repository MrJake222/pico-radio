#include "lfsutil.hpp"

namespace lfsutil {

int skip_line(lfs_t* lfs, lfs_file_t* file) {
    int ret;
    char chr;

    do {
        ret = lfs_file_read(lfs, file, &chr, 1);
        if (ret < 0)
            return ret;

        // read until \n is found (\n or \r\n endings)
    } while (chr != '\n');

    return 0;
}

int skip_lines(lfs_t* lfs, lfs_file_t* file, int n) {
    while (n--) {
        int r = skip_line(lfs, file);
        if (r < 0)
            return r;
    }

    return 0;
}

} // namespace lfsutil