#include "radiofav.hpp"

#include <listm3u.hpp>
#include <static.hpp>

void RadioFav::task() {

    // load favourites from lfs
    int r;
    int errored = 0;
    bool error = false;
    List* list = &listm3u;

    r = lfsa.open_read_create(PATH_FAVOURITES);
    if (r < 0) {
        errored++;
        goto end;
    }

    // size = lfs_file_size(get_lfs(), &file);
    list->begin(stations, stations_count);
    r = list->consume_all(&lfsa, should_abort, error);
    if (r < 0) {
        // failed
        errored++;
        goto end;
    }

    stations_offset += list->get_stations_found();
    printf("done loading all favourites, loaded %d stations, error %d\n", stations_offset, errored);

end:
    lfsa.close();

    if (!should_abort)
        call_all_loaded(errored);

    printf("radiofav unused stack: %ld\n", uxTaskGetStackHighWaterMark(nullptr));
    vTaskDelete(nullptr);
}
