#include "m3u.hpp"

#include <lfsaccess.hpp>
#include <config.hpp>
#include <static.hpp>

// used for read() operation
#include <listm3u.hpp>

// needed because some function require reference to
// bool <abort> or <error>, and we never abort/error out
static const bool FALSE = false;

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

int get(const char* path, const char* name, char* url, int url_max_len) {
    int r;
    acc.begin(path);

    r = acc.open_r();
    if (r < 0)
        return r;

    // this operation abuses ListM3U to read m3u file entry-by-entry (entries_len = 1)
    // each reading requires call to <begin> and <consume_all> (which bails out after maxing out entries)
    List* list = &listm3u;

    // entry buffer (only of length 1)
    ListEntry ent{};
    bool found = false;
    bool error = false;

    while (acc.more_content()) {
        // this resets consumed entry counter to 0
        list->begin(&ent, 1);
        // this reads one m3u entry
        r = list->consume_all(&acc, FALSE, FALSE);
        if (r < 0) {
            error = true;
            goto end;
        }

        // now ent is some entry read from file
        if (strcmp(name, ent.get_name()) == 0) {
            // match found, copy name and return 0
            strncpy(url, ent.get_url(), url_max_len);
            found = true;
            goto end;
        }
    }

end:
    r = acc.close();
    if (r < 0)
        return r;

    if (error || !found)
        return -1;

    // no error and found
    return 0;
}

} // namespace fav