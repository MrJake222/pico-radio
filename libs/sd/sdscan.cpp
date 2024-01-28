#include "sdscan.hpp"

#include <ff.h>
#include <filetype.hpp>

const char* sdscan_lfs_path = ".tmp_local";

void SDScan::begin(const char* fatfs_path_) {
    LfsScan::begin(sdscan_lfs_path, strcasecmp);
    fatfs_path = fatfs_path_;
}

const char* SDScan::format_encode_dir(bool is_dir) {
    return is_dir ? "0" : "1";
}

const char* SDScan::format_decode_path(const char* buf) {
    return buf + 1;
}

bool SDScan::format_decode_is_dir(const char* buf) {
    return buf[0] == '0';
}

bool SDScan::fatfs_is_valid() {
    if (fileinfo.fname[0] == '.')
        // hidden file/dir
        return false;

    bool is_dir = fileinfo.fattrib & AM_DIR;
    if (!is_dir && filetype_from_name(fileinfo.fname) == FileType::UNSUPPORTED) {
        // file not supported
        return false;
    }

    return true;
}

int SDScan::scan_internal(flagref_t should_abort) {
    int r;
    bool errored = false;

    res = f_opendir(&dir, fatfs_path);
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

        if (!fatfs_is_valid())
            continue;

        r = write(2,
                  format_encode_dir(fileinfo.fattrib & AM_DIR),
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

const char* SDScan::prepend_path(const char* filepath) {
    // same as strcpy but returns end pointer
    char* end = stpcpy(buf, fatfs_path);
    // end points to null terminator

    const int buf_left = FATFS_MAX_PATH_LEN - (end - buf);
    strncpy(end, filepath, buf_left);

    return buf;
}
