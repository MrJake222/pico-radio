#pragma once

#include <lfsaccess.hpp>
#include <lfsorter.hpp>
#include <ff.h>

#define PATH_LEN   (FATFS_MAX_PATH_LEN)

class SDScan {

    const char* path;

    // FatFS
    FRESULT res;
    DIR dir;
    FILINFO fileinfo;

    bool fatfs_is_valid();
    int fatfs_list_dir(flagref_t should_abort, const char* path);

    LfsAccess& acc;

    // used in <prepend_path> to prepend the path of the entry
    // gets returned by it, so use with caution
    char buf[PATH_LEN];

public:
    SDScan(LfsAccess& acc_)
        : acc(acc_)
    { }

    void begin(const char* path_);

    static const char* format_encode_dir(bool is_dir);
    static const char* format_decode_path(const char* buf);
    static bool format_decode_is_dir(const char* buf);

    // read directory listing and save them as LFS file
    // each line contains one file/dir name prepended with
    // 0 if it's a dir (sorted on top) or 1 for a file (sorted after folders)
    int scan(flagref_t should_abort, const char* path);

    // skips k first files/dirs and returns n top in a callback
    // returns strings in format, use decoding functions
    // this function is not async, after it returns all entries had been loaded and passed to callback
    int read(flagref_t should_abort, int n, int k, void* cb_arg, lfsorter::res_cb_fn cb);

    // count the entries
    int count();

    void abort_wait();

    const char* prepend_path(const char* filepath);
};