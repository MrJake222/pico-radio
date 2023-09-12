#include "loaderlocal.hpp"

#include <cstring>

void LoaderLocal::begin(const char* path_) {
    strncpy(path, path_, FATFS_MAX_PATH_LEN);
}
// TODO pagination
void LoaderLocal::task() {
    FRESULT res;
    int errored = 0;

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
    entry->is_dir = is_dir;

    entries_offset++;
}

int LoaderLocal::get_page_count() {
    return 1;
}
