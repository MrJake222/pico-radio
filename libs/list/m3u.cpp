#include "m3u.hpp"

#include <lfsaccess.hpp>
#include <config.hpp>
#include <static.hpp>

namespace m3u {

static LfsAccess acc(get_lfs());

int create(const char* path) {
    int r;
    acc.begin(path);

    r = acc.open_w_create();
    if (r < 0)
        return r;

    r = acc.write_str("#EXTM3U\n");
    if (r < 0)
        return r;

    r = acc.close();
    if (r < 0)
        return r;

    return 0;
}

int add(const char* path, const ListEntry* entry) {
    int r;
    char buf[LIST_MAX_LINE_LENGTH];
    acc.begin(path);

    r = acc.open_rw();
    if (r < 0)
        return r;

    const int lines = acc.skip_all_lines();
    const int entry_idx = (lines - 1) / 2;

    sprintf(buf, "#EXTINF:-1,%s\n", entry->get_name());
    r = acc.write_str(buf);
    if (r < 0)
        return r;

    sprintf(buf, "%s\n", entry->get_url());
    r = acc.write_str(buf);
    if (r < 0)
        return r;

    r = acc.close();
    if (r < 0)
        return r;

    return entry_idx;
}

int remove(const char* path, int index) {
    int r;
    char buf[LIST_MAX_LINE_LENGTH];
    acc.begin(path);

    r = acc.open_rw();
    if (r < 0)
        return r;

    // skip preamble, and <index> entries
    acc.skip_lines(1 + 2*index);

    // file is now set on first character of the station to be deleted
    const int pos = acc.tell();
    if (pos < 0)
        return pos;

    // skip 2 more lines (skip over the entry to be deleted)
    acc.skip_lines(2);
    const int pos_after = acc.tell();
    if (pos_after < 0)
        return pos_after;

    const int skip = pos_after - pos;

    int read;
    while (true) {
        // read at new position
        read = acc.read_raw(buf, LIST_MAX_LINE_LENGTH);
        if (read < 0)
            return read;

        // rewind to old position
        acc.seek(-(read + skip), LFS_SEEK_CUR);

        int write = acc.write_raw(buf, read);
        if (write < 0)
            return write;

        assert(read == write);

        // break here not to skip
        if (read < LIST_MAX_LINE_LENGTH)
            break;

        // skip to new position
        acc.seek(skip, LFS_SEEK_CUR);
    }

    // truncate file to current position
    acc.truncate(acc.tell());

    r = acc.close();
    if (r < 0)
        return r;

    return 0;
}

} // namespace fav