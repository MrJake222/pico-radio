#include "loaderfav.hpp"

#include <listm3u.hpp>
#include <static.hpp>
#include <fav.hpp>
#include <lfsutil.hpp>

void LoaderFav::update(const char* info) {
    if (upd_cb)
        upd_cb(get_cb_arg(), info);
}

void LoaderFav::task() {

    // load favourites from lfs
    int r;
    int errored = 0;
    bool error = false;
    List* list = &listm3u;

    update("Przygotowanie");

retry:
    rd.begin(PATH_FAVOURITES);
    r = rd.open();
    if (r < 0) {
        if (r == LFS_ERR_NOENT) {
            // does not exist
            puts("creating new favourites file");
            update("Tworzenie");
            fav::create(lfs);
            goto retry;
        }

        errored++;
        goto end;
    }

    update("Ładowanie");

    // pagination support
    // skip lines: 1 (header) + 2*page*per_page
    rd.skip_lines(1 + 2*page*MAX_STATIONS);

    list->begin(stations, stations_max);
    r = list->consume_all(&rd, should_abort, error);
    if (r < 0) {
        // failed
        errored++;
        goto end;
    }

    stations_offset += list->get_stations_found();
    printf("done loading all favourites, loaded %d stations, error %d\n", stations_offset, errored);
    update("Gotowe");

end:
    rd.close();

    if (!should_abort)
        call_all_loaded(errored);

    printf("radiofav unused stack: %ld\n", uxTaskGetStackHighWaterMark(nullptr));
    vTaskDelete(nullptr);
}

int LoaderFav::get_page_count() {
    int r;
    lfs_file_t file;

    // open read-only
    r = lfs_file_open(lfs, &file, PATH_FAVOURITES, LFS_O_RDONLY);
    if (r < 0) {
        printf("littlefs: failed to open code %d\n", r);
        return r;
    }

    const int lines = lfsutil::skip_all_lines(lfs, &file);

    r = lfs_file_close(lfs, &file);
    if (r < 0) {
        printf("littlefs: failed to close code %d\n", r);
        return r;
    }

    const int entries = (lines - 1) / 2;

    int pages = entries / MAX_STATIONS;
    if (entries % MAX_STATIONS)
        // remainder left
        pages++;

    return pages;
}
