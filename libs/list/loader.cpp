#include "loader.hpp"

#include <cstdio>
#include <config.hpp>

#include <FreeRTOS.h>
#include <task.h>

void ll_task(void* arg) {
    auto ld = (Loader*) arg;
    ld->task();
    ld->in_progress = false;

    printf("list loader unused stack: %ld\n", uxTaskGetStackHighWaterMark(nullptr));
    vTaskDelete(nullptr);
}

void Loader::begin() {
    all_loaded_cb = nullptr;
}

void Loader::call_all_loaded(int errored) {
    if (all_loaded_cb)
        all_loaded_cb(cb_arg, errored);
}

void Loader::load(int page_) {
    page = page_;
    should_abort = false;
    in_progress = true;

    entries_offset = 0;

    xTaskCreate(ll_task,
                "ll",
                STACK_LIST_LOADER,
                this,
                PRI_LIST_LOADER,
                nullptr);
}

void Loader::load_abort() {
    should_abort = true;
}

int Loader::get_page_count() {
    int cnt = get_entry_count_whole();
    if (cnt < 0)
        return cnt;

    int pages = cnt / entries_max;
    if (cnt % entries_max)
        // remainder left
        pages++;

    return pages;
}
