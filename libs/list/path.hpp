#pragma once

#include <config.hpp>

#define MAX_PATH_LEN    (FATFS_MAX_PATH_LEN)

class Path {

    // path should always contain trailing slash
    char path[MAX_PATH_LEN + 1];

    // returns pointer to second-last slash in <path>
    // if top-level returns start of path
    // can be interpreted as leaf directory (last entered, with leading+trailing slashes)
    char* last_entered();

public:
    void begin(const char* path_);

    const char* str() { return path; }

    // returns last part of path for ex "Album 123/"
    // skips leading slash, but leaves trailing one
    // when top-level: empty string
    const char* leaf() { return last_entered() + 1; }

    // preserve trailing slash
    // append folder path
    int go(const char* dirpath);

    // when possible, strip last part of path
    // essentially going one level up in a directory tree
    // can fail with return value -1 if already at top-level
    int up();
};
