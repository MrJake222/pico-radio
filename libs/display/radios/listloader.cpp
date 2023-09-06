#include "listloader.hpp"

#include <config.hpp>

#include <FreeRTOS.h>
#include <task.h>

void ll_task(void* arg) {
    ((ListLoader*) arg)->task();
}

void ListLoader::begin() {
    all_loaded_cb = nullptr;
}

void ListLoader::call_all_loaded(int errored) {
    if (all_loaded_cb)
        all_loaded_cb(cb_arg, errored);
}

void ListLoader::load_stations(int page_) {
    page = page_;
    should_abort = false;
    stations_offset = 0;

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
