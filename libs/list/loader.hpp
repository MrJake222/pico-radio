#pragma once

#include <listentry.hpp>

typedef void(*all_ld_cb_fn)(void* arg, int errored);

class Loader {

    void* cb_arg;
    all_ld_cb_fn all_loaded_cb;

    // wrapper to call task() from rtos
    friend void ll_task(void* arg);
    // real task, should respect <should_abort>
    virtual void task() = 0;

protected:
    void* get_cb_arg() { return cb_arg; }
    void call_all_loaded(int errored);

    volatile bool should_abort;

    ListEntry* const entries;
    const int entries_max;
    int entries_offset;

    int page;

    // get how many entries are there (don't care about pagination)
    // -1 is infinite pages (or failed to determine)
    virtual int get_entry_count_whole() = 0;

public:
    Loader(ListEntry* entries_, int entries_max_)
        : entries(entries_)
        , entries_max(entries_max_)
        { }

    void begin();

    // set callback to call when all loading is done
    void set_cb_arg(void* arg) { cb_arg = arg; }
    void set_all_loaded_cb(all_ld_cb_fn cb) { all_loaded_cb = cb; }

    // start loading
    virtual void load(int page_);
    // stop loading
    virtual void load_abort();

    int get_entry_count() { return entries_offset; }
    ListEntry* get_entry(int i) { return &entries[i]; }
    // can be overridden to handle special url updates
    // such as *.pls format or recursive path updates
    virtual int check_entry_url(int i) { return 0; }

    // should return max pages to switch to
    // return 1 to disable, -1 to infinite pages
    // must be called in all_loaded callback
    // warning: can be resource hungry
    int get_page_count();
};
