#include "loaderfav.hpp"

#include <listm3u.hpp>
#include <static.hpp>
#include <fav.hpp>

void LoaderFav::task() {

    // load favourites from lfs
    int r;
    int errored = 0;
    bool error = false;
    List* list = &listm3u;

retry:
    rd.begin(PATH_FAVOURITES);
    r = rd.open();
    if (r < 0) {
        if (r == LFS_ERR_NOENT) {
            // does not exist
            puts("creating new favourites file");
            fav::create(lfs);
            goto retry;
        }

        errored++;
        goto end;
    }

    // size = lfs_file_size(get_lfs(), &file);
    list->begin(stations, stations_count);
    r = list->consume_all(&rd, should_abort, error);
    if (r < 0) {
        // failed
        errored++;
        goto end;
    }

    stations_offset += list->get_stations_found();
    printf("done loading all favourites, loaded %d stations, error %d\n", stations_offset, errored);

end:
    rd.close();

    if (!should_abort)
        call_all_loaded(errored);

    printf("radiofav unused stack: %ld\n", uxTaskGetStackHighWaterMark(nullptr));
    vTaskDelete(nullptr);
}
