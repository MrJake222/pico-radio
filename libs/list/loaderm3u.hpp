#pragma once

#include <loader.hpp>
#include <lfsaccess.hpp>

typedef void(*ld_fav_update_cb)(void* arg, const char* info);

// This class loads M3U format
// loads EXTINF=ignored,<name> and the following line as <url>
class LoaderM3U : public Loader {

    const char* const path;

    LfsAccess& acc;

    ld_fav_update_cb upd_cb;
    void update_cb(const char* info);

    int get_entry_count_whole() override;

protected:
    // to be called from task()
    // task() should call <set_next_entry> and <call_all_loaded> after this
    // returns how many entries were loaded, or -1 on error
    int load_m3u();

public:
    LoaderM3U(ListEntry* entries_, int entries_max_,
              LfsAccess& acc_, const char* path_)
        : Loader(entries_, entries_max_)
        , acc(acc_)
        , path(path_)
        { }

    void set_update_cb(ld_fav_update_cb cb) { upd_cb = cb; }
};
