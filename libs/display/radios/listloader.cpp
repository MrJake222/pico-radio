#include "listloader.hpp"

#include <config.hpp>

#include <FreeRTOS.h>
#include <task.h>

void ll_task(void* arg) {
    ((ListLoader*) arg)->task();
}

void ListLoader::begin() {
    should_abort = false;

    stations_offset = 0;
    all_loaded_cb = nullptr;
}

void ListLoader::call_all_loaded(int errored) {
    if (all_loaded_cb)
        all_loaded_cb(cb_arg, errored);
}

void ListLoader::set_all_loaded_cb(void* arg, all_ld_cb_fn cb) {
    cb_arg = arg;
    all_loaded_cb = cb;
}

void ListLoader::load_stations() {
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
