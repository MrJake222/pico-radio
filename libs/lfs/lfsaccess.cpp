#include "lfsaccess.hpp"

void LfsAccess::begin(const char* path_) {
    strcpy(path, path_);
    is_open_ = false;
}

int LfsAccess::open(int flags) {
    int r;

    memset(&file_cfg, 0, sizeof(file_cfg));
    file_cfg.buffer = &file_buf;

    r = lfs_file_opencfg(lfs, &file, path, flags, &file_cfg);
    if (r < 0) {
        printf("littlefs: failed to open code %d\n", r);
        return r;
    }

    bytes_read = 0;
    is_open_ = true;

    return 0;
}

int LfsAccess::close() {
    int r;

    r = lfs_file_close(lfs, &file);
    if (r < 0) {
        printf("littlefs: failed to close code %d\n", r);
        return r;
    }

    is_open_ = false;

    return 0;
}

int LfsAccess::read_char(char* chr) {
    int r = lfs_file_read(lfs, &file, chr, 1);
    if (r < 0) {
        printf("littlefs: failed to read in read_char code %d\n", r);
        return r;
    }
    if (r == 0) {
        printf("littlefs: eof in read_char (shouldn't happen, check <more_content> before calling this)");
        assert(false);
        return -1;
    }

    bytes_read += 1;

    return 0;
}

bool LfsAccess::more_content() {
    // <bytes_read> is used to detect EOF
    // (and cannot be abandoned, it's the only method http server (other subclass) can tell EOF)
    return bytes_read < size();
}

int LfsAccess::skip_line() {
    // <read_line> uses <read_char> which updates <bytes_read> (DataSource interface)
    int line_length;
    int r = read_line(this, nullptr, 0, &line_length);
    if (r == RL_ERROR)
        return -1;

    return line_length;
}

int LfsAccess::skip_lines(int n) {
    int line_lengths = 0;

    while (n--) {
        int r = skip_line();
        if (r < 0)
            return r;

        line_lengths += r;
    }

    return line_lengths;
}

int LfsAccess::skip_all_lines() {
    int n = 0;

    while (more_content()) {
        int r = skip_line();
        if (r < 0)
            return r;

        n += 1;
    }

    return n;
}

int LfsAccess::write_str(const char* str) {
    int r = lfs_file_write(lfs, &file, str, strlen(str));
    if (r < 0) {
        printf("littlefs: failed to write in write_str code %d\n", r);
        return r;
    }

    assert(r == strlen(str));

    return 0;
}

int LfsAccess::seek(int off, int whence) {
    int pos = lfs_file_seek(lfs, &file, off, whence);

    if (pos >= 0)
        // update only if no error
        bytes_read = pos;

    return pos;
}

int LfsAccess::read_raw(char* buf, int buflen) {
    int read = lfs_file_read(lfs, &file, buf, buflen);

    if (read >= 0)
        // update only if no error
        bytes_read += read;

    return read;
}
