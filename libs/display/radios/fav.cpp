#include "fav.hpp"

#include <lfs.h>
#include <lfsutil.hpp>
#include <config.hpp>

namespace fav {

int create(lfs_t* lfs) {
    int r;
    lfs_file_t file;

    r = lfs_file_open(lfs, &file, PATH_FAVOURITES, LFS_O_WRONLY | LFS_O_CREAT);
    if (r < 0)
        return r;

    r = lfs_file_write(lfs, &file, "#EXTM3U\n", strlen("#EXTM3U\n"));
    if (r < 0)
        return r;

    r = lfs_file_close(lfs, &file);
    if (r < 0)
        return r;

    return 0;
}

int add(lfs_t* lfs, const struct station* st) {
    int r;
    lfs_file_t file;
    char buf[LIST_MAX_LINE_LENGTH];

    r = lfs_file_open(lfs, &file, PATH_FAVOURITES, LFS_O_RDWR);
    if (r < 0)
        return r;

    const int lines = lfsutil::skip_all_lines(lfs, &file);
    const int entry_idx = (lines - 1) / 2;

    sprintf(buf, "#EXTINF:-1,%s\n", st->name);
    r = lfs_file_write(lfs, &file, buf, strlen(buf));
    if (r < 0)
        return r;

    sprintf(buf, "%s\n", st->url);
    r = lfs_file_write(lfs, &file, buf, strlen(buf));
    if (r < 0)
        return r;

    r = lfs_file_close(lfs, &file);
    if (r < 0)
        return r;

    return entry_idx;
}

int remove(lfs_t* lfs, int index) {
    int r;
    lfs_file_t file;
    char buf[LIST_MAX_LINE_LENGTH];

    r = lfs_file_open(lfs, &file, PATH_FAVOURITES, LFS_O_RDWR);
    if (r < 0)
        return r;

    // skip preamble, and <index> entries
    lfsutil::skip_lines(lfs, &file, 1 + 2*index);

    // file is now set on first character of the station to be deleted
    const int pos = lfs_file_tell(lfs, &file);
    if (pos < 0)
        return pos;

    // skip 2 more lines (skip over the entry to be deleted)
    lfsutil::skip_lines(lfs, &file, 2);
    const int pos_after = lfs_file_tell(lfs, &file);
    if (pos_after < 0)
        return pos_after;

    const int skip = pos_after - pos;

    int read;
    while (true) {
        // read at new position
        read = lfs_file_read(lfs, &file, buf, LIST_MAX_LINE_LENGTH);
        if (read < 0)
            return read;

        // rewind to old position
        lfs_file_seek(lfs, &file, -(read + skip), LFS_SEEK_CUR);

        int write = lfs_file_write(lfs, &file, buf, read);
        if (write < 0)
            return write;

        assert(read == write);

        // break here not to skip
        if (read < LIST_MAX_LINE_LENGTH)
            break;

        // skip to new position
        lfs_file_seek(lfs, &file, skip, LFS_SEEK_CUR);
    }

    // truncate file to current position
    lfs_file_truncate(lfs, &file, lfs_file_tell(lfs, &file));

    r = lfs_file_close(lfs, &file);
    if (r < 0)
        return r;

    return 0;
}

} // namespace fav