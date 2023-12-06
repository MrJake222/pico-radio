#include "loaderm3u.hpp"

#include <m3u.hpp>

void LoaderM3U::update_cb(const char* info) {
    if (upd_cb)
        upd_cb(get_cb_arg(), info);
}

int LoaderM3U::load_m3u() {

    // load m3u from lfs
    int r;
    bool errored = false;
    bool error = false;

    update_cb("Przygotowanie");

retry:
    acc.begin(path);
    r = acc.open_r();
    if (r < 0) {
        if (r == LFS_ERR_NOENT) {
            // does not exist
            puts("creating new m3u file");
            update_cb("Tworzenie");
            m3u::create(path);
            goto retry;
        }

        errored = true;
        goto end_noclose;
    }

    update_cb("Ładowanie");

    // pagination support
    m3u::skip(acc, page * MAX_ENTRIES);

    list.begin(entries + get_entries_offset(),
               entries_max - get_entries_offset());
    r = list.consume_all(&acc, should_abort, error);
    if (r < 0) {
        // failed
        errored = true;
        goto end;
    }

    printf("done loading all m3u, loaded %d entries, error %d\n", get_entries_offset(), errored);
    update_cb("Gotowe");

end:
    acc.close();

end_noclose:
    return errored ? -1 : list.get_entries_found();
}

int LoaderM3U::get_entry_count_whole() {
    int r;
    acc.begin(path);

    r = acc.open_r();
    if (r < 0)
        return r;

    const int lines = acc.skip_all_lines();

    r = acc.close();
    if (r < 0)
        return r;

    return (lines - 1) / 2;
}
