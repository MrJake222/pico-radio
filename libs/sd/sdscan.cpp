#include "sdscan.hpp"

#include <ff.h>
#include <filetype.hpp>

const char* lfs_path = ".tmp_local";

void SDScan::begin(const char* path_) {
    path = path_;
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

int SDScan::fatfs_list_dir(flagref_t should_abort, const char* path) {
    int r;
    bool errored = false;

    res = f_opendir(&dir, path);
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

        r = lfsorter::write(acc, 2,
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

int SDScan::scan(flagref_t should_abort, const char* path) {
    int r;
    bool errored = false;

    acc.begin(lfs_path);

    r = acc.open_rw_create_truncate();
    if (r) {
        errored = true;
        goto end_noclose;
    }

    fatfs_list_dir(should_abort, path);

    acc.close();

end_noclose:
    return errored ? -1 : 0;
}

int SDScan::read(flagref_t should_abort, int n, int k, void* cb_arg, lfsorter::res_cb_fn cb) {
    int r;
    bool errored = false;
    acc.begin(lfs_path);

    r = acc.open_r();
    if (r) {
        errored = true;
        goto end_noclose;
    }

    int cnt;
    cnt = lfsorter::get_smallest_n_skip_k(
            acc, should_abort,
            n, k, strcasecmp,
            cb_arg, cb);

    acc.close();

end_noclose:
    return errored ? -1 : cnt;
}

int SDScan::count() {

    // this is called from all_loaded callback
    // -> can use cache

    int r;
    bool errored = false;
    acc.begin(lfs_path);

    r = acc.open_r();
    if (r) {
        errored = true;
        goto end_noclose;
    }

    r = acc.skip_all_lines();
    if (r < 0) {
        errored = true;
        goto end;
    }

    printf("wifi: scanned %d networks\n", r);

end:
    acc.close();

end_noclose:
    return errored ? -1 : r;
}

void SDScan::abort_wait() {
    while (acc.is_open()) {
        vTaskDelay(pdMS_TO_TICKS(WIFI_SCAN_POLL_MS / 2));
    }
}

const char* SDScan::prepend_path(const char* filepath) {
    // same as strcpy but returns end pointer
    char* end = stpcpy(buf, path);
    // end points to null terminator

    const int buf_left = PATH_LEN - (end - buf);
    strncpy(end, filepath, buf_left);

    return buf;
}
