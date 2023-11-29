#include "lfsorter.hpp"

#include <cstdarg>
#include <hardware/timer.h>

namespace lfsorter {

int create_open(LfsAccess& acc) {
    int r;
    acc.begin(".tmp");

    r = acc.open_rw_create_truncate();
    if (r < 0)
        return r;

    return 0;
}

int write(LfsAccess& acc, int n, ...) {
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

int get_smallest_n_skip_k(LfsAccess& acc, int n, int k, cmp_fn cmp, void* res_cb_arg, res_cb_fn res_cb) {

    // line buffer
    char line[ENT_NAME_LEN + 10];

    // smallest elements
    // currently searched
    char s_curr[ENT_NAME_LEN + 10];
    // previously found
    char s_prev[ENT_NAME_LEN + 10] = {0x00}; // infinitely small

    uint32_t start = time_us_32();

    // find k+n smallest elements
    for (int i=0; i<(k+n); i++) {
        acc.seek(0, LFS_SEEK_SET);

        // infinitely large
        s_curr[0] = 0xff;

        // iterate over whole file line-by-line
        while (acc.more_content()) {
            int ll;
            read_line(&acc, line, LIST_MAX_LINE_LENGTH, &ll);

            // must be strictly larger (implies difference)
            if (cmp(s_prev, line) < 0 && cmp(line, s_curr) < 0) {
                strcpy(s_curr, line);
            }
        }

        if (i >= k) {
            // skipped k, now report the rest
            res_cb(res_cb_arg, s_curr);
        }

        // printf("lfsorter: %s\n", s_curr);

        strcpy(s_prev, s_curr);
    }

    printf("took: %5.2fms\n", (float(time_us_32() - start)) / 1000.0f);

    return 0;
}

} // namespace