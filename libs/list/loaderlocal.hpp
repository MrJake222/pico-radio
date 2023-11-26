#pragma once

#include <loader.hpp>
#include <ff.h>

#define PATH_LEN   (FATFS_MAX_PATH_LEN)
#define BUF_LEN    (PATH_LEN)

class LoaderLocal : public Loader {

    DIR dir;
    FILINFO fileinfo;

    // path should always contain trailing slash
    char path[PATH_LEN];
    char buf[PATH_LEN];

    void task() override;
    void set_file(const char* path_, bool is_dir);

    // returns pointer to second-last slash in <path>
    // if top-level returns start of path
    // can be interpreted as leaf directory (last entered, with leading+trailing slashes)
    char* path_get_last_entered();

    // should the file be listed
    // eiter a dir or a supported file by player
    bool is_valid();

    int get_entry_count_whole() override;

public:
    LoaderLocal(ListEntry* entries_, int entries_max_)
            : Loader(entries_, entries_max_)
    { }

    void begin(const char* path_);

    // returns last part of path for ex "Album 123/"
    // skips leading slash, but leaves trailing one
    // when top-level: empty string
    const char* path_leaf() { return path_get_last_entered() + 1; }

    // preserve trailing slash
    // append folder path
    int go(const char* dirpath);
    // when possible, strip last part of path
    // essentially going one level up in a directory tree
    // can fail with return value -1 if already at top-level
    int up();

    int check_entry_url(int i) override;
};
