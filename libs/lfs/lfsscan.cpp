#include "lfsscan.hpp"

#include <cstdarg>
#include <hardware/timer.h>

void LfsScan::begin(const char* lfs_path_, lfsscan_cmp_fn cmp_) {
    lfs_path = lfs_path_;

    // append "_srt" to <lfs_path> and save as <lfs_path_sorted>
    char* end = stpncpy(lfs_path_sorted, lfs_path, LFS_NAME_MAX);
    // end points to null terminator
    const int buf_left = LFS_NAME_MAX - (end - lfs_path_sorted);
    strncpy(end, "_srt", buf_left);

    cmp = cmp_;
}

int LfsScan::write(int n, ...) {
    int r;

    va_list ptr;
    va_start(ptr, n);
    for (int i = 0; i < n; i++) {
        r = acc.write_str(va_arg(ptr, const char*));
        if (r < 0)
            return r;
    }
    va_end(ptr);

    r = acc.write_raw("\n", 1);
    if (r < 0)
        return r;

    return 0;
}

int LfsScan::sort(flagref_t should_abort) {

    // line buffer
    char line[LFSS_BUF_SIZE];

    // smallest elements
    // currently searched
    char s_curr[LFSS_BUF_SIZE];
    // previously found
    char s_prev[LFSS_BUF_SIZE] = {0x00}; // infinitely small

    int start = (int) time_us_32();
    int disk_took = 0;

    int cnt = 0;

    // find all smallest elements (selection sort)
    while(true) {
        acc.seek(0, LFS_SEEK_SET);

        // infinitely large
        s_curr[0] = 0xff;

        // iterate over whole file line-by-line
        while (acc.more_content()) {
            int ll;
            int disk_start = (int) time_us_32();
            read_line(&acc, line, LFSS_BUF_SIZE, &ll);
            disk_took += ((int)time_us_32()) - disk_start;

            // must be strictly larger (implies difference)
            if (cmp(s_prev, line) < 0 && cmp(line, s_curr) < 0) {
                strcpy(s_curr, line);
            }

            if (should_abort)
                return cnt;
        }

        if (s_curr[0] == 0xff) {
            // next entry not found
            break;
        }

        // write the smallest one to accs
        accs.write_str(s_curr);
        accs.write_raw("\n", 1);
        cnt++;

        // printf("lfsorter: %s\n", s_curr);

        strcpy(s_prev, s_curr);
    }

    int end = (int) time_us_32();
    printf("lfsscan (sort): took %5.2fms incl. disk %5.2fms\n",
           (float(end - start)) / 1000.0f,
           (float(disk_took)) / 1000.0f);

    return cnt;
}

int LfsScan::scan(flagref_t should_abort) {
    int r;
    bool errored = false;

    acc.begin(lfs_path);

    r = acc.open_rw_create_truncate();
    if (r) {
        errored = true;
        goto end_noclose;
    }

    r = scan_internal(should_abort);
    if (r) {
        errored = true;
        goto end_noclose_accs;
    }

    accs.begin(lfs_path_sorted);

    r = accs.open_rw_create_truncate();
    if (r) {
        errored = true;
        goto end;
    }

    r = sort(should_abort);
    if (r < 0) {
        errored = true;
        goto end;
    }

end:
    accs.close();

end_noclose_accs:
    acc.close();

end_noclose:
    return errored ? -1 : 0;
}

int LfsScan::get_smallest_n_skip_k(flagref_t should_abort, int n, int k, void* res_cb_arg, lfsscan_res_cb_fn res_cb) {
    int r;
    bool errored = false;
    int cnt = 0;
    acc.begin(lfs_path_sorted);

    r = acc.open_r();
    if (r) {
        errored = true;
        goto end_noclose;
    }

    acc.skip_lines(k);

    while (acc.more_content() && !should_abort && cnt < n) {
        // line buffer
        int ll;
        char line[LFSS_BUF_SIZE];

        read_line(&acc, line, LFSS_BUF_SIZE, &ll);
        res_cb(res_cb_arg, line);
        cnt++;
    }

    acc.close();

end_noclose:
    return errored ? -1 : cnt;
}

int LfsScan::count() {

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

    printf("sd: scanned %d files/dirs\n", r);

end:
    acc.close();

end_noclose:
    return errored ? -1 : r;
}

void LfsScan::abort_wait() {
    while (acc.is_open()) {
        vTaskDelay(pdMS_TO_TICKS(WIFI_SCAN_POLL_MS / 2));
    }
}

bool LfsScan::is_duplicate(lfsscan_cmp_fn cmp_dup, const char* buf) {
    const int begin_pos = acc.tell();
    char line[LFSS_BUF_SIZE];
    bool match = false;

    acc.seek(0, LFS_SEEK_SET);

    while (acc.more_content()) {
        int ll;
        read_line(&acc, line, LFSS_BUF_SIZE, &ll);

        if (cmp_dup(line, buf) == 0) {
            match = true;
            break;
        }
    }

    acc.seek(begin_pos, LFS_SEEK_SET);

    return match;
}
