#include "listloader.hpp"

#include <config.hpp>

#include <FreeRTOS.h>
#include <task.h>

void ll_task(void* arg) {
    ((ListLoader*) arg)->task();
}

void ListLoader::begin() {
    reset();
    all_loaded_cb = nullptr;
}

void ListLoader::reset() {
    should_abort = false;
    stations_offset = 0;
}

void ListLoader::call_all_loaded(int errored) {
    if (all_loaded_cb)
        all_loaded_cb(cb_arg, errored);
}

void ListLoader::load_stations(int page_) {
    page = page_;

    xTaskCreate(ll_task,
                "ll",
                STACK_LIST_LOADER,
                this,
                PRI_LIST_LOADER,
                nullptr);
}

void ListLoader::load_abort() {
    should_abort = true;
}

void ListLoader::add_station(const struct station* st) {
    stations[stations_offset++] = *st;
}

void ListLoader::remove_station(int index) {
    for (int i=index; i<stations_offset-1; i++) {
        stations[i] = stations[i+1];
    }

    stations_offset--;
}
