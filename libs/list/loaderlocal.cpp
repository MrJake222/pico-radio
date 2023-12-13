#include "loaderlocal.hpp"

#include <sdscan.hpp>

void LoaderLocal::begin(Path* path_) {
    Loader::begin();
    path = path_;
}

void llocal_res_cb(void* arg, const char* res) {
    auto ll = (LoaderLocal*) arg;

    const char* name = SDScan::format_decode_name(res);
    bool is_dir = SDScan::format_decode_is_dir(res);

    ll->set_file(name, is_dir);
}

void LoaderLocal::set_file(const char* path_, bool is_dir) {
    ListEntry* entry = get_current_entry();
    entry->set_name("");
    entry->set_url(path_);
    entry->llocal.is_dir = is_dir;
    entry->llocal.has_full_path = false;

    set_next_entry(1);
}

void LoaderLocal::task() {
    int r;
    bool errored = false;

    if (!can_use_cache) {
        r = scan.scan(path->str());
        if (r) {
            errored = true;
            goto end;
        }
    }

    r = scan.read(entries_max, page * entries_max,
                  this, llocal_res_cb);

    if (r < 0) {
        errored = true;
        goto end;
    }

    end:
    call_all_loaded(errored ? 1 : 0);
}

void LoaderLocal::load_abort() {
    Loader::load_abort();
    scan.abort_wait();
}

int LoaderLocal::get_entry_count_whole() {
    return scan.count();
}

int LoaderLocal::check_entry_url(int i) {
    if (entries[i].llocal.has_full_path)
        return 0;

    // same as strcpy but returns end pointer
    char* end = stpcpy(buf, path->str());
    // end points to null terminator

    const int buf_left = BUF_LEN - (end - buf);

    strncpy(end, entries[i].get_url(), buf_left);
    entries[i].set_url(buf);
    entries[i].llocal.has_full_path = true;

    return 0;
}