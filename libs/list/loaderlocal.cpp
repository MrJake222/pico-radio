#include "loaderlocal.hpp"

#include <cstring>
#include <filetype.hpp>

void LoaderLocal::begin(const char* path_) {
    strncpy(path, path_, FATFS_MAX_PATH_LEN);
}

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

        if (!is_valid())
            continue;

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
    char* end = stpcpy(buf, path);
    // end points to null terminator

    const int buf_left = BUF_LEN - (end - buf);

    strncpy(end, entries[i].get_url(), buf_left);
    entries[i].set_url(buf);
    entries[i].dir_added = true;

    return 0;
}

char* LoaderLocal::path_get_last_entered() {
    int slash_seen = 0;
    char* end;

    // iterate from end to begin
    // return pointer to second-last slash
    for (end = path+strlen(path); end != path; end--) {
        if (*end == '/')
            slash_seen++;

        // trailing + second-last
        if (slash_seen == 2)
            break;
    }

    // either because of break (second-last slash)
    // or end == start (top-level)
    return end;
}

int LoaderLocal::go(const char* dirpath) {
    if (strlen(path) + strlen(dirpath) + 1 >= PATH_LEN)
        // buffer will overflow
        return -1;

    char* end = strrchr(path, '/');
    if (!end)
        // no separator found
        return -1;

    // skip separator
    end += 1;

    // same as strcpy but returns end pointer
    end = stpcpy(end, dirpath);
    // add trailing slash
    *end++ = '/';
    *end++ = '\0';

    return 0;
}

int LoaderLocal::up() {
    if (strcmp(path, "/") == 0)
        // top-level directory
        return -1;

    // not top-level, will return second-last slash pointer
    char* slash = path_get_last_entered();

    // cut off at last dividing part (not counting trailing)
    slash++;       // skip slash
    *slash = '\0'; // trim right after

    return 0;
}