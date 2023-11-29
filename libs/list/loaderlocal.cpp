#include "loaderlocal.hpp"

#include <cstring>
#include <filetype.hpp>

#include <static.hpp>
#include <lfsaccess.hpp>
#include <lfsorter.hpp>

static LfsAccess acc(get_lfs());

void LoaderLocal::begin(Path* path_) {
    Loader::begin();
    path = path_;
}

void llocal_res_cb(void* arg, const char* res) {
    auto ll = (LoaderLocal*) arg;

    const char* name = res + 1;  // skip dir indicator
    bool is_dir = res[0] == '0'; // folders were sorted first, remember?

    ll->set_file(name, is_dir);
}

int LoaderLocal::fatfs_list_dir() {
    int r;
    FRESULT res;
    bool errored = false;

    res = f_opendir(&dir, path->str());
    if (res != FR_OK) {
        errored = true;
        goto end_noclose;
    }

    while (!should_abort) {
        res = f_readdir(&dir, &fileinfo);
        if (res != FR_OK) {
            errored = true;
            goto end;
        }

        if (!fileinfo.fname[0]) {
            // empty string -> end of folder
            break;
        }

        if (!is_valid())
            continue;

        r = lfsorter::write(acc, 2,
                            fileinfo.fattrib & AM_DIR ? "0" : "1", // prepend 0 to folders to be sorted first
                            fileinfo.fname);
        if (r) {
            errored = true;
            goto end;
        }
    }

end:
    f_closedir(&dir);

end_noclose:
    return errored ? -1 : 0;
}

void LoaderLocal::task() {
    FRESULT res;
    int r;
    int errored = 0;

    if (can_use_cache) {
        // using cache
        // only open the existing file
        r = lfsorter::open(acc);
        if (r) {
            errored++;
            goto end_noclose;
        }
    }
    else {
        // can't use cache
        // need to create new file and populate it with FatFS dir listing

        r = lfsorter::open_create_truncate(acc);
        if (r) {
            errored++;
            goto end_noclose;
        }

        r = fatfs_list_dir();
        if (r) {
            errored++;
            goto end;
        }
    }

    // this automatically rewinds created file to the beginning
    lfsorter::get_smallest_n_skip_k(acc, entries_max, entries_max * page,strcasecmp,
                                    this, llocal_res_cb);

end:
    acc.close();

end_noclose:
    call_all_loaded(errored);
}

void LoaderLocal::set_file(const char* path_, bool is_dir) {
    ListEntry* entry = &entries[entries_offset];
    entry->set_name("");
    entry->set_url(path_);
    entry->type = is_dir ? ListEntry::le_type_dir
                         : ListEntry::le_type_local;
    entry->dir_added = false;

    entries_offset++;
}

int LoaderLocal::get_entry_count_whole() {
    FRESULT res;

    res = f_opendir(&dir, path->str());
    if (res != FR_OK) {
        return -1;
    }

    int files_dirs = 0;

    while (!should_abort) {
        res = f_readdir(&dir, &fileinfo);
        if (res != FR_OK) {
            f_closedir(&dir);
            return -1;
        }

        if (!fileinfo.fname[0]) {
            // empty string -> end of folder
            break;
        }

        if (!is_valid())
            continue;

        files_dirs++;
    }

    f_closedir(&dir);
    return files_dirs;
}

bool LoaderLocal::is_valid() {
    if (fileinfo.fname[0] == '.')
        // hidden file/dir
        return false;

    bool is_dir = fileinfo.fattrib & AM_DIR;
    if (!is_dir && filetype_from_name(fileinfo.fname) == FileType::UNSUPPORTED) {
        // file not supporteed
        return false;
    }

    return true;
}

int LoaderLocal::check_entry_url(int i) {
    if (entries[i].dir_added)
        return 0;

    // same as strcpy but returns end pointer
    char* end = stpcpy(buf, path->str());
    // end points to null terminator

    const int buf_left = BUF_LEN - (end - buf);

    strncpy(end, entries[i].get_url(), buf_left);
    entries[i].set_url(buf);
    entries[i].dir_added = true;

    return 0;
}