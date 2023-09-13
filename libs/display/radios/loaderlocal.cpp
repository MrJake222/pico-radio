#include "loaderlocal.hpp"

#include <cstring>

void LoaderLocal::begin(const char* path_) {
    strncpy(path, path_, FATFS_MAX_PATH_LEN);
}
// TODO pagination
void LoaderLocal::task() {
    FRESULT res;
    int errored = 0;
    int to_skip = page * entries_max;

    res = f_opendir(&dir, path);
    if (res != FR_OK) {
        errored++;
        goto end_noclose;
    }

    while (!should_abort) {
        res = f_readdir(&dir, &fileinfo);
        if (res != FR_OK) {
            errored++;
            goto end;
        }

        if (!fileinfo.fname[0]) {
            // empty string -> end of folder
            break;
        }

        if (to_skip) {
            to_skip--;
            continue;
        }

        set_file(fileinfo.fname, fileinfo.fattrib & AM_DIR);
        if (entries_offset == entries_max)
            break;
    }

end:
    f_closedir(&dir);

end_noclose:
    if (!should_abort)
        call_all_loaded(errored);
}

void LoaderLocal::set_file(const char* path_, bool is_dir) {
    ListEntry* entry = &entries[entries_offset];
    entry->set_name("");
    entry->set_url(path_);
    entry->type = is_dir ? ListEntry::le_type_dir
                         : ListEntry::le_type_local;

    entries_offset++;
}

int LoaderLocal::get_entry_count_whole() {
    FRESULT res;

    res = f_opendir(&dir, path);
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

        files_dirs++;
    }

    f_closedir(&dir);
    return files_dirs;
}
