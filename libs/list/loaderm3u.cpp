#include "loaderm3u.hpp"

#include <listm3u.hpp>
#include <m3u.hpp>

void LoaderM3U::begin(const char* path_) {
    Loader::begin();
    path = path_;
}

void LoaderM3U::update(const char* info) {
    if (upd_cb)
        upd_cb(get_cb_arg(), info);
}

void LoaderM3U::task() {

    // load favourites from lfs
    int r;
    int errored = 0;
    bool error = false;
    List* list = &listm3u;

    update("Przygotowanie");

retry:
    rd.begin(path);
    r = rd.open_r();
    if (r < 0) {
        if (r == LFS_ERR_NOENT) {
            // does not exist
            puts("creating new favourites file");
            update("Tworzenie");
            m3u::create(path);
            goto retry;
        }

        errored++;
        goto end_noclose;
    }

    update("Ładowanie");

    // pagination support
    // skip lines: 1 (header) + 2*page*per_page
    rd.skip_lines(1 + 2 * page * MAX_ENTRIES);

    list->begin(entries + entries_offset,
                entries_max - entries_offset);
    r = list->consume_all(&rd, should_abort, error);
    if (r < 0) {
        // failed
        errored++;
        goto end;
    }

    entries_offset += list->get_entries_found();
    printf("done loading all favourites, loaded %d stations, error %d\n", entries_offset, errored);
    update("Gotowe");

end:
    rd.close();

end_noclose:
    call_all_loaded(errored);
}

int LoaderM3U::get_entry_count_whole() {
    int r;
    rd.begin(path);

    r = rd.open_r();
    if (r < 0)
        return r;

    const int lines = rd.skip_all_lines();

    r = rd.close();
    if (r < 0)
        return r;

    return (lines - 1) / 2;
}
