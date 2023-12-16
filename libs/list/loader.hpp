#pragma once

#include <listentry.hpp>

typedef void(*all_ld_cb_fn)(void* arg, int errored);

class Loader {

    void* cb_arg;
    all_ld_cb_fn all_loaded_cb;

    // wrapper to call task() from rtos
    friend void ll_task(void* arg);
    // real task, should respect <should_abort>, <page>
    // must run <call_all_loaded> when finished
    virtual void task() = 0;

    // this is made private to force setting
    // ListEntry::type via <set_next_entry>
    int entries_offset;
    // called from <set_next_entry> to set any necessary info
    // for ex. type of entry
    virtual void setup_entry(ListEntry* ent) = 0;

protected:
    void* get_cb_arg() { return cb_arg; }
    // when called with no callback or when aborted, does nothing
    void call_all_loaded(int errored);

    int page;
    volatile bool should_abort;
    volatile bool in_progress;

    ListEntry* const entries;
    const int entries_max;
    int get_entries_offset() { return entries_offset; }
    void set_next_entry(int skip);
    // helper functions
    ListEntry* get_current_entry() { return &entries[entries_offset]; }
    bool can_fit_more_entries() { return entries_offset < entries_max; }

    // certain loaders may use lfs cache
    // this flag is set by screens on page switches (for example)
    bool can_use_cache;

    // get how many entries are there (don't care about pagination)
    // -1 is infinite pages (or failed to determine)
    // it gets called only once in <get_page_count> from <all_loaded_cb>
    // can assume the <task> has completed
    virtual int get_entry_count_whole() = 0;

public:
    Loader(ListEntry* entries_, int entries_max_)
        : entries(entries_)
        , entries_max(entries_max_)
        { }

    void begin();

    void use_cache(bool cache) { can_use_cache = cache; }

    // set callback to call when all loading is done
    void set_cb_arg(void* arg) { cb_arg = arg; }
    void set_all_loaded_cb(all_ld_cb_fn cb) { all_loaded_cb = cb; }

    // start loading
    void load(int page_);
    // stop loading
    virtual void load_abort();

    int get_entry_count() { return entries_offset; }
    int get_entry_max_count() { return entries_max; }
    ListEntry* get_entry(int i) { return &entries[i]; }
    // can be overridden to handle special url updates
    // such as *.pls format or recursive path updates
    virtual int check_entry_url(int i) { return 0; }

    // should return max pages to switch to (-1 for infinite pages)
    // must be called in all_loaded callback
    // warning: can be resource hungry
    int get_page_count();

    bool is_in_progress() { return in_progress; }
};
