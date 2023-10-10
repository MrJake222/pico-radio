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

    bool is_valid();

    int get_entry_count_whole() override;

public:
    LoaderLocal(ListEntry* entries_, int entries_count_)
            : Loader(entries_, entries_count_)
    { }

    void begin(const char* path_);

    // preserve trailing slash
    // append folder path
    int go(const char* dirpath);
    // when possible, strip last part of path
    // essentially going one level up in a directory tree
    // can fail with return value -1 if already at top-level
    int up();

    int check_entry_url(int i) override;
};
