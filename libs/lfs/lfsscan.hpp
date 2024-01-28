#pragma once

#include <lfsaccess.hpp>
#include <util.hpp>

// entry buffer, preserves the entire entry length
// (important, local playback saves paths here), needed for duplicate detection and loading
// +10 for any metadata before the line (for example local folders get "0" prefix to be sorted first)
#define LFSS_BUF_SIZE    (ENT_URL_LEN + 10)

// compare function, generally inputs are strings but
// 0x00 at the beginning indicates infinitely small value
// 0xff at the beginning indicates infinitely large value
typedef int(*lfsscan_cmp_fn)(const char* e1, const char* e2);
typedef void(*lfsscan_res_cb_fn)(void* arg, const char* res);

class LfsScan {

    // contains path of temporary file storing entries on flash
    const char* lfs_path;
    char lfs_path_sorted[LFS_NAME_MAX];

    LfsAccess& acc;
    LfsAccess& accs; // access to sorted file

    lfsscan_cmp_fn cmp;

    // called from <scan> with the <acc> open
    // should use <write> to create an unsorted file
    virtual int scan_internal(flagref_t should_abort) = 0;

    // run by <scan>, after <scan_internal>
    // sorts entries (using <cmp>) from <lfs_path> to <lfs_path_sorted>
    // which then can be read by <get_smallest_n_skip_k>
    int sort(flagref_t should_abort);

protected:
    // writes a list of char pointers
    int write(int n, ...);

    // uses different comparison function
    bool is_duplicate(lfsscan_cmp_fn cmp_dup, const char* buf);

public:
    LfsScan(LfsAccess& acc_, LfsAccess& acc2_)
            : acc(acc_)
            , accs(acc2_)
    { }

    void begin(const char* lfs_path_, lfsscan_cmp_fn cmp_);

    // read some listing and save them as LFS file
    // each line contains one entry (unsorted)
    int scan(flagref_t should_abort);

    // skips k first entries and returns next n in a callback
    // returns strings in format, use decoding functions
    // this function is not async, after it returns all entries had been loaded and passed to callback
    int get_smallest_n_skip_k(flagref_t should_abort, int n, int k, void* res_cb_arg, lfsscan_res_cb_fn res_cb);

    // count the entries
    int count();

    void abort_wait();
};