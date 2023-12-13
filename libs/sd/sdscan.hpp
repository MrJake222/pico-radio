#pragma once

#include <lfsaccess.hpp>
#include <lfsorter.hpp>
#include <ff.h>

class SDScan {

    // FatFS
    FRESULT res;
    DIR dir;
    FILINFO fileinfo;

    bool fatfs_is_valid();
    int fatfs_list_dir(const char* path);

    LfsAccess& acc;
    flagref_t should_abort;

public:
    SDScan(LfsAccess& acc_, flagref_t should_abort_)
        : acc(acc_)
        , should_abort(should_abort_)
    { }

    static const char* format_encode_dir(bool is_dir);
    static const char* format_decode_name(const char* buf);
    static bool format_decode_is_dir(const char* buf);

    // read directory listing and save them as LFS file
    // each line contains one file/dir name prepended with
    // 0 if it's a dir (sorted on top) or 1 for a file (sorted after folders)
    int scan(const char* path);

    // skips k first files/dirs and returns n top in a callback
    // returns strings in format, use decoding functions
    // this function is not async, after it returns all entries had been loaded and passed to callback
    int read(int n, int k, void* cb_arg, lfsorter::res_cb_fn cb);

    // count the entries
    int count();

    void abort_wait();
};