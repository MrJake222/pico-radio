#pragma once

#include <ff.h>
#include <lfsscan.hpp>

class SDScan : public LfsScan {

    const char* fatfs_path;

    // FatFS
    FRESULT res;
    DIR dir;
    FILINFO fileinfo;

    bool fatfs_is_valid();

    // used in <prepend_path> to prepend the path of the entry
    // gets returned by it, so use with caution
    char buf[FATFS_MAX_PATH_LEN];

    // read directory listing and save them as LFS file
    // each line contains one file/dir name prepended with
    // 0 if it's a dir (sorted on top) or 1 for a file (sorted after folders)
    int scan_internal(flagref_t should_abort) override;

public:
    using LfsScan::LfsScan;

    void begin(const char* fatfs_path_);

    static const char* format_encode_dir(bool is_dir);
    static const char* format_decode_path(const char* buf);
    static bool format_decode_is_dir(const char* buf);

    const char* prepend_path(const char* filepath);
};